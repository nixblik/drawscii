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
#include "common.h"
#include <QColor>
#include <QRect>
#include <QString>
#include <QVector>
class QFontMetrics;



class Paragraph
{
  public:
    using const_iterator = QVector<QString>::const_iterator;

    Paragraph(QString&& line, int x, int y);

    int numberOfLines() const noexcept
    { return mLines.size(); }

    const QString& operator[](int index) const
    { return mLines[index]; }

    const QRect& rect() const noexcept
    { return mRect; }

    int bottom() const noexcept
    { return mRect.bottom(); }

    const QPoint& innerPoint() const noexcept
    { return mFirstPt; }

    bool addLine(QString&& line, int x, int y);
    Qt::Alignment alignment() const noexcept;
    int pixelWidth(const QFontMetrics& fm) const noexcept;

    QColor color;

  private:
    QVector<QString> mLines;
    QRect mRect;
    QPoint mFirstPt;
    int mLastLineIndent;
};
