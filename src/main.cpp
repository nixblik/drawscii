#include "render.h"
#include "graph.h"
#include "runtimeerror.h"
#include "textimg.h"
#include <iostream>
#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>



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



TextImg readTextImg(QString fname)
{
  QFile fd{fname};
  if (!fd.open(QFile::ReadOnly))
    throw RuntimeError{fname, ": ", fd.errorString()};

  QTextStream in{&fd};
  TextImg txt;
  txt.read(in);

  return txt;
}



int main(int argc, char* argv[])
try
{
  QGuiApplication app{argc, argv};
  app.setApplicationName("draawsci");
  app.setApplicationVersion(VERSION);
  processCmdLine(app);

  auto txt   = readTextImg(inputFile);
  auto graph = Graph::from(txt);

  Render render{graph, txt};
  render.setFont(QFont{"Open Sans"}); // FIXME font

  // FIXME: What if outputFile is empty?
  {
    QFile fd{outputFile};
    if (!fd.open(QFile::WriteOnly|QFile::Truncate))
      throw RuntimeError{outputFile, ": ", fd.errorString()};

    QFileInfo fi{outputFile};
    if (fi.suffix() == "svg")
    {
      // FIXME: paint svg using QSvgGenerator
    }
    else
    {
      QImage img{render.size(), QImage::Format_ARGB32_Premultiplied};
      render.paint(&img);

      if (!img.save(&fd))
        throw RuntimeError{outputFile, ": Failed to write image file"};
    }
  }

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
