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
#include "hints.h"
#include "shapes.h"
#include "paragraphs.h"
#include <forward_list>
#include <QPainter>
class TextImage;



enum class Shadow
{ None, Simple, Blurred };



class Render
{
  public:
    Render(const TextImage& txt, const Graph& graph, const Shapes& shapes, const Hints& hints, const ParagraphList& paragraphs);
    ~Render();

    QSize size() const noexcept;
    void setFont(const QFont& font);
    void setLineWidth(float lineWd);
    void setShadows(Shadow mode);
    void setAntialias(bool enable);
    void paint(QPaintDevice* dev);

  private:
    struct ShapePath;
    using ShapePaths = std::forward_list<ShapePath>;

    void computeRenderParams();
    QPoint graphToImage(Point p) const noexcept;
    QPoint textToImage(int x, int y) const noexcept;
    QRect textToImage(const Paragraph& p) const noexcept;
    ShapePaths shapePaths(const Shapes::List& shapes, int delta) const;
    void applyHints(const QColor& defaultColor);
    void drawShapes(const ShapePaths& shapes, const QColor& defaultColor);
    void drawLines();
    void drawMarks();
    void drawRoundCorner(Node node, Point pos);
    void drawArrow(Point pos);
    void drawParagraphs();

    const TextImage& mTxt;
    const Graph& mGraph;
    const Shapes& mShapes;
    const Hints& mHints;
    const ParagraphList& mParagraphs;
    QFont mFont;
    QPen mSolidPen;
    QPen mDoubleOuterPen;
    QPen mDoubleInnerPen;
    QPen mDashedPen;
    QBrush mBrush;
    QPainter mPainter;
    QPolygonF mMarks[7];
    QPainterPath mLeapfrog;
    Shadow mShadowMode;
    bool mAntialias;

    double mScaleX;
    double mScaleY;
    double mRadius;
    int mShadowDelta;
    int mCircle;
    QRect mBoundingBox;

    ShapePaths mOuterShapes;
    ShapePaths mInnerShapes;
};
