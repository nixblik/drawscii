#include "graph.h"
#include "textimage.h"



class GraphCreator
{
  public:
    GraphCreator(TextImage& text, GRaph& graph) noexcept;
    void pass1();

  private:
    bool isPartOfWord(int x, int y) const;
    void readJunction(int x, int y, NOde::Mark mark);
    void readHorzLine(int x, int y, Edge::Style style);
    void readHorzLine(int x, int y, NOde::Mark mark);
    void readVertLine(int x, int y, Edge::Style style);
    void readVertLine(int x, int y, NOde::Mark mark);
    void readCorner(int x, int y, int dx, int dy, NOde::Form form);

    TextImage& mText;
    GRaph& mGraph;
};



inline GraphCreator::GraphCreator(TextImage& text, GRaph& graph) noexcept
  : mText{text},
    mGraph{graph}
{}



void GraphCreator::pass1()
{
  for (int y = 0; y < mText.height(); ++y)
  {
    auto xe = static_cast<int>(mText[y].size());
    for (int x = 0; x < xe; ++x)
    {
      switch (mText(x, y))
      {
        case '+': readJunction(x, y, NOde::NoMark); break;
        case '*': readJunction(x, y, NOde::FilledCircle); break;
        case 'o': readJunction(x, y, NOde::EmptyCircle); break;

        case '-': readHorzLine(x, y, Edge::Solid); break;
        case '=': readHorzLine(x, y, Edge::Double); break;
        case '|': readVertLine(x, y, Edge::Solid); break;
        case ':': readVertLine(x, y, Edge::Dashed); break;

        case '<': readHorzLine(x, y, NOde::LeftArrow); break;
        case '>': readHorzLine(x, y, NOde::RightArrow); break;
        case '^': readVertLine(x, y, NOde::UpArrow); break;
        case 'v': readVertLine(x, y, NOde::DownArrow); break;

        case '/': readCorner(x, y, +1, +1, NOde::Straight);
                  readCorner(x, y, -1, -1, NOde::Straight);
                  break;

        case '\\': readCorner(x, y, -1, +1, NOde::Straight);
                   readCorner(x, y, +1, -1, NOde::Straight);
                   break;
      }
    }
  }
}



inline bool GraphCreator::isPartOfWord(int x, int y) const
{ return iswalpha(mText(x-1, y)) || iswalpha(mText(x+1, y)); }



void GraphCreator::readJunction(int x, int y, NOde::Mark mark)
{
  if (mark == NOde::EmptyCircle && isPartOfWord(x, y))
    return;

  switch (mText(x+1, y))
  {
    case '-':
      mGraph.moveTo(2*x, 2*y).setMark(mark);
      mGraph.relLine(+3, +0, Edge::Solid);
      break;

    case '=':
      mGraph.moveTo(2*x, 2*y).setMark(mark);
      mGraph.relLine(+3, +0, Edge::Double);
      break;
  }

  switch (mText(x, y+1))
  {
    case '|':
      mGraph.moveTo(2*x, 2*y).setMark(mark);
      mGraph.relLine(+0, +3, Edge::Solid);
      break;

    case ':':
      mGraph.moveTo(2*x, 2*y).setMark(mark);
      mGraph.relLine(+0, +3, Edge::Dashed);
      break;
  }
}



void GraphCreator::readHorzLine(int x, int y, Edge::Style style)
{
  switch (mText(x+1, y))
  {
    case '-':
      if (style != Edge::Solid) break;
      mGraph.moveTo(2*x-1, 2*y);
      mGraph.relLine(+4, +0, Edge::Solid);
      break;

    case '=':
      if (style != Edge::Double) break;
      mGraph.moveTo(2*x-1, 2*y);
      mGraph.relLine(+4, +0, Edge::Double);
      break;

    case '+':
      mGraph.moveTo(2*x-1, 2*y);
      mGraph.relLine(+3, +0, style);
      break;

    case '>':
      mGraph.moveTo(2*x-1, 2*y);
      mGraph.relLine(+3, +0, style).setMark(NOde::LeftArrow);
      break;

    case '<':
      mGraph.moveTo(2*x-1, 2*y);
      mGraph.relLine(+3, +0, style).setMark(NOde::LeftArrow);
      break;
  }
}



void GraphCreator::readHorzLine(int x, int y, NOde::Mark mark)
{
  switch (mText(x+1, y))
  {
    case '-':
      mGraph.moveTo(2*x, 2*y).setMark(mark);
      mGraph.relLine(+3, +0, Edge::Solid);
      break;

    case '=':
      mGraph.moveTo(2*x, 2*y).setMark(mark);
      mGraph.relLine(+3, +0, Edge::Double);
      break;
  }
}



void GraphCreator::readVertLine(int x, int y, Edge::Style style)
{
  switch (mText(x, y+1))
  {
    case '|':
      if (style != Edge::Solid) break;
      mGraph.moveTo(2*x, 2*y-1);
      mGraph.relLine(+0, +4, Edge::Solid);
      break;

    case ':':
      if (style != Edge::Dashed) break;
      mGraph.moveTo(2*x, 2*y-1);
      mGraph.relLine(+0, +4, Edge::Dashed);
      break;

    case '+':
      mGraph.moveTo(2*x, 2*y-1);
      mGraph.relLine(+0, +3, style);
      break;

    case '^':
      mGraph.moveTo(2*x, 2*y-1);
      mGraph.relLine(+0, +3, style).setMark(NOde::UpArrow);
      break;

    case 'v':
      if (isPartOfWord(x, y)) break;
      mGraph.moveTo(2*x, 2*y-1);
      mGraph.relLine(+0, +3, style).setMark(NOde::DownArrow);
      break;
  }
}



void GraphCreator::readVertLine(int x, int y, NOde::Mark mark)
{
  switch (mText(x+1, y))
  {
    case '|':
      mGraph.moveTo(2*x, 2*y).setMark(mark);
      mGraph.relLine(+0, +3, Edge::Solid);
      break;

    case ':':
      mGraph.moveTo(2*x, 2*y).setMark(mark);
      mGraph.relLine(+0, +3, Edge::Dashed);
      break;
  }
}



void GraphCreator::readCorner(int x, int y, int dx, int dy, NOde::Form form)
{
  auto hch = mText(x+dx, y);
  auto vch = mText(x, y+dy);

  if (!wcschr(L"|:+/", vch))
    return;

  int dx0 = 3;
  int dx1 = 3 - (form == NOde::Straight);
  Edge::Style style;

  switch (hch)
  {
    case '-': style = Edge::Solid; break;
    case '=': style = Edge::Double; break;
    case '+': style = Edge::Solid; dx0 = 2; --dx1; break;
    default:  return;
  }

  mGraph.moveTo(2*x + dx0*dx, 2*y);
  mGraph.relLine(-dx1*dx, +0, style).setForm(form);
  mGraph.relLine((dx1-dx0)*dx, +dy, style);

  int dy1 = 2;
  switch (vch)
  {
    case '|': style = Edge::Solid; break;
    case ':': style = Edge::Dashed; break;
    case '+': style = Edge::Solid; dy1 = 1; break;
    case '/': return; // dy1 = 0
  }

  mGraph.relLine(+0, +dy1*dy, style);
}



GRaph createGraph(TextImage& text)
{
  GRaph graph;
  GraphCreator creator{text, graph};
  creator.pass1();

  return graph;
}
