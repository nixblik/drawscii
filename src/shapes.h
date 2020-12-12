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
#pragma once
#include "graph.h"
#include <forward_list>
#include <vector>
class QPainterPath;



class Shape
{
  public:
    Shape() noexcept;
    ~Shape();

    void moveTo(Point p);
    void lineTo(Point p);
    void arcTo(Point p, Point ctrl);

    const Point& topLeft() const noexcept
    { return mTopLeft; }

    Edge::Style style() const noexcept
    { return mStyle; }

    void setStyle(Edge::Style style) noexcept
    { mStyle = style; }

    QPainterPath path(double xScale, double yScale, double radius) const;

  private:
    enum ElementKind { Move, Line, Arc };
    struct Element;

    std::vector<Element> mPath;
    Point mTopLeft;
    Edge::Style mStyle;
};



struct Shapes
{
  using List = std::forward_list<Shape>;

  List outer;
  List inner;
  List lines;
};



/// Analyzes the \a graph and finds all closed Shapes in it. A closed shape has
/// lines all around it, and no arrows on the border.
///
Shapes findShapes(Graph& graph);
