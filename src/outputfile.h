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
#include <QFile>



/// Simple QFile wrapper for the output image file that removes the created
/// file if done() is not called - which happens upon errors.
///
class OutputFile : public QFile
{
  public:
    explicit OutputFile(const QString& name);
    ~OutputFile() override;

    /// Called upon successful generation of the output image file; it will no
    /// longer be removed upon destruction of this object.
    void done();

  private:
    bool mDone;
};
