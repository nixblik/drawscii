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
#include "flags.h"
#include "matrix.h"
#include <QChar>
#include <QSize>
class TextImg;



enum NodeKind : quint8
{ Empty, Text, Line, Round, Arrow };



enum Direction : quint8
{
  Up         = 0x01,
  UpRight    = 0x02,
  Right      = 0x04,
  DownRight  = 0x08,
  Down       = 0x10,
  DownLeft   = 0x20,
  Left       = 0x40,
  UpLeft     = 0x80,
};

using Directions = Flags<Direction>;


constexpr Directions operator|(Direction a, Direction b) noexcept
{ return Directions{a} | Directions{b}; }

constexpr Direction opposite(Direction dir) noexcept
{ return static_cast<Direction>(dir <= DownRight ? dir << 4 : dir >> 4); }

constexpr Direction turnedRight45(Direction dir) noexcept
{ return static_cast<Direction>(dir == UpLeft ? Up : dir << 1); }

constexpr Direction turnedRight90(Direction dir) noexcept
{ return static_cast<Direction>(dir >= Left ? dir >> 6 : dir << 2); }

constexpr Direction turnedLeft45(Direction dir) noexcept
{ return static_cast<Direction>(dir == Up ? UpLeft : dir >> 1); }

constexpr Direction turnedLeft90(Direction dir) noexcept
{ return static_cast<Direction>(dir <= UpRight ? dir << 6 : dir >> 2); }

constexpr int deltaX(Direction dir) noexcept
{ return bool{(UpRight|Right|DownRight) & dir} - bool{(UpLeft|Left|DownLeft) & dir}; }

constexpr int deltaY(Direction dir) noexcept
{ return bool{(DownLeft|Down|DownRight) & dir} - bool{(UpLeft|Up|UpRight) & dir}; }

int angle(Direction d1, Direction d2) noexcept;



class Node
{
  public:
    constexpr Node() noexcept
      : mKind{Text},
        mDashed{false},
        mEdges{}
    {}

    NodeKind kind() const noexcept
    { return mKind; }

    bool isLine() const noexcept
    { return mKind != Text; }

    bool hasEdge(Direction dir) const noexcept
    { return bool{mEdges & dir}; }

    Directions edges() const noexcept
    { return mEdges; }

    bool isDashed() const noexcept
    { return mDashed; }

    Node& set(NodeKind kind) noexcept;
    Node& addEdge(Direction dir) noexcept;
    Node& addEdges(Directions dir) noexcept;
    void setDashed() noexcept;

  private:
    NodeKind mKind;
    bool mDashed;
    Directions mEdges;
};



class Graph : public Matrix<Node>
{
  public:
    static Graph from(const TextImg& txt);

    Direction walkCorner(Direction dir, int x, int y, QChar cornerCh) const noexcept;
    void setEmpty(int x, int y, int len);

  private:
    Graph(int width, int height);
    void readFrom(const TextImg& txt);
    void pass1(const TextImg& txt);
    void pass2(const TextImg& txt);
    void pass3(const TextImg& txt);
    void findMoreCorners(const TextImg& txt, int x, int y);
    void spreadDashing(const TextImg& txt, int x0, int y0, Direction dir0);
    void makeEdge(int x, int y, NodeKind from, Direction dir, NodeKind to);
    void makeCorner(int x, int y, NodeKind from, Direction dir1, NodeKind to1, Direction dir2, NodeKind to2);
    void makeRevEdge(int x, int y, Directions dir, NodeKind k2);
    Node& node(int x, int y) noexcept;
};
