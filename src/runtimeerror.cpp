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
#include "runtimeerror.h"



RuntimeError::~RuntimeError()
= default;



std::string RuntimeError::format(const std::initializer_list<QString>& strs)
{
  int totalSize = 0;
  for (const auto& s: strs)
    totalSize += s.size();

  QString result;
  result.reserve(totalSize);

  for (const auto& s: strs)
    result.append(s);

  return result.toStdString();
}
