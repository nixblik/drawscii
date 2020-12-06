#include "graph.h"
#include "textimage.h"



class GraphCreator
{
  public:
    GraphCreator(TextImage& text, GRaph& graph) noexcept;
    void pass1();

  private:
    void parseJunction(int x, int y, wchar_t junctionCh);
    void parseHLine(int x, int y);
    void parseVLine(int x, int y);
    void parseLlCorner(int x, int y, wchar_t cornerCh);
    void parseLrCorner(int x, int y, wchar_t cornerCh);
    void parseUlCorner(int x, int y, wchar_t cornerCh);
    void parseUrCorner(int x, int y, wchar_t cornerCh);

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
      switch (auto ch = mText(x, y))
      {
        case '+':
        case '*':
        case 'o': parseJunction(x, y, ch); break; // FIXME: o is special because o- and -o and o= could occur!

        case '-':
        case '=': parseHLine(x, y); break;

        case '|':
        case ':': parseVLine(x, y); break;

        case '<':
        case '>': parseHLine(x, y); break;

        case '^':
        case 'v': parseVLine(x, y); break;

        case '/': parseLrCorner(x, y, ch);
                  parseUlCorner(x, y, ch); break;

        case '\\': parseLlCorner(x, y, ch);
                   parseUrCorner(x, y, ch); break;
      }
    }
  }
}



void GraphCreator::parseJunction(int x, int y, wchar_t junctionCh)
{
  switch (mText(x+1, y))
  {
    case '-': mGraph.line(2*x, 2*y, +3, +0, Edge::Line); break;
    case '=': mGraph.line(2*x, 2*y, +3, +0, Edge::Double); break;
  }

  switch (mText(x, y+1))
  {
    case '|': mGraph.line(2*x, 2*y, +0, +3, Edge::Line); break;
    case ':': mGraph.line(2*x, 2*y, +0, +3, Edge::Dashed); break;
  }
}



void GraphCreator::parseHLine(int x, int y)
{
  switch (mText(x+1, y))
  {
    case '-': mGraph.line(2*x-1, 2*y, +4, +0, Edge::Line); break;
    case '=': mGraph.line(2*x-1, 2*y, +4, +0, Edge::Dashed); break;
    case '+': mGraph.line(2*x-1, 2*y, +3, +0, Edge::Line); break;
    case '>':
    case '<': mGraph.line(2*x-1, 2*y, +3, +0, Edge::Line); break;
  }
}



void GraphCreator::parseVLine(int x, int y)
{
  switch (mText(x, y+1))
  {
    case '|': mGraph.line(2*x, 2*y-1, +0, +4, Edge::Line); break;
    case ':': mGraph.line(2*x, 2*y-1, +0, +4, Edge::Dashed); break;
    case '+': mGraph.line(2*x, 2*y-1, +0, +3, Edge::Line); break;
    case '^':
    case 'v': mGraph.line(2*x, 2*y-1, +0, +3, Edge::Line); break;
  }
}



void GraphCreator::parseLlCorner(int x, int y, wchar_t cornerCh)
{
  if (mText(x-1,y) == '-' || mText(x-1,y) == '=') switch (mText(x,y+1))
  {
    case '|':
    case ':':
    case '/':
      mGraph.moveTo(2*x, 2*y+3);
      mGraph.relLine(+0, -3, Edge::Line).setForm(NOde::Bezier);
      mGraph.relLine(-3, +0, Edge::Line);
      break;
  }
}



void GraphCreator::parseLrCorner(int x, int y, wchar_t cornerCh)
{
  if (mText(x+1,y) == '-') switch (mText(x,y+1))
  {
    case '|':
    case ':':
    case '\\':
      mGraph.moveTo(2*x, 2*y+3);
      mGraph.relLine(+0, -3, Edge::Line).setForm(NOde::Bezier);
      mGraph.relLine(+3, +0, Edge::Line);
      break;
  }
}



void GraphCreator::parseUlCorner(int x, int y, wchar_t cornerCh)
{
  if (mText(x-1,y) == '-') switch (mText(x,y-1))
  {
    case '|':
    case ':':
      mGraph.moveTo(2*x, 2*y-3);
      mGraph.relLine(+0, +3, Edge::Line).setForm(NOde::Bezier);
      mGraph.relLine(-3, +0, Edge::Line);
      break;
  }
}



void GraphCreator::parseUrCorner(int x, int y, wchar_t cornerCh)
{
  if (mText(x+1,y) == '-' || mText(x+1,y) == '=') switch (mText(x,y-1))
  {
    case '|':
    case ':':
    case '/':
      mGraph.moveTo(2*x, 2*y-3);
      mGraph.relLine(+0, +3, Edge::Line).setForm(NOde::Bezier);
      mGraph.relLine(+3, +0, Edge::Line);
      break;
  }
}



GRaph createGraph(TextImage& text)
{
  GRaph graph;
  GraphCreator creator{text, graph};
  creator.pass1();

  return graph;
}
