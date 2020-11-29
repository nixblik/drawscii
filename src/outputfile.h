#pragma once
#include "common.h"
#include <QFile>



class OutputFile : public QFile
{
  public:
    explicit OutputFile(const QString& name);
    ~OutputFile() override;
    void done();

  private:
    bool mDone;
};
