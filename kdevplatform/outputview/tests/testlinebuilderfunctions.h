/*
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTLINEBUILDERFUNCTIONS_H
#define KDEVPLATFORM_TESTLINEBUILDERFUNCTIONS_H

#include <QString>

namespace KDevelop
{

enum TestPathType {
    UnixFilePathNoSpaces,
    UnixFilePathWithSpaces,
    WindowsFilePathNoSpaces,
    WindowsFilePathWithSpaces
};

}

Q_DECLARE_METATYPE( KDevelop::TestPathType)

namespace KDevelop
{

// TODO: extend with other potential path patterns (network shares?)
static QString projectPath(TestPathType pathType = UnixFilePathNoSpaces)
{
    switch (pathType)
    {
        case WindowsFilePathNoSpaces:
            return QStringLiteral("C:/some/path/to/a/project");
        case WindowsFilePathWithSpaces:
            return QStringLiteral("C:/some/path with spaces/to/a/project");
        case UnixFilePathNoSpaces:
            return QStringLiteral("/some/path/to/a/project");
        case UnixFilePathWithSpaces:
            return QStringLiteral("/some/path with spaces/to/a/project");
    }
    Q_UNREACHABLE();
}

QString buildCppCheckErrorLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    /// Test CPP check output
    QString outputline(QStringLiteral("["));
    outputline.append(projectPath(pathType));
    outputline.append("main.cpp:26]: (error) Memory leak: str");
    return outputline;
}

QString buildKrazyErrorLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    /// Test krazy2 output
    QString outputline(QStringLiteral("\t"));
    outputline.append(projectPath(pathType));
    outputline.append("main.cpp: line#22 (1)");
    return outputline;
}

QString buildKrazyErrorLine2(TestPathType pathType = UnixFilePathNoSpaces)
{
    /// Test krazy2 output
    QString outputline(QStringLiteral("\t"));
    outputline.append(projectPath(pathType));
    outputline.append("main.cpp: missing tags: email address line#2  (1)");
    return outputline;
}

QString buildKrazyErrorLine3(TestPathType pathType = UnixFilePathNoSpaces)
{
    /// Test krazy2 output
    QString outputline(QStringLiteral("\t"));
    outputline.append(projectPath(pathType));
    outputline.append("main.cpp: non-const ref iterator line#451 (1)");
    return outputline;
}

QString buildKrazyErrorLineNoLineInfo(TestPathType pathType = UnixFilePathNoSpaces)
{
    /// Test krazy2 output
    QString outputline(QStringLiteral("\t"));
    outputline.append(projectPath(pathType));
    outputline.append("main.cpp: missing license");
    return outputline;
}

QString buildCompilerLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    /// Test with compiler output
    QString outputline;
    outputline.append(projectPath(pathType));
    outputline.append(">make");
    return outputline;
}

QString buildCompilerErrorLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    QString outputline;
    outputline.append(projectPath(pathType));
    outputline.append("main.cpp:5:5: error: ‘RingBuffer’ was not declared in this scope");
    return outputline;
}

QString buildCompilerInformationLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    QString outputline;
    outputline.append(projectPath(pathType));
    outputline.append("main.cpp:6:14: instantiated from here");
    return outputline;
}

QString buildInfileIncludedFromFirstLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    QString outputline(QStringLiteral("In file included from "));
    outputline.append(projectPath(pathType));
    outputline.append("PriorityFactory.h:52:0,");
    return outputline;
}

QString buildInfileIncludedFromSecondLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    QString outputline(QStringLiteral("    from "));
    outputline.append(projectPath(pathType));
    outputline.append("PatchBasedInpainting.hxx:29,");
    return outputline;
}

QString buildCompilerActionLine()
{
    return QStringLiteral("linking testCustombuild (g++)");
}

QString buildCmakeConfigureMultiLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    QString outputline;
    outputline.append(projectPath(pathType));
    outputline.append("CMakeLists.txt:10:");
    return outputline;
}

QString buildAutoMocLine(TestPathType pathType = UnixFilePathNoSpaces, bool useAutoMoc = true)
{
    QString outputline;
    useAutoMoc ? outputline.append("AUTOMOC: error: ") : outputline.append("AUTOGEN: error: ");
    outputline.append(projectPath(pathType));
    outputline.append("bar.cpp The file includes the moc file \"moc_bar1.cpp\"");
    return outputline;
}

QString buildOldAutoMocLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    QString outputline;
    outputline.append("automoc4: The file \"");
    outputline.append(projectPath(pathType));
    outputline.append("bar.cpp\" includes the moc file \"bar1.moc\"");
    return outputline;
}

QString buildLinkerErrorLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    return projectPath(pathType) + QLatin1String("Buffer.cpp:66: undefined reference to `Buffer::does_not_exist()'");
}

QString buildPythonErrorLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    QString outputline(QStringLiteral("File \""));
    outputline.append(projectPath(pathType));
    outputline.append("pythonExample.py\", line 10");
    return outputline;
}

QString buildCppCheckInformationLine()
{
    return QStringLiteral("(information) Cppcheck cannot find all the include files. Cpppcheck can check the code without the include\
    files found. But the results will probably be more accurate if all the include files are found. Please check your project's \
    include directories and add all of them as include directories for Cppcheck. To see what files Cppcheck cannot find use --check-config.");
}

QString buildTscErrorLine(TestPathType pathType = UnixFilePathNoSpaces)
{
    QString outputline;
    outputline.append(projectPath(pathType));
    outputline.append("(43,9): error TS2304: Cannot find name 'testDevice'.");
    return outputline;
}

QString buildGtestErrorLine(TestPathType pathType)
{
    return projectPath(pathType) + QLatin1String("test_foo_impl.cpp:311: Failure");
}

} // end namespace

#endif
