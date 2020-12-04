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
#pragma once
#include "textpos.h"
#include <QColor>
#include <QRect>
#include <QString>
#include <QVector>
class QFontMetrics;



class Paragraph
{
  public:
    using const_iterator = QVector<QString>::const_iterator;

    Paragraph(QString&& line, TextPos pos);

    int numberOfLines() const noexcept
    { return mLines.size(); }

    const QString& operator[](int index) const
    { return mLines[index]; }

    int width() const noexcept
    { return mRect.width(); }

    int height() const noexcept
    { return mRect.height(); }

    int bottom() const noexcept
    { return mRect.bottom(); }

    TextPos topLeft() const noexcept
    { return TextPos{mRect.x(), mRect.y()}; }

    const TextPos& innerPoint() const noexcept
    { return mFirstPt; }

    bool addLine(QString&& line, TextPos pos);
    Qt::Alignment alignment() const noexcept;
    int pixelWidth(const QFontMetrics& fm) const noexcept;

    QColor color;

  private:
    QVector<QString> mLines;
    QRect mRect;
    TextPos mFirstPt;
    int mLastLineIndent;
};
