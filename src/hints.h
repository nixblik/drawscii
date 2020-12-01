#pragma once
#include "common.h"
#include <list>
#include <QColor>
class TextImg;
class Graph;



struct Hint
{
  Hint(int nx, int ny, const QColor& col) noexcept
    : x{nx},
      y{ny},
      color{col}
  {}

  int x;
  int y;
  QColor color;
};



class Hints : public std::list<Hint>
{
  public:
    static Hints from(const TextImg& txt, Graph& graph);
};
