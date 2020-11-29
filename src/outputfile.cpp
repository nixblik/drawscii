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
