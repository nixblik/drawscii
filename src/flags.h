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
#include <type_traits>



template<typename T>
class Flags // FIXME: See whether still needed
{
  static_assert(std::is_enum<T>::value, "Flags template parameter must be an enum");
  using Base = typename std::underlying_type<T>::type;

  public:
    constexpr Flags() noexcept
      : mValue{}
    {}

    constexpr Flags(T other) noexcept
      : mValue{static_cast<Base>(other)}
    {}

    constexpr explicit operator bool() const noexcept
    { return mValue; }

    constexpr bool operator!() const noexcept
    { return !mValue; }

    constexpr Flags operator~() const noexcept
    { return Flags{static_cast<Base>(~mValue)}; }

    Flags& operator=(T other) noexcept
    { mValue = static_cast<Base>(other); return *this; }

    Flags& operator&=(T other) noexcept
    { mValue &= static_cast<Base>(other); return *this; }

    Flags& operator|=(T other) noexcept
    { mValue |= static_cast<Base>(other); return *this; }

    Flags& operator|=(Flags other) noexcept
    { mValue |= other.mValue; return *this; }

    constexpr Flags operator&(Flags other) const noexcept
    { return Flags{static_cast<Base>(mValue & other.mValue)}; }

    constexpr Flags operator|(Flags other) const noexcept
    { return Flags{static_cast<Base>(mValue | other.mValue)}; }

  private:
    constexpr explicit Flags(Base value) noexcept
      : mValue{value}
    {}

    Base mValue;
};
