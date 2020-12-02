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
#include "hints.h"
#include "textimg.h"
#include "graph.h"
#include <QRegExp>



Hints Hints::from(const TextImg& txt, Graph& graph)
{
  static QRegExp colorRegex{R"(\bc[0-9A-Z]{3}\b)"};

  Hints result;
  for (int y = 0; y < txt.height(); ++y)
  {
    auto& line = txt[y];
    for (int idx = 0; (idx = colorRegex.indexIn(line, idx)) != -1; idx += colorRegex.matchedLength())
    {
      QColor color;
      auto name = line.midRef(idx+1, 3);

      if      (name == "RED") color = QColor("#FF4136");
      else if (name == "GRE") color = QColor("#2ECC40");
      else if (name == "BLU") color = QColor("#0074d9");
      else if (name == "PNK") color = QColor("#BC35CF");
      else if (name == "YEL") color = QColor("#FFDC00");
      else if (name == "BLK") color = QColor("#111111");
      else
        color = QColor("#" + name);

      if (!color.isValid())
        continue;

      result.emplace_back(idx, y, color);
      graph.setEmpty(idx, y, colorRegex.matchedLength());
    }
  }

  return result;
}
