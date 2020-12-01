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
#pragma once
#include "common.h"
#include <valarray>



template<typename T>
class Matrix
{
  public:
    Matrix(int width, int height)
      : mWidth{width},
        mHeight{height}
    {
      mData.resize(width * height);
    }

    int width() const noexcept
    { return mWidth; }

    int height() const noexcept
    { return mHeight; }

    const T& operator()(int x, int y) const noexcept
    {
      assert(x >= 0 && x < mWidth && y >= 0 && y < mHeight);
      return mData[static_cast<size_t>(y * mWidth + x)];
    }

    T& operator()(int x, int y) noexcept
    {
      assert(x >= 0 && x < mWidth && y >= 0 && y < mHeight);
      return mData[static_cast<size_t>(y * mWidth + x)];
    }

    void clear() noexcept
    { mData = T{}; }

  private:
    int mWidth;
    int mHeight;
    std::valarray<T> mData;
};
