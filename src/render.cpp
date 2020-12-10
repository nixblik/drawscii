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
  mScaleX = fm.width("w");
  mScaleY = fm.height(); // TODO: Provide compression for x and y axis, but this depends on the font, so be careful
  mDeltaX = qRound(mScaleX * 0.5);
  mDeltaY = qRound(mScaleY * 0.5);
  mRadius = (mScaleX + mScaleY) * 0.333333;
  mCircle = qRound((mScaleX + mScaleY) * 0.2);
  mShadowDelta = 2;

  // Predraw the arrow marks
  auto& arrow = mMarks[Node::RightArrow];
  arrow.clear();
  arrow.append(QPoint{2 * mDeltaX, 0});
  arrow.append(QPoint{-mDeltaX, qRound(-0.8 * mDeltaY)});
  arrow.append(QPoint{-mDeltaX, qRound( 0.8 * mDeltaY)});

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
{ return QSize{mGraph.width() * mScaleX + mShadowDelta, mGraph.height() * mScaleY + mShadowDelta}; }


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
  mPainter.translate(mDeltaX, mDeltaY);

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
  auto scaleX = mScaleX * 0.5;
  auto scaleY = mScaleY * 0.5;
  auto radius = mRadius * 0.5;

  for (auto& shape: shapes)
  {
    auto path = shape.path(scaleX, scaleY, radius);
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
  auto scaleX = mScaleX * 0.5;
  auto scaleY = mScaleY * 0.5;
  auto radius = mRadius * 0.5;

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
        mPainter.drawPath(line.path(scaleX, scaleY, radius));
        mPainter.setPen(mDoubleInnerPen);
        break;
    }

    mPainter.drawPath(line.path(scaleX, scaleY, radius));
  }
}



inline QPoint Render::toImage(Point pos) const noexcept
{ return QPoint{pos.x*mScaleX, pos.y*mScaleY}; }



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
        mPainter.drawPolygon(mMarks[node.mark()].translated(toImage(node.point())));
        break;

      case Node::EmptyCircle:
        mPainter.setPen(mSolidPen);
        mPainter.setBrush(Qt::white);
        mPainter.drawEllipse(toImage(node.point()), mCircle, mCircle);
        break;

      case Node::FilledCircle:
        mPainter.setPen(Qt::NoPen);
        mPainter.setBrush(mBrush);
        mPainter.drawEllipse(toImage(node.point()), mCircle, mCircle);
        break;
    }
  }
}



inline QRect Render::imageRect(const Paragraph& p) const noexcept
{ return QRect{p.left()*mScaleX-mDeltaX, p.top()*mScaleY-mDeltaY, p.width()*mScaleX, p.height()*mScaleY}; }
// FIXME: Eigentlich mScaleX/2


void Render::drawParagraphs()
{
  QFontMetrics fm{mFont};
  mPainter.setPen(mSolidPen);

  for (auto& para: mParagraphs)
  {
    auto align = para.alignment();
    auto rect  = imageRect(para);

    if (para.color.isValid())
      mPainter.setPen(para.color);

    for (int rowIdx = 0; rowIdx < para.height(); ++rowIdx)
    {
      auto& rowTxt = para[rowIdx];
      QRect lrect{rect.x(), rect.y() + rowIdx*mScaleY, rect.width(), mScaleY};

      if (align == Qt::Alignment{})
        lrect.adjust(para.indent(rowIdx) * mScaleX, 0, 0, 0);

      mPainter.drawText(lrect, static_cast<int>(align), QString::fromStdWString(rowTxt)); // FIXME: QString conversion is expensive
    }
  }
}
