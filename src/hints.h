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
#include <list>
#include <QColor>
class TextImg;
class Graph;



struct Hint
{
  Hint(int nx, int ny, const QColor& col) noexcept
    : x{nx},
      y{ny},
      color{col}
  {}

  int x;
  int y;
  QColor color;
};



class Hints : public std::list<Hint>
{
  public:
    static Hints from(const TextImg& txt, Graph& graph);
};
