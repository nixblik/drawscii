#include "graph.h"
#include <cstdlib>
#include <limits>
#include <stdexcept>



Angle Angle::relativeTo(Angle other) const noexcept
{
  auto diff = mDegrees - other.mDegrees;
  if (diff > 180)
    return Angle{diff - 360};

  if (diff <= -180)
    return Angle{diff + 360};

  return Angle{diff};
}



constexpr Node::Node() noexcept
  : mEdges{},
    mX{-32768}, // Sentinel for cuckoo hash
    mY{-32768},
    mMark{NoMark},
    mForm{Straight},
    mDone{0xFF},
    mDone0{0xFF}
{}



inline Node::Node(int x, int y)
  : mEdges{},
    mX{static_cast<int16_t>(x)},
    mY{static_cast<int16_t>(y)},
    mMark{NoMark},
    mForm{Straight},
    mDone{0xFF},
    mDone0{0xFF}
{
  if (x > std::numeric_limits<int16_t>::max() || y > std::numeric_limits<int16_t>::max())
    throw std::runtime_error{"drawing is too large"};
}



void Node::EdgeRef::setStyle(Node::EdgeRef::Style style) noexcept
{
  assert(style != Edge::None);
  mNode->mEdges[mIndex].setStyle(style);

  auto clrBit    = ~(1u << mIndex);
  mNode->mDone  &= clrBit;
  mNode->mDone0 &= clrBit;
}



Point Node::EdgeRef::target() const noexcept
{
  auto p = mNode->point();
  return Point{p.x + dx(), p.y + dy()};
}



int Node::EdgeRef::dx() const noexcept
{
  assert(mIndex >= 0 && mIndex < 8);

  constexpr static int dxs[] = { +1, +1, 0, -1, -1, -1, 0, +1 };
  return dxs[mIndex];
}



int Node::EdgeRef::dy() const noexcept
{
  assert(mIndex >= 0 && mIndex < 8);

  constexpr static int dys[] = { 0, -1, -1, -1, 0, +1, +1, +1 };
  return dys[mIndex];
}



namespace {

inline int edgeIndexFromDxDy(int dx, int dy)
{
  assert(dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1 && (dx || dy));

  constexpr static int edges[] = { 3, 2, 1, 4, -1, 0, 5, 6, 7 };
  return edges[4+dx+dy*3];
}



inline int reverseEdgeIndex(int idx)
{
  assert(idx >= 0);
  return (idx + 4) & 7;
}


static Node noEdgesNode{};
} // namespace



auto Node::nextRightwardTodoEdge(Angle prevAngle) noexcept -> edge_ptr
{
  assert(prevAngle.degrees() >= 0 && prevAngle.degrees() < 360);

  int idx0 = reverseEdgeIndex(prevAngle.degrees() / 45);
  int idx  = idx0;

  while ((idx = (idx + 1) & 7) != idx0)
    if (mEdges[idx].exists() && !(mDone & (1u << idx)))
      return edge(idx);

  assert(!noEdgesNode.edge(0));
  return noEdgesNode.edge(0);
}



auto Node::reverseEdge(const_edge_ptr edge) noexcept -> edge_ptr
{ return edge_ptr{this, reverseEdgeIndex(edge->index())}; }



auto Node::continueLine(const_edge_ptr edge) noexcept -> edge_ptr
{
  switch (mForm)
  {
    case Straight: return edge_ptr{this, edge->index()};

    case Bezier: {
      int rev = reverseEdgeIndex(edge->index());
      for (int idx = 0; idx < 8; ++idx)
        if (idx != edge->index() && idx != rev && mEdges[idx].exists())
          return edge_ptr{this, idx};
      break;
    }
  }

  assert(false);
}



Graph::Graph() noexcept
  : mCurNode{nullptr},
    mSize{0},
    mCapacity{0},
    mLeft{0},
    mRight{0},
    mTop{0},
    mBottom{0}
{}



namespace {

// FNV-1a hash of two 16-bit integers
template<typename Int>
inline uint hash(Int x, Int y) noexcept
{
  uint32_t h = 2166136261u;

  auto x2 = static_cast<uint>(x);
  h       = (h ^ (x2 & 0xFFu)) * 16777619u;
  x2    >>= 8;
  h       = (h ^ (x2 & 0xFFu)) * 16777619u;

  auto y2 = static_cast<uint>(y);
  h       = (h ^ (y2 & 0xFFu)) * 16777619u;
  y2    >>= 8;
  h       = (h ^ (x2 & 0xFFu)) * 16777619u;

  return h;
}
} // namespace



inline uint Node::hash() const noexcept
{ return ::hash(mX, mY); }



void Graph::reserve(uint capacity)
{
  auto newCapa = std::max(mCapacity, 256u);
  while (newCapa < capacity)
    newCapa *= 2;

  if (newCapa <= mCapacity)
    return;

  auto newTable = std::make_unique<Node[]>(newCapa+1);
  new(&newTable[newCapa]) Node{0, 0}; // Valid node at the end stops iteration

  if (mTable)
  {
    auto cap = newCapa - 1;
    for (Node& node: *this)
    {
      Node* pos;
      for (auto h = node.hash(); !(pos = &newTable[h&cap])->isSentinel(); ++h)
      {}

      new(pos) Node{std::move(node)};
    }
  }

  mTable    = std::move(newTable);
  mCapacity = newCapa;
}



Node& Graph::operator[](Point p)
{
  auto  cap = mCapacity - 1;
  Node* pos;

  for (auto h = hash(p.x, p.y); !(pos = &mTable[h&cap])->isSentinel(); ++h)
    if (pos->point() == p)
      return *pos;

  throw std::invalid_argument{"invalid graph node accessed"};
}



Node& Graph::moveTo(int x, int y)
{
  for (;;)
  {
    auto  cap = mCapacity - 1;
    Node* pos;

    for (auto h = hash(x, y); !(pos = &mTable[h&cap])->isSentinel(); ++h)
      if (pos->x() == x && pos->y() == y)
        return *(mCurNode = pos);

    if (++mSize * 2 >= mCapacity)
    {
      reserve(mCapacity * 4);
      continue;
    }

    mCurNode = new(pos) Node{x, y};
    mLeft    = std::min(mLeft, x);
    mRight   = std::max(mRight, x);
    mTop     = std::min(mTop, y);
    mBottom  = std::max(mBottom, y);
    return *mCurNode;
  }
}



namespace {
int signum(int x) noexcept
{ return x < 0 ? -1 : (x > 0); }
} // namespace



Node& Graph::lineTo(int dx, int dy, Edge::Style style)
{
  assert(dx == 0 || dy == 0 || abs(dx) == abs(dy));
  assert(mCurNode);

  int x    = mCurNode->x();
  int y    = mCurNode->y();
  int xe   = x + dx;
  int ye   = y + dy;
  dx       = signum(dx);
  dy       = signum(dy);
  int edge = edgeIndexFromDxDy(dx, dy);
  int reve = reverseEdgeIndex(edge);

  while (x != xe || y != ye)
  {
    mCurNode->edge(edge)->setStyle(style);

    x += dx;
    y += dy;

    moveTo(x, y);
    mCurNode->edge(reve)->setStyle(style);
  }

  return *mCurNode;
}



void Graph::clearEdgesDone() noexcept
{
  for (auto& node : *this)
    node.clearEdgesDone();
}
