#include "graph.h"
#include "outputfile.h"
#include "render.h"
#include "runtimeerror.h"
#include "textimg.h"
#include <iostream>
#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QImageWriter>
#include <QSvgGenerator>



struct CmdLineArgs
{
  QString inputFile;
  QString outputFile;
  QFont font;
  QColor bg{Qt::white};
  int lineWd{1};
  int shadows{0};
};



CmdLineArgs processCmdLine(const QCoreApplication& app)
{
  // TODO: Rework command line names and descriptions. More application description, in particular output formats.
  QCommandLineOption outputFileOpt{"o", "Set the name of the output file to write to.", "path"};
  QCommandLineOption fontOpt{"font", "Set the font for drawing.", "font"};
  QCommandLineOption fontSizeOpt{"font-size", "Set the font size for drawing.", "pixels"};
  QCommandLineOption bgOpt{{"bg", "background"}, "Set the background color. The following notations are understood: #RGB, #RRGGBB, #AARRGGBB, transparent, and common color names.", "color"};
  QCommandLineOption lineWdOpt{"line-width", "Set the width of lines.", "pixels"};
  QCommandLineOption shadowOpt{"shadows", "Enable shadows under closed shapes."};
  QCommandLineOption noShadowOpt{"no-shadows", "Disable shadows under closed shapes."};

  QCommandLineParser parser;
  parser.setApplicationDescription("This program converts ASCII art drawings into proper graphics files.");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addOption(outputFileOpt);
  parser.addOption(fontOpt);
  parser.addOption(fontSizeOpt);
  parser.addOption(bgOpt);
  parser.addOption(lineWdOpt);
  parser.addOption(shadowOpt);
  parser.addOption(noShadowOpt);
  parser.addPositionalArgument("input", "Input file");
  parser.process(app);

  auto inputs = parser.positionalArguments();
  if (inputs.isEmpty())
    throw std::runtime_error{"Missing input file"};

  if (inputs.size() > 1)
    throw std::runtime_error{"Too many input files"};

  CmdLineArgs result;
  result.inputFile  = inputs.first();
  result.outputFile = parser.value(outputFileOpt);

  if (result.outputFile.isEmpty())
    throw std::runtime_error{"Missing output file"};

  if (parser.isSet(fontOpt))
    result.font.setFamily(parser.value(fontOpt));

  try {
    if (parser.isSet(fontSizeOpt))
      result.font.setPixelSize(std::stoi(parser.value(fontSizeOpt).toUtf8().toStdString()));
  }
  catch (const std::exception&)
  { throw std::runtime_error{"Invalid font size"}; }

  if (parser.isSet(bgOpt))
  {
    result.bg.setNamedColor(parser.value(bgOpt));
    if (!result.bg.isValid())
      throw std::runtime_error{"Invalid background color"};
  }

  try {
    if (parser.isSet(lineWdOpt))
      result.lineWd = std::stoi(parser.value(lineWdOpt).toUtf8().toStdString());
  }
  catch (const std::exception&)
  { throw std::runtime_error{"Invalid line width"}; }

  if (parser.isSet(shadowOpt))
    result.shadows = 1;
  else if (parser.isSet(noShadowOpt))
    result.shadows = -1;

  return result;
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



// TODO: Fill closed shapes (what about empty ones?)
// TODO: Draw shadows under closed shapes
// TODO: Pandoc filter
// TODO: EPS output format
//
int main(int argc, char* argv[])
try
{
  QGuiApplication app{argc, argv};
  app.setApplicationName("draawsci");
  app.setApplicationVersion(VERSION);

  auto args  = processCmdLine(app);
  auto txt   = readTextImg(args.inputFile);
  auto graph = Graph::from(txt);

  Render render{graph, txt, args.lineWd};
  render.setFont(args.font);

  auto suffix = QFileInfo{args.outputFile}.suffix().toLatin1();
  if (suffix == "svg")
  {
    OutputFile fd{args.outputFile};
    QSvgGenerator svg;
    svg.setOutputDevice(&fd);
    svg.setViewBox(QRect{QPoint{0, 0}, render.size()});
    render.setShadows(args.shadows > 0);
    render.paint(&svg);
    fd.done();
  }
  else if (QImageWriter::supportedImageFormats().contains(suffix))
  {
    bool transparency = (args.bg.alpha() != 255);
    if (transparency && suffix != "png")
      throw std::runtime_error{"Must use PNG output format for transparent background"};

    QImage img{render.size(), (transparency ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32)};
    img.fill(args.bg);
    render.setShadows(args.shadows >= 0);
    render.paint(&img);

    OutputFile fd{args.outputFile};
    QImageWriter writer{&fd, suffix};
    if (!writer.write(img))
      throw RuntimeError{fd.fileName(), ": ", writer.errorString()};

    fd.done();
  }
  else
    throw RuntimeError{args.outputFile, ": Unknown graphics format"};

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
