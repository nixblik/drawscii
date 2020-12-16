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
#include <QImage>



/// Convolutes \a img with a Gaussian blur of radius \a r.
void blurImage(QImage& img, int r);

/// An ARGB32 image where the alpha channel of each pixel is taken from \a
/// alpha, while red, green, blue are set to \a color.
QImage filledImage(QColor color, const QImage& alpha);
