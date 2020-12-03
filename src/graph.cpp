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
#include "graph.h"
#include "textimg.h"



int angle(Direction d1, Direction d2) noexcept
{
  auto dir = opposite(d1);
  for (int angle = -180; angle < 180; dir = turnedLeft45(dir), angle += 45)
    if (dir == d2)
      return angle;

  Q_UNREACHABLE(); // GCOV_EXCL_LINE
}                  // GCOV_EXCL_LINE



inline Node& Node::set(NodeKind kind) noexcept
{ mKind = kind; return *this; }

inline Node& Node::addEdge(Direction dir) noexcept
{ mEdges |= dir; return *this; }

inline Node& Node::addEdges(Directions dir) noexcept
{ mEdges |= dir; return *this; }

inline void Node::setDashed() noexcept
{ mDashed = true; }



Graph Graph::from(const TextImg& txt)
{
  Graph gr{txt.width(), txt.height()};
  gr.readFrom(txt);
  return gr;
}



inline Graph::Graph(int width, int height)
  : Matrix{width, height}
{}


inline Node& Graph::node(int x, int y) noexcept
{ return operator()(x, y); }



inline void Graph::readFrom(const TextImg& txt)
{
  pass1(txt);
  pass2(txt);
  pass3(txt);
}



// First pass of graph construction from txt. It detects the basic drawing
// elements (corners, edges and arrows) if they include horizontal or vertical
// dash characters.
//
void Graph::pass1(const TextImg& txt)
{
  for (int y = 0; y < height(); ++y)
  {
    for (int x = 0; x < width(); ++x)
    {
      switch (txt(x,y).toLatin1())
      {
        case '+':
          if (txt(x+1,y).isOneOf("-=")) makeEdge(x, y, Line, Right, Line);
          if (txt(x,y+1).isOneOf("|:")) makeEdge(x, y, Line, Down, Line);
          break;

        case '-':
        case '=':
          if (txt(x+1,y).isOneOf("-=+")) makeEdge(x, y, Line, Right, Line);
          if (txt(x+1,y).isOneOf("><"))  makeEdge(x, y, Line, Right, Arrow);
          break;

        case '|':
        case ':':
          if (txt(x,y+1).isOneOf("|:+")) makeEdge(x, y, Line, Down, Line);
          if (txt(x,y+1).isOneOf("^vV") && !txt.isPartOfWord(x,y+1)) makeEdge(x, y, Line, Down, Arrow);
          break;

        case '<':
        case '>':
          if (txt(x+1,y).isOneOf("-=")) makeEdge(x, y, Arrow, Right, Line);
          break;

        case '^':
        case 'v':
        case 'V':
          if (txt(x,y+1).isOneOf("|:") && !txt.isPartOfWord(x,y)) makeEdge(x, y, Arrow, Down, Line);
          break;

        case '/':
          if (txt(x,y+1).isOneOf("|:+\\") && txt(x+1,y).isOneOf("-=+")) makeCorner(x, y, Round, Right, Line, Down, Line);
          if (txt(x,y-1).isOneOf("|:+")   && txt(x-1,y).isOneOf("-=+")) makeCorner(x, y, Round, Left, Line, Up, Line);
          if (txt(x,y-1) == '\\'          && txt(x-1,y).isOneOf("-=+")) makeCorner(x, y, Round, Left, Line, Up, Round);
          if (txt(x-1,y+1) == '/')                                      makeEdge(x, y, Line, DownLeft, Line);
          break;

        case '\\':
          if (txt(x,y+1).isOneOf("|:+/") && txt(x-1,y).isOneOf("-=+")) makeCorner(x, y, Round, Left, Line, Down, Line);
          if (txt(x,y-1).isOneOf("|:+")  && txt(x+1,y).isOneOf("-=+")) makeCorner(x, y, Round, Right, Line, Up, Line);
          if (txt(x,y-1) == '/'          && txt(x+1,y).isOneOf("-=+")) makeCorner(x, y, Round, Right, Line, Up, Round);
          if (txt(x+1,y+1) == '\\')                                    makeEdge(x, y, Line, DownRight, Line);
          break;
      }
    }
  }
}



inline void Graph::makeEdge(int x, int y, NodeKind from, Direction dir, NodeKind to)
{
  node(x, y).set(from).addEdge(dir);
  node(x + deltaX(dir), y + deltaY(dir)).set(to).addEdge(opposite(dir));
}



inline void Graph::makeCorner(int x, int y, NodeKind from, Direction dir1, NodeKind to1, Direction dir2, NodeKind to2)
{
  node(x, y).set(from).addEdges(dir1|dir2);
  node(x + deltaX(dir1), y + deltaY(dir1)).set(to1).addEdge(opposite(dir1));
  node(x + deltaX(dir2), y + deltaY(dir2)).set(to2).addEdge(opposite(dir2));
}



// Second pass of graph construction from txt. Recursively makes adjacent
// corner characters lines if at least one of them is a line already.
//
void Graph::pass2(const TextImg& txt)
{
  for (int y = 0; y < height(); ++y)
    for (int x = 0; x < width(); ++x)
      if (txt(x,y) == '+' && node(x,y).isLine())
        findMoreCorners(txt, x, y);
}



void Graph::findMoreCorners(const TextImg& txt, int x, int y)
{
  if (txt(x-1,y) == '+')
  {
    node(x,y).set(Line).addEdge(Left);
    if (node(x-1,y).kind() == Text)
      findMoreCorners(txt, x-1, y);
  }

  if (txt(x+1,y) == '+')
  {
    node(x,y).set(Line).addEdge(Right);
    if (node(x+1,y).kind() == Text)
      findMoreCorners(txt, x+1, y);
  }

  if (txt(x,y-1) == '+')
  {
    node(x,y).set(Line).addEdge(Up);
    if (node(x,y-1).kind() == Text)
      findMoreCorners(txt, x, y-1);
  }

  if (txt(x,y+1) == '+')
  {
    node(x,y).set(Line).addEdge(Down);
    if (node(x,y+1).kind() == Text)
      findMoreCorners(txt, x, y+1);
  }
}



// Third pass of graph construction from txt. Determines line dashing.
//
void Graph::pass3(const TextImg& txt)
{
  for (int y = 0; y < height(); ++y)
  {
    for (int x = 0; x < width(); ++x)
    {
      auto nd = node(x, y);
      if (!nd.isLine())
        continue;

      switch (txt(x,y).toLatin1())
      {
        case '=': if (!nd.isDashed()) spreadDashing(txt, x, y, Left); break;
        case ':': if (!nd.isDashed()) spreadDashing(txt, x, y, Up); break;
      }
    }
  }
}



void Graph::spreadDashing(const TextImg& txt, int x0, int y0, Direction dir0)
{
  auto& nd0 = node(x0, y0);
  nd0.setDashed();

  for (int i = 0; i < 2; ++i, dir0 = opposite(dir0))
  {
    Direction dir = dir0;
    if (!nd0.hasEdge(dir))
      continue;

    int x = x0;
    int y = y0;

    for (;;)
    {
      x += deltaX(dir);
      y += deltaY(dir);

      auto& nd = node(x, y);
      if (nd.isDashed())
        break;

      nd.setDashed();
      if (nd.kind() == Round)
        dir = walkCorner(dir, x, y, txt(x,y));
      else if (nd.kind() == Arrow)
        break;

      if (!nd.hasEdge(dir))
        break;
    }
  }
}



Direction Graph::walkCorner(Direction dir, int x, int y, QChar cornerCh) const noexcept
{
  Q_UNUSED(x); // Needed later when corners can lead to inclined lines
  Q_UNUSED(y);

  bool horzDir = bool{(Left|Right) & dir};
  bool vertDir = bool{(Up|Down) & dir};

  if ((horzDir && cornerCh == '/') || (vertDir && cornerCh == '\\'))
    return turnedLeft90(dir);

  if ((horzDir && cornerCh == '\\') || (vertDir && cornerCh == '/'))
    return turnedRight90(dir);

  Q_UNREACHABLE(); // GCOV_EXCL_LINE
}                  // GCOV_EXCL_LINE



void Graph::setEmpty(int x, int y, int len)
{
  assert(x >= 0 && x + len <= width() && y >= 0 && y < height());

  auto p = &node(x, y);
  for (auto pe = p + len; p != pe; ++p)
    p->set(Empty);
}
