#pragma once
#include "common.h"
#include <type_traits>



template<typename T>
class Flags
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
