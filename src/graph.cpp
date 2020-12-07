#include "graph.h"
#include <stdexcept>



inline NOde::NOde(int x, int y) noexcept
  : mEdges{},
    mX(x),
    mY(y),
    mMark{NoMark},
    mForm{Straight},
    mDone{0}
{}



NOde& GRaph::operator[](Point p)
{
  for (auto i = mNodes.begin(); i != mNodes.end(); ++i)
    if (i->point() == p)
      return *i;

  throw std::invalid_argument{"invalid graph node accessed"};
}



NOde& GRaph::moveTo(int x, int y)
{
  for (auto i = mNodes.begin(); i != mNodes.end(); ++i)
    if (i->point().x == x && i->point().y == y)
      return *(mCur = i);

  mNodes.emplace_back(x, y);
  return *(mCur = mNodes.end() - 1);
}



namespace {

int edgeFromDxy(int dx, int dy)
{
  if (dx < 0)
    return dy < 0 ? 0 : dy > 0 ? 1 : 2;

  if (dx > 0)
    return dy < 0 ? 3 : dy > 0 ? 4 : 5;

  return dy < 0 ? 6 : 7;
}

int dxFromEdge(int edge)
{
  constexpr int res[] = {-1, -1, -1, 1, 1, 1, 0, 0};
  return res[edge];
}

int dyFromEdge(int edge)
{
  constexpr int res[] = {-1, +1, 0, -1, +1, 0, -1, +1};
  return res[edge];
}
} // namespace



#include <cstdlib>
NOde& GRaph::lineTo(int dx, int dy, Edge::Style style)
{
  assert(dx == 0 || dy == 0 || abs(dx) == abs(dy));

  int ddx = dx ? dx / abs(dx) : 0;
  int ddy = dy ? dy / abs(dy) : 0;
  int x   = mCur->point().x;
  int y   = mCur->point().y;
  int xe  = x + dx;
  int ye  = y + dy;

  while (x != xe || y != ye)
  {
    mCur->edge(edgeFromDxy(ddx,ddy)).setStyle(style);
    moveTo(x += ddx, y += ddy);
    mCur->edge(edgeFromDxy(-ddx,-ddy)).setStyle(style);
  }

  return *mCur;
}



void GRaph::line(int x, int y, int dx, int dy, Edge::Style style)
{
  moveTo(x, y);
  lineTo(dx, dy, style);
}



#include <QImage>
#include <QPainter>
void GRaph::dump(const char* fname) const
{
  int xmax = 0;
  int ymax = 0;

  for (auto& node: mNodes)
  {
    xmax = std::max(xmax, node.point().x + 1);
    ymax = std::max(ymax, node.point().y + 1);
  }

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
