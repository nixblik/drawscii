/*  Copyright 2020 Uwe Salomon <post@uwesalomon.de>

    This file is part of Draawsci.

    Draawsci is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Draawsci is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Draawsci.  If not, see <http://www.gnu.org/licenses/>.
*/
import qbs

QtApplication {
  name: "draawsci"
  files: [
        "common.h",
        "flags.h",
        "graph.cpp",
        "graph.h",
        "hints.cpp",
        "hints.h",
        "main.cpp",
        "matrix.h",
        "outputfile.cpp",
        "outputfile.h",
        "render.cpp",
        "render.h",
        "runtimeerror.cpp",
        "runtimeerror.h",
        "textimg.cpp",
        "textimg.h",
    ]

  Depends { name:"Qt"; submodules:["core","gui","svg"] }
  Depends { name:"coverage" }

  consoleApplication: true
  cpp.cxxLanguageVersion: "c++11"
  cpp.defines: [
    'QT_DEPRECATED_WARNINGS',
    'VERSION="' + project.version + '"',
  ]

  Group {
    fileTagsFilter: "application"
    qbs.install: true
    qbs.installDir: project.bindir
  }
}
