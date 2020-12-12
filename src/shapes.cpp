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
#include <QPainterPath>



struct Shape::Element
{
  Element(Point np, ElementKind k) noexcept;

  Point p;
  ElementKind kind;
};



inline Shape::Element::Element(Point np, ElementKind k) noexcept
  : p{np},
    kind{k}
{}



struct ShapePoint
{
  ShapePoint(Node* n, Angle a, Angle as, int d) noexcept;

  Node* node;
  Angle angle;
  Angle angleSum;
  int dashCt;
};



inline ShapePoint::ShapePoint(Node* n, Angle a, Angle as, int d) noexcept
  : node{n},
    angle{a},
    angleSum{as},
    dashCt{d}
{}



class ShapeFinder
{
  using ShapePoints = std::vector<ShapePoint>;

  public:
    ShapeFinder(Graph& graph) noexcept;
    Shapes findShapes();

  private:
    void findClosedShapes();
    void findClosedShapeAt(Node::edge_ptr edge0);
    void addClosedShape(ShapePoints::const_iterator begin, ShapePoints::const_iterator end, Angle angle, int dashCt);
    void findLines();
    void findLineAt(Node::edge_ptr edge0);

    Graph& mGraph;
    ShapePoints mShapePts;
    Shapes mShapes;
};



Shapes findShapes(Graph& graph)
{
  ShapeFinder finder{graph};
  return finder.findShapes();
}



ShapeFinder::ShapeFinder(Graph& graph) noexcept
  : mGraph{graph}
{}



Shapes ShapeFinder::findShapes()
{
  findClosedShapes();
  mGraph.clearEdgesDone();
  findLines();

  return std::move(mShapes);
}



namespace {
inline bool isAboveLeft(const Point& a, const Point& b) noexcept
{ return a.y < b.y || (a.y == b.y && a.x < b.x); }
} // namespace



struct ShapeUpLeft
{
  inline bool operator()(const Shape& s1, const Shape& s2)
  { return isAboveLeft(s1.topLeft(), s2.topLeft()); }
};



void ShapeFinder::findClosedShapes()
{
  for (auto& node: mGraph)
  {
    if (node.edgesAllDone() || node.form() != Node::Straight)
      continue;

    for (int i = 0, endi = node.numberOfEdges(); i < endi; ++i)
      if (auto edge = node.edge(i))
        if (!edge->done())
          findClosedShapeAt(edge);
  }

  mShapes.inner.sort(ShapeUpLeft{});
}



void ShapeFinder::findClosedShapeAt(Node::edge_ptr edge0)
{
  edge0->setDone();
  auto node1 = &mGraph[edge0->target()];
  if (!node1->isClosedMark())
    return;

  mShapePts.clear();
  mShapePts.emplace_back(edge0->source(), Angle{0}, Angle{0}, 0);
  mShapePts.emplace_back(node1, edge0->angle(), Angle{0}, (edge0->style() == Edge::Dashed));

  while (mShapePts.size() > 1)
  {
    auto& cur  = mShapePts.back();
    auto  edge = cur.node->nextRightwardTodoEdge(cur.angle);

    if (!edge)
    {
      mShapePts.pop_back();
      continue;
    }

    // Check that new point is ok for shape
    edge->setDone();
    auto nextNode  = &mGraph[edge->target()];
    if (!nextNode->isClosedMark())
      continue;

    auto nextAngle = edge->angle();
    auto angleSum  = cur.angleSum + nextAngle.relativeTo(cur.angle);
    auto dashCt    = cur.dashCt + (edge->style() == Edge::Dashed);
    mShapePts.emplace_back(nextNode, nextAngle, angleSum, dashCt);

    // Check whether new point closes the shape
    for (auto i = mShapePts.begin(); i != mShapePts.end(); ++i)
    {
      if (i->node == nextNode)
      {
        const auto& back = mShapePts.back();
        addClosedShape(i, mShapePts.end(), back.angleSum - i->angleSum, back.dashCt - i->dashCt);
        mShapePts.erase(i + 1, mShapePts.end());
        break;
      }
    }
  }
}



void ShapeFinder::addClosedShape(ShapePoints::const_iterator begin, ShapePoints::const_iterator end, Angle angle, int dashCt)
{
  Shape shape;
  shape.moveTo(begin->node->point());

  for (auto i = begin + 1; i != end; ++i)
  {
    auto node = i->node;
    if (node->form() == Node::Bezier)
    {
      assert(i + 1 != end);
      auto next = (++i)->node;
      shape.arcTo(next->point(), node->point());
    }
    else
      shape.lineTo(node->point());
  }

  if (angle.degrees() < 0)
    mShapes.inner.emplace_front(std::move(shape));
  else if (dashCt * 4 < end - begin)
    mShapes.outer.emplace_front(std::move(shape));
}



void ShapeFinder::findLines()
{
  for (auto& node: mGraph)
  {
    if (node.edgesAllDone() || node.form() != Node::Straight)
      continue;

    for (int i = 0, endi = node.numberOfEdges(); i < endi; ++i)
      if (auto edge = node.edge(i))
        if (!edge->done())
          findLineAt(edge);
  }
}



void ShapeFinder::findLineAt(Node::edge_ptr edge0)
{
  Shape shape;
  shape.moveTo(edge0->source()->point());

  auto curEdge = edge0;
  auto style   = edge0->style();
  bool drawCur = false;

  while (!curEdge->done())
  {
    curEdge->setDone();
    drawCur = true;

    auto curTarget = &mGraph[curEdge->target()];
    curTarget->reverseEdge(curEdge)->setDone();

    // Follow the line to the next edge and check that we can draw on
    auto nextEdge = curTarget->continueLine(curEdge);
    if (!nextEdge)
      break;

    auto nextStyle = nextEdge->style();
    if (style == Edge::Weak)
      style = nextStyle;
    else if (style != nextStyle && nextStyle != Edge::Weak)
      break;

    // Only curved corners have to be drawn, otherwise lines are straight
    if (curTarget->form() == Node::Bezier)
    {
      assert(!nextEdge->done());
      shape.lineTo(curEdge->source()->point());
      shape.arcTo(nextEdge->target(), curTarget->point());
    }
    else
      shape.lineTo(curEdge->target());

    drawCur = false;
    curEdge = nextEdge;
  }

  if (drawCur)
    shape.lineTo(curEdge->target());

  shape.setStyle(style);
  mShapes.lines.emplace_front(std::move(shape));
}



inline Shape::Shape() noexcept
  : mTopLeft{std::numeric_limits<int>::max(), std::numeric_limits<int>::max()},
    mStyle{Edge::Solid}
{}


Shape::~Shape()
= default;


void Shape::moveTo(Point p)
{
  if (mPath.empty() || isAboveLeft(p, mTopLeft))
    mTopLeft = p;

  mPath.emplace_back(p, Move);
}



void Shape::lineTo(Point p)
{
  assert(!mPath.empty());

  if (isAboveLeft(p, mTopLeft))
    mTopLeft = p;

  // Attempt to merge straight lines
  if (mPath.back().kind == Line)
  {
    const auto& p0 = (mPath.end() - 2)->p;
    const auto& p1 = mPath.back().p;

    if ((p1.x - p0.x) * (p.y - p1.y) == (p.x - p1.x) * (p1.y - p0.y))
    {
      mPath.back().p = p;
      return;
    }
  }

  mPath.emplace_back(p, Line);
}



void Shape::arcTo(Point tgt, Point ctrl)
{
  assert(!mPath.empty());

  if (isAboveLeft(tgt, mTopLeft))
    mTopLeft = tgt;

  mPath.emplace_back(ctrl, Arc);
  mPath.emplace_back(tgt, Arc);
}



namespace {
inline double length(const QPointF& p) noexcept
{ return sqrt(p.x()*p.x() + p.y()*p.y()); }
} // namespace



QPainterPath Shape::path(double xScale, double yScale, double radius) const
{
  auto scaled = [=](Point p) -> QPointF
  { return QPointF(round(p.x * xScale), round(p.y * yScale)); };

  QPainterPath result;
  for (auto i = mPath.begin(); i != mPath.end(); ++i)
  {
    auto pi = scaled(i->p);
    switch (i->kind)
    {
      case Move: result.moveTo(pi); continue;
      case Line: result.lineTo(pi); continue;

      case Arc: {
        // Draw a Bézier curve that approximates a circle
        auto p2 = scaled((++i)->p);
        auto r1 = result.currentPosition() - pi;
        auto r2 = p2 - pi;
        auto l1 = length(r1);
        auto l2 = length(r2);
        auto r  = std::min(radius, std::min(l1, l2));

        assert(r > 0);
        r1 *= r/l1;
        r2 *= r/l2;

        result.lineTo(pi + r1);
        result.cubicTo(pi + r1 * 0.44771525, pi + r2 * 0.44771525, pi + r2);
        result.lineTo(p2);
        continue;
      }
    }
  }

  return result;
}
