#pragma once
#include "graph.h"
#include "matrix.h"
#include <list>
#include <vector>
#include <QImage>
#include <QPainter>
class Paragraph;
class TextImg;




using ParagraphList = std::list<Paragraph>;



class Render
{
  public:
    Render(const Graph& graph, const TextImg& txt, int lineWd);
    ~Render();

    QSize size() const noexcept;
    void setFont(const QFont& font);
    void paint(QPaintDevice* dev);

  private:
    struct ShapePt {
      int x;
      int y;
      Direction dir;
      int angle;
    };
    using ShapePts = std::vector<ShapePt>;

    void computeRenderParams();
    void findShapes();
    void findShape(int x0, int y0);
    Direction findNextShapeDir(Node node, int x, int y, Direction lastDir);
    void registerShape(ShapePts::const_iterator begin, ShapePts::const_iterator end, int angle);
    void drawLines();
    void drawLineFrom(int x0, int y0, Direction dir);
    void drawRoundCorner(Node node, int x, int y);
    void drawArrow(int x, int y);
    void findParagraphs();
    void addLineToParagraphs(QString&& line, int x, int y);
    void drawParagraphs();
    QPoint point(int x, int y) const noexcept;
    QRect textRect(const QRect& r) const noexcept;

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
    ParagraphList mParagraphs;
    ParagraphList mActives;

    int mScaleX;
    int mScaleY;
    int mDeltaX;
    int mDeltaY;
    int mRadius;
};
