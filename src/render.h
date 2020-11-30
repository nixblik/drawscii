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
    void setShadows(bool enable);
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
    void findShapeAt(int x0, int y0, Direction dir0);
    Direction findNextShapeDir(Node node, int x, int y, Direction lastDir);
    void registerShape(ShapePts::const_iterator begin, ShapePts::const_iterator end, int angle);
    void drawShadows();
    void drawShapes();
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
    std::list<QPainterPath> mShadows;
    std::list<QPainterPath> mShapes;
    ParagraphList mParagraphs;
    ParagraphList mActives;
    bool mShadowsEnabled;

    int mScaleX;
    int mScaleY;
    int mDeltaX;
    int mDeltaY;
    int mRadius;
};
