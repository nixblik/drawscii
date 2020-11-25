#include "graph.h"
#include "textimg.h"



int Direction::dx() const noexcept
{
  switch (mDir)
  {
    case Up:
    case Down:      return 0;
    case DownLeft:
    case Left:
    case UpLeft:    return -1;
    case UpRight:
    case Right:
    case DownRight: return +1;
  }

  Q_UNREACHABLE();
}



int Direction::dy() const noexcept
{
  switch (mDir)
  {
    case Right:
    case Left:      return 0;
    case UpLeft:
    case Up:
    case UpRight:   return -1;
    case DownRight:
    case Down:
    case DownLeft:  return +1;
  }

  Q_UNREACHABLE();
}



inline void Node::set(NodeKind kind, Directions edges)
{
  mKind   = kind;
  mEdges |= edges;
}



inline void Node::set(NodeKind kind, Direction edge)
{ set(kind, edge.flag()); }


inline void Node::setDashed()
{ mDashed = true; }



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
  pass3(txt);
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



// Second pass of graph construction from txt. Recursively makes adjacent
// corner characters lines if at least one of them is a line already.
//
void Graph::pass2(const TextImg& txt)
{
  for (int y = 0; y < mSize.height(); ++y)
    for (int x = 0; x < mSize.width(); ++x)
      if (txt(x,y) == '+' && node(x,y).isLine())
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



// Third pass of graph construction from txt. Determines line dashing.
//
void Graph::pass3(const TextImg& txt)
{
  for (int y = 0; y < mSize.height(); ++y)
  {
    for (int x = 0; x < mSize.width(); ++x)
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

  for (int i = 0; i < 2; ++i, dir0 = dir0.opposite())
  {
    Direction dir = dir0;
    if (!nd0.hasEdge(dir))
      continue;

    int x = x0;
    int y = y0;

    for (;;)
    {
      x += dir.dx();
      y += dir.dy();

      auto& nd = node(x, y);
      if (nd.isDashed())
        break;

      nd.setDashed();
      if (nd.kind() == Round)
        dir = walkRoundCorner(dir, x, y, txt(x,y));
      else if (nd.kind() == Arrow)
        break;

      if (!nd.hasEdge(dir))
        break;
    }
  }
}



Direction Graph::walkRoundCorner(Direction dir, int x, int y, QChar cornerCh) const noexcept
{
  Q_UNUSED(x); // Needed later when corners can lead to inclined lines
  Q_UNUSED(y);

  bool horzDir = (dir == Left || dir == Right);
  bool vertDir = (dir == Down || dir == Up);

  if ((horzDir && cornerCh == '/') || (vertDir && cornerCh == '\\'))
    return dir.turnedLeft().turnedLeft();

  if ((horzDir && cornerCh == '\\') || (vertDir && cornerCh == '/'))
    return dir.turnedRight().turnedRight();

  Q_UNREACHABLE();
}



void Graph::makeEdge(int x, int y, NodeKind k, Direction dir2, NodeKind k2)
{
  node(x, y).set(k, dir2);
  node(x + dir2.dx(), y + dir2.dy()).set(k2, dir2.opposite());
}



void Graph::makeCorner(int x, int y, NodeKind k1, Direction dir2, NodeKind k2, Direction dir3, NodeKind k3)
{
  node(x, y).set(k1, dir2|dir3);
  node(x + dir2.dx(), y + dir2.dy()).set(k2, dir2.opposite());
  node(x + dir3.dx(), y + dir3.dy()).set(k3, dir3.opposite());
}
