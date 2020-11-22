#pragma once
#include "common.h"
#include <QSize>
#include <QVector>
class TextImg;



enum NodeKind : quint8
{ Text, Line, Round, Arrow };

enum Direction : quint8
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



class Node
{
  public:
    constexpr Node() noexcept
      : mKind{Text},
        mEdges{0}
    {}

    NodeKind kind() const noexcept
    { return mKind; }

    bool hasEdge(Direction dir) const noexcept
    { return mEdges & dir; }

    void set(NodeKind kind, Direction edge);

  private:
    NodeKind mKind;
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

  private:
    explicit Graph(const QSize& sz);
    void readFrom(const TextImg& txt);
    void pass1(const TextImg& txt);
    void pass2(const TextImg& txt);
    void findMoreCorners(const TextImg& txt, int x, int y);
    void makeEdge(int x, int y, NodeKind k1, Direction dir, NodeKind k2);
    void makeCorner(int x, int y, NodeKind k1, Direction dir2, NodeKind k2, Direction dir3, NodeKind k3);
    void makeRevEdge(int x, int y, Direction dir, NodeKind k2);

    QSize mSize;
    QVector<Node> mNodes;
};
