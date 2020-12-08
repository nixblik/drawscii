/*  Copyright 2020 Uwe Salomon <post@uwesalomon.de>

    This file is part of Drawscii.

    Drawscii is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Drawscii is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drawscii.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "graph.h"
#include "shapes.h"
#include <cmath>
#include <vector>



struct ShapePoint
{
  ShapePoint(NOde* n, Angle a, Angle as) noexcept;

  NOde* node;
  Angle angle;
  Angle angleSum;
};



inline ShapePoint::ShapePoint(NOde* n, Angle a, Angle as) noexcept
  : node{n},
    angle{a},
    angleSum{as}
{}



class ShapeFinder
{
  using ShapePoints = std::vector<ShapePoint>;

  public:
    ShapeFinder(GRaph& graph) noexcept;
    Shapes findShapes();

  private:
    void findShapeAt(NOde::EdgeRef edge0);
    void addShape(ShapePoints::const_iterator begin, ShapePoints::const_iterator end, Angle angle);

    GRaph& mGraph;
    ShapePoints mShapePts;
    Shapes mShapes;
};



Shapes findShapes(GRaph& graph)
{
  ShapeFinder finder{graph};
  return finder.findShapes();
}



ShapeFinder::ShapeFinder(GRaph& graph) noexcept
  : mGraph{graph}
{}



Shapes ShapeFinder::findShapes()
{
  for (auto& node: mGraph)
  {
    if (node.edgesAllDone())
      continue;

    if (node.form() != NOde::Straight)
      continue;

    for (int i = 0, endi = node.numberOfEdges(); i < endi; ++i)
    {
      auto edge = node.edge(i);
      if (edge.todo())
        findShapeAt(edge);
    }
  }

  mGraph.clearEdgesDone();
  mShapes.outer.reverse();
  mShapes.inner.reverse();
  return std::move(mShapes);
}



void ShapeFinder::findShapeAt(NOde::EdgeRef edge0)
{
  edge0.setDone();
  auto node1 = &mGraph[edge0.target()];

  if (node1->mark() >= NOde::RightArrow) // FIXME: Reconsider arrow handling, and improve this conditional
    return;

  mShapePts.clear();
  mShapePts.emplace_back(edge0.source(), Angle{0}, Angle{0});
  mShapePts.emplace_back(node1, edge0.angle(), Angle{0});

qDebug("%i/%i and %i/%i", edge0.source()->x(), edge0.source()->y(), node1->x(), node1->y());
  while (mShapePts.size() > 1)
  {
    auto& cur  = mShapePts.back();
    auto  edge = cur.node->nextRightwardTodoEdge(cur.angle);

    if (!edge)
    {
qDebug("  backtrack");
      mShapePts.pop_back();
      continue;
    }

    // Check that new point is ok for shape
    edge.setDone();
    auto nextNode  = &mGraph[edge.target()];
    auto nextAngle = edge.angle();
qDebug(" -> %i/%i angle=%i (sum was %i)", nextNode->x(), nextNode->y(), nextAngle.degrees(), cur.angleSum);

    if (nextNode->mark() >= NOde::RightArrow) // FIXME: Reconsider arrow handling, and improve this conditional
      continue;

    // Check whether new point closes the shape
    for (auto i = mShapePts.begin(); i != mShapePts.end(); ++i)
    {
      if (i->node == nextNode)
      {
        addShape(i, mShapePts.end(), cur.angleSum - i->angleSum);
        mShapePts.erase(i + 1, mShapePts.end());
        goto ContinueOuterLoop;
      }
    }

    // Continue shape-finding at new point
    mShapePts.emplace_back(nextNode, nextAngle, cur.angleSum + nextAngle.relativeTo(cur.angle));
  ContinueOuterLoop:;
  }
}



void ShapeFinder::addShape(ShapePoints::const_iterator begin, ShapePoints::const_iterator end, Angle angle)
{
  SHape shape;
  shape.moveTo(begin->node->point());
  int dashCt = 0;

  for (auto i = begin + 1; i != end; ++i)
  {
    // FIXME: dashCt

    auto node = i->node;
    if (node->form() == NOde::Bezier)
    {
      assert(i + 1 != end);
      auto next = (++i)->node;
      shape.arcTo(next->point(), node->point());
    }
    else
      shape.lineTo(node->point());
  }

qDebug("add shape %li points angle=%i", end - begin, angle.degrees());

  if (angle.degrees() < 0)
    mShapes.inner.emplace_front(std::move(shape));
  else
    mShapes.outer.emplace_front(std::move(shape));
}



void SHape::moveTo(Point p)
{
  mPath.moveTo(p.x, p.y);
  mPos = p;
}



void SHape::lineTo(Point p)
{
  mPath.lineTo(p.x, p.y);
  mPos = p;
}



namespace {

inline double length(const QPointF& p) noexcept
{ return sqrt(p.x()*p.x() + p.y()*p.y()); }


inline QPointF operator+(const Point& p1, const QPointF& p2) noexcept
{ return QPointF{p1.x + p2.x(), p1.y + p2.y()}; }

} // namespace



void SHape::arcTo(Point tgt, Point ctrl)
{
  QPointF r1(mPos.x - ctrl.x, mPos.y - ctrl.y);
  QPointF r2(tgt.x  - ctrl.x, tgt.y  - ctrl.y);

  auto l1 = length(r1);
  auto l2 = length(r2);
  auto r  = std::min(1.0, std::min(l1, l2));

  assert(r > 0);
  r1 *= r/l1;
  r2 *= r/l2;

  mPath.lineTo(ctrl + r1);
  mPath.cubicTo(ctrl + r1 * 0.44771525, ctrl + r2 * 0.44771525, ctrl + r2);
  mPath.lineTo(tgt.x, tgt.y);
  mPos = tgt;
}



#include <QtDebug>
#include <QImage>
#include <QPainter>
void Shapes::dump(const char* fname) const
{
  QRect size{};
  for (auto& path: outer)
    size = size.united(path.mPath.controlPointRect().toRect());

  qDebug() << size;
  if (size.isEmpty())
    return;

  QImage img(QSize{(size.right()+5)*5, (size.bottom()+5)*5}, QImage::Format_RGB32);
  img.fill(Qt::white);

  QPainter pa;
  pa.begin(&img);
  pa.scale(5, 5);
  pa.setPen(QPen(Qt::red, 3));
  for (auto& path: outer)
    pa.drawPath(path.mPath.simplified());

  pa.setPen(QPen(Qt::black, 1));
  for (auto& path: inner)
    pa.drawPath(path.mPath.simplified());

  pa.end();
  img.save(fname);
}
