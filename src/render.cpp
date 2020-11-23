#include "render.h"
#include "graph.h"
#include "textimg.h"
#include <QImage>
#include <QPainter>



Render::Render(const Graph& graph, const TextImg& txt)
  : mTxt{txt},
    mGraph{graph},
    mPainter{nullptr}
{}



QImage Render::image()
{
  QFont font;
  QFontMetrics fm(font);
  sx = fm.width("w");
  sy = fm.height();
  dx = sx/2;
  dy = sy/2;

  QImage img{mGraph.width()*sx, mGraph.height()*sy, QImage::Format_ARGB32_Premultiplied};
  QPainter painter(&img);
  painter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing|QPainter::SmoothPixmapTransform);
  //painter.translate(0.5, 0.5);
  painter.setPen(QPen(Qt::black, 2));
  mPainter = &painter;

  draw();
  return img;
}



void Render::svg()
{}



void Render::draw()
{
  for (int y = 0; y < mGraph.height(); ++y)
  {
    for (int x = 0; x < mGraph.width(); ++x)
    {
      auto node = mGraph.node(x,y);
      switch (node.kind())
      {
        case Text: {
          // FIXME: Postprocess text and draw much differently
          QChar ch = mTxt(x,y);
          if (ch.isPrint())
            mPainter->drawText(QRect(x*sx, y*sy, sx, sy), Qt::AlignHCenter, QString(ch));
          break;
        }

        // FIXME: Try to convert all straight lines into a single draw, like,
        // up to the corner. Follow the curved lines. Might do a transformation
        // graph -> list<painterpath>, all the while keeping a bitmap where
        // nodes are checked off.
        case Line:  drawLine(node, x, y); break;
        case Round: drawRound(node, x, y); break;
        case Arrow:
          break;
      }
    }
  }
}



inline void Render::drawLine(Node node, int x, int y)
{
  auto lx = x*sx + dx;
  auto ly = y*sy + dy;

  if (node.hasEdge(Right) && mGraph.node(x+1,y).kind() != Round)
    drawLine(lx, ly, lx+sx, ly);

  if (node.hasEdge(Down) && mGraph.node(x,y+1).kind() != Round)
    drawLine(lx, ly, lx, ly+sy);
}



inline void Render::drawLine(int x1, int y1, int x2, int y2)
{
  if (mPainter)
    mPainter->drawLine(x1, y1, x2, y2);
}



inline void Render::drawRound(Node node, int x, int y)
{
  auto lx = x*sx + dx;
  auto ly = y*sy + dy;
  int  r  = (sx + sy) / 3;

  switch (mTxt(x,y).toLatin1())
  {
    case '/': {
      if (node.hasEdge(Left))
      {
        mPainter->drawLine(lx-r, ly, lx-sx, ly);
        mPainter->drawArc(lx-2*r, ly-2*r, 2*r, 2*r, 270*16, 90*16);
        mPainter->drawLine(lx, ly-r, lx, ly-sy);
      }

      if (node.hasEdge(Right))
      {
        mPainter->drawLine(lx+r, ly, lx+sx, ly);
        mPainter->drawArc(lx, ly, 2*r, 2*r, 90*16, 90*16);
        mPainter->drawLine(lx, ly+r, lx, ly+sy);
      }

      break;
    }

    case '\\': {
      if (node.hasEdge(Left))
      {
        QPainterPath pp;
        pp.moveTo(-sx, 0);
        pp.lineTo(-r, 0);
        pp.arcTo(-2*r, 0, 2*r, 2*r, 90, -90);
        pp.lineTo(0, sy);
        mPainter->drawPath(pp.translated(lx, ly));

//        mPainter->drawLine(lx-r, ly, lx-sx, ly);
//        mPainter->drawArc(lx-2*r, ly, 2*r, 2*r, 0*16, 90*16);
//        mPainter->drawLine(lx, ly+r, lx, ly+sy);
      }

      if (node.hasEdge(Right))
      {
        mPainter->drawLine(lx+r, ly, lx+sx, ly);
        mPainter->drawArc(lx, ly-2*r, 2*r, 2*r, 180*16, 90*16);
        mPainter->drawLine(lx, ly-r, lx, ly-sy);
      }

      break;
    }

    default: Q_UNREACHABLE();
  }
}
