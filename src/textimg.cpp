#include "textimg.h"



TextImg::TextImg() noexcept
  : mWidth{0}
{}



Char TextImg::operator()(int x, int y) const noexcept
{
  if (x < 0 || y < 0 || y >= height())
    return ' ';

  auto& line = mLines[y];
  if (x >= line.size())
    return ' ';

  return line[x];
}



void TextImg::read(QTextStream& in)
{
  assert(mLines.isEmpty());
  mLines.reserve(128);

  for (;;)
  {
    auto line = in.readLine();
    if (line.isNull())
      break;

    mWidth = qMax(mWidth, line.size());
    mLines.push_back(std::move(line));
  }
}
