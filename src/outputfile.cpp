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
#include "outputfile.h"
#include "runtimeerror.h"



OutputFile::OutputFile(const QString& name)
  : QFile{name},
    mDone{false}
{
  if (!open(WriteOnly|Truncate))
    throw RuntimeError{name, ": ", errorString()};
}



OutputFile::~OutputFile()
{
  if (!mDone)
  {
    close();
    remove();
  }
}



void OutputFile::done()
{
  close();
  mDone = true;
}
