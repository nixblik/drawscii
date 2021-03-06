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
#include <memory>
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
/// Nodes and Edges.
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
    /// Drawing style of the edge.
    enum Style { None, Invisible, Weak, Solid, Double, Dashed };

    constexpr Edge() noexcept
      : mStyle{None}
    {}

    bool exists() const noexcept
    { return mStyle != Edge::None; }

    Style style() const noexcept
    { return static_cast<Style>(mStyle); }

    void setStyle(Style style) noexcept
    { mStyle = style; }

  private:
    uint8_t mStyle;
};



/// A node in a planar Graph. Nodes must have an integer position. They are
/// connected by edges. Edges have a direction from one node to the other, i.e.
/// if two nodes are connected, each node has an edge to the other node. The
/// Graph ensures that edges are always created pairwise.
///
/// This particular implementation allows only connections between a node and
/// the 8 nodes immediately adjacent to it:
///
///      3   2   1
///       *  *  *
///        \ | /
///         \|/
///     4 *--o--* 0       This graph element
///         /|\           of 9 nodes is the size
///        / | \          of one character
///       *  *  *
///      5   6   7
///
/// Edges can be marked "done" using EdgeRef::setDone(). The marks can be
/// cleared again using clearEdgesDone(). This aids the implementation of
/// algorithms that traverse the Graph.
///
class Node
{
  public:
    class EdgeRef;
    class edge_ptr;
    class const_edge_ptr;

    /// Form of the edges touching this node.
    enum Form { Straight, Curved };

    /// Mark to be drawn on top of the node, like circles or arrows.
    enum Mark {
      NoMark = 0,
      EmptyCircle, FilledCircle,
      RightArrow, UpArrow, LeftArrow, DownArrow, // Order matters for isClosedMark()
      Leapfrog
    };

    /// \internal
    constexpr Node() noexcept;

    /// \internal
    bool isSentinel() const noexcept
    { return mX == -32768; }

    /// \internal
    uint hash() const noexcept;

    /// \internal
    Node(int x, int y);

    Node(Node&&) noexcept
    = default;

    Node(const Node&) =delete;
    Node& operator=(Node&&) noexcept =delete;
    Node& operator=(const Node&) =delete;

    int x() const noexcept
    { return mX; }

    int y() const noexcept
    { return mY; }

    Point point() const noexcept
    { return Point{mX, mY}; }

    Mark mark() const noexcept
    { return static_cast<Mark>(mMark); }

    /// Whether the mark() can be part of a closed shape.
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

    /// Kind-of-a pointer to the edge with \a index, which must be in [0, \a
    /// numberOfEdges()]. The function may return a non-existing edge, which
    /// must be checked by the caller.
    const_edge_ptr edge(int index) const noexcept;

    /// \overload
    edge_ptr edge(int index) noexcept;

    /// The next edge to the right of \a prevAngle that exists and is not
    /// marked done. If no such edge remains, an invalid edge pointer is
    /// returned. The function does not allow a "u-turn".
    ///
    /// Consider the following example:
    ///
    ///        (2)
    ///     *   *   *
    ///         |
    ///         |
    ///     *   o---*  ---> prevAngle
    ///         |
    ///         |
    ///     *   *   *
    ///        (1)
    ///
    /// For this node and the indicated value of prevAngle, the function would
    /// first return a pointer to edge (1), then if that were marked done, to
    /// edge (2), and if that were also marked done, an invalid edge pointer.
    ///
    edge_ptr nextRightwardTodoEdge(Angle prevAngle) noexcept;

    /// The edge in the opposite direction of \a edge.
    edge_ptr oppositeEdge(const_edge_ptr edge) noexcept;

    /// The edge that continues from this node in the same direction as \a
    /// prevEdge (which is an edge of another, usually adjacent node).
    edge_ptr continueLine(const_edge_ptr prevEdge) noexcept;

    bool edgesAllDone() const noexcept
    { return mDone == 0xFF; }

    void clearEdgesDone() noexcept
    { mDone = mDone0; }

  private:
    Edge mEdges[8];
    const int16_t mX;
    const int16_t mY;
    uint8_t mMark;
    uint8_t mForm;
    uint8_t mDone;
    uint8_t mDone0;
};



/// Helper class that behaves like a reference to an Edge of a planar Graph. Is
/// always used through Node::edge_ptr or Node::const_edge_ptr.
///
class Node::EdgeRef
{
  public:
    using Style = Edge::Style;

    EdgeRef(Node* node, int index) noexcept
      : mNode{node},
        mIndex{index}
    {}

    bool exists() const noexcept
    { return mNode->mEdges[mIndex].exists(); }

    Style style() const noexcept
    { return mNode->mEdges[mIndex].style(); }

    void setStyle(Style style) noexcept;

    const Node* source() const noexcept
    { return mNode; }

    Node* source() noexcept
    { return mNode; }

    Point target() const noexcept;
    int dx() const noexcept;
    int dy() const noexcept;

    Angle angle() const noexcept
    { return Angle{mIndex * 45}; }

    bool done() const noexcept
    { return mNode->mDone & (1u << mIndex); }

    void setDone() noexcept
    { mNode->mDone |= (1u << index()); }

    /// \internal
    int index() const noexcept
    { return mIndex; }

  private:
    Node* mNode;
    int mIndex;
};



/// A (modifiable) kind-of-a pointer to an Edge in a planar Graph. While there
/// is no direct equivalent of a null pointer, it is still possible for an
/// edge_ptr to point to an invalid (non-existing) edge.
///
class Node::edge_ptr
{
  public:
    edge_ptr(Node* node, int index) noexcept
      : mRef{node, index}
    {}

    /// Whether the edge pointed to exists at all.
    explicit operator bool() const noexcept
    { return mRef.exists(); }

    EdgeRef* operator->() noexcept
    { return &mRef; }

  private:
    EdgeRef mRef;
};



/// A constant kind-of-a pointer to an Edge in a planar Graph. While there is
/// no direct equivalent of a null pointer, it is still possible for a
/// const_edge_ptr to point to an invalid (non-existing) edge.
///
class Node::const_edge_ptr
{
  public:
    const_edge_ptr(edge_ptr other) noexcept
      : mRef{other->source(), other->index()}
    {}

    const_edge_ptr(const Node* node, int index) noexcept
      : mRef{const_cast<Node*>(node), index}
    {}

    /// Whether the edge pointed to exists at all.
    explicit operator bool() const noexcept
    { return mRef.exists(); }

    const EdgeRef* operator->() noexcept
    { return &mRef; }

  private:
    EdgeRef mRef;
};



inline auto Node::edge(int index) const noexcept -> const_edge_ptr
{ return const_edge_ptr{this, index}; }


inline auto Node::edge(int index) noexcept -> edge_ptr
{ return edge_ptr{this, index}; }



/// Iterator for the Nodes in a planar Graph.
///
template<typename Node>
class GraphIterator
{
  public:
    explicit GraphIterator(Node* iter) noexcept
      : mIter{iter}
    {}

    Node& operator*() const noexcept
    { return *mIter; }

    Node* operator->() const noexcept
    { return mIter; }

    GraphIterator& operator++() noexcept
    {
      do { ++mIter; } while (mIter->isSentinel());
      return *this;
    }

    bool operator!=(GraphIterator other) const noexcept
    { return mIter != other.mIter; }

  private:
    Node* mIter;
};



/// A planar Graph that consists of Nodes and Edges.
///
class Graph
{
  public:
    using iterator       = GraphIterator<Node>;
    using const_iterator = GraphIterator<const Node>;

    explicit Graph() noexcept;

    /// Reserves spaces for \a capacity Nodes.
    void reserve(uint capacity);

    iterator begin() noexcept
    { return iterator{&mTable[0]}; }

    const_iterator begin() const noexcept
    { return const_iterator{&mTable[0]}; }

    iterator end() noexcept
    { return iterator{&mTable[mCapacity]}; }

    const_iterator end() const noexcept
    { return const_iterator{&mTable[mCapacity]}; }

    int left() const noexcept
    { return mLeft; }

    int right() const noexcept
    { return mRight; }

    int top() const noexcept
    { return mTop; }

    int bottom() const noexcept
    { return mBottom; }

    /// Reference to the Node at position \a p. The node must already exist.
    Node& operator[](Point p);

    /// Moves the "drawing cursor" to the Node at position \a x, \a y. The node
    /// is created if it does not exist yet.
    Node& moveTo(int x, int y);

    /// Creates edges with \a style from the current position of the "drawing
    /// cursor" to the Node which is at \a dx, \a dy relative to the current
    /// position. All nodes on the way are created if necessary.
    Node& lineTo(int dx, int dy, Edge::Style style);

    /// Resets the "done" marker of all Edges in the graph.
    void clearEdgesDone() noexcept;

  public:
    std::unique_ptr<Node[]> mTable;
    Node* mCurNode;
    uint mSize;
    uint mCapacity;

    int mLeft;
    int mRight;
    int mTop;
    int mBottom;
};
