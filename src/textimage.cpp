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
#include "textimage.h"
#include <cerrno>
#include <istream>
#include <limits>
#include <system_error>



namespace  {
void replace_all(std::wstring& s, wchar_t old, const std::wstring& rep)
{
  size_t pos = 0;
  while ((pos = s.find(old, pos)) != std::wstring::npos)
  {
    s.replace(pos, 1, rep);
    pos += rep.size();
  }
}
} // namespace



TextImage TextImage::read(std::wistream& in, uint tabWidth)
{
  TextImage txt;
  txt.mLines.reserve(128);

  std::wstring spaces(tabWidth, L' ');
  std::wstring line;

  // FIXME: Before call to read, check in.is_open()
  while (std::getline(in, line))
  {
    replace_all(line, '\t', spaces);
    if (line.size() > std::numeric_limits<int>::max() - 2)
      throw std::runtime_error{"line too long"};

    txt.mLines.emplace_back(std::move(line));
  }

  if (in.bad())
    throw std::system_error{errno, std::system_category(), "failed to read file"};

  return txt;
}



inline TextImage::TextImage() noexcept
= default;



inline TextImage::Line::Line(std::wstring s)
  : str{std::move(s)}
{
  drawing.resize(str.size(), false);
}



wchar_t TextImage::operator()(int x, int y) const noexcept
{
  if (x < 0 || y < 0 || y >= height())
    return ' ';

  auto& line = mLines[static_cast<size_t>(y)].str;
  auto  xu   = static_cast<size_t>(x);

  if (xu >= line.size())
    return ' ';

  return line[xu];
}



bool TextImage::isDrawing(int x, int y) const noexcept
{
  assert(x >= 0 && y >= 0 && y < height());

  auto& line = mLines[static_cast<size_t>(y)].drawing;
  auto  xu   = static_cast<size_t>(x);

  assert(xu < line.size());
  return line[xu];
}



uint8_t& TextImage::drawing(int x, int y) noexcept
{
  assert(x >= 0 && y >= 0 && y < height());

  auto& line = mLines[static_cast<size_t>(y)].drawing;
  auto  xu   = static_cast<size_t>(x);

  assert(xu < line.size());
  return line[xu];
}



bool TextImage::isPartOfWord(int x, int y) const noexcept
{
  auto& txt = *this;
  return iswalpha(txt(x,y)) && (iswalpha(txt(x-1,y)) || iswalpha(txt(x+1,y)));
}
