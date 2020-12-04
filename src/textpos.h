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
#include "direction.h"



/// An x, y position in the input text file. The coordinates can give the
/// column and row of a character. On the contrary, QPoint is used in this
/// program to denote pixel positions in the output image.
///
struct TextPos
{
  constexpr TextPos(int nx, int ny) noexcept
    : x{nx},
      y{ny}
  {}

  TextPos& operator+=(Direction dir) noexcept
  {
    x += deltaX(dir);
    y += deltaY(dir);
    return *this;
  }

  int x;
  int y;
};



inline TextPos operator+(TextPos pt, Direction dir) noexcept
{ return pt += dir; }


inline bool operator==(const TextPos& p1, const TextPos& p2) noexcept
{ return p1.x == p2.x && p1.y == p2.y; }
