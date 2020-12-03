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
#include <QImage>
#include <QProcess>
QTEST_MAIN(TestDrawscii)



void TestDrawscii::verifyImageOutput_data()
{
  QTest::addColumn<QString>("fbasename");
  QTest::addColumn<QString>("suffix");

  QTest::newRow("arrow_vs_text.png")   << "arrow_vs_text"   << "png";
  QTest::newRow("can.png")             << "can"             << "png";
  QTest::newRow("color_codes.png")     << "color_codes"     << "png";
  QTest::newRow("color_more.png")      << "color_more"      << "png";
  QTest::newRow("corner.png")          << "corner"          << "png";
  QTest::newRow("cornered_line.png")   << "cornered_line"   << "png";
  QTest::newRow("cross.png")           << "cross"           << "png";
  QTest::newRow("diag_tree.png")       << "diag_tree"       << "png";
  QTest::newRow("hell.png")            << "hell"            << "png";
  QTest::newRow("intertwined.png")     << "intertwined"     << "png";
  QTest::newRow("letters.png")         << "letters"         << "png";
  QTest::newRow("linked_shapes.png")   << "linked_shapes"   << "png";
  QTest::newRow("rect_intersect.png")  << "rect_intersect"  << "png";
  QTest::newRow("rect_parallel.png")   << "rect_parallel"   << "png";
  QTest::newRow("square_shade.png")    << "square_shade"    << "png";
  QTest::newRow("text_align.png")      << "text_align"      << "png";
  QTest::newRow("text_bg_color.png")   << "text_bg_color"   << "png";
  QTest::newRow("text_separation.png") << "text_separation" << "png";
  QTest::newRow("turnkey.png")         << "turnkey"         << "png";
}

void TestDrawscii::verifyImageOutput()
try
{
  QFETCH(QString, fbasename);
  QFETCH(QString, suffix);

  auto input = QFINDTESTDATA("input/" + fbasename + ".txt");
  auto truth = QFINDTESTDATA("output/" + fbasename + "." + suffix);

  TempFile output{suffix};
  QCOMPARE(runDrawscii({"-o", output.fileName(), input}), 0);
  checkImagesEqual(output.fileName(), truth);
}
catch (const std::exception& e)
{ QFAIL(e.what()); }



int TestDrawscii::runDrawscii(const QStringList& args)
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

  return proc.exitCode();
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
  QVERIFY(delta < 4);

  if (delta > 0.1)
    QWARN(qPrintable("images are not identical: TSS/N = " + QString::number(delta)));
}
