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
#include "test_drawscii.h"
#include "tempfile.h"
#include <stdexcept>
#include <QFont>
#include <QImage>
#include <QProcess>
QTEST_MAIN(TestDrawscii)



void TestDrawscii::initTestCase()
{
  mTmpDir = QFINDTESTDATA("tmp");
  QVERIFY(!mTmpDir.isEmpty());

  QFont defaultFont{"Open Sans"};
  defaultFont.setPixelSize(12);
  QVERIFY(defaultFont.exactMatch());
}



void TestDrawscii::verifyImageOutput_data()
{
  QTest::addColumn<QString>("fbasename");
  QTest::addColumn<QString>("suffix");
  QTest::addColumn<QString>("args");

  QTest::newRow("arrow_vs_text.png")   << "arrow_vs_text"   << "png" << "";
  QTest::newRow("can.png")             << "can"             << "png" << "";
  QTest::newRow("color_codes.png")     << "color_codes"     << "png" << "";
  QTest::newRow("color_more.png")      << "color_more"      << "png" << "--no-shadows";
  QTest::newRow("corner.png")          << "corner"          << "png" << "";
  QTest::newRow("cornered_line.png")   << "cornered_line"   << "png" << "";
  QTest::newRow("cross.png")           << "cross"           << "png" << "--line-width 5";
  QTest::newRow("dashed_lines.svg")    << "dashed_lines"    << "svg" << "";
  QTest::newRow("diag_tree.bmp")       << "diag_tree"       << "bmp" << "";
  QTest::newRow("hell.png")            << "hell"            << "png" << "--font-size 20";
  QTest::newRow("hell.svg")            << "hell"            << "svg" << "--font-size 20";
  QTest::newRow("intertwined.png")     << "intertwined"     << "png" << "";
  QTest::newRow("leapfrog.png")        << "leapfrog"        << "png" << "";
  QTest::newRow("letters.png")         << "letters"         << "png" << "--background transparent";
  QTest::newRow("linked_shapes.png")   << "linked_shapes"   << "png" << "";
  QTest::newRow("parallelogram.png")   << "parallelogram"   << "png" << "--background darkgray";
  QTest::newRow("rect_intersect.svg")  << "rect_intersect"  << "svg" << "--shadows";
  QTest::newRow("rect_parallel.png")   << "rect_parallel"   << "png" << "";
  QTest::newRow("square_shade.png")    << "square_shade"    << "png" << "";
  QTest::newRow("text_align.png")      << "text_align"      << "png" << "";
  QTest::newRow("text_bg_color.png")   << "text_bg_color"   << "png" << "";
  QTest::newRow("text_separation.png") << "text_separation" << "png" << "";
  QTest::newRow("turnkey.png")         << "turnkey"         << "png" << "--no-antialias";
  QTest::newRow("underscore.png")      << "underscore"      << "png" << "";
  QTest::newRow("unicode.png")         << "unicode"         << "png" << "--encoding UTF-8";

  std::locale loc{""};
  if (loc.name().find("UTF-8") != std::string::npos)
    QTest::newRow("unicode.bmp")       << "unicode"         << "bmp" << "";
  else
    QWARN(("Current locale " + loc.name() + " does not use UTF-8 encoding").c_str());
}

void TestDrawscii::verifyImageOutput()
try
{
  QFETCH(QString, fbasename);
  QFETCH(QString, suffix);
  QFETCH(QString, args);

  auto input  = QFINDTESTDATA("input/" + fbasename + ".txt");
  auto output = mTmpDir + "/" + fbasename + "." + suffix;
  auto truth  = QFINDTESTDATA("output/" + fbasename + "." + suffix);

  QVERIFY(runDrawscii(args.split(' ', QString::SkipEmptyParts) << "-o" << output << input, 0));
  checkImagesEqual(output, truth);
}
catch (const std::exception& e)
{ QFAIL(e.what()); }



void TestDrawscii::errors()
{
  QVERIFY(runDrawscii({}, 1));
  QVERIFY(mStderr.contains("error:"));

  QVERIFY(runDrawscii({QFINDTESTDATA("input/can.txt")}, 1));
  QVERIFY(mStderr.contains("error:"));

  TempFile tmp{"png"};
  QVERIFY(runDrawscii({tmp.fileName(), QFINDTESTDATA("input/can.txt")}, 1));
  QVERIFY(mStderr.contains("error:"));

  QVERIFY(runDrawscii({"-o", tmp.fileName(), "does/not/exist.txt"}, 1));
  QVERIFY(mStderr.contains("error:"));
  QVERIFY(mStderr.contains("does/not/exist.txt"));
}



bool TestDrawscii::runDrawscii(const QStringList& args, int expectedExitCode)
{
  QProcess proc;
  proc.start(DRAWSCII_BINARY, args);

  if (!proc.waitForStarted())
    throw std::runtime_error{"failed to start drawscii: " + proc.errorString().toStdString()};

  if (!proc.waitForFinished(15000))
  {
    proc.terminate();
    if (!proc.waitForFinished(3000))
      proc.kill();

    throw std::runtime_error{"draawsci failed to terminate"};
  }

  if (proc.exitStatus() != QProcess::NormalExit)
    throw std::runtime_error{"draawsci terminated abnormally: " + proc.errorString().toStdString()};

  mStderr = proc.readAllStandardError().simplified();

  if (proc.exitCode() != expectedExitCode)
  {
    qDebug("ERROR: drawscii exit code mismatch (expected=%i, actual=%i)", expectedExitCode, proc.exitCode());
    qDebug("%s", qPrintable(mStderr));
    return false;
  }

  return true;
}



namespace {
inline unsigned sqrDiff(int a, int b)
{
  auto d = a - b;
  return static_cast<unsigned>(d * d);
}
} // namespace



void TestDrawscii::checkImagesEqual(const QString& output, const QString& truth)
{
  QImage outputImg;
  QImage truthImg;

  QVERIFY(outputImg.load(output));
  QVERIFY(truthImg.load(truth));
  QCOMPARE(outputImg.size(), truthImg.size());
  QCOMPARE(outputImg.format(), truthImg.format());

  auto bpl = outputImg.bytesPerLine();
  Q_ASSERT(bpl == truthImg.bytesPerLine());

  quint64 diff = 0;
  for (int y = 0; y < outputImg.height(); ++y)
  {
    auto f = outputImg.constScanLine(y);
    auto s = truthImg.constScanLine(y);

    for (auto fe = f + bpl; f != fe; ++f, ++s)
      diff += sqrDiff(*f, *s);
  }

  auto delta = static_cast<double>(diff) / (outputImg.width() * outputImg.height());
  if (delta > 4)
    QFAIL(qPrintable("images are not identical: TSS/N = " + QString::number(delta)));

  if (delta > 0.1)
    QWARN(qPrintable("images are not identical: TSS/N = " + QString::number(delta)));
}
