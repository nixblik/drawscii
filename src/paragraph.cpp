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
#include "paragraph.h"
#include <QFontMetrics>



namespace {
inline int leadingSpaces(const QString& s)
{
  for (int i = 0; i < s.size(); ++i)
    if (!s[i].isSpace())
      return i;

  Q_UNREACHABLE(); // GCOV_EXCL_LINE
}                  // GCOV_EXCL_LINE
} // namespace



Paragraph::Paragraph(QString&& line, TextPos pos)
  : mRect{pos.x, pos.y, line.size(), 1},
    mFirstPt{pos},
    mLastLineIndent{0}
{
  mLines.append(std::move(line));
}



bool Paragraph::addLine(QString&& line, TextPos pos)
{
  assert(pos.y <= mRect.bottom() + 1);
  assert(!mLines.empty());

  int endx = pos.x + line.size();
  if (endx <= mRect.left() || pos.x > mRect.right() || pos.y <= mRect.bottom())
    return false;

  if (endx <= mRect.left() + mLastLineIndent || pos.x >= mRect.left() + mLines.back().length())
    return false;

  static QString spaces;
  spaces.resize(qAbs(pos.x - mRect.x()), ' ');
  mLastLineIndent = 0;

  if (pos.x > mRect.x())
  {
    line.prepend(spaces);
    mLastLineIndent = spaces.length();
  }
  else if (pos.x < mRect.x())
  {
    for (auto& l: mLines)
      l.prepend(spaces);
  }

  mLines.append(std::move(line));
  mRect = mRect.united(QRect{pos.x, pos.y, endx - pos.x, 1});

  return true;
}



Qt::Alignment Paragraph::alignment() const noexcept
{
  if (mLines.size() <= 1)
    return Qt::AlignHCenter;

  int left   = 0;
  int center = 0;
  int right  = 0;

  for (auto& line: mLines)
  {
    int spc = leadingSpaces(line);
    left   += (spc == 0);
    center += qAbs(mRect.width() - line.size() - spc) <= 1;
    right  += (line.size() == mRect.width());
  }

  if (right == mLines.size())
    return left == mLines.size() ? Qt::AlignHCenter : Qt::AlignRight;
  else if (left == mLines.size())
    return Qt::AlignLeft;
  else if (center == mLines.size())
    return Qt::AlignHCenter;
  else
    return Qt::Alignment{};
}



int Paragraph::pixelWidth(const QFontMetrics& fm) const noexcept
{
  int wd = 0;
  for (auto& line: mLines)
    wd = qMax(wd, fm.width(line));

  return wd;
}
