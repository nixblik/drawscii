#pragma once
#include "graph.h"
#include <QImage>
class TextImg;



class Render
{
  public:
    Render(const Graph& graph, const TextImg& txt);

    QImage image();
    void svg();

  private:
    void draw();
    void drawLine(Node node, int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawRound(Node node, int x, int y);

    const TextImg& mTxt;
    const Graph& mGraph;
    QPainter* mPainter;

    int sx;
    int sy;
    int dx;
    int dy;
};



