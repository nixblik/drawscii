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
#include <vector>



struct ShapePoint
{
  ShapePoint(NOde* n, Angle a, Angle as) noexcept;

  NOde* node;
  Angle angle;
  Angle angleSum;
};


using ShapePoints = std::vector<ShapePoint>;
using ShapeList   = std::list<SHape>;



inline ShapePoint::ShapePoint(NOde* n, Angle a, Angle as) noexcept
  : node{n},
    angle{a},
    angleSum{as}
{}



class ShapeFinder
{
  public:
    ShapeFinder(GRaph& graph);
    void findShapes();

  private:
    void findShapeAt(NOde::EdgeRef edge0);
    void addShape(ShapePoints::const_iterator begin, ShapePoints::const_iterator end, Angle angle);

    GRaph& mGraph;
    ShapePoints mShapePts;
    ShapeList mOuterShapes;
    ShapeList mInnerShapes;
};



void ShapeFinder::findShapes()
{
  for (auto& node: mGraph)
  {
    if (node.edgesAllDone())
      continue;

    if (node.form() != NOde::Straight) // FIXME: Also ignore arrows?
      continue;

    for (int i = 0, endi = node.numberOfEdges(); i < endi; ++i)
    {
      auto edge = node.edge(i);
      if (edge.todo())
        findShapeAt(edge);
    }
  }
}



void ShapeFinder::findShapeAt(NOde::EdgeRef edge0)
{
  edge0.setDone();
  auto node1 = &mGraph[edge0.target()];

  if (node1->mark() >= NOde::LeftArrow) // FIXME: Reconsider arrow handling, and improve this conditional
    return;

  mShapePts.clear();
  mShapePts.emplace_back(edge0.source(), Angle{0}, Angle{0});
  mShapePts.emplace_back(node1, edge0.angle(), Angle{0});

  while (mShapePts.size() > 1)
  {
    auto& cur  = mShapePts.back();
    auto  edge = cur.node->nextTodoEdge(cur.angle); // FIXME: Maybe it must say "rightwards"?

    if (!edge)
    {
      mShapePts.pop_back();
      continue;
    }

    // Check that new point is ok for shape
    edge.setDone();
    auto nextNode  = &mGraph[edge.target()];
    auto nextAngle = edge.angle();

    if (nextNode->mark() >= NOde::LeftArrow) // FIXME: Reconsider arrow handling, and improve this conditional
      return;

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
}
