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
#include "main.h"



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


/// Whether \a ch is contained in \a haystack's string.
bool operator==(wchar_t ch, const OneOf& haystack) noexcept
{ return haystack.contains(ch); }



/// Helper object for graph construction from a text image.
class GraphConstructor
{
  public:
    GraphConstructor(TextImage& text, GRaph& graph) noexcept;
    void createSomeEdges();
    void createMoreEdges();

  private:
    void createCorner(int x, int y, int dx, int dy, NOde::Form form);
    void createLeapfrog(int x, int y, int dx);
    void createHorzLine(int x, int y, Edge::Style style);
    void createVertLine(int x, int y, Edge::Style style);
    void createDiagLine(int x, int y, int dx, Edge::Style style);
    void createHorzJunction(int x, int y, NOde::Mark mark);
    void createVertJunction(int x, int y, NOde::Mark mark);
    void createJunction(int x, int y, NOde::Mark mark);
    bool createJunctionTo(int x, int y, NOde::Mark mark, int dx, int dy, const OneOf& oneOfThem);

    TextImage& mText;
    GRaph& mGraph;
};



GRaph constructGraph(TextImage& text)
{
  GRaph graph;
  GraphConstructor creator{text, graph};

  creator.createSomeEdges();
  creator.createMoreEdges();

  return graph;
}



inline GraphConstructor::GraphConstructor(TextImage& text, GRaph& graph) noexcept
  : mText{text},
    mGraph{graph}
{}



/// First pass of graph construction: Finds complex corner structures, marks
/// their characters as "drawing", and creates the lines in the corner. These
/// kind of structures are recognized:
///
///                 |
///     .-    .-   -)-
///     |    /      |
///
/// This step is needed for the second pass where adjacent characters might
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
          createCorner(x, y, +1, +1, NOde::Straight);
          createCorner(x, y, -1, -1, NOde::Straight);
          break;

        case '\\':
          createCorner(x, y, +1, -1, NOde::Straight);
          createCorner(x, y, -1, +1, NOde::Straight);
          break;

        case '.':
          createCorner(x, y, +1, +1, NOde::Bezier);
          createCorner(x, y, -1, +1, NOde::Bezier);
          break;

        case '\'':
          createCorner(x, y, +1, -1, NOde::Bezier);
          createCorner(x, y, -1, -1, NOde::Bezier);
          break;

        case ',': createCorner(x, y, +1, +1, NOde::Bezier); break;
        case '`': createCorner(x, y, +1, -1, NOde::Bezier); break;

        case ')': createLeapfrog(x, y, +1); break;
        case '(': createLeapfrog(x, y, -1); break;
      }
    }
  }
}



void GraphConstructor::createCorner(int x, int y, int dx, int dy, NOde::Form form)
{
  if (mText(x+dx, y) == OneOf{L"-=+"})
  {
    if (mText(x, y+dy) == OneOf{L"|!:+"})
    {
      mText.drawing(x, y)    = true;
      mText.drawing(x+dx, y) = true;
      mText.drawing(x, y+dy) = true;

      mGraph.moveTo(2*x, 2*y+dy);
      if (form != NOde::Straight)
      {
        mGraph.lineTo(+0, -dy, Edge::Weak).setForm(form);
        mGraph.lineTo(+dx, +0, Edge::Weak);
      }
      else
        mGraph.lineTo(+dx, -dy, Edge::Weak);
    }

    if (form == NOde::Bezier && mText(x-dx,y+dy) == (dx*dy > 0 ? '/' : '\\'))
    {
      mText.drawing(x, y)       = true;
      mText.drawing(x+dx, y)    = true;
      mText.drawing(x-dx, y+dy) = true;

      mGraph.moveTo(2*x-dx, 2*y+dy);
      mGraph.lineTo(+dx, -dy, Edge::Weak).setForm(form);
      mGraph.lineTo(+dx,  +0, Edge::Weak);
    }
  }
}



void GraphConstructor::createLeapfrog(int x, int y, int dx)
{
  if (mText(x, y-1) == '|' && mText(x, y+1) == '|')
  {
    auto ch = mText(x-1, y);
    if (ch == OneOf{L"-="} && mText(x+1, y) == ch)
    {
      mText.drawing(x,   y-1) = true;
      mText.drawing(x-1, y)   = true;
      mText.drawing(x,   y)   = true;
      mText.drawing(x+1, y)   = true;
      mText.drawing(x,   y+1) = true;

      mGraph.moveTo(2*x, 2*y-1);
      mGraph.lineTo(+dx, +1, Edge::Solid).setForm(NOde::Bezier);
      mGraph.lineTo(-dx, +1, Edge::Solid);

      mGraph.moveTo(2*x-1, 2*y);
      mGraph.lineTo(+2, +0, ch == '=' ? Edge::Double : Edge::Solid);
    }
  }
}



/// Second pass of graph construction: Creates the remaining edges. This
/// detects all of the regular, straight lines and junctions:
///
///     ---   | ! :    / \    +-    +-   -+    -->   ^ |
///     ===   | ! :   /   \   |    /       \   <--   | v
///
/// Edges are created as soon as two adjacent characters to match. The
/// algorithm therefore searches only rightwards and downwards, thus avoiding
/// double detections. Line endings at junctions are recognized because the
/// first pass marked them as "drawing".
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

        case '|': createVertLine(x, y, Edge::Solid); break;
        case '!':
        case ':': createVertLine(x, y, Edge::Dashed); break;

        case '/': createDiagLine(x, y, -1, Edge::Solid); break;
        case '\\':createDiagLine(x, y, +1, Edge::Solid); break;

        case '<': createHorzJunction(x, y, NOde::LeftArrow); break;
        case '>': createHorzJunction(x, y, NOde::RightArrow); break;
        case '^': createVertJunction(x, y, NOde::UpArrow); break;
        case 'v':
        case 'V': createVertJunction(x, y, NOde::DownArrow); break;

        case '+': createJunction(x, y, NOde::NoMark); break;
        case '*': createJunction(x, y, NOde::FilledCircle); break;
        case 'o': createJunction(x, y, NOde::EmptyCircle); break;
      }
    }
  }
}



void GraphConstructor::createHorzLine(int x, int y, Edge::Style style)
{
  auto& drawing = mText.drawing(x, y);

  auto ch = mText(x+1, y);
  if (ch == OneOf{L"-=+*<>"} || (ch == 'o' && !mText.isPartOfWord(x+1, y)))
  {
    drawing = true;
    mText.drawing(x+1, y) = true;
  }

  if (drawing)
  {
    mGraph.moveTo(2*x-1, 2*y);
    mGraph.lineTo(+2, +0, style);
  }
}



void GraphConstructor::createVertLine(int x, int y, Edge::Style style)
{
  auto& drawing = mText.drawing(x, y);

  auto ch = mText(x, y+1);
  if (ch == OneOf{L"|!:+*^"} || (ch == OneOf{L"ovV"} && !mText.isPartOfWord(x, y+1)))
  {
    drawing = true;
    mText.drawing(x, y+1) = true;
  }

  if (drawing)
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
    mText.drawing(x+dx, y+1) = true;
  }

  if (draw)
  {
    mText.drawing(x, y) = true;
    mGraph.moveTo(2*x-dx, 2*y-1);
    mGraph.lineTo(+2*dx, +2, style);
  }
}



void GraphConstructor::createHorzJunction(int x, int y, NOde::Mark mark)
{
  auto& drawing = mText.drawing(x, y);
  if (drawing)
  {
    // This has been marked already, by a line character to the left
    mGraph.moveTo(2*x, 2*y).setMark(mark);
    mGraph.lineTo(-1, +0, Edge::Weak);
  }

  if (mText(x+1, y) == OneOf{L"-="})
  {
    drawing = true;
    mText.drawing(x+1, y) = true;

    mGraph.moveTo(2*x, 2*y).setMark(mark);
    mGraph.lineTo(+1, +0, Edge::Weak);
  }
}



void GraphConstructor::createVertJunction(int x, int y, NOde::Mark mark)
{
  if (mark == NOde::DownArrow && mText.isPartOfWord(x, y))
    return;

  auto& drawing = mText.drawing(x, y);
  if (drawing)
  {
    // This has been marked already, by a line character to the top
    mGraph.moveTo(2*x, 2*y).setMark(mark);
    mGraph.lineTo(+0, -1, Edge::Weak);
  }

  if (mText(x, y+1) == OneOf{L"|!:"})
  {
    drawing = true;
    mText.drawing(x, y+1) = true;

    mGraph.moveTo(2*x, 2*y).setMark(mark);
    mGraph.lineTo(+0, +1, Edge::Weak);
  }
}



void GraphConstructor::createJunction(int x, int y, NOde::Mark mark)
{
  if (mark == NOde::EmptyCircle && mText.isPartOfWord(x, y))
    return;

  auto& drawing = mText.drawing(x, y);
  if (drawing)
  {
    // This has been marked already, by a line character towards the upper left
    createJunctionTo(x, y, mark, -1, -1, OneOf{L"\\"});
    createJunctionTo(x, y, mark, +0, -1, OneOf{L"|!:+"});
    createJunctionTo(x, y, mark, +1, -1, OneOf{L"/"});
    createJunctionTo(x, y, mark, -1, +0, OneOf{L"-=+"});
  }

  drawing |= createJunctionTo(x, y, mark, +1, +0, OneOf{L"-=+"});
  drawing |= createJunctionTo(x, y, mark, -1, +1, OneOf{L"/"});
  drawing |= createJunctionTo(x, y, mark, +0, +1, OneOf{L"|!:+"});
  drawing |= createJunctionTo(x, y, mark, +1, +1, OneOf{L"\\"});
}



inline bool GraphConstructor::createJunctionTo(int x, int y, NOde::Mark mark, int dx, int dy, const OneOf& oneOfThem)
{
  if (mText(x+dx, y+dy) == oneOfThem)
  {
    mText.drawing(x+dx, y+dy) = true;
    mGraph.moveTo(2*x, 2*y).setMark(mark);
    mGraph.lineTo(+dx, +dy, Edge::Weak);
    return true;
  }

  return false;
}
