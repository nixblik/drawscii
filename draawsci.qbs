import qbs

QtApplication {
  files: [
        "src/common.h",
        "src/graph.cpp",
        "src/graph.h",
        "src/main.cpp",
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
