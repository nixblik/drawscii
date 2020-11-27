#pragma once
#include "graph.h"
#include <list>
#include <QImage>
#include <QPainter>
class TextImg;



// FIXME: Write a matrix template, perhaps use std::valarray instead of QVector
class DirectionImg
{
  public:
    explicit DirectionImg(QSize sz);
    void clear() noexcept;
    Directions operator()(int x, int y) const;
    Directions& operator()(int x, int y);

  private:
    QSize mSize;
    QVector<Directions> mImg;
};



class Paragraph
{
  public:
    using const_iterator = QVector<QString>::const_iterator;

    Paragraph(QString&& line, int x, int y);
    int size() const noexcept;
    const QString& operator[](int index) const;
    const QRect& rect() const noexcept;
    int bottom() const noexcept;
    bool addLine(QString&& line, int x, int y);
    Qt::Alignment alignment() const noexcept;
    int pixelWidth(const QFontMetrics& fm) const noexcept;

  private:
    QRect mRect;
    QVector<QString> mLines;
};

using ParagraphList = std::list<Paragraph>;



class Render
{
  public:
    Render(const Graph& graph, const TextImg& txt);

    QSize size() const noexcept;

    void setFont(const QFont& font);
    void paint(QPaintDevice* dev);

  private:
    void computeRenderParams();
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
    QPen mArrowPen;
    QBrush mBrush;
    QPainter mPainter;
    DirectionImg mDone;
    QPolygonF mArrows[4];
    ParagraphList mParagraphs;
    ParagraphList mActives;

    int mScaleX;
    int mScaleY;
    int mDeltaX;
    int mDeltaY;
    int mRadius;
};
