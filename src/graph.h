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



struct Point
{
  int x;
  int y;
};



class Edge
{
  public:
    enum Style { None, Weak, Solid, Double, Dashed };

    explicit operator bool() const noexcept
    { return mStyle != None; }

    Edge() : mStyle{None} {}
    Style style() const noexcept;
    void setStyle(Style style) noexcept { mStyle = style; }
    bool done() const noexcept;
    void setDone() noexcept;

  private:
    uint8_t mStyle : 7;
    bool    mDone  : 1;  // FIXME: Clearing is faster if extra
};



class NOde
{
  friend class GRaph; // FIXME: No friends
  public:
    enum Mark {
      NoMark = 0,
      RightArrow, UpArrow, LeftArrow, DownArrow,
      EmptyCircle, FilledCircle,
    };

    enum Form { Straight, Bezier };

    NOde(int x, int y) noexcept;
    Mark mark() const noexcept;
    Form form() const noexcept;
    constexpr int numberOfEdges() const noexcept;
    void setMark(Mark mark) noexcept { mMark = mark; }
    void setForm(Form form) noexcept { mForm = form; }
    Edge& edge(int idx) noexcept;
    Point target(int idx) noexcept;

  private:
    Edge mEdges[8];
    int16_t mX;
    int16_t mY;
    uint8_t mMark;
    uint8_t mForm;
};



class GRaph
{
  public:
    const NOde& operator[](Point p) const;
    NOde& operator[](Point p);

    NOde& moveTo(int x, int y);
    NOde& lineTo(int dx, int dy, Edge::Style style);
    void line(int x, int y, int dx, int dy, Edge::Style style);
    void dump(const char* fname) const;

  private:
    using Nodes = std::vector<NOde>; // FIXME: Not optimal at all
    Nodes mNodes;
    Nodes::iterator mCur;
};
