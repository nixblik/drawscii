#include "runtimeerror.h"



RuntimeError::~RuntimeError()
= default;



std::string RuntimeError::format(const std::initializer_list<QString>& strs)
{
  int totalSize = 0;
  for (const auto& s: strs)
    totalSize += s.size();

  QString result;
  result.reserve(totalSize);

  for (const auto& s: strs)
    result.append(s);

  return result.toStdString();
}
