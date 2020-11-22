#include "textimg.h"
#include "graph.h"
#include <iostream>
#include <stdexcept>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QFile>



namespace {
QString inputFile;
QString outputFile;
} // namespace



void processCmdLine(const QCoreApplication& app)
{
  QCommandLineOption outputFileOpt{"o", "Set the name of the output file to write to.", "path"};

  QCommandLineParser parser;
  parser.setApplicationDescription("This program converts ASCII art drawings into proper graphics files.");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addOption(outputFileOpt);
  parser.addPositionalArgument("input", "Input file");
  parser.process(app);

  auto input = parser.positionalArguments();
  if (input.isEmpty())
    throw std::runtime_error{"Missing input file"};

  if (input.size() > 1)
    throw std::runtime_error{"Too many input files"};

  inputFile  = parser.positionalArguments().front();
  outputFile = parser.value(outputFileOpt);
}



void plot(const Graph& graph)
{
  for (int y = 0; y < graph.height(); ++y)
  {
    for (int x = 0; x < graph.width(); ++x)
    {
      auto node = graph.node(x,y);
      switch (node.kind())
      {
        case Text:  std::cout << " "; break;
        case Line:  std::cout << "·"; break;
        case Round: std::cout << "r"; break;
        case Arrow: std::cout << "a"; break;
      }

      std::cout << (node.hasEdge(Right) ? "-" : " ");
    }
    std::cout << '\n';

    for (int x = 0; x < graph.width(); ++x)
    {
      auto node = graph.node(x,y);
      std::cout << (node.hasEdge(Down) ? "|" : " ");
      std::cout << ' ';
    }
    std::cout << '\n';
  }
}



#include <QImage>
#include <QPainter>
void draw(const Graph& graph, const TextImg& txt)
{
  QFont fn;
  QFontMetrics fm(fn);
  int fac = fm.height();
  QImage img(graph.size() * fac, QImage::Format_ARGB32_Premultiplied);
  QPainter painter(&img);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);

  for (int y = 0; y < graph.height(); ++y)
  {
    for (int x = 0; x < graph.width(); ++x)
    {
      auto node = graph.node(x,y);
      if (node.kind() == Text)
      {
        QChar ch = txt(x,y);
        if (ch.isPrint())
          painter.drawText(QRect(x*fac-fac/2, y*fac-fac/2, fac, fac), Qt::AlignHCenter, QString(ch));
      }
      else
      {
        if (node.hasEdge(Right))
          painter.drawLine(x*fac, y*fac, (x+1)*fac, y*fac);

        if (node.hasEdge(Down))
          painter.drawLine(x*fac, y*fac, x*fac, (y+1)*fac);
      }
    }
  }

  if (!img.save(outputFile)) // FIXME: use QFile for better error messages
    throw std::runtime_error{qPrintable(outputFile + ": Failed to write file")};
}



int main(int argc, char* argv[])
try
{
  QGuiApplication app{argc, argv};
  app.setApplicationName("draawsci");
  app.setApplicationVersion(VERSION);

  processCmdLine(app);

  QFile fd{inputFile};
  if (!fd.open(QFile::ReadOnly))
    throw std::runtime_error(qPrintable(inputFile + ": " + fd.errorString()));

  QTextStream in{&fd};
  TextImg txt;
  txt.read(in);

  auto graph = Graph::from(txt);
  if (!outputFile.isEmpty())
    draw(graph, txt);
  else
    plot(graph);

  return 0;
}
catch (const std::runtime_error& e)
{
  std::clog << "error: " << e.what() << '\n';
  return 1;
}
catch (const std::exception& e)
{
  std::clog << "internal error: " << e.what() << '\n';
  return 1;
}
