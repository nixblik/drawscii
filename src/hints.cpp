#include "hints.h"
#include "textimg.h"
#include "graph.h"
#include <QRegExp>



Hints Hints::from(const TextImg& txt, Graph& graph)
{
  static QRegExp colorRegex{R"(\bc(?:[0-9]{3}|[A-Z]{3})\b)"};

  Hints result;
  for (int y = 0; y < txt.height(); ++y)
  {
    auto& line = txt[y];
    for (int idx = 0; (idx = colorRegex.indexIn(line, idx)) != -1; idx += colorRegex.matchedLength())
    {
      QColor color;
      if (line[idx+1].isDigit())
      {
        color.setRed(line[idx+1].digitValue() * 255 / 9);
        color.setGreen(line[idx+2].digitValue() * 255 / 9);
        color.setBlue(line[idx+3].digitValue() * 255 / 9);
      }
      else
      {
        auto name = line.midRef(idx+1, 3);
        if      (name == "RED") color = QColor("#FF4136");
        else if (name == "GRE") color = QColor("#2ECC40");
        else if (name == "BLU") color = QColor("#0074d9");
        else if (name == "PNK") color = QColor("#BC35CF");//"#B10DC9");
        else if (name == "YEL") color = QColor("#FFDC00");
        else if (name == "BLK") color = QColor("#111111");
        else
          continue;
      }

      result.emplace_back(idx, y, color);
      graph.setEmpty(idx, y, colorRegex.matchedLength());
    }
  }

  return result;
}
