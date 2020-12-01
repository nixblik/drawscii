/*  Copyright 2020 Uwe Salomon <post@uwesalomon.de>

    This file is part of Draawsci.

    Draawsci is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Draawsci is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Draawsci.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "textimg.h"



TextImg::TextImg() noexcept
  : mWidth{0}
{}



Char TextImg::operator()(int x, int y) const noexcept
{
  if (x < 0 || y < 0 || y >= height())
    return ' ';

  auto& line = mLines[y];
  if (x >= line.size())
    return ' ';

  return line[x];
}



void TextImg::read(QTextStream& in)
{
  assert(mLines.isEmpty());
  mLines.reserve(128);

  for (;;)
  {
    auto line = in.readLine();
    if (line.isNull())
      break;

    mWidth = qMax(mWidth, line.size());
    mLines.push_back(std::move(line));
  }
}
