#include "graph.h"
#include "textimg.h"



inline void Node::set(NodeKind kind, Direction edge)
{
  mKind   = kind;
  mEdges |= edge;
}



Graph Graph::from(const TextImg& txt)
{
  Graph gr{txt.size()};
  gr.readFrom(txt);
  return gr;
}



Graph::Graph(const QSize& sz)
  : mSize{sz}
{
  mNodes.resize(sz.width() * sz.height());
}



void Graph::readFrom(const TextImg& txt)
{
  pass1(txt);
  pass2(txt);
}



// First pass of graph construction from txt. It detects the basic drawing
// elements (corners, edges and arrows) if they include horizontal or vertical
// dash characters.
//
void Graph::pass1(const TextImg& txt)
{
  for (int y = 0; y < mSize.height(); ++y)
  {
    for (int x = 0; x < mSize.width(); ++x)
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
          if (txt(x,y+1).isOneOf("^vV")) makeEdge(x, y, Line, Down, Arrow);
          break;

        case '<':
        case '>':
          if (txt(x+1,y).isOneOf("-=")) makeEdge(x, y, Arrow, Right, Line);
          break;

        case '^':
        case 'v':
        case 'V':
          if (txt(x,y+1).isOneOf("|:")) makeEdge(x, y, Arrow, Down, Line);
          break;

        case '/':
          if (txt(x+1,y).isOneOf("-=+") && txt(x,y+1).isOneOf("|:+")) makeCorner(x, y, Round, Right, Line, Down, Line);
          if (txt(x-1,y).isOneOf("-=+") && txt(x,y-1).isOneOf("|:+")) makeCorner(x, y, Round, Left, Line, Up, Line);
          break;

        case '\\':
          if (txt(x-1,y).isOneOf("-=+") && txt(x,y+1).isOneOf("|:+")) makeCorner(x, y, Round, Left, Line, Down, Line);
          if (txt(x+1,y).isOneOf("-=+") && txt(x,y-1).isOneOf("|:+")) makeCorner(x, y, Round, Right, Line, Up, Line);
          break;
      }
    }
  }
}



// Second pass of graph construction from txt. Recursively makes adjacent
// corner characters lines if at least one of them is a line already.
//
void Graph::pass2(const TextImg& txt)
{
  for (int y = 0; y < mSize.height(); ++y)
    for (int x = 0; x < mSize.width(); ++x)
      if (txt(x,y) == '+' && node(x,y).kind() != Text)
        findMoreCorners(txt, x, y);
}



void Graph::findMoreCorners(const TextImg& txt, int x, int y)
{
  if (txt(x-1,y) == '+')
  {
    node(x,y).set(Line, Left);
    if (node(x-1,y).kind() == Text)
      findMoreCorners(txt, x-1, y);
  }

  if (txt(x+1,y) == '+')
  {
    node(x,y).set(Line, Right);
    if (node(x+1,y).kind() == Text)
      findMoreCorners(txt, x+1, y);
  }

  if (txt(x,y-1) == '+')
  {
    node(x,y).set(Line, Up);
    if (node(x,y-1).kind() == Text)
      findMoreCorners(txt, x, y-1);
  }

  if (txt(x,y+1) == '+')
  {
    node(x,y).set(Line, Down);
    if (node(x,y+1).kind() == Text)
      findMoreCorners(txt, x, y+1);
  }
}



void Graph::makeEdge(int x, int y, NodeKind k1, Direction dir2, NodeKind k2)
{
  node(x, y).set(k1, dir2);
  makeRevEdge(x, y, dir2, k2);
}



void Graph::makeCorner(int x, int y, NodeKind k1, Direction dir2, NodeKind k2, Direction dir3, NodeKind k3)
{
  node(x, y).set(k1, static_cast<Direction>(dir2|dir3));
  makeRevEdge(x, y, dir2, k2);
  makeRevEdge(x, y, dir3, k3);
}



void Graph::makeRevEdge(int x, int y, Direction dir2, NodeKind k2)
{
  switch (dir2)
  {
    case Left:  --x; dir2 = Right; break;
    case Right: ++x; dir2 = Left;  break;
    case Up:    --y; dir2 = Down;  break;
    case Down:  ++y; dir2 = Up;    break;
    default:    Q_UNREACHABLE();
  }

  node(x,y).set(k2, dir2);
}
