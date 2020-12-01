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

Project
{
  property string version: "1.0"
  property string bindir: "bin"
  property bool testcoverage: false

  minimumQbsVersion: "1.11"
  qbsSearchPaths: ["qbs"]

  references: [
    "src/draawsci.qbs",
  ]

  Product {
    name: "Project"
    files: [
          "README.md",
          "configure.info",
          "configure",
          "makefile",
      ]
  }
}
