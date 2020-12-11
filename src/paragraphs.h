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
#include "color.h"
#include <list>
#include <string>
#include <vector>
class TextImage;



class Paragraph
{
  public:
    Paragraph(std::wstring row, int x, int y);

    int top() const noexcept
    { return mTop; }

    int left() const noexcept
    { return mLeft; }

    int width() const noexcept
    { return mRight - mLeft + 1; }

    int height() const noexcept
    { return static_cast<int>(mRows.size()); }

    int bottom() const noexcept
    { return mTop + static_cast<int>(mRows.size()) - 1; }

    int topInnerX() const noexcept
    { return mX0; }

    const std::wstring& operator[](int row) const noexcept
    {
      assert(row >= 0 && static_cast<size_t>(row) < mRows.size());
      return mRows[static_cast<size_t>(row)].str;
    }

    int indent(int row) const noexcept
    {
      assert(row >= 0 && static_cast<size_t>(row) < mRows.size());
      return mX0 + mRows[static_cast<size_t>(row)].indent - mLeft;
    }

    bool addRow(std::wstring&& row, int x, int y);
    Qt::Alignment alignment() const noexcept;
    mutable Color color;

  private:
    struct Row {
      Row(std::wstring s, int i) noexcept;
      int length() const noexcept;

      std::wstring str; // FIXME: Don't copy. Use stringrefs into TextImage
      int indent;
    };

    std::vector<Row> mRows;
    const int mTop;
    const int mX0;
    int mLeft;
    int mRight;
};



using ParagraphList = std::list<Paragraph>;
ParagraphList findParagraphs(const TextImage& text);
