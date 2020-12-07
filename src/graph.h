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
    constexpr explicit Angle(int value) noexcept
      : mValue{value}
    {}

    Angle& operator+=(Angle other) noexcept
    { mValue += other.mValue; return *this; }

    Angle& operator-=(Angle other) noexcept
    { mValue -= other.mValue; return *this; }

    Angle relativeTo(Angle other) const noexcept;

  private:
    int mValue;
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

    Edge() noexcept
      : mStyle{None}
    {}

    Style style() const noexcept
    { return static_cast<Style>(mStyle); }

    void setStyle(Style style) noexcept
    { mStyle = style; }

  private:
    uint8_t mStyle;
};



class NOde
{
  public:
    enum Form { Straight, Bezier };
    enum Mark {
      NoMark = 0,
      RightArrow, UpArrow, LeftArrow, DownArrow,
      EmptyCircle, FilledCircle,
    };

    class EdgeRef;
    class ConstEdgeRef;

    NOde(int x, int y) noexcept;
    NOde(NOde&&) noexcept =default;
    NOde& operator=(NOde&&) noexcept =default;

    NOde(const NOde&) =delete;
    NOde& operator=(const NOde&) =delete;

    Point point() const noexcept
    { return Point{mX, mY}; }

    Mark mark() const noexcept
    { return static_cast<Mark>(mMark); }

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
    EdgeRef nextTodoEdge(Angle prevAngle) noexcept;

    bool edgesAllDone() const noexcept
    { return mDone == 0xFF; }

  private:
    Edge mEdges[8];
    int16_t mX;
    int16_t mY;
    uint8_t mMark;
    uint8_t mForm;
    uint8_t mDone;
};



class NOde::ConstEdgeRef
{
  public:
    using Style = Edge::Style;

    ConstEdgeRef(const NOde* node, int index) noexcept
      : mNode{node},
        mIndex{index}
    {}

    explicit operator bool() const noexcept
    { return style() != Edge::None; }

    Style style() const noexcept
    { return mNode->mEdges[mIndex].style(); }

    const NOde* source() const noexcept
    { return mNode; }

    Point target() const noexcept;
    int dx() const noexcept;
    int dy() const noexcept;

    Angle angle() const noexcept
    { return Angle{mIndex}; } // FIXME: Uuuh

    bool done() const noexcept
    { return mNode->mDone & (1u << mIndex); }

    bool todo() const noexcept
    { return style() != Edge::None && !done(); }

  protected:
    int index() const noexcept
    { return mIndex; }

  private:
    const NOde* mNode;
    int mIndex;
};



class NOde::EdgeRef : public NOde::ConstEdgeRef
{
  public:
    using Style = Edge::Style;

    EdgeRef(NOde* node, int index) noexcept
      : ConstEdgeRef{node, index}
    {}

    NOde* source() noexcept
    { return const_cast<NOde*>(ConstEdgeRef::source()); }

    void setStyle(Style style) noexcept
    { source()->mEdges[index()].setStyle(style); }

    void setDone() noexcept
    { source()->mDone |= (1u << index()); }
};



inline auto NOde::edge(int idx) const noexcept -> ConstEdgeRef
{ return {this, idx}; }

inline auto NOde::edge(int idx) noexcept -> EdgeRef
{ return {this, idx}; }



class GRaph
{
  public:
    NOde* begin() noexcept
    { return &*mNodes.begin(); }

    NOde* end() noexcept
    { return &*mNodes.end(); }

    const NOde& operator[](Point p) const;
    NOde& operator[](Point p);

    NOde& moveTo(int x, int y);
    NOde& lineTo(int dx, int dy, Edge::Style style);
    void line(int x, int y, int dx, int dy, Edge::Style style);
    void dump(const char* fname) const;

  private:
    using Nodes = std::vector<NOde>; // FIXME: Not optimal at all, use hashmap; approximate size from number of chars in TextImage
    Nodes mNodes;
    Nodes::iterator mCur;
};
