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



Paragraph::Paragraph(QString&& line, int x, int y)
  : mRect{x, y, line.size(), 1},
    mFirstPt{x, y},
    mLastLineIndent{0}
{
  mLines.append(std::move(line));
}



bool Paragraph::addLine(QString&& line, int x, int y)
{
  assert(y <= mRect.bottom() + 1);
  assert(!mLines.empty());

  int endx = x + line.size();
  if (endx <= mRect.left() || x > mRect.right() || y <= mRect.bottom())
    return false;

  if (endx <= mRect.left() + mLastLineIndent || x >= mRect.left() + mLines.back().length())
    return false;

  static QString spaces;
  spaces.resize(qAbs(x - mRect.x()), ' ');
  mLastLineIndent = 0;

  if (x > mRect.x())
  {
    line.prepend(spaces);
    mLastLineIndent = spaces.length();
  }
  else if (x < mRect.x())
  {
    for (auto& l: mLines)
      l.prepend(spaces);
  }

  mLines.append(std::move(line));
  mRect = mRect.united(QRect{x, y, endx - x, 1});

  return true;
}



Qt::Alignment Paragraph::alignment() const noexcept
{
  int left   = 0;
  int center = 0;
  int right  = 0;

  for (auto& line: mLines)
  {
    int spc = leadingSpaces(line);
    left   += (spc == 0);
    center += qAbs(mRect.width() - line.size() - spc) <= 1;
    right  += line.size() == mRect.width();
  }

  if (left > center)
    return right > left ? Qt::AlignRight : Qt::AlignLeft;
  else
    return right > center ? Qt::AlignRight : Qt::AlignHCenter;
}



int Paragraph::pixelWidth(const QFontMetrics& fm) const noexcept
{
  int wd = 0;
  for (auto& line: mLines)
    wd = qMax(wd, fm.width(line));

  return wd;
}
