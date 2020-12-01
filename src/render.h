#pragma once
#include "graph.h"
#include "matrix.h"
#include <list>
#include <vector>
#include <QPainter>
class Hints;
class Paragraph;
struct Shape;
class TextImg;



class Render
{
  public:
    Render(const Graph& graph, const TextImg& txt, const QFont& font, int lineWd);
    ~Render();

    QSize size() const noexcept;
    void setShadows(bool enable);
    void apply(const Hints& hints);
    void paint(QPaintDevice* dev);

  private:
    struct ShapePt {
      int x;
      int y;
      Direction dir;
      int angle;
    };

    using ShapePts      = std::vector<ShapePt>;
    using ShapeList     = std::list<Shape>;
    using ParagraphList = std::list<Paragraph>;

    void computeRenderParams();
    QPoint point(int x, int y) const noexcept;
    QRect textRect(const QRect& r) const noexcept;
    void findShapes();
    void findShapeAt(int x0, int y0, Direction dir0);
    Direction findNextShapeDir(Node node, int x, int y, Direction lastDir);
    void registerShape(ShapePts::const_iterator begin, ShapePts::const_iterator end, int angle);
    void findParagraphs();
    void addLineToParagraphs(QString&& line, int x, int y);
    void applyColor(Shape& shape, const QColor& color);
    void drawShapes(const ShapeList& shapes, const QColor& defaultColor);
    void drawLines();
    void drawLineFrom(int x0, int y0, Direction dir);
    void drawRoundCorner(Node node, int x, int y);
    void drawArrow(int x, int y);
    void drawParagraphs();

    const TextImg& mTxt;
    const Graph& mGraph;
    QFont mFont;
    QPen mSolidPen;
    QPen mDashedPen;
    QBrush mBrush;
    QPainter mPainter;
    ShapePts mShapePts;
    Matrix<Directions> mDone;
    QPolygonF mArrows[4];
    ShapeList mShadows;
    ShapeList mShapes;
    ParagraphList mParagraphs;
    ParagraphList mActives;
    bool mShadowsEnabled;

    int mScaleX;
    int mScaleY;
    int mDeltaX;
    int mDeltaY;
    int mRadius;
};
