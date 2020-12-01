/*  Copyright 2020 Uwe Salomon <post@uwesalomon.de>

    This file is part of Draawsci.

    Draawsci is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Draawsci is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Draawsci.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "blur.h"
#include <cmath>
#include <array>



constexpr int BoxesCt = 3;
using Boxes = std::array<uint,BoxesCt>;



// The whole code in this module was taken from
// <http://blog.ivank.net/fastest-gaussian-blur.html>
// and modified for even less readability.
//
namespace {

Boxes computeGaussBoxes(double sigma)
{
  Boxes boxes;

  int n  = boxes.size();
  int wl = static_cast<int>(floor(sqrt(12*sigma*sigma/n + 1)));
  if (wl % 2 == 0)
    --wl;

  int wu = wl+2;
  int m  = static_cast<int>(round((12*sigma*sigma - n*wl*wl - 4*n*wl - 3*n) / (-4*wl - 4)));

  for (int i = 0; i < n; ++i)
    boxes[static_cast<size_t>(i)] = static_cast<uint>(i < m ? wl : wu);

  return boxes;
}



void blurHorz(QImage& tgt, QImage& src, Boxes boxes)
{
  uint width  = static_cast<uint>(src.width());
  int  height = src.height();

  for (int y = 0; y < height; ++y)
  {
    auto s = src.scanLine(y);
    auto t = tgt.scanLine(y);

    for (auto r: boxes)
    {
      auto first = s[0];
      auto last  = s[width-1];
      uint acc   = first * (r + 1);

      for (uint x = 0; x < r; ++x)
        acc += s[x];

      auto tx  = t;
      auto sx  = s - r - 1;
      uint dia = 2*r + 1;

      for (; tx <= t + r;        ++tx, ++sx) *tx = static_cast<uchar>((acc += sx[dia] - first) / dia);
      for (; tx < t + width - r; ++tx, ++sx) *tx = static_cast<uchar>((acc += sx[dia] - *sx) / dia);
      for (; tx < t + width;     ++tx, ++sx) *tx = static_cast<uchar>((acc += last    - *sx) / dia);

      std::swap(s, t);
    }
  }
}



void blurVert(QImage& tgt, QImage& src, Boxes boxes)
{
  int  width  = src.width();
  uint height = static_cast<uint>(src.height());

  auto s0  = src.bits();
  auto t0  = tgt.bits();
  auto bpl = static_cast<uint>(src.bytesPerLine());
  assert(static_cast<int>(bpl) == tgt.bytesPerLine());

  for (int x = 0; x < width; ++x)
  {
    auto s = s0 + x;
    auto t = t0 + x;

    for (auto r: boxes)
    {
      auto first = s[0];
      auto last  = s[bpl*(height-1)];
      uint acc   = first * (r + 1);

      for (uint y = 0; y < r; ++y)
        acc += s[y*bpl];

      auto ty   = t;
      auto sy   = s - bpl*(r+1);
      uint dia  = 2*r + 1;
      uint diaB = bpl * dia;
      uint rB   = bpl * r;
      uint htB  = bpl * height;

      for (; ty <= t + rB;      ty+=bpl, sy+=bpl) *ty = static_cast<uchar>((acc += sy[diaB] - first) / dia);
      for (; ty < t + htB - rB; ty+=bpl, sy+=bpl) *ty = static_cast<uchar>((acc += sy[diaB] - *sy) / dia);
      for (; ty < t + htB;      ty+=bpl, sy+=bpl) *ty = static_cast<uchar>((acc += last     - *sy) / dia);

      std::swap(s, t);
    }
  }
}
} // namespace



void blurImage(QImage& img, int r)
{
  assert(img.format() == QImage::Format_Alpha8);

  auto boxes = computeGaussBoxes(r / 2.57);
  static_assert(boxes.size() % 2 == 1, "number of Gauss boxes must be odd"); // see below

  QImage tmp{img.size(), img.format()};
  blurHorz(tmp, img, boxes); // odd number of boxes -> result in tmp
  blurVert(img, tmp, boxes); // odd number of boxes -> result in img
}



QImage filledImage(QColor color, const QImage& alpha)
{
  assert(alpha.format() == QImage::Format_Alpha8);

  int  width = alpha.width();
  QRgb rgb   = color.rgb() & 0x00ffffffu;

  QImage result{alpha.size(), QImage::Format_ARGB32_Premultiplied};
  for (int y = 0; y < result.height(); ++y)
  {
    auto ry = reinterpret_cast<uint32_t*>(result.scanLine(y));
    auto ay = alpha.scanLine(y);

    for (int x = 0; x < width; ++x)
      ry[x] = qPremultiply(rgb | (uint32_t{ay[x]} << 24u));
  }

  return result;
}
