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
#include <QtGlobal>



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
