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
  files: [
        "blur.cpp",
        "blur.h",
        "common.h",
        "construct_graph.cpp",
        "direction.cpp",
        "direction.h",
        "flags.h",
        "graph.cpp",
        "graph.h",
        "graph2.cpp",
        "graph2.h",
        "hints.cpp",
        "hints.h",
        "main.cpp",
        "matrix.h",
        "outputfile.cpp",
        "outputfile.h",
        "paragraph.cpp",
        "paragraph.h",
        "render.cpp",
        "render.h",
        "runtimeerror.cpp",
        "runtimeerror.h",
        "textimage.cpp",
        "textimage.h",
        "textimg.cpp",
        "textimg.h",
        "textpos.h",
    ]

  Depends { name:"Qt"; submodules:["core","gui","svg"] }
  Depends { name:"coverage" }
  cpp.cxxLanguageVersion: "c++14"
  cpp.defines: [
    'QT_DEPRECATED_WARNINGS',
    'VERSION="' + project.version + '"',
  ]

  Export {
    Depends { name:"cpp" }
    property string binary: product.buildDirectory + "/" + product.targetName
    cpp.defines: ['DRAWSCII_BINARY="' + binary + '"']
  }

  Group {
    fileTagsFilter: "application"
    qbs.install: true
    qbs.installDir: project.bindir
  }
}
