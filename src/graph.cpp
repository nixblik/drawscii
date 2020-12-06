#include "graph.h"



inline NOde::NOde(int x, int y) noexcept
  : mEdges{},
    mX(x),
    mY(y),
    mMark{None},
    mForm{}
{}



NOde& GRaph::moveTo(int x, int y)
{
  for (auto i = mNodes.begin(); i != mNodes.end(); ++i)
    if (i->mX == x && i->mY == y)
      return *(mCur = i);

  mNodes.emplace_back(x, y);
  return *(mCur = mNodes.end() - 1);
}



#include "direction.h" // FIXME: Remove
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



#include <cstdlib>
NOde& GRaph::relLine(int dx, int dy, Edge::Style style)
{
  int ddx = dx ? dx / abs(dx) : 0;
  int ddy = dy ? dy / abs(dy) : 0;
  int x   = mCur->mX;
  int y   = mCur->mY;
  int xe  = x + dx;
  int ye  = y + dy;

  while (x != xe || y != ye)
  {
    mCur->mEdges[edgeFromDxy(ddx,ddy)].setStyle(style);
    moveTo(x += ddx, y += ddy);
    mCur->mEdges[edgeFromDxy(-ddx,-ddy)].setStyle(style);
  }

  return *mCur;
}



void GRaph::line(int x, int y, int dx, int dy, Edge::Style style)
{
  moveTo(x, y);
  relLine(dx, dy, style);
}



#include <QImage>
#include <QPainter>
void GRaph::dump(const char* fname) const
{
  int xmax = 0;
  int ymax = 0;

  for (auto& n: mNodes)
  {
    xmax = std::max(xmax, int{n.mX} + 1);
    ymax = std::max(ymax, int{n.mY} + 1);
  }

  QImage img(QSize{xmax*5, ymax*5}, QImage::Format_RGB32);
  img.fill(Qt::white);

  QPainter pa;
  pa.begin(&img);
  pa.setPen(Qt::black);
  for (auto& n: mNodes)
  {
    QPoint np{n.mX*5, n.mY*5};
    for (int dir = 0; dir < 8; ++dir)
    {
      if (n.mEdges[dir])
        pa.drawLine(np, np + QPoint{dxFromEdge(dir)*5, dyFromEdge(dir)*5});
    }
  }

  pa.end();
  img.save(fname);
}
