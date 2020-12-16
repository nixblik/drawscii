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
#include <cctype>
#include <cerrno>
#include <experimental/filesystem>
#include <experimental/string_view>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unistd.h>
namespace fs = std::experimental::filesystem;
using std::experimental::string_view;



string_view truncated(string_view s) noexcept
{
  while (!s.empty() && isspace(s.front()))
    s.remove_prefix(1);

  while (!s.empty() && isspace(s.back()))
    s.remove_suffix(1);

  return s;
}



void convert(const std::string& txtFile)
{
  fs::path outPath{txtFile};
  outPath.replace_extension(".png");

  auto outFile = outPath.string();
  auto command = DRAWSCII_BINARY " \"" + txtFile + "\" -o \"" + outFile + "\"";
  auto retcode = system(command.c_str());

  if (retcode == 0)
    std::cout << "generated " << outFile << '\n';
  else
    throw std::runtime_error{"failed to generate " + outFile};
}



void extract(std::istream& in, std::string outFile)
{
  std::ofstream out{outFile};
  if (!out.is_open())
    throw std::system_error{errno, std::system_category(), outFile};

  std::string line;
  while (std::getline(in, line))
  {
    if (line.compare(0, 3, "```") == 0)
    {
      out.close();
      std::cout << "wrote " << outFile << '\n';
      convert(outFile);
      return;
    }

    out << line << '\n';
  }

  throw std::runtime_error{"unterminated code block in input"};
}



void parse(std::istream& in, std::string inDir)
{
  std::string line;
  while (std::getline(in, line))
  {
    if (line.compare(0, 6, "```txt") != 0)
      continue;

    auto p1 = line.find("<!--");
    auto p2 = line.find("-->", p1);
    if (p2 == line.npos)
      continue;

    p1        += 4;
    auto fname = truncated(string_view{&line[p1], p2 - p1});

    if (inDir.empty())
      extract(in, fname.to_string());
    else
      extract(in, inDir + "/" + fname.to_string());
  }

  if (in.bad())
    throw std::system_error{errno, std::system_category(), "failed to read file"};
}



int main(int argc, char** argv)
try
{
  if (argc != 2)
    throw std::runtime_error{"missing input file"};

  std::ifstream in{argv[1]};
  if (!in.is_open())
    throw std::system_error{errno, std::system_category(), argv[1]};

  fs::path path{argv[1]};
  parse(in, path.parent_path());

  return 0;
}
catch (const std::exception& e)
{
  std::clog << "error: " << e.what() << '\n';
  return 1;
}
