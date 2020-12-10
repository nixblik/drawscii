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
    mX{0},
    mY{0},
    mMark{NoMark},
    mForm{Straight},
    mDone{0}
{}



inline Node::Node(int x, int y)
  : mEdges{},
    mX{static_cast<int16_t>(x)},
    mY{static_cast<int16_t>(y)},
    mMark{NoMark},
    mForm{Straight},
    mDone{0}
{
  if (x > std::numeric_limits<int16_t>::max() || y > std::numeric_limits<int16_t>::max())
    throw std::runtime_error{"drawing is too large"};
}



Point Node::ConstEdgeRef::target() const noexcept
{
  auto p = mNode->point();
  return Point{p.x + dx(), p.y + dy()};
}



int Node::ConstEdgeRef::dx() const noexcept
{
  assert(mIndex >= 0 && mIndex < 8);

  constexpr static int dxs[] = { +1, +1, 0, -1, -1, -1, 0, +1 };
  return dxs[mIndex];
}



int Node::ConstEdgeRef::dy() const noexcept
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



auto Node::nextRightwardTodoEdge(Angle prevAngle) noexcept -> EdgeRef
{
  assert(prevAngle.degrees() >= 0 && prevAngle.degrees() < 360);

  int idx0 = reverseEdgeIndex(prevAngle.degrees() / 45);
  int idx  = idx0;

  while ((idx = (idx + 1) & 7) != idx0)
    if (mEdges[idx] && !(mDone & (1u << idx)))
      return edge(idx);

  assert(!noEdgesNode.edge(0));
  return noEdgesNode.edge(0);
}



auto Node::reverseEdge(const ConstEdgeRef& edge) noexcept -> EdgeRef
{ return EdgeRef{this, reverseEdgeIndex(edge.index())}; }



auto Node::continueLine(const ConstEdgeRef& edge) noexcept -> EdgeRef // FIXME: Consider continuing the line also if it's not straight!
{
  switch (mForm)
  {
    case Straight: return EdgeRef{this, edge.index()};

    case Bezier: {
      for (int idx = 0; idx < 8; ++idx)
        if (idx != edge.index() && mEdges[idx])
          return EdgeRef{this, idx};
      break;
    }
  }

  assert(false);
}



Graph::Graph() noexcept
  : mCurNode{nullptr},
    mLeft{0},
    mRight{0},
    mTop{0},
    mBottom{0}
{}



Node& Graph::operator[](Point p)
{
  for (auto i = mNodes.begin(); i != mNodes.end(); ++i)
    if (i->point() == p)
      return *i;

  throw std::invalid_argument{"invalid graph node accessed"};
}



Node& Graph::moveTo(int x, int y)
{
  for (auto i = mNodes.begin(); i != mNodes.end(); ++i)
    if (i->x() == x && i->y() == y)
      return *(mCurNode = &*i);

  mNodes.emplace_back(x, y);
  mCurNode = &mNodes.back();
  mLeft    = std::min(mLeft, x);
  mRight   = std::max(mRight, x);
  mTop     = std::min(mTop, y);
  mBottom  = std::max(mBottom, y);

  return *mCurNode;
}



namespace {
int signum(int x) noexcept
{
  return x < 0 ? -1 : (x > 0);
}
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
    mCurNode->edge(edge).setStyle(style);

    x += dx;
    y += dy;

    moveTo(x, y);
    mCurNode->edge(reve).setStyle(style);
  }

  return *mCurNode;
}



void Graph::clearEdgesDone() noexcept
{
  for (auto& node: mNodes)
    node.clearEdgesDone();
}



#include <QImage>
#include <QPainter>
void Graph::dump(const char* fname) const
{
  int xmax = 0;
  int ymax = 0;

  for (auto& node: mNodes)
  {
    xmax = std::max(xmax, node.point().x + 1);
    ymax = std::max(ymax, node.point().y + 1);
  }

  qDebug("%ix%i", xmax, ymax);

  QImage img(QSize{xmax*5, ymax*5}, QImage::Format_RGB32);
  img.fill(Qt::white);

  QPainter pa;
  pa.begin(&img);
  pa.setPen(Qt::black);
  for (auto& node: mNodes)
  {
    QPoint np{node.point().x*5, node.point().y*5};
    for (int dir = 0; dir < node.numberOfEdges(); ++dir)
      if (auto edge = node.edge(dir))
        pa.drawLine(np, np + QPoint{edge.dx()*5, edge.dy()*5});
  }

  pa.end();
  img.save(fname);
}
