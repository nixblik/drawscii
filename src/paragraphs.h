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
#include <experimental/string_view>
#include <vector>
class TextImage;



/// A text paragraph in a TextImage, which is a collection of consecutive text
/// rows that "overlap" each other in x direction. A paragraph has an
/// alignment() deduced from the arrangement of the lines in the input file,
/// and is output as one unit, preserving the alignment.
///
class Paragraph
{
  using wstring_view = std::experimental::wstring_view;

  public:
    /// Creates the paragraph with the first \a row at position \a x, \a y.
    Paragraph(wstring_view row, int x, int y);

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

    /// A position at the top of the paragraph that is definitely "inside" the
    /// paragraph's text, not only inside the bounding rectangle as {left(),
    /// top()} would be, for example.
    int topInnerX() const noexcept
    { return mX0; }

    /// The text row with index \a row.
    const wstring_view& operator[](int row) const noexcept
    {
      assert(row >= 0 && static_cast<size_t>(row) < mRows.size());
      return mRows[static_cast<size_t>(row)].str;
    }

    /// Indentation of the text \a row relative to left().
    int indent(int row) const noexcept
    {
      assert(row >= 0 && static_cast<size_t>(row) < mRows.size());
      return mX0 + mRows[static_cast<size_t>(row)].indent - mLeft;
    }

    /// Adds another \a row to the paragraph at the bottom if it overlaps with
    /// the row currently at the bottom() of the paragraph. The \a row is
    /// located at \a x, \a y. Returns true if the row was added.
    bool addRow(wstring_view row, int x, int y);

    /// Alignment of the whole paragraph as deduced from text input.
    Qt::Alignment alignment() const noexcept;

    mutable Color color;

  private:
    struct Row {
      Row(wstring_view s, int i) noexcept;
      int length() const noexcept;

      wstring_view str;
      int indent;
    };

    std::vector<Row> mRows;
    const int mTop;
    const int mX0;
    int mLeft;
    int mRight;
};



using ParagraphList = std::list<Paragraph>;

/// Extracts all Paragraphs from the \a text image.
ParagraphList findParagraphs(const TextImage& text);
