import qbs

Project
{

  QtApplication {
    name: "draawsci"
    files: [
          "src/common.h",
          "src/flags.h",
          "src/graph.cpp",
          "src/graph.h",
          "src/main.cpp",
          "src/matrix.h",
          "src/render.cpp",
          "src/render.h",
          "src/runtimeerror.cpp",
          "src/runtimeerror.h",
          "src/textimg.cpp",
          "src/textimg.h",
      ]

    Depends { name:"Qt.gui" }
    consoleApplication: true
    cpp.cxxLanguageVersion: "c++11"
    cpp.defines: [
      'QT_DEPRECATED_WARNINGS',
      'VERSION="1.0"',
    ]

    Group {
      fileTagsFilter: "application"
      qbs.install: true
      qbs.installDir: "bin"
    }
  }

  Product {
    name: "Tests"
    files: [
          "test/input/art1.txt",
          "test/input/art10.txt",
          "test/input/art11.txt",
          "test/input/art12.txt",
          "test/input/art13.txt",
          "test/input/art14.txt",
          "test/input/art15.txt",
          "test/input/art16.txt",
          "test/input/art17.txt",
          "test/input/art18.txt",
          "test/input/art19.txt",
          "test/input/art2.txt",
          "test/input/art20.txt",
          "test/input/art2_5.txt",
          "test/input/art3.txt",
          "test/input/art3_5.txt",
          "test/input/art4.txt",
          "test/input/art5.txt",
          "test/input/art6.txt",
          "test/input/art7.txt",
          "test/input/art8.txt",
          "test/input/art_text.txt",
          "test/input/bug1.txt",
          "test/input/bug10.txt",
          "test/input/bug11.txt",
          "test/input/bug12.txt",
          "test/input/bug13.txt",
          "test/input/bug14.txt",
          "test/input/bug15.txt",
          "test/input/bug16.txt",
          "test/input/bug17.txt",
          "test/input/bug18.txt",
          "test/input/bug2.txt",
          "test/input/bug3.txt",
          "test/input/bug4.txt",
          "test/input/bug5.txt",
          "test/input/bug6.txt",
          "test/input/bug7.txt",
          "test/input/bug8.txt",
          "test/input/bug9.txt",
          "test/input/bug9_5.txt",
          "test/input/color_codes.txt",
          "test/input/corner.txt",
          "test/input/corner_case01.txt",
          "test/input/corner_case02.txt",
          "test/input/cornered_line.txt",
          "test/input/ditaa_bug.txt",
          "test/input/ditaa_bug2.txt",
          "test/input/hell.txt",
          "test/input/logo.txt",
          "test/input/simple_S01.txt",
          "test/input/simple_U01.txt",
          "test/input/simple_square01.txt",
          "test/input/turnkey.txt",
      ]
  }
}
