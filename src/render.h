#pragma once
#include "graph.h"
#include <list>
#include <QImage>
#include <QPainter>
class TextImg;



class Bitmap
{
  public:
    explicit Bitmap(QSize sz);
    bool get(int x, int y) const;
    void clear();
    void set(int x, int y, bool v = true);

  private:
    QSize mSize;
    QVector<bool> mBits;
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
    QPainter mPainter;
    Bitmap mDone;
    QPainterPath mArrows[4];
    ParagraphList mParagraphs;
    ParagraphList mActives;

    int mScaleX;
    int mScaleY;
    int mDeltaX;
    int mDeltaY;
    int mRadius;
};
