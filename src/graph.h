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
#include "common.h"
#include <vector>



/// An integer position in a planar Graph.
///
struct Point
{
  int x;
  int y;
};



inline bool operator==(const Point& a, const Point& b) noexcept
{ return a.x == b.x && a.y == b.y; }



/// Something roughly equivalent to an angle in planar geometry. The class is
/// introduced for type safety here; and to hide implementation details of
/// NOdes and Edges.
///
class Angle
{
  public:
    constexpr explicit Angle(int degrees) noexcept
      : mDegrees{degrees}
    {}

    int degrees() const noexcept
    { return mDegrees; }

    Angle& operator+=(Angle other) noexcept
    { mDegrees += other.mDegrees; return *this; }

    Angle& operator-=(Angle other) noexcept
    { mDegrees -= other.mDegrees; return *this; }

    /// How many degrees this angle turns relative to the \a other angle. The
    /// result is in the range [-179, 180] degrees. Positive values represent
    /// a turn to the left, negative values to the right.
    Angle relativeTo(Angle other) const noexcept;

  private:
    int mDegrees;
};



inline Angle operator+(Angle a, Angle b) noexcept
{ return a += b; }


inline Angle operator-(Angle a, Angle b) noexcept
{ return a -= b; }



/// An edge between two Nodes in a planar Graph. Most of the public edge
/// manipulation interface is provided by Node::EdgeRef, though.
///
class Edge
{
  public:
    enum Style { None, Weak, Solid, Double, Dashed };

    constexpr Edge() noexcept
      : mStyle{None}
    {}

    explicit operator bool() const noexcept
    { return style() != Edge::None; }

    Style style() const noexcept
    { return static_cast<Style>(mStyle); }

    void setStyle(Style style) noexcept
    { mStyle = style; }

  private:
    uint8_t mStyle;
};



/// A node in a planar Graph.
///
class Node
{
  public:
    class EdgeRef;
    class ConstEdgeRef;

    enum Form { Straight, Bezier };
    enum Mark {
      NoMark = 0,
      EmptyCircle, FilledCircle,
      RightArrow, UpArrow, LeftArrow, DownArrow, // Order matters for isClosedMark()
    };

    /// \internal
    constexpr Node() noexcept;

    /// \internal
    Node(int x, int y);

    Node(Node&&) noexcept
    =default;

    Node& operator=(Node&&) noexcept
    =default;

    Node(const Node&) =delete;
    Node& operator=(const Node&) =delete;

    int x() const noexcept
    { return mX; }

    int y() const noexcept
    { return mY; }

    Point point() const noexcept
    { return Point{mX, mY}; }

    Mark mark() const noexcept
    { return static_cast<Mark>(mMark); }

    bool isClosedMark() const noexcept
    { return mMark < RightArrow; }

    Form form() const noexcept
    { return static_cast<Form>(mForm); }

    void setMark(Mark mark) noexcept
    { mMark = mark; }

    void setForm(Form form) noexcept
    { mForm = form; }

    constexpr int numberOfEdges() const noexcept
    { return 8; }

    ConstEdgeRef edge(int idx) const noexcept;
    EdgeRef edge(int idx) noexcept;
    EdgeRef nextRightwardTodoEdge(Angle prevAngle) noexcept;
    EdgeRef reverseEdge(const ConstEdgeRef& edge) noexcept;
    EdgeRef continueLine(const ConstEdgeRef& prevEdge) noexcept;

    bool edgesAllDone() const noexcept
    { return mDone == 0xFF; }

    void clearEdgesDone() noexcept
    { mDone = 0; }

  private:
    Edge mEdges[8];
    int16_t mX;
    int16_t mY;
    uint8_t mMark;
    uint8_t mForm;
    uint8_t mDone;
};



class Node::ConstEdgeRef
{
  public:
    using Style = Edge::Style;

    ConstEdgeRef(const Node* node, int index) noexcept
      : mNode{node},
        mIndex{index}
    {}

    explicit operator bool() const noexcept
    { return static_cast<bool>(mNode->mEdges[mIndex]); }

    Style style() const noexcept
    { return mNode->mEdges[mIndex].style(); }

    const Node* source() const noexcept
    { return mNode; }

    Point target() const noexcept;
    int dx() const noexcept;
    int dy() const noexcept;

    Angle angle() const noexcept
    { return Angle{mIndex * 45}; }

    bool done() const noexcept // FIXME: Too expensive. Start out by marking only edges todo that exist. Then the test will be much quicker in Node::allEdgesDone()
    { return mNode->mDone & (1u << mIndex); }

    /// \internal
    int index() const noexcept
    { return mIndex; }

  private:
    const Node* mNode;
    int mIndex;
};



class Node::EdgeRef : public Node::ConstEdgeRef
{
  public:
    using Style = Edge::Style;

    EdgeRef(Node* node, int index) noexcept
      : ConstEdgeRef{node, index}
    {}

    Node* source() noexcept
    { return const_cast<Node*>(ConstEdgeRef::source()); }

    void setStyle(Style style) noexcept
    { source()->mEdges[index()].setStyle(style); }

    void setDone() noexcept
    { source()->mDone |= (1u << index()); }
};



inline auto Node::edge(int idx) const noexcept -> ConstEdgeRef
{ return {this, idx}; }

inline auto Node::edge(int idx) noexcept -> EdgeRef
{ return {this, idx}; }



class Graph
{
  public:
    Graph() noexcept;

    Node* begin() noexcept
    { return &*mNodes.begin(); }

    const Node* begin() const noexcept
    { return &*mNodes.begin(); }

    Node* end() noexcept
    { return &*mNodes.end(); }

    const Node* end() const noexcept
    { return &*mNodes.end(); }

    int left() const noexcept
    { return mLeft; }

    int right() const noexcept
    { return mRight; }

    int top() const noexcept
    { return mTop; }

    int bottom() const noexcept
    { return mBottom; }

    const Node& operator[](Point p) const;
    Node& operator[](Point p);
    Node& moveTo(int x, int y);
    Node& lineTo(int dx, int dy, Edge::Style style);
    void clearEdgesDone() noexcept;

  private:
    using Nodes = std::vector<Node>; // FIXME: Not optimal at all, use hashmap; approximate size from number of chars in TextImage

    Nodes mNodes;
    Node* mCurNode;
    int mLeft;
    int mRight;
    int mTop;
    int mBottom;
};
