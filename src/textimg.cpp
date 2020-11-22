#include "textimg.h"



TextImg::TextImg() noexcept
  : mWidth{0}
{}



Char TextImg::operator()(int x, int y) const noexcept
{
  if (y < 0 || y >= height())
    return Char{};

  auto& line = mLines[y];
  if (x < 0 || x >= line.size())
    return Char{};

  return line[x];
}



void TextImg::read(QTextStream& in)
{
  for (;;)
  {
    auto line = in.readLine();
    if (line.isNull())
      break;

    mWidth = qMax(mWidth, line.size());
    mLines.append(std::move(line));
  }
}
