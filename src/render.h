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
#pragma once
#include "graph.h"
#include "shapes.h"
#include "paragraphs.h"
#include <list>
#include <QPainter>
class Hints;
class TextImage;



enum class Shadow
{ None, Simple, Blurred };



class Render
{
  public:
    Render(const TextImage& txt, const Graph& graph, const Shapes& shapes, const ParagraphList& paragraphs);
    ~Render();

    QSize size() const noexcept;
    void setFont(const QFont& font); // FIXME: Might try to set string instead
    void setLineWidth(float lineWd);
    void setShadows(Shadow mode);
    void setAntialias(bool enable);
    void apply(const Hints& hints);
    void paint(QPaintDevice* dev);

  private:
    void computeRenderParams();
    QPoint graphToImage(Point p) const noexcept;
    QRect textRectToImage(const Paragraph& p) const noexcept;
    void drawShapes(const Shapes::List& shapes, const QColor& defaultColor, int delta);
    void drawLines();
    void drawMarks();
    void drawRoundCorner(Node node, Point pos);
    void drawArrow(Point pos);
    void drawParagraphs();

    const TextImage& mTxt;
    const Graph& mGraph;
    const Shapes& mShapes;
    const ParagraphList& mParagraphs;
    QFont mFont;
    QPen mSolidPen;
    QPen mDoubleOuterPen;
    QPen mDoubleInnerPen;
    QPen mDashedPen;
    QBrush mBrush;
    QPainter mPainter;
    QPolygonF mMarks[7];
    Shadow mShadowMode;
    bool mAntialias;

    double mScaleX;
    double mScaleY;
    double mRadius;
    int mShadowDelta;
    int mCircle;
    QRect mBoundingBox;
};
