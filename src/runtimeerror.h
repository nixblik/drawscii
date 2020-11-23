#include "common.h"
#include <stdexcept>
#include <initializer_list>
#include <QString>



class RuntimeError : public std::runtime_error
{
  public:
    RuntimeError(std::initializer_list<QString> strs)
      : std::runtime_error{format(strs)}
    {}

    ~RuntimeError() override;

  private:
    std::string format(const std::initializer_list<QString>& strs);
};
