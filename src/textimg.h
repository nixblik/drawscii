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
#include <cstring>
#include <QSize>
#include <QString>
#include <QTextStream>
#include <QVector>



class Char : public QChar
{
  public:
    constexpr Char(char ch) noexcept
      : QChar{ch}
    {}

    constexpr Char(QChar ch) noexcept
      : QChar{ch}
    {}

    bool isOneOf(const char* s) const noexcept
    {
      char l1 = toLatin1();
      return l1 && strchr(s, l1);
    }
};



class TextImg
{
  public:
    TextImg() noexcept;

    int width() const noexcept
    { return mWidth; }

    int height() const noexcept
    { return mLines.size(); }

    const QString& operator[](int y) const noexcept
    { return mLines[y]; }

    Char operator()(int x, int y) const noexcept;
    bool isPartOfWord(int x, int y) const noexcept;
    void read(QTextStream& in, int tabWidth = 8);

  private:
    int mWidth;
    QVector<QString> mLines;
};
