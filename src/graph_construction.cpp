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
#include "graph_construction.h"



/// \internal
/// Provides syntactic sugar for testing whether a constant string contains a
/// particular character.
///
class OneOf
{
  public:
    constexpr OneOf(const wchar_t* haystack) noexcept
      : mHaystack(haystack)
    {}

    bool contains(wchar_t ch) const noexcept
    { return wcschr(mHaystack, ch); }

  private:
    const wchar_t* mHaystack;
};


/// \internal
/// Whether \a ch is contained in \a haystack's string.
bool operator==(wchar_t ch, const OneOf& haystack) noexcept
{ return haystack.contains(ch); }



/// \internal
/// Helper object for graph construction from a text image.
///
class GraphConstructor
{
  public:
    GraphConstructor(TextImage& text, Graph& graph) noexcept;
    void createSomeEdges();
    void createMoreEdges();

  private:
    void createCorner(int x, int y, int dx, int dy, Node::Form form);
    void createLeapfrog(int x, int y, int dx);
    void createHorzLine(int x, int y, Edge::Style style);
    void createLowHorzLine(int x, int y, Edge::Style style);
    void createVertLine(int x, int y, Edge::Style style);
    void createDiagLine(int x, int y, int dx, Edge::Style style);
    void createHorzJunction(int x, int y, Node::Mark mark);
    void createVertJunction(int x, int y, Node::Mark mark);
    void createJunction(int x, int y, Node::Mark mark);
    bool createJunctionTo(int x, int y, Node::Mark mark, int dx, int dy, const OneOf& oneOfThem);

    TextImage& mText;
    Graph& mGraph;
};



Graph constructGraph(TextImage& text)
{
  Graph graph;
  GraphConstructor creator{text, graph};

  size_t chars = 0;
  for (int y = 0; y < text.height(); ++y)
    chars += text[y].size();

  graph.reserve(static_cast<uint>(chars / 2));
  creator.createSomeEdges();
  creator.createMoreEdges();

  return graph;
}



inline GraphConstructor::GraphConstructor(TextImage& text, Graph& graph) noexcept
  : mText{text},
    mGraph{graph}
{}



/// First pass of graph construction: Finds complex corner structures, marks
/// their characters as "drawing", and creates the lines in the corner. This
/// step is needed for the second pass where adjacent characters might
/// otherwise not be recognized as edges.
///
void GraphConstructor::createSomeEdges()
{
  for (int y = 0; y < mText.height(); ++y)
  {
    auto xe = static_cast<int>(mText[y].size());
    for (int x = 0; x < xe; ++x)
    {
      switch (mText(x, y))
      {
        case '/':
          createCorner(x, y, +1, +1, Node::Straight);
          createCorner(x, y, -1, -1, Node::Straight);
          break;

        case '\\':
          createCorner(x, y, +1, -1, Node::Straight);
          createCorner(x, y, -1, +1, Node::Straight);
          break;

        case '.':
          createCorner(x, y, +1, +1, Node::Curved);
          createCorner(x, y, -1, +1, Node::Curved);
          break;

        case '\'':
          createCorner(x, y, +1, -1, Node::Curved);
          createCorner(x, y, -1, -1, Node::Curved);
          break;

        case ',': createCorner(x, y, +1, +1, Node::Curved); break;
        case '`': createCorner(x, y, +1, -1, Node::Curved); break;

        case ')': createLeapfrog(x, y, +1); break;
        case '(': createLeapfrog(x, y, -1); break;
      }
    }
  }
}



void GraphConstructor::createCorner(int x, int y, int dx, int dy, Node::Form form)
{
  if (mText(x+dx, y) == OneOf{L"-=+"})
  {
    auto ch2 = mText(x, y+dy);
    if ((ch2 == OneOf{L"|!:+"}) ||
        (ch2 == (dx*dy < 0 ? '/' : '\\') && form == Node::Straight) ||
        (ch2 == (   dy < 0 ? '.' : '\'') && form == Node::Curved))
    {
      mText.category(x, y)    = Category::Drawing;
      mText.category(x+dx, y) = Category::Drawing;
      mText.category(x, y+dy) = Category::Drawing;

      mGraph.moveTo(2*x, 2*y+dy);
      switch (form)
      {
        case Node::Straight:
          mGraph.lineTo(+dx, -dy, Edge::Weak);
          break;

        case Node::Curved:
          mGraph.lineTo(+0, -dy, Edge::Weak).setForm(form);
          mGraph.lineTo(+dx, +0, Edge::Weak);
          break;
      }
    }
    else if (ch2 == (dx*dy < 0 ? '/' : '\\') && form == Node::Curved)
    {
      mText.category(x, y)    = Category::Drawing;
      mText.category(x+dx, y) = Category::Drawing;
      mText.category(x, y+dy) = Category::Drawing;

      mGraph.moveTo(2*x-dx, 2*y+dy);
      mGraph.lineTo(-dx, -dy, Edge::Weak).setForm(form);
      mGraph.lineTo(+3*dx, +0, Edge::Weak);
    }

    if (form == Node::Curved && mText(x-dx,y+dy) == (dx*dy > 0 ? '/' : '\\'))
    {
      mText.category(x, y)       = Category::Drawing;
      mText.category(x+dx, y)    = Category::Drawing;
      mText.category(x-dx, y+dy) = Category::Drawing;

      mGraph.moveTo(2*x-dx, 2*y+dy);
      mGraph.lineTo(+dx, -dy, Edge::Weak).setForm(form);
      mGraph.lineTo(+dx,  +0, Edge::Weak);
    }
  }
}



void GraphConstructor::createLeapfrog(int x, int y, int)
{
  if (mText(x, y-1) == '|' && mText(x, y+1) == '|')
  {
    auto ch = mText(x-1, y);
    if (ch == OneOf{L"-="} && mText(x+1, y) == ch)
    {
      mText.category(x,   y-1) = Category::Drawing;
      mText.category(x-1, y)   = Category::Drawing;
      mText.category(x,   y)   = Category::Drawing;
      mText.category(x+1, y)   = Category::Drawing;
      mText.category(x,   y+1) = Category::Drawing;

      mGraph.moveTo(2*x, 2*y-1);
      mGraph.lineTo(+0, +2, Edge::Invisible);

      mGraph.moveTo(2*x-1, 2*y);
      mGraph.lineTo(+1, +0, ch == '=' ? Edge::Double : Edge::Solid).setMark(Node::Leapfrog);
      mGraph.lineTo(+1, +0, Edge::Invisible);
    }
  }
}



/// Second pass of graph construction: Creates the remaining edges. This
/// detects all of the regular, straight lines and junctions. Edges are created
/// as soon as two adjacent characters to match. The algorithm therefore
/// searches only rightwards and downwards, avoiding double detections. Line
/// endings at junctions are recognized because the first pass marked them as
/// "drawing".
///
void GraphConstructor::createMoreEdges()
{
  for (int y = 0; y < mText.height(); ++y)
  {
    auto xe = static_cast<int>(mText[y].size());
    for (int x = 0; x < xe; ++x)
    {
      switch (mText(x, y))
      {
        case '-': createHorzLine(x, y, Edge::Solid); break;
        case '=': createHorzLine(x, y, Edge::Double); break;
        case '_': createLowHorzLine(x, y, Edge::Solid); break;

        case '|': createVertLine(x, y, Edge::Solid); break;
        case '!':
        case ':': createVertLine(x, y, Edge::Dashed); break;

        case '/': createDiagLine(x, y, -1, Edge::Solid); break;
        case '\\':createDiagLine(x, y, +1, Edge::Solid); break;

        case '<': createHorzJunction(x, y, Node::LeftArrow); break;
        case '>': createHorzJunction(x, y, Node::RightArrow); break;
        case '^': createVertJunction(x, y, Node::UpArrow); break;
        case 'v':
        case 'V': createVertJunction(x, y, Node::DownArrow); break;

        case '+': createJunction(x, y, Node::NoMark); break;
        case '*': createJunction(x, y, Node::FilledCircle); break;
        case 'o': createJunction(x, y, Node::EmptyCircle); break;
      }
    }
  }
}



void GraphConstructor::createHorzLine(int x, int y, Edge::Style style)
{
  auto& categoryXY = mText.category(x, y);
  int   length     = 2;
  bool  checkDash  = false;

  auto ch = mText(x+1, y);
  if (ch == OneOf{L"-="})
  {
    checkDash              = (ch == '-' && iswspace(mText(x+2, y)));
    categoryXY             = Category::Drawing;
    mText.category(x+1, y) = Category::Drawing;
  }
  else if (ch == OneOf{L"+*<>"} || (ch == 'o' && !mText.isPartOfWord(x+1, y)))
  {
    checkDash              = true;
    categoryXY             = Category::Drawing;
    mText.category(x+1, y) = Category::Drawing;
  }
  else if (style == Edge::Solid && iswspace(ch) && mText(x+2, y) == '-')
  {
    style                  = Edge::Dashed;
    length                 = 4;
    categoryXY             = Category::Drawing;
    mText.category(x+1, y) = Category::Drawing;
    mText.category(x+2, y) = Category::Drawing;
  }
  else
    checkDash = true;

  if (categoryXY != Category::Drawing)
    return;

  if (checkDash && style == Edge::Solid && mText(x-2, y) == '-' && iswspace(mText(x-1, y)))
    style = Edge::Dashed;

  mGraph.moveTo(2*x-1, 2*y);
  mGraph.lineTo(+length, +0, style);
}



void GraphConstructor::createLowHorzLine(int x, int y, Edge::Style style)
{
  auto& categoryXY = mText.category(x, y);

  auto ch = mText(x+1, y);
  if (ch == OneOf{L"_/"})
  {
    categoryXY             = Category::Drawing;
    mText.category(x+1, y) = Category::Drawing;
  }

  if (mText(x-1, y+1) == '/')
  {
    categoryXY               = Category::Drawing;
    mText.category(x-1, y+1) = Category::Drawing;
  }

  if (mText(x+1, y+1) == '\\')
  {
    categoryXY               = Category::Drawing;
    mText.category(x+1, y+1) = Category::Drawing;
  }

  if (categoryXY == Category::Drawing)
  {
    mGraph.moveTo(2*x-1, 2*y+1);
    mGraph.lineTo(+2, +0, style);
  }
}



void GraphConstructor::createVertLine(int x, int y, Edge::Style style)
{
  auto& categoryXY = mText.category(x, y);

  auto ch = mText(x, y+1);
  if (ch == OneOf{L"|!:+*^"} || (ch == OneOf{L"ovV"} && !mText.isPartOfWord(x, y+1)))
  {
    categoryXY             = Category::Drawing;
    mText.category(x, y+1) = Category::Drawing;
  }

  if (categoryXY == Category::Drawing)
  {
    mGraph.moveTo(2*x, 2*y-1);
    mGraph.lineTo(+0, +2, style);
  }
}



void GraphConstructor::createDiagLine(int x, int y, int dx, Edge::Style style)
{
  OneOf oneOfThem{dx < 0 ? L"/+*" : L"\\+*"};
  bool  draw = false;

  auto ch = mText(x-dx, y-1);
  if (ch == oneOfThem || (ch == 'o' && !mText.isPartOfWord(x-dx, y-1)))
    draw = true;

  ch = mText(x+dx, y+1);
  if (ch == oneOfThem || (ch == 'o' && !mText.isPartOfWord(x+dx, y+1)))
  {
    draw = true;
    mText.category(x+dx, y+1) = Category::Drawing;
  }

  if (mText(x-dx, y-1) == '_')
    draw = true;

  if (mText(x+dx, y) == '_')
  {
    draw = true;
    mText.category(x+dx, y) = Category::Drawing;
  }

  auto& categoryXY = mText.category(x, y);
  if (!draw && categoryXY == Category::Drawing)
  {
    // The character is part of a corner; but which part?
    if (mText(x-dx, y-1) == OneOf{L".,"} || mText(x, y-1) == OneOf{L".,"} ||
        mText(x+dx, y+1) == OneOf{L"'`"} || mText(x, y+1) == OneOf{L"'`"})
      draw = true;
  }

  if (draw)
  {
    categoryXY = Category::Drawing;
    mGraph.moveTo(2*x-dx, 2*y-1);
    mGraph.lineTo(+2*dx, +2, style);
  }
}



void GraphConstructor::createHorzJunction(int x, int y, Node::Mark mark)
{
  auto& categoryXY = mText.category(x, y);
  if (categoryXY == Category::Drawing)
  {
    // This has been marked already, by a line character to the left
    mGraph.moveTo(2*x, 2*y).setMark(mark);
    mGraph.lineTo(-1, +0, Edge::Weak);
  }

  if (mText(x+1, y) == OneOf{L"-="})
  {
    categoryXY             = Category::Drawing;
    mText.category(x+1, y) = Category::Drawing;

    mGraph.moveTo(2*x, 2*y).setMark(mark);
    mGraph.lineTo(+1, +0, Edge::Weak);
  }
}



void GraphConstructor::createVertJunction(int x, int y, Node::Mark mark)
{
  if (mark == Node::DownArrow && mText.isPartOfWord(x, y))
    return;

  auto& categoryXY = mText.category(x, y);
  if (categoryXY == Category::Drawing)
  {
    // This has been marked already, by a line character to the top
    mGraph.moveTo(2*x, 2*y).setMark(mark);
    mGraph.lineTo(+0, -1, Edge::Weak);
  }

  if (mText(x, y+1) == OneOf{L"|!:"})
  {
    categoryXY             = Category::Drawing;
    mText.category(x, y+1) = Category::Drawing;

    mGraph.moveTo(2*x, 2*y).setMark(mark);
    mGraph.lineTo(+0, +1, Edge::Weak);
  }
}



void GraphConstructor::createJunction(int x, int y, Node::Mark mark)
{
  if (mark == Node::EmptyCircle && mText.isPartOfWord(x, y))
    return;

  auto& categoryXY = mText.category(x, y);
  if (categoryXY == Category::Drawing)
  {
    // This has been marked already, by a line character towards the upper left
    createJunctionTo(x, y, mark, -1, -1, OneOf{L"\\"});
    createJunctionTo(x, y, mark, +0, -1, OneOf{L"|!:+"});
    createJunctionTo(x, y, mark, +1, -1, OneOf{L"/"});
    createJunctionTo(x, y, mark, -1, +0, OneOf{L"-=+"});
  }

  bool drawn = createJunctionTo(x, y, mark, +1, +0, OneOf{L"-=+"});
  drawn     |= createJunctionTo(x, y, mark, -1, +1, OneOf{L"/"});
  drawn     |= createJunctionTo(x, y, mark, +0, +1, OneOf{L"|!:+"});
  drawn     |= createJunctionTo(x, y, mark, +1, +1, OneOf{L"\\"});

  if (drawn)
    categoryXY = Category::Drawing;
}



inline bool GraphConstructor::createJunctionTo(int x, int y, Node::Mark mark, int dx, int dy, const OneOf& oneOfThem)
{
  if (mText(x+dx, y+dy) == oneOfThem)
  {
    mText.category(x+dx, y+dy) = Category::Drawing;
    mGraph.moveTo(2*x, 2*y).setMark(mark);
    mGraph.lineTo(+dx, +dy, Edge::Weak);
    return true;
  }

  return false;
}
