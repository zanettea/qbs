import qbs 1.0

Project {
    CppApplication {
        condition: qbs.targetOS === "mac"
        Depends { name: "qt.core" }
        files: "main.mm"
        cpp.frameworks: [ "Foundation" ]
    }
}
