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
    void read(QTextStream& in);

  private:
    int mWidth;
    QVector<QString> mLines;
};
