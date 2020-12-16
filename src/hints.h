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
#include <forward_list>
class TextImage;



/// Represents a drawing hint for a Shape, that is, a small piece of text which
/// will not be rendered but changes how the shape is drawn.
///
struct Hint
{
  /// Creates a (fill) color hint for the shape surrounding the point at \a nx,
  /// \a ny.
  Hint(int nx, int ny, Color c) noexcept;

  const int x;
  const int y;
  Color color;
};



using Hints = std::forward_list<Hint>;

/// Extracts and returns all hints in the \a text image.
Hints findHints(TextImage& text);
