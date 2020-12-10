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
#include "paragraphs.h"
#include "textimage.h"
#include <cstdlib>



inline Paragraph::Row::Row(std::wstring s, int i) noexcept
  : str{std::move(s)},
    indent{i}
{}


inline int Paragraph::Row::length() const noexcept
{ return indent + static_cast<int>(str.size()); }



Paragraph::Paragraph(std::wstring line, int x, int y)
  : mTop{y},
    mX0{x},
    mLeft{x},
    mRight{x + static_cast<int>(line.size()) - 1}
{
  mRows.emplace_back(std::move(line), 0);
}



bool Paragraph::addRow(std::wstring&& row, int x, int y)
{
  assert(y <= bottom() + 1);
  assert(!mRows.empty());

  int endx = x + static_cast<int>(row.size());
  if (endx <= mLeft || x > mRight || y <= bottom())
    return false;

  if (endx <= mX0 + mRows.back().indent || x >= mX0 + mRows.back().length())
    return false;

  mRows.emplace_back(std::move(row), x - mX0);
  mLeft  = std::min(mLeft, x);
  mRight = std::max(mRight, endx - 1);

  return true;
}



Qt::Alignment Paragraph::alignment() const noexcept
{
  if (mRows.size() <= 1)
    return Qt::AlignHCenter; // TODO: Infer alignment from surrounding drawing

  int left    = 0;
  int center  = 0;
  int right   = 0;
  int ctrFuzz = width() / 15;

  for (auto& row: mRows)
  {
    int lmarg = mX0 + row.indent - mLeft;
    int rmarg = mRight - mX0 - row.length() + 1;

    left   += (lmarg == 0);
    right  += (rmarg == 0);
    center += abs(rmarg - lmarg) <= 1 + ctrFuzz;
  }

  if (right == height())
    return left == height() ? Qt::AlignHCenter : Qt::AlignRight;
  else if (left == height())
    return Qt::AlignLeft;
  else if (center == height())
    return Qt::AlignHCenter;
  else
    return Qt::Alignment{};
}



ParagraphList findParagraphs(const TextImage& text)
{
  ParagraphList actives;
  ParagraphList paragraphs;
  std::wstring row;
  size_t spaces = 0;
  int    lineX  = 0;

  for (int y = 0; y < text.height(); ++y)
  {
    auto xe = static_cast<int>(text[y].size());
    for (int x = 0; x < xe; ++x)
    {
      if (!text.isDrawing(x, y))
      {
        auto ch  = text(x, y);
        bool spc = iswspace(ch);
        spaces   = spc ? spaces + 1 : 0;

        if (row.empty())
          lineX = x;

        if (!spc || !row.empty())
          row.push_back(ch);

        if (spaces <= 1 && x + 1 < xe)
          continue;
      }

      if (row.empty())
        continue;

      // Try to add row to one of the paragraphs
      row.erase(row.size() - spaces);

      for (auto para = actives.begin(); para != actives.end(); )
      {
        if (y > para->bottom() + 1)
        {
          auto old = para++;
          paragraphs.splice(paragraphs.end(), actives, old);
        }
        else if (para->addRow(std::move(row), lineX, y))
          break;
        else
          ++para;
      }

      // If the row fits nowhere, make a new paragraph
      if (!row.empty())
        actives.emplace_back(std::move(row), lineX, y);
    }
  }

  paragraphs.splice(paragraphs.end(), actives);
  return paragraphs;
}
