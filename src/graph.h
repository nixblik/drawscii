#pragma once
#include "common.h"
#include <QSize>
#include <QVector>
class TextImg;
class QFile;



enum NodeKind : quint8
{ Text, Line, Round, Arrow };



enum Directions : quint8
{
  Up         = 0x01,
  UpRight    = 0x02,
  Right      = 0x04,
  DownRight  = 0x08,
  Down       = 0x10,
  DownLeft   = 0x20,
  Left       = 0x40,
  UpLeft     = 0x80,
};


constexpr Directions operator|(Directions a, Directions b) noexcept
{ return static_cast<Directions>(unsigned{a} | unsigned{b}); }



class Direction
{
  public:
    constexpr Direction(Directions dir) noexcept
      : mDir{dir}
    {}

    constexpr Directions flag() const noexcept
    { return mDir; }

    constexpr Direction opposite() const noexcept
    { return static_cast<Directions>(mDir <= DownRight ? mDir << 4 : mDir >> 4); }

    constexpr Direction turnedRight() const noexcept
    { return static_cast<Directions>(mDir == UpLeft ? Up : mDir << 1); }

    constexpr Direction turnedLeft() const noexcept
    { return static_cast<Directions>(mDir == Up ? UpLeft : mDir >> 1); }

    int dx() const noexcept;
    int dy() const noexcept;

  private:
    Directions mDir;
};


constexpr Directions operator|(Direction a, Direction b) noexcept
{ return a.flag() | b.flag(); }

constexpr bool operator!=(Direction a, Direction b) noexcept
{ return a.flag() != b.flag(); }

constexpr bool operator!=(Direction a, Directions b) noexcept
{ return a.flag() != b; }

constexpr bool operator==(Direction a, Directions b) noexcept
{ return a.flag() == b; }



class Node
{
  public:
    constexpr Node() noexcept
      : mKind{Text},
        mDashed{false},
        mEdges{0}
    {}

    NodeKind kind() const noexcept
    { return mKind; }

    bool isLine() const noexcept
    { return mKind != Text; }

    bool hasEdge(Direction dir) const noexcept
    { return mEdges & dir.flag(); }

    bool isDashed() const noexcept
    { return mDashed; }

    void set(NodeKind kind, Directions edges);
    void set(NodeKind kind, Direction edge);
    void setDashed();

  private:
    NodeKind mKind : 7;
    bool   mDashed : 1;
    quint8 mEdges;
};



class Graph
{
  public:
    static Graph from(const TextImg& txt);

    int width() const noexcept
    { return mSize.width(); }

    int height() const noexcept
    { return mSize.height(); }

    QSize size() const noexcept
    { return mSize; }

    Node node(int x, int y) const
    {
      Q_ASSERT(x >= 0 && x < mSize.width() && y >= 0 && y < mSize.height());
      return mNodes[y * mSize.width() + x];
    }

    Node& node(int x, int y)
    {
      Q_ASSERT(x >= 0 && x < mSize.width() && y >= 0 && y < mSize.height());
      return mNodes[y * mSize.width() + x];
    }

    Direction walkRoundCorner(Direction dir, int x, int y, QChar cornerCh) const noexcept;

  private:
    explicit Graph(const QSize& sz);
    void readFrom(const TextImg& txt);
    void pass1(const TextImg& txt);
    void pass2(const TextImg& txt);
    void pass3(const TextImg& txt);
    void findMoreCorners(const TextImg& txt, int x, int y);
    void spreadDashing(const TextImg& txt, int x, int y, Direction dir);
    void makeEdge(int x, int y, NodeKind k1, Direction dir, NodeKind k2);
    void makeCorner(int x, int y, NodeKind k1, Direction dir2, NodeKind k2, Direction dir3, NodeKind k3);
    void makeRevEdge(int x, int y, Direction dir, NodeKind k2);

    QSize mSize;
    QVector<Node> mNodes;
};
