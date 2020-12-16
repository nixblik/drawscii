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



/// A shape in a planar graph is a single continuous line or polygon,
/// optionally with some of the edges rounded.
///
class Shape
{
  public:
    Shape() noexcept;
    ~Shape();

    /// Moves the current position to \a p. Must be called exactly once, at the
    /// beginning of creating the shape.
    void moveTo(Point p);

    /// Draws a line from the current position to \a p.
    void lineTo(Point p);

    /// Draws an arc from the current position to \a p in such a way that it is
    /// tangent to the lines between the current position and \a ctrl, and
    /// between \a ctrl and \a p:
    ///
    ///     current     ctrl
    ///        *---------*
    ///            '·-.  |
    ///                '.|
    ///                 '|
    ///                  * p
    ///
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



/// Holds all shapes that will be detected in the input image. Consider the
/// following example:
///
///     +------+-+ +--+
///     | +-+ B|C| |D |
///     | |A|  | | +--+
///     | +-+  | |
///     +------+-+
///
/// This drawing has:
///
/// - 3 outer shapes, A, B+C and D
/// - 4 inner shapes, A, B, C and D
/// - 13 lines (but depends on the algorithm, whether it goes around the
///   corners or not)
///
/// When rendering, first the outer shapes get a drop shadow. Then inner shapes
/// are filled. At last, lines will be drawn.
///
struct Shapes
{
  using List = std::forward_list<Shape>;

  /// All closed shapes that cannot be joined with adjacent shapes to an even
  /// larger shape.
  List outer;

  /// All closed shapes that cannot be split in even smaller closed shapes.
  List inner;

  /// All lines to be drawn, of a single style. They could be part of a closed
  /// shape or an "open" line.
  List lines;
};



/// Analyzes the \a graph and finds all Shapes in it.
Shapes findShapes(Graph& graph);
