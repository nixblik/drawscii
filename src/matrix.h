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
