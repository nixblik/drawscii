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
#include <cmath>
#include <QImage>
#include <QPainter>



Render::Render(const TextImage& txt, const Graph& graph, const Shapes& shapes, const ParagraphList& paragraphs)
  : mTxt{txt},
    mGraph{graph},
    mShapes{shapes},
    mParagraphs{paragraphs},
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
  mSolidPen       = QPen{Qt::black, static_cast<qreal>(lineWd), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin};
  mDoubleOuterPen = QPen{Qt::black, static_cast<qreal>(lineWd * 3), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin};
  mDoubleInnerPen = QPen{Qt::white, static_cast<qreal>(lineWd), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin};
  mDashedPen      = QPen{Qt::black, static_cast<qreal>(lineWd), Qt::CustomDashLine, Qt::FlatCap, Qt::MiterJoin};
  mDashedPen.setDashPattern({5, 3});
}



void Render::setShadows(Shadow mode)
{ mShadowMode = mode; }


void Render::setAntialias(bool enable)
{ mAntialias = enable; }



void Render::computeRenderParams()
{
  QFontMetrics fm{mFont};
  mScaleX = 0.5 * fm.width("w");
  mScaleY = 0.5 * fm.height(); // TODO: Provide compression for x and y axis, but this depends on the font, so be careful
  mRadius = (mScaleX + mScaleY) * 0.333333;
  mCircle = qRound((mScaleX + mScaleY) * 0.2);
  mShadowDelta = 2;

  // Determine size and position of the output
  mBoundingBox = QRect{graphToImage({mGraph.left(), mGraph.top()}), graphToImage({mGraph.right(), mGraph.bottom()})};
  for (auto& para: mParagraphs)
    mBoundingBox = mBoundingBox.united(textRectToImage(para));

  int ltExtend = qRound((mScaleX + mScaleY) * 0.5);
  int rbExtend = ltExtend + (mShadowMode == Shadow::None ? 0 : mShadowDelta);
  mBoundingBox.adjust(-ltExtend, -ltExtend, rbExtend, rbExtend);

  // Predraw the arrow marks
  auto& arrow = mMarks[Node::RightArrow];
  arrow.clear();
  arrow.append(QPoint{qRound(mScaleX), 0});
  arrow.append(QPoint{qRound(-0.5 * mScaleX), qRound(-0.4 * mScaleY)});
  arrow.append(QPoint{qRound(-0.5 * mScaleX), qRound( 0.4 * mScaleY)});

  auto rotated = [](const QPolygonF& polygon, int angle)
  {
    QTransform t;
    t.rotate(angle);
    return t.map(polygon);
  };

  mMarks[Node::UpArrow]   = rotated(arrow, 270);
  mMarks[Node::LeftArrow] = rotated(arrow, 180);
  mMarks[Node::DownArrow] = rotated(arrow, 90);
}



QSize Render::size() const noexcept
{
  assert(mBoundingBox.isValid());
  return mBoundingBox.size();
}


/*
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
  mPainter.translate(-mBoundingBox.topLeft());

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
  drawMarks();
  drawParagraphs();

  mPainter.end();
}



void Render::drawShapes(const Shapes::List& shapes, const QColor& defaultColor, int delta)
{
  QPen pen{mSolidPen};

  for (auto& shape: shapes)
  {
    auto path = shape.path(mScaleX, mScaleY, mRadius);
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
      case Edge::Dashed: mPainter.setPen(mDashedPen); break;
      case Edge::None:   assert(false);

      case Edge::Double:
        mPainter.setPen(mDoubleOuterPen);
        mPainter.drawPath(line.path(mScaleX, mScaleY, mRadius));
        mPainter.setPen(mDoubleInnerPen);
        break;
    }

    mPainter.drawPath(line.path(mScaleX, mScaleY, mRadius));
  }
}



inline QPoint Render::graphToImage(Point p) const noexcept
{ return QPoint(qRound(p.x * mScaleX), qRound(p.y * mScaleY)); };



void Render::drawMarks()
{
  for (auto& node: mGraph)
  {
    switch (node.mark())
    {
      case Node::NoMark:
        continue;

      case Node::RightArrow:
      case Node::UpArrow:
      case Node::LeftArrow:
      case Node::DownArrow:
        mPainter.setPen(Qt::NoPen);
        mPainter.setBrush(mBrush);
        mPainter.drawPolygon(mMarks[node.mark()].translated(graphToImage(node.point())));
        break;

      case Node::EmptyCircle:
        mPainter.setPen(mSolidPen);
        mPainter.setBrush(Qt::white);
        mPainter.drawEllipse(graphToImage(node.point()), mCircle, mCircle);
        break;

      case Node::FilledCircle:
        mPainter.setPen(Qt::NoPen);
        mPainter.setBrush(mBrush);
        mPainter.drawEllipse(graphToImage(node.point()), mCircle, mCircle);
        break;
    }
  }
}



inline QRect Render::textRectToImage(const Paragraph& p) const noexcept
{
  auto left = qRound(mScaleX * (p.left() * 2 - 1));
  auto wd   = qRound(mScaleX * p.width() * 2);
  auto top  = qRound(mScaleY * (p.top() * 2 - 1));
  auto ht   = qRound(mScaleY * p.height() * 2);

  return QRect{left, top, wd, ht};
}



void Render::drawParagraphs()
{
  QFontMetrics fm{mFont};
  mPainter.setPen(mSolidPen);

  for (auto& para: mParagraphs)
  {
    auto align = para.alignment();
    auto rect  = textRectToImage(para);

    if (para.color.isValid())
      mPainter.setPen(para.color);

    for (int rowIdx = 0; rowIdx < para.height(); ++rowIdx)
    {
      auto& rowTxt = para[rowIdx];
      auto  lrect  = rect.adjusted(0, qRound(rowIdx * 2 * mScaleY), 0, 0); // FIXME: We might as well just use textRectToImage here and below

      if (align == Qt::Alignment{})
        lrect.adjust(qRound(para.indent(rowIdx) * 2 * mScaleX), 0, 0, 0);

      mPainter.drawText(lrect, static_cast<int>(align), QString::fromStdWString(rowTxt)); // FIXME: QString conversion is expensive
    }
  }
}
