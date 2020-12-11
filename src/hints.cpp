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
#include "hints.h"
#include "textimage.h"
#include <regex>



inline Hint::Hint(int nx, int ny, Color c) noexcept
  : x{nx},
    y{ny},
    color{c}
{}



Hints findHints(TextImage& text)
{
  static std::wregex colorRegex{LR"(\bc[0-9A-Z]{3}\b)"};

  Hints result;
  for (int y = 0; y < text.height(); ++y)
  {
    auto& line  = text[y];
    auto  match = std::wsregex_iterator{line.begin(), line.end(), colorRegex};
    auto  mend  = std::wsregex_iterator{};

    for (auto i = match; i != mend; ++i)
    {
      auto&  name = (*i)[0];
      QColor color;

      if      (name == L"cRED") color = QColor("#FF4136");
      else if (name == L"cGRE") color = QColor("#2ECC40");
      else if (name == L"cBLU") color = QColor("#0074d9");
      else if (name == L"cPNK") color = QColor("#BC35CF");
      else if (name == L"cYEL") color = QColor("#FFDC00");
      else if (name == L"cBLK") color = QColor("#111111");
      else
        color = QColor{QString::fromStdWString(name).replace(0, 1, '#')};

      if (!color.isValid())
        continue;

      result.emplace_front(i->position(), y, color);

      for (auto x = i->position(), endx = x + i->length(); x != endx; ++x)
        text.category(static_cast<int>(x), y) = Category::Other;
    }
  }

  return result;
}
