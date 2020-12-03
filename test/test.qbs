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
import qbs

QtApplication {
  type: ["application","autotest"]
  files: [
        "generate_output",
        "tempfile.cpp",
        "tempfile.h",
        "test_drawscii.cpp",
        "test_drawscii.h",
    ]

  Depends { name:"Qt"; submodules:["core","gui","testlib"] }
  Depends { name:"drawscii" }
  cpp.cxxLanguageVersion: "c++14"
  cpp.defines: [
    'QT_DEPRECATED_WARNINGS',
  ]

  Group {
    name: "Input files"
    files: [
          "input/arrow_vs_text.txt",
          "input/can.txt",
          "input/color_codes.txt",
          "input/color_more.txt",
          "input/corner.txt",
          "input/cornered_line.txt",
          "input/cross.txt",
          "input/diag_tree.txt",
          "input/hell.txt",
          "input/intertwined.txt",
          "input/letters.txt",
          "input/linked_shapes.txt",
          "input/rect_intersect.txt",
          "input/rect_parallel.txt",
          "input/square_shade.txt",
          "input/text_align.txt",
          "input/text_bg_color.txt",
          "input/text_separation.txt",
          "input/turnkey.txt",
      ]
  }
}
