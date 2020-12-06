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
#include <cwchar>
#include <cwctype>
#include <iosfwd>
#include <string>
#include <vector>



/// Provides an image-like interface to a text, where characters can be
/// accessed by row and column. Access out of the image bounds is allowed in
/// order to ease scanning the text for patterns. For the same reason, indices
/// are signed integers.
///
class TextImage
{
  public:
    /// Reads the image from \a in line by line, replacing all tabs with \a
    /// tabWidth spaces.
    static TextImage read(std::wistream& in, uint tabWidth = 8);

    TextImage(TextImage&&) noexcept
    = default;

    TextImage& operator=(TextImage&&) noexcept
    = default;

    /// The number of rows.
    int height() const noexcept
    { return static_cast<int>(mLines.size()); }

    /// The line in row \a y.
    const std::wstring& operator[](int y) const noexcept
    {
      assert(y >= 0 && y < height());
      return mLines[static_cast<size_t>(y)];
    }

    /// The character in column \a x, row \a y. If the position is out of
    /// bounds, a space character is returned.
    wchar_t operator()(int x, int y) const noexcept;

    /// Whether the character in column \a x, row \a y has an adjacent letter
    /// at its left or right. The position can be out of bounds.
    bool isPartOfWord(int x, int y) const noexcept;

  private:
    TextImage() noexcept;

    std::vector<std::wstring> mLines;
};
