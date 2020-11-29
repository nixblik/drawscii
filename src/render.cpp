#include "render.h"
#include "graph.h"
#include "textimg.h"
#include <QImage>
#include <QPainter>



class Paragraph
{
  public:
    using const_iterator = QVector<QString>::const_iterator;

    Paragraph(QString&& line, int x, int y)
      : mRect{x, y, line.size(), 1}
    { mLines.append(std::move(line)); }

    int numberOfLines() const noexcept
    { return mLines.size(); }

    const QString& operator[](int index) const
    { return mLines[index]; }

    const QRect& rect() const noexcept
    { return mRect; }

    int bottom() const noexcept
    { return mRect.bottom(); }

    bool addLine(QString&& line, int x, int y);
    Qt::Alignment alignment() const noexcept;
    int pixelWidth(const QFontMetrics& fm) const noexcept;

  private:
    QRect mRect;
    QVector<QString> mLines;
};



bool Paragraph::addLine(QString&& line, int x, int y)
{
  assert(y <= mRect.bottom() + 1);

  int endx = x + line.size();
  if (endx <= mRect.left() || x > mRect.right() || y <= mRect.bottom())
    return false;

  static QString spaces;
  spaces.resize(qAbs(x - mRect.x()), ' ');

  if (x > mRect.x())
    line.prepend(spaces);
  else if (x < mRect.x())
    for (auto& l: mLines)
      l.prepend(spaces);

  mLines.append(std::move(line));
  mRect = mRect.united(QRect{x, y, endx - x, 1});

  return true;
}



namespace {
inline int leadingSpaces(const QString& s)
{
  for (int i = 0; i < s.size(); ++i)
    if (!s[i].isSpace())
      return i;

  Q_UNREACHABLE();
}
} // namespace



Qt::Alignment Paragraph::alignment() const noexcept
{
  int left   = 0;
  int center = 0;
  int right  = 0;

  for (auto& line: mLines)
  {
    int spc = leadingSpaces(line);
    left   += (spc == 0);
    center += qAbs(mRect.width() - line.size() - spc) <= 1;
    right  += line.size() == mRect.width();
  }

  if (left > center)
    return right > left ? Qt::AlignRight : Qt::AlignLeft;
  else
    return right > center ? Qt::AlignRight : Qt::AlignHCenter;
}



int Paragraph::pixelWidth(const QFontMetrics& fm) const noexcept
{
  int wd = 0;
  for (auto& line: mLines)
    wd = qMax(wd, fm.width(line));

  return wd;
}



Render::Render(const Graph& graph, const TextImg& txt, int lineWd)
  : mTxt{txt},
    mGraph{graph},
    mSolidPen{Qt::black, static_cast<qreal>(lineWd)},
    mDashedPen{Qt::black, static_cast<qreal>(lineWd), Qt::CustomDashLine},
    mBrush{Qt::black},
    mDone{graph.width(), graph.height()}
{
  mDashedPen.setDashPattern({5, 3});
  computeRenderParams();
}



Render::~Render()
= default;



void Render::setFont(const QFont& font)
{
  mFont = font;
  computeRenderParams();
}



void Render::computeRenderParams()
{
  QFontMetrics fm{mFont};
  mScaleX = fm.width("w");
  mScaleY = fm.height(); // TODO: Provide compression for x and y axis, but this depends on the font, so be careful
  mDeltaX = qRound(mScaleX * 0.5);
  mDeltaY = qRound(mScaleY * 0.5);
  mRadius = qRound((mScaleX + mScaleY) * 0.33333);

  auto& arrow = mArrows[0];
  arrow.clear();
  arrow.append(QPoint{mDeltaX, 0});
  arrow.append(QPoint{qRound(-0.5 * mDeltaX), qRound(-0.4 * mDeltaY)});
  arrow.append(QPoint{qRound(-0.5 * mDeltaX), qRound( 0.4 * mDeltaY)});

  for (int i = 1; i < 4; ++i)
  {
    QTransform t;
    t.rotate(90*i);
    mArrows[i] = t.map(arrow);
  }
}



QSize Render::size() const noexcept
{ return QSize{mGraph.width() * mScaleX, mGraph.height() * mScaleY}; }


inline QPoint Render::point(int x, int y) const noexcept
{ return QPoint{x*mScaleX + mDeltaX, y*mScaleY + mDeltaY}; }


inline QRect Render::textRect(const QRect& r) const noexcept
{ return QRect{r.x()*mScaleX, r.y()*mScaleY, r.width()*mScaleX, r.height()*mScaleY}; }



void Render::paint(QPaintDevice* dev)
{
  mPainter.begin(dev);
  mPainter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);
  mPainter.setRenderHint(QPainter::TextAntialiasing);
  mPainter.setRenderHint(QPainter::SmoothPixmapTransform);
  mPainter.setFont(mFont);

  if (mSolidPen.width() & 1)
    mPainter.translate(0.5, 0.5);

  mPainter.setBrush(Qt::black);
  drawLines();

  findParagraphs();
  drawParagraphs();

  mPainter.end();
}



void Render::drawLines()
{
  mDone.clear();
  for (int y = 0; y < mGraph.height(); ++y)
  {
    for (int x = 0; x < mGraph.width(); ++x)
    {
      auto node = mGraph(x,y);
      if (!node.isLine())
        continue;

      // If edges are left to be done...
      if (node.edges() & ~mDone(x,y) & (Right|DownRight|Down|DownLeft))
        for (Direction dir = Right; dir != Left; dir = turnedRight45(dir))
          if (node.edges() & ~mDone(x,y) & dir)
            drawLineFrom(x, y, dir);

      if (node.kind() == Arrow)
        drawArrow(x, y);
    }
  }
}



void Render::drawLineFrom(int x0, int y0, Direction dir)
{
  auto revDir = opposite(dir);
  int  dx     = deltaX(dir);
  int  dy     = deltaY(dir);
  int  x      = x0;
  int  y      = y0;
  bool dashed = true;
  Node node   = mGraph(x,y);

  QPainterPath path;
  if (node.kind() != Round)
    path.moveTo(point(x, y));
  else if (dir == Right)
    path.moveTo(point(x, y) + QPoint{dx*mRadius, dy*mRadius});
  else
  {
    // This is a rounded corner like so:  /---    or like so:  /---
    // And it is not a closed shape       |       (future)    /
    QPoint pt  = point(x, y);
    QPoint r1  = QPoint{-mRadius, 0};
    QPoint r2  = QPoint{dx*mRadius, dy*mRadius};

    path.moveTo(pt - r1);
    path.cubicTo(pt - r1 * 0.44771525, pt + r2 * 0.44771525, pt + r2);
  }

  while (node.edges() & ~mDone(x,y) & dir)
  {
    mDone(x,y) |= dir;
    x          += dx;
    y          += dy;

    mDone(x,y) |= revDir;
    node        = mGraph(x,y);
    dashed     &= node.isDashed();

    if (node.kind() == Round)
    {
      // Rounded corner is drawn with a Bézier curve
      QPoint pt = point(x, y);
      QPoint r1 = QPoint{dx*mRadius, dy*mRadius};

      dir    = mGraph.walkCorner(dir, x, y, mTxt(x,y));
      revDir = opposite(dir);
      dx     = deltaX(dir);
      dy     = deltaY(dir);

      QPoint r2 = QPoint{dx*mRadius, dy*mRadius};
      path.lineTo(pt - r1);
      path.cubicTo(pt - r1 * 0.44771525, pt + r2 * 0.44771525, pt + r2);
    }
  }

  if (node.kind() != Round) // would only happen for closed shape
    path.lineTo(point(x, y));

  mPainter.setPen(dashed ? mDashedPen : mSolidPen);
  mPainter.setBrush(Qt::NoBrush);
  mPainter.drawPath(path);
}



void Render::drawArrow(int x, int y)
{
  int arrowIdx;
  switch (mTxt(x,y).toLatin1())
  {
    case '>': arrowIdx = 0; break;
    case 'v':
    case 'V': arrowIdx = 1; break;
    case '<': arrowIdx = 2; break;
    case '^': arrowIdx = 3; break;
    default:  Q_UNREACHABLE();
  }

  mPainter.setPen(Qt::NoPen);
  mPainter.setBrush(mBrush);
  mPainter.drawPolygon(mArrows[arrowIdx].translated(point(x, y)));
}



void Render::findParagraphs()
{
  QList<Paragraph> active;
  QString line;
  int spaces = 0;
  int lineX  = 0;

  for (int y = 0; y < mGraph.height(); ++y)
  {
    for (int x = 0; x < mGraph.width(); ++x)
    {
      if (mGraph(x,y).kind() == Text)
      {
        auto ch  = mTxt(x,y);
        bool spc = ch.isSpace();
        spaces   = spc ? spaces + 1 : 0;

        if (line.isEmpty())
          lineX = x;

        if (!spc || !line.isEmpty())
          line.append(ch);

        if (spaces <= 1 && x + 1 < mTxt.width())
          continue;
      }

      if (line.isEmpty())
        continue;

      line.truncate(line.size() - spaces);
      addLineToParagraphs(std::move(line), lineX, y);
    }
  }

  mParagraphs.splice(mParagraphs.end(), mActives);
}



void Render::addLineToParagraphs(QString&& line, int x, int y)
{
  for (auto para = mActives.begin(); para != mActives.end(); )
  {
    if (y > para->bottom() + 1)
    {
      auto old = para++;
      mParagraphs.splice(mParagraphs.end(), mActives, old);
    }
    else if (para->addLine(std::move(line), x, y))
      return;
    else
      ++para;
  }

  mActives.emplace_back(std::move(line), x, y);
}



namespace {
QRect alignedRect(Qt::Alignment alignment, int width, const QRect& rect)
{
  int newX;
  switch (alignment)
  {
    case Qt::AlignLeft:    newX = rect.x(); break;
    case Qt::AlignHCenter: newX = rect.x() + (rect.width() - width) / 2; break;
    case Qt::AlignRight:   newX = rect.x() + rect.width() - width; break;
    default: Q_UNREACHABLE();
  }

  return QRect{newX, rect.y(), width, rect.height()};
}
} // namespace



void Render::drawParagraphs()
{
  QFontMetrics fm{mFont};

  for (const auto& para: mParagraphs)
  {
    auto align = para.alignment();
    auto pixwd = para.pixelWidth(fm);
    auto orect = textRect(para.rect());
    auto rect  = alignedRect(align, pixwd, orect);

    for (int i = 0; i < para.numberOfLines(); ++i)
    {
      auto& line = para[i];
      int   ly   = rect.y() + i*mScaleY;

      int lalign;
      if (align == Qt::AlignLeft && !line[0].isSpace())
        lalign = Qt::AlignLeft;
      else if (align == Qt::AlignRight && line.size() == para.rect().width())
        lalign = Qt::AlignRight;
      else
        lalign = Qt::AlignHCenter;

      QRect lrect{rect.x(), ly, rect.width(), mScaleY};
      mPainter.drawText(lrect, lalign, line.trimmed());
    }
  }
}
