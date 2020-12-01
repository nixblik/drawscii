/*  Copyright 2020 Uwe Salomon <post@uwesalomon.de>

    This file is part of Draawsci.

    Draawsci is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Draawsci is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Draawsci.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "graph.h"
#include "hints.h"
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
#include <QTextCodec>



enum class Mode { Draawsci, Ditaa };


struct CmdLineArgs
{
  QString inputFile;
  QString outputFile;
  QTextCodec* codec{nullptr};
  QFont font;
  QColor bg{Qt::white};
  int lineWd{1};
  int shadows{0};
  int tabWidth{8};
  bool antialias{true};
};



int parseIntArg(const QString& value, const char* error)
try
{
  return std::stoi(value.toUtf8().toStdString());
}
catch (const std::exception&)
{ throw std::runtime_error{error}; }



QColor parseColorArg(const QString& value, const char* error)
{
  QColor result{value};
  if (!result.isValid())
    throw std::runtime_error{error};

  return result;
}



CmdLineArgs processCmdLine(const QCoreApplication& app, Mode mode)
{
  // Draawsci options (some of them will be modified for Ditaa compatibility mode)
  QCommandLineOption antialiasOpt{"no-antialias", "Disables anti-aliasing."};
  QCommandLineOption backgroundOpt{"background", "Sets the background color for the output image. The following notations are understood: #RGB, #RRGGBB, #AARRGGBB, transparent, and common color names.", "color"};
  QCommandLineOption encodingOpt{{"e", "encoding"}, "Sets the encoding of the input file.", "encoding"};
  QCommandLineOption fontOpt{"font", "Sets the font family for the output image.", "font"};
  QCommandLineOption fontSizeOpt{"font-size", "Sets the font size for the output image.", "pixels"};
  QCommandLineOption lineWdOpt{"line-width", "Sets the width of lines for the output image.", "pixels"};
  QCommandLineOption outputFileOpt{"o", "Sets the name of the output file to write to. The file type is determined from the file extension.", "path"};
  QCommandLineOption shadowOpt{"shadows", "Enables drawing drop shadows under closed shapes."};
  QCommandLineOption noShadowOpt{{"S", "no-shadows"}, "Disables drawing drop shadows under closed shapes."};
  QCommandLineOption tabsOpt{{"t", "--tabs"}, "Sets the tab width for the input file.", "spaces"};

  // Ditaa compatibility mode options
  QCommandLineOption debugOpt{{"d", "debug"}, "Does nothing. Provided for ditaa command-line compatibility."};
  QCommandLineOption noSepOpt{{"E", "no-separation"}, "Does nothing. Provided for ditaa command-line compatibility."};
  QCommandLineOption htmlOpt{{"h", "html"}, "Does nothing. Provided for ditaa command-line compatibility."};
  QCommandLineOption roundOpt{{"r", "round-corners"}, "Does nothing. Provided for ditaa command-line compatibility."};
  QCommandLineOption scaleOpt{{"s", "scale"}, "Does nothing. Provided for ditaa command-line compatibility.", "scale"};
  QCommandLineOption transpOpt{{"T", "transparent"}, "Causes the diagram to be rendered on a transparent background. Overrides --background."};
  QCommandLineOption fixedSlopeOpt{{"W", "fixed-slope"}, "Does nothing. Provided for ditaa command-line compatibility."};

  if (mode == Mode::Ditaa)
  {
    antialiasOpt  = QCommandLineOption{{"A", "no-antialias"}, "Disables anti-aliasing."};
    backgroundOpt = QCommandLineOption{{"b", "background"}, "Sets the background color for the output image. The following notations are understood: RRGGBB, AARRGGBB."};
    outputFileOpt = QCommandLineOption{{"o", "overwrite"}, "Enables overwriting the output file if it already exists."};
  }

  QCommandLineParser parser;
  parser.setApplicationDescription("This program converts ASCII art drawings into proper graphics files.");

  switch (mode)
  {
    case Mode::Draawsci:
      parser.addOption(antialiasOpt);
      parser.addOption(backgroundOpt);
      parser.addOption(encodingOpt);
      parser.addOption(fontOpt);
      parser.addOption(fontSizeOpt);
      parser.addHelpOption();
      parser.addOption(lineWdOpt);
      parser.addOption(outputFileOpt);
      parser.addOption(shadowOpt);
      parser.addOption(noShadowOpt);
      parser.addOption(tabsOpt);
      parser.addVersionOption();
      parser.addPositionalArgument("input", "Input file that contains the drawing.");
      parser.process(app);
      break;

    case Mode::Ditaa:
      parser.addOption(antialiasOpt);
      parser.addOption(backgroundOpt);
      parser.addOption(debugOpt);
      parser.addOption(noSepOpt);
      parser.addOption(encodingOpt);
      parser.addOption(htmlOpt);
      parser.addHelpOption();
      parser.addOption(outputFileOpt);
      parser.addOption(roundOpt);
      parser.addOption(noShadowOpt);
      parser.addOption(scaleOpt);
      parser.addOption(transpOpt);
      parser.addOption(tabsOpt);
      parser.addVersionOption();
      parser.addOption(fixedSlopeOpt);
      parser.addPositionalArgument("input", "Input file that contains the drawing.");
      parser.addPositionalArgument("output", "Output file for writing the result to. The file type is determined from the file extension.");
      parser.process(app);
      break;
  }

  auto posArgs = parser.positionalArguments();
  if (posArgs.isEmpty())
    throw std::runtime_error{"Missing input file"};

  CmdLineArgs result;
  switch (mode)
  {
    case Mode::Draawsci:
    {
      if (posArgs.size() > 1)
        throw std::runtime_error{"Too many input files"};

      if (parser.isSet(fontOpt))
        result.font.setFamily(parser.value(fontOpt));

      if (parser.isSet(fontSizeOpt))
        result.font.setPixelSize(parseIntArg(parser.value(fontSizeOpt), "Invalid font size"));

      if (parser.isSet(lineWdOpt))
        result.lineWd = parseIntArg(parser.value(lineWdOpt), "Invalid line width");

      if (parser.isSet(backgroundOpt))
        result.bg = parseColorArg(parser.value(backgroundOpt), "Invalid background color");

      result.antialias  = !parser.isSet(antialiasOpt);
      result.shadows    = parser.isSet(shadowOpt) - parser.isSet(noShadowOpt);
      result.inputFile  = posArgs[0];
      result.outputFile = parser.value(outputFileOpt);
      break;
    }

    case Mode::Ditaa:
    {
      if (posArgs.size() < 2)
        throw std::runtime_error{"Missing output file"};

      if (posArgs.size() > 2)
        throw std::runtime_error{"Too many file arguments"};

      if (parser.isSet(backgroundOpt))
        result.bg = parseColorArg("#" + parser.value(backgroundOpt), "Invalid background color");

      if (parser.isSet(transpOpt))
        result.bg = Qt::transparent;

      result.antialias  = !parser.isSet(antialiasOpt);
      result.shadows    = (parser.isSet(noShadowOpt) ? -1 : 1);
      result.inputFile  = posArgs[0];
      result.outputFile = posArgs[1];
      break;
    }
  }

  if (parser.isSet(encodingOpt))
    if (!(result.codec = QTextCodec::codecForName(parser.value(encodingOpt).toLatin1())))
      throw std::runtime_error{"Unknown encoding"};

  if (parser.isSet(tabsOpt))
    result.tabWidth = parseIntArg(parser.value(tabsOpt), "Invalid tab width");

  if (result.outputFile.isEmpty())
    throw std::runtime_error{"Missing output file"};

  return result;
}



TextImg readTextImg(QString fname, QTextCodec* codec, int tabWidth)
{
  QFile fd{fname};
  if (!fd.open(QFile::ReadOnly))
    throw RuntimeError{fname, ": ", fd.errorString()};

  QTextStream in{&fd};
  if (codec)
    in.setCodec(codec);

  TextImg txt;
  txt.read(in, tabWidth);

  return txt;
}



// TODO: Blur shadows under closed shapes
// TODO: Pandoc filter
// TODO: EPS output format
// TODO: ditaa compatibility mode
//
int main(int argc, char* argv[])
try
{
  QGuiApplication app{argc, argv};
  app.setApplicationName("draawsci");
  app.setApplicationVersion(VERSION);

  auto mode  = (QFileInfo{argv[0]}.fileName() == "ditaa" ? Mode::Ditaa : Mode::Draawsci);
  auto args  = processCmdLine(app, mode);
  auto txt   = readTextImg(args.inputFile, args.codec, args.tabWidth);
  auto graph = Graph::from(txt);
  auto hints = Hints::from(txt, graph);

  Render render{graph, txt, args.font, args.lineWd};
  render.apply(hints);

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
    render.setAntialias(args.antialias);
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
