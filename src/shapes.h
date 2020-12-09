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
#include <QColor>
class QPainterPath;



class Shape
{
  public:
    Shape() noexcept;
    ~Shape();

    void moveTo(Point p);
    void lineTo(Point p);
    void arcTo(Point p, Point ctrl);
    void done();

    QPainterPath path(int xScale, int yScale, double radius) const;
    QColor color;
    Edge::Style style;

  private:
    enum ElementKind { Move, Line, Arc };
    struct Element;

    std::vector<Element> mPath;
};



struct Shapes
{
  using List = std::forward_list<Shape>;

  List outer;
  List inner;
  List lines;

  void dump(const char* fname) const;
};



/// Analyzes the \a graph and finds all closed Shapes in it. A closed shape has
/// lines all around it, and no arrows on the border.
///
Shapes findShapes(Graph& graph);
