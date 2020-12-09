/*  Copyright 2020 Uwe Salomon <post@uwesalomon.de>

    This file is part of Drawscii.

    Drawscii is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Drawscii is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drawscii.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "render.h"
#include "blur.h"
#include "textimage.h"
#include <QImage>
#include <QPainter>



Render::Render(const TextImage& txt, const Graph& graph, const Shapes& shapes)
  : mTxt{txt},
    mGraph{graph},
    mShapes{shapes},
    mBrush{Qt::black},
    mShadowMode{Shadow::None},
    mAntialias{true}
{
  computeRenderParams();
}



Render::~Render()
= default;



void Render::setFont(const QFont& font)
{
  mFont = font;
  computeRenderParams();
}



void Render::setLineWidth(float lineWd)
{
  mSolidPen  = QPen{Qt::black, static_cast<qreal>(lineWd)};
  mDashedPen = QPen{Qt::black, static_cast<qreal>(lineWd), Qt::CustomDashLine};
  mDashedPen.setDashPattern({5, 3});
}



void Render::setShadows(Shadow mode)
{ mShadowMode = mode; }


void Render::setAntialias(bool enable)
{ mAntialias = enable; }



void Render::computeRenderParams()
{
  QFontMetrics fm{mFont};
  mScaleX = fm.width("w");
  mScaleY = fm.height(); // TODO: Provide compression for x and y axis, but this depends on the font, so be careful
  mDeltaX = qRound(mScaleX * 0.5);
  mDeltaY = qRound(mScaleY * 0.5);
  mShadowDelta = 2;

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
{ return QSize{mGraph.width() * mScaleX + mShadowDelta, mGraph.height() * mScaleY + mShadowDelta}; }


inline QPoint Render::toImage(Point pos) const noexcept
{ return QPoint{pos.x*mScaleX + mDeltaX, pos.y*mScaleY + mDeltaY}; }


/*
inline QRect Render::imageRect(const Paragraph& p) const noexcept
{
  auto pt = p.topLeft();
  return QRect{pt.x*mScaleX, pt.y*mScaleY, p.width()*mScaleX, p.height()*mScaleY};
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
      addLineToParagraphs(std::move(line), TextPos{lineX, y});
    }
  }

  mParagraphs.splice(mParagraphs.end(), mActives);
}



void Render::addLineToParagraphs(QString&& line, TextPos pos)
{
  for (auto para = mActives.begin(); para != mActives.end(); )
  {
    if (pos.y > para->bottom() + 1)
    {
      auto old = para++;
      mParagraphs.splice(mParagraphs.end(), mActives, old);
    }
    else if (para->addLine(std::move(line), pos))
      return;
    else
      ++para;
  }

  mActives.emplace_back(std::move(line), pos);
}



void Render::apply(const Hints& hints)
{
  for (auto& hint: hints)
  {
    auto hintPt = toImage(hint.pos);
    if (hint.color.isValid())
    {
      for (auto i = mShapes.rbegin(); i != mShapes.rend(); ++i)
        if (i->path.contains(hintPt))
        { i->bg = hint.color; break; }
    }
  }

  for (auto& para: mParagraphs)
  {
    auto paraPt = toImage(para.innerPoint());

    for (auto& shape: mShapes)
      if (shape.path.contains(paraPt))
        para.color = shape.bg.isValid() && shape.bg.lightness() < 100 ? Qt::white : Qt::black;
  }
}
*/



void Render::paint(QPaintDevice* dev)
{
  QImage shadowImg;
  if (mShadowMode == Shadow::Blurred)
  {
    shadowImg = QImage{size(), QImage::Format_Alpha8};
    shadowImg.fill(Qt::transparent);
    mPainter.begin(&shadowImg);
    drawShapes(mShapes.outer, Qt::black, mShadowDelta);
    mPainter.end();
  }

  mPainter.begin(dev);
  mPainter.setRenderHint(QPainter::SmoothPixmapTransform);
  mPainter.setFont(mFont);

  if (mAntialias)
  {
    mPainter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);
    mPainter.setRenderHint(QPainter::TextAntialiasing);
  }

  if (mSolidPen.width() & 1)
    mPainter.translate(0.5, 0.5);

  switch (mShadowMode)
  {
    case Shadow::None:
      break;

    case Shadow::Simple:
      drawShapes(mShapes.outer, Qt::lightGray, mShadowDelta);
      break;

    case Shadow::Blurred:
      blurImage(shadowImg, mShadowDelta);
      mPainter.drawImage(0, 0, filledImage(Qt::darkGray, shadowImg));
      break;
  }

  drawShapes(mShapes.inner, Qt::white, 0);
  drawLines();
//drawParagraphs();

  mPainter.end();
}



void Render::drawShapes(const Shapes::List& shapes, const QColor& defaultColor, int delta)
{
  QPen pen{mSolidPen};
  for (auto& shape: shapes)
  {
    auto path = shape.path(mScaleX, mScaleY);
    if (delta)
      path.translate(delta, delta);

    auto color = shape.color.isValid() ? shape.color : defaultColor;
    pen.setColor(color);
    mPainter.setPen(pen);
    mPainter.setBrush(color);
    mPainter.drawPath(path);
  }
}



void Render::drawLines()
{
  mPainter.setBrush(Qt::NoBrush);

  for (auto& line: mShapes.lines)
  {
    switch (line.style)
    {
      case Edge::Weak:
      case Edge::Solid:  mPainter.setPen(mSolidPen); break;
      case Edge::Double: mPainter.setPen(mSolidPen); break; // FIXME: Double lines could be done by drawing a thick black line, then a thin white line
      case Edge::Dashed: mPainter.setPen(mDashedPen); break;
      case Edge::None:   assert(false);
    }

    mPainter.drawPath(line.path(mScaleX, mScaleY));
  }
}


/*
void Render::drawLineFrom(TextPos pos0, Direction dir)
{
  auto revDir = opposite(dir);
  auto pos    = pos0;
  bool dashed = true;
  Node node   = mGraph[pos];

  QPainterPath path;
  if (node.kind() != Round)
    path.moveTo(toImage(pos));
  else if (dir == Right)
    path.moveTo(toImage(pos) + QPoint{deltaX(dir)*mRadius, deltaY(dir)*mRadius});
  else
  {
    // This is a rounded corner like so:  /---    or like so:  /---
    // And it is not a closed shape       |       (future)    /
    QPoint pt  = toImage(pos);
    QPoint r1  = QPoint{-mRadius, 0};
    QPoint r2  = QPoint{deltaX(dir)*mRadius, deltaY(dir)*mRadius};

    path.moveTo(pt - r1);
    path.cubicTo(pt - r1 * 0.44771525, pt + r2 * 0.44771525, pt + r2);
  }

  while (node.edges() & ~mDone[pos] & dir)
  {
    mDone[pos] |= dir;
    pos        += dir;

    mDone[pos] |= revDir;
    node        = mGraph[pos];
    dashed     &= node.isDashed();

    if (node.kind() == Round)
    {
      // Rounded corner is drawn with a Bézier curve
      QPoint pt = toImage(pos);
      QPoint r1 = QPoint{deltaX(dir)*mRadius, deltaY(dir)*mRadius};

      dir    = mGraph.walkCorner(dir, pos, mTxt[pos]);
      revDir = opposite(dir);

      QPoint r2 = QPoint{deltaX(dir)*mRadius, deltaY(dir)*mRadius};
      path.lineTo(pt - r1);
      path.cubicTo(pt - r1 * 0.44771525, pt + r2 * 0.44771525, pt + r2);
    }
  }

  if (node.kind() != Round) // would only happen for closed shape
    path.lineTo(toImage(pos));

  mPainter.setPen(dashed ? mDashedPen : mSolidPen);
  mPainter.setBrush(Qt::NoBrush);
  mPainter.drawPath(path);
}



void Render::drawArrow(TextPos pos)
{
  int arrowIdx;
  switch (mTxt[pos].toLatin1())
  {
    case '>': arrowIdx = 0; break;
    case 'v':
    case 'V': arrowIdx = 1; break;
    case '<': arrowIdx = 2; break;
    case '^': arrowIdx = 3; break;
    default:  Q_UNREACHABLE(); // GCOV_EXCL_LINE
  }

  mPainter.setPen(Qt::NoPen);
  mPainter.setBrush(mBrush);
  mPainter.drawPolygon(mArrows[arrowIdx].translated(toImage(pos)));
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
    default:               return rect;
  }

  return QRect{newX, rect.y(), width, rect.height()};
}
} // namespace



namespace {
inline int leadingSpaces(const QString& s)
{
  for (int i = 0; i < s.size(); ++i)
    if (!s[i].isSpace())
      return i;

  Q_UNREACHABLE(); // GCOV_EXCL_LINE
}                  // GCOV_EXCL_LINE
} // namespace



void Render::drawParagraphs()
{
  QFontMetrics fm{mFont};
  mPainter.setPen(mSolidPen);

  for (const auto& para: mParagraphs)
  {
    auto align = para.alignment();
    auto pixwd = para.pixelWidth(fm);
    auto orect = imageRect(para);
    auto rect  = alignedRect(align, pixwd, orect);

    if (para.color.isValid())
      mPainter.setPen(para.color);

    for (int i = 0; i < para.numberOfLines(); ++i)
    {
      auto& line = para[i];
      QRect lrect{rect.x(), rect.y() + i*mScaleY, rect.width(), mScaleY};

      if (align == Qt::Alignment{})
        lrect.adjust(leadingSpaces(line)*mScaleX, 0, 0, 0);

      mPainter.drawText(lrect, static_cast<int>(align), line.trimmed());
    }
  }
}
*/
