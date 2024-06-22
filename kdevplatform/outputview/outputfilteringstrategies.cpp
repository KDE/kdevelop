/*
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "outputfilteringstrategies.h"
#include "outputformats.h"
#include "filtereditem.h"
#include <util/path.h>

#include <KLocalizedString>

#include <QFileInfo>

#include <algorithm>

namespace KDevelop
{
void initializeFilteredItem(FilteredItem& item, const ErrorFormat& filter, const QRegularExpressionMatch& match)
{
    item.lineNo = match.capturedView(filter.lineGroup).toInt() - 1;
    item.columnNo = filter.columnNumber(match);
}


template<typename ErrorFormats>
FilteredItem match(const ErrorFormats& errorFormats, const QString& line)
{
    FilteredItem item(line);
    for( const ErrorFormat& curErrFilter : errorFormats ) {
        const auto match = curErrFilter.expression.match(line);
        if( match.hasMatch() ) {
            initializeFilteredItem(item, curErrFilter, match);
            item.url = QUrl::fromUserInput(match.captured( curErrFilter.fileGroup ));

            item.type = FilteredItem::ErrorItem;

            // Make the item clickable if it comes with the necessary file & line number information
            if (curErrFilter.fileGroup > 0 && curErrFilter.lineGroup > 0) {
                item.isActivatable = true;
            }
            break;
        }
    }
    return item;
}

/// --- No filter strategy ---

NoFilterStrategy::NoFilterStrategy()
{
}

FilteredItem NoFilterStrategy::actionInLine(const QString& line)
{
    return FilteredItem( line );
}

FilteredItem NoFilterStrategy::errorInLine(const QString& line)
{
    return FilteredItem( line );
}

/// --- Compiler error filter strategy ---

/// Impl. of CompilerFilterStrategy.
class CompilerFilterStrategyPrivate
{
public:
    explicit CompilerFilterStrategyPrivate(const QUrl& buildDir);
    Path pathForFile( const QString& ) const;
    bool isMultiLineCase(const ErrorFormat& curErrFilter) const;
    void putDirAtEnd(const Path& pathToInsert);

    QVector<Path> m_currentDirs;
    Path m_buildDir;

    using PositionMap = QHash<Path, int>;
    PositionMap m_positionInCurrentDirs;
};

CompilerFilterStrategyPrivate::CompilerFilterStrategyPrivate(const QUrl& buildDir)
    : m_buildDir(buildDir)
{
}

Path CompilerFilterStrategyPrivate::pathForFile(const QString& filename) const
{
    QFileInfo fi( filename );
    Path currentPath;
    if( fi.isRelative() ) {
        if( m_currentDirs.isEmpty() ) {
            return Path(m_buildDir, filename );
        }

        auto it = m_currentDirs.constEnd() - 1;
        do {
            currentPath = Path(*it, filename);
        } while( (it-- !=  m_currentDirs.constBegin()) && !QFileInfo::exists(currentPath.toLocalFile()) );

        return currentPath;
    } else {
        currentPath = Path(filename);
    }
    return currentPath;
}

bool CompilerFilterStrategyPrivate::isMultiLineCase(const KDevelop::ErrorFormat& curErrFilter) const
{
    if(curErrFilter.compiler == QLatin1String("gfortran") || curErrFilter.compiler == QLatin1String("cmake")) {
        return true;
    }
    return false;
}

void CompilerFilterStrategyPrivate::putDirAtEnd(const Path& pathToInsert)
{
    CompilerFilterStrategyPrivate::PositionMap::iterator it = m_positionInCurrentDirs.find( pathToInsert );
    // Encountered new build directory?
    if (it == m_positionInCurrentDirs.end()) {
        m_currentDirs.push_back( pathToInsert );
        m_positionInCurrentDirs.insert( pathToInsert, m_currentDirs.size() - 1 );
    } else {
        // Build dir already in currentDirs, but move it to back of currentDirs list
        // (this gives us most-recently-used semantics in pathForFile)
        std::rotate(m_currentDirs.begin() + it.value(), m_currentDirs.begin() + it.value() + 1, m_currentDirs.end() );
        it.value() = m_currentDirs.size() - 1;
    }
}

CompilerFilterStrategy::CompilerFilterStrategy(const QUrl& buildDir)
    : d_ptr(new CompilerFilterStrategyPrivate(buildDir))
{
}

CompilerFilterStrategy::~CompilerFilterStrategy() = default;

QVector<QString> CompilerFilterStrategy::currentDirs() const
{
    Q_D(const CompilerFilterStrategy);

    QVector<QString> ret;
    ret.reserve(d->m_currentDirs.size());
    for (const auto& path : std::as_const(d->m_currentDirs)) {
        ret << path.pathOrUrl();
    }
    return ret;
}

FilteredItem CompilerFilterStrategy::actionInLine(const QString& line)
{
    Q_D(CompilerFilterStrategy);

    // A list of filters for possible compiler, linker, and make actions
    static const ActionFormat ACTION_FILTERS[] = {
        ActionFormat( 2,
                      QStringLiteral("(?:^|[^=])\\b(gcc|CC|cc|distcc|c\\+\\+|g\\+\\+|clang(?:\\+\\+)|mpicc|icc|icpc)\\s+.*-c.*[/ '\\\\]+(\\w+\\.(?:cpp|CPP|c|C|cxx|CXX|cs|java|hpf|f|F|f90|F90|f95|F95))")),
        //moc and uic
        ActionFormat( 2, QStringLiteral("/(moc|uic)\\b.*\\s-o\\s([^\\s;]+)")),
        //libtool linking
        ActionFormat( QStringLiteral("libtool"), QStringLiteral("/bin/sh\\s.*libtool.*--mode=link\\s.*\\s-o\\s([^\\s;]+)"), 1 ),
        //unsermake
        ActionFormat( 1, QStringLiteral("^compiling (.*)") ),
        ActionFormat( 2, QStringLiteral("^generating (.*)") ),
        ActionFormat( 2, QStringLiteral("(gcc|cc|c\\+\\+|g\\+\\+|clang(?:\\+\\+)|mpicc|icc|icpc)\\S* (?:\\S* )*-o ([^\\s;]+)")),
        ActionFormat( 2, QStringLiteral("^linking (.*)") ),
        //cmake
        ActionFormat( 1, QStringLiteral("\\[.+%\\] Built target (.*)") ),
        ActionFormat( QStringLiteral("cmake"),
                      QStringLiteral("\\[.+%\\] Building .* object (.*)"), 1 ),
        ActionFormat( 1, QStringLiteral("\\[.+%\\] Generating (.*)") ),
        ActionFormat( 1, QStringLiteral("^Linking (.*)") ),
        ActionFormat( QStringLiteral("cmake"),
                      QStringLiteral("(-- (?:Configuring|Generating) (?:done|incomplete)|-- Found|-- Adding|-- Enabling)"), -1 ),
        ActionFormat( 1, QStringLiteral("-- Installing (.*)") ),
        //cmake - cd - filter for project directory
        ActionFormat( QStringLiteral("cd"),
                      QStringLiteral("cmake(?:\\.exe|\\.bat)? (?:.*?) ((?:[A-Za-z]:|/).*$)"), 1),
        //libtool install
        ActionFormat( {},
                      QStringLiteral("/(?:bin/sh\\s.*mkinstalldirs).*\\s([^\\s;]+)"), 1 ),
        ActionFormat( {},
                      QStringLiteral("/(?:usr/bin/install|bin/sh\\s.*mkinstalldirs|bin/sh\\s.*libtool.*--mode=install).*\\s([^\\s;]+)"), 1 ),
        //dcop
        ActionFormat( QStringLiteral("dcopidl"),
                      QStringLiteral("dcopidl .* > ([^\\s;]+)"), 1 ),
        ActionFormat( QStringLiteral("dcopidl2cpp"),
                      QStringLiteral("dcopidl2cpp (?:\\S* )*([^\\s;]+)"), 1 ),
        // match against Entering directory to update current build dir
        ActionFormat( QStringLiteral("cd"),
                      QStringLiteral("make\\[\\d+\\]: Entering directory (\\`|\\')(.+)'"), 2),
        // waf and scons use the same basic convention as make
        ActionFormat( QStringLiteral("cd"),
                      QStringLiteral("(Waf|scons): Entering directory (\\`|\\')(.+)'"), 3)
    };

    FilteredItem item(line);
    for (const auto& curActFilter : ACTION_FILTERS) {
        const auto match = curActFilter.expression.match(line);
        if( match.hasMatch() ) {
            item.type = FilteredItem::ActionItem;

            if( curActFilter.tool == QLatin1String("cd") ) {
                const Path path(match.captured(curActFilter.fileGroup));
                d->m_currentDirs.push_back( path );
                d->m_positionInCurrentDirs.insert( path , d->m_currentDirs.size() - 1 );
            }

            // Special case for cmake: we parse the "Compiling <objectfile>" expression
            // and use it to find out about the build paths encountered during a build.
            // They are later searched by pathForFile to find source files corresponding to
            // compiler errors.
            // Note: CMake objectfile has the format: "/path/to/four/CMakeFiles/file.o"
            if ( curActFilter.fileGroup != -1 && curActFilter.tool == QLatin1String("cmake") && line.contains(QLatin1String("Building"))) {
                const auto objectFile = match.captured(curActFilter.fileGroup);
                const auto dir = objectFile.section(QStringLiteral("CMakeFiles/"), 0, 0);
                d->putDirAtEnd(Path(d->m_buildDir, dir));
            }
            break;
        }
    }
    return item;
}

FilteredItem CompilerFilterStrategy::errorInLine(const QString& line)
{
    Q_D(CompilerFilterStrategy);

    // All the possible string that indicate an error if we via Regex have been able to
    // extract file and linenumber from a given outputline
    // TODO: This seems clumsy -- and requires another scan of the line.
    // Merge this information into ErrorFormat? --Kevin
    using Indicator = QPair<QString, FilteredItem::FilteredOutputItemType>;
    static const Indicator INDICATORS[] = {
        // ld
        Indicator(QStringLiteral("undefined reference"), FilteredItem::ErrorItem),
        Indicator(QStringLiteral("undefined symbol"), FilteredItem::ErrorItem),
        Indicator(QStringLiteral("ld: cannot find"), FilteredItem::ErrorItem),
        Indicator(QStringLiteral("no such file"), FilteredItem::ErrorItem),
        // gcc
        Indicator(QStringLiteral("error"), FilteredItem::ErrorItem),
        // generic
        Indicator(QStringLiteral("warning"), FilteredItem::WarningItem),
        Indicator(QStringLiteral("info"), FilteredItem::InformationItem),
        Indicator(QStringLiteral("note"), FilteredItem::InformationItem),
    };

    // A list of filters for possible compiler, linker, and make errors
    static const ErrorFormat ERROR_FILTERS[] = {
#ifdef Q_OS_WIN
        // MSVC
        ErrorFormat( QStringLiteral("^([a-zA-Z]:\\\\.+)\\(([1-9][0-9]*)\\): ((?:error|warning) .+\\:).*$"), 1, 2, 3 ),
#endif
        // GCC - another case, eg. for #include "pixmap.xpm" which does not exists
        ErrorFormat( QStringLiteral("^(.:?[^:\\t]+):([0-9]+):([0-9]+):([^0-9]+)"), 1, 2, 4, 3 ),
        // ant
        ErrorFormat( QStringLiteral("\\[javac\\][\\s]+([^:\\t]+):([0-9]+): (warning: .*|error: .*)"), 1, 2, 3, QStringLiteral("javac")),
        // GCC
        ErrorFormat( QStringLiteral("^(.:?[^:\\t]+):([0-9]+):([^0-9]+)"), 1, 2, 3 ),
        // GCC
        ErrorFormat( QStringLiteral("^(In file included from |[ ]+from )(..[^:\\t]+):([0-9]+)(:|,)(|[0-9]+)"), 2, 3, 5 ),
        // ICC
        ErrorFormat( QStringLiteral("^(.:?[^:\\t]+)\\(([0-9]+)\\):([^0-9]+)"), 1, 2, 3, QStringLiteral("intel") ),
        //libtool link
        ErrorFormat( QStringLiteral("^(libtool):( link):( warning): "), 0, 0, 0 ),
        // make
        ErrorFormat( QStringLiteral("No rule to make target"), 0, 0, 0 ),
        // cmake - multiline expression
        ErrorFormat( QStringLiteral("((^\\/|^[a-zA-Z]:)[\\w|\\/| |\\.]+):([0-9]+):"), 1, 2, 0, QStringLiteral("cmake") ),
        // cmake
        ErrorFormat( QStringLiteral("CMake (Error|Warning) (|\\([a-zA-Z]+\\) )(in|at) ([^:]+):($|[0-9]+)"), 4, 5, 1, QStringLiteral("cmake") ),
        // cmake/automoc
        // example: AUTOMOC: error: /foo/bar.cpp The file includes (...),
        // example: AUTOMOC: error: /foo/bar.cpp: The file includes (...)
        // note: ':' after file name isn't always appended, see https://cmake.org/gitweb?p=cmake.git;a=commitdiff;h=317d8498aa02c9f486bf5071963bb2034777cdd6
        // example: AUTOGEN: error: /foo/bar.cpp: The file includes (...)
        // note: AUTOMOC got renamed to AUTOGEN at some point
        ErrorFormat( QStringLiteral("^(AUTOMOC|AUTOGEN): error: (.*?) (The file .*)$"), 2, 0, 0 ),
        // via qt4_automoc
        // example: automoc4: The file "/foo/bar.cpp" includes the moc file "bar1.moc", but ...
        ErrorFormat( QStringLiteral("^automoc4: The file \"([^\"]+)\" includes the moc file"), 1, 0, 0 ),
        // Fortran
        ErrorFormat( QStringLiteral("\"(.*)\", line ([0-9]+):(.*)"), 1, 2, 3 ),
        // GFortran
        ErrorFormat( QStringLiteral("^(.*):([0-9]+)\\.([0-9]+):(.*)"), 1, 2, 4, QStringLiteral("gfortran"), 3 ),
        // Jade
        ErrorFormat( QStringLiteral("^[a-zA-Z]+:([^:\\t]+):([0-9]+):[0-9]+:[a-zA-Z]:(.*)"), 1, 2, 3 ),
        // ifort
        ErrorFormat( QStringLiteral("^fortcom: (.*): (.*), line ([0-9]+):(.*)"), 2, 3, 1, QStringLiteral("intel") ),
        // PGI
        ErrorFormat( QStringLiteral("PGF9(.*)-(.*)-(.*)-(.*) \\((.*): ([0-9]+)\\)"), 5, 6, 4, QStringLiteral("pgi") ),
        // PGI (2)
        ErrorFormat( QStringLiteral("PGF9(.*)-(.*)-(.*)-Symbol, (.*) \\((.*)\\)"), 5, 5, 4, QStringLiteral("pgi") ),
        // TypeScript
        ErrorFormat(QStringLiteral("^(.*)\\(([0-9]+),([0-9]+)\\): ((?:[Ww]arning|[Ee]rror) TS[0-9]+: .*)"), 1, 2, 4,
                    QStringLiteral("tsc"), 3),
    };

    FilteredItem item(line);
    for (const auto& curErrFilter : ERROR_FILTERS) {
        const auto match = curErrFilter.expression.match(line);
        if( match.hasMatch() && !( line.contains( QLatin1String("Each undeclared identifier is reported only once") )
                               || line.contains( QLatin1String("for each function it appears in.") ) ) )
        {
            if(curErrFilter.fileGroup > 0) {
                if( curErrFilter.compiler == QLatin1String("cmake") ) { // Unfortunately we cannot know if an error or an action comes first in cmake, and therefore we need to do this
                    if( d->m_currentDirs.empty() ) {
                        d->putDirAtEnd( d->m_buildDir.parent() );
                    }
                }
                item.url = d->pathForFile( match.captured( curErrFilter.fileGroup ) ).toUrl();
            }
            initializeFilteredItem(item, curErrFilter, match);

            const auto txt = match.capturedView(curErrFilter.textGroup);

            // Find the indicator which happens most early.
            int earliestIndicatorIdx = txt.length();
            for (const auto& curIndicator : INDICATORS) {
                int curIndicatorIdx = txt.indexOf(curIndicator.first, 0, Qt::CaseInsensitive);
                if((curIndicatorIdx >= 0) && (earliestIndicatorIdx > curIndicatorIdx)) {
                    earliestIndicatorIdx = curIndicatorIdx;
                    item.type = curIndicator.second;
                }
            }

            // Make the item clickable if it comes with the necessary file information
            if (item.url.isValid()) {
                item.isActivatable = true;
                if(item.type == FilteredItem::InvalidItem) {
                    // If there are no error indicators in the line
                    // maybe this is a multiline case
                    if(d->isMultiLineCase(curErrFilter)) {
                        item.type = FilteredItem::ErrorItem;
                    } else {
                        // Okay so we couldn't find anything to indicate an error, but we have file and lineGroup
                        // Lets keep this item clickable and indicate this to the user.
                        item.type = FilteredItem::InformationItem;
                    }
                }
            }
            break;
        }
    }
    return item;
}


/// --- Script error filter strategy ---

ScriptErrorFilterStrategy::ScriptErrorFilterStrategy()
{
}

FilteredItem ScriptErrorFilterStrategy::actionInLine(const QString& line)
{
    return FilteredItem(line);
}

FilteredItem ScriptErrorFilterStrategy::errorInLine(const QString& line)
{
    // A list of filters for possible Python and PHP errors
    static const ErrorFormat SCRIPT_ERROR_FILTERS[] = {
        ErrorFormat( QStringLiteral("^  File \"(.*)\", line ([0-9]+)(.*$|, in(.*)$)"), 1, 2, -1 ),
        ErrorFormat( QStringLiteral("^.*(/.*):([0-9]+).*$"), 1, 2, -1 ),
        ErrorFormat( QStringLiteral("^.* in (/.*) on line ([0-9]+).*$"), 1, 2, -1 )
    };

    return match(SCRIPT_ERROR_FILTERS, line);
}

/// --- Native application error filter strategy ---

NativeAppErrorFilterStrategy::NativeAppErrorFilterStrategy()
{
}

FilteredItem NativeAppErrorFilterStrategy::actionInLine(const QString& line)
{
    return FilteredItem(line);
}

FilteredItem NativeAppErrorFilterStrategy::errorInLine(const QString& line)
{
    static const ErrorFormat NATIVE_APPLICATION_ERROR_FILTERS[] = {
        // BEGIN: C++

        // assert(false)
        // a.out: test.cpp:5: int main(): Assertion `false' failed.

        // this may also match custom assert message like this one from OSM2go:
        // code at: /osm2go/tests/osm_edit.cpp:47: int main(): Assertion foo = bar failed: foo = 5, bar = 4
        // If there is a technical reason not to do so: fine. But for the moment it's easy enough to catch it.
        ErrorFormat(QStringLiteral("^.+: (.+):([1-9][0-9]*): .+: Assertion .+ failed"), 1, 2, -1),

        // assert_perror(42)
        // a.out: test.cpp:2009: void {anonymous}::test(): Unexpected error: Broken pipe.
        ErrorFormat(QStringLiteral("^.+: (.+):([1-9][0-9]*): .+: Unexpected error: "), 1, 2, -1),

        // END: C++

        // BEGIN: Qt

        // Note: Scan the whole line for catching Qt runtime warnings (-> no ^$)
        // Reasoning: With QT_MESSAGE_PATTERN you can configure the output to your desire,
        // which means the output could be arbitrarily prefixed or suffixed with other content

        // QObject::connect related errors, also see err_method_notfound() in qobject.cpp
        // QObject::connect: No such slot Foo::bar() in /foo/bar.cpp:313
        ErrorFormat(QStringLiteral("QObject::connect: (?:No such|Parentheses expected,) (?:slot|signal) [^ ]* in (.*):([0-9]+)"), 1, 2, -1),
        // ASSERT: "errors().isEmpty()" in file /foo/bar.cpp, line 49
        ErrorFormat(QStringLiteral("ASSERT: \"(.*)\" in file (.*), line ([0-9]+)"), 2, 3, -1),
        // Catch:
        // FAIL!  : FooTest::testBar() Compared pointers are not the same
        //    Actual   ...
        //    Expected ...
        //    Loc: [/foo/bar.cpp(33)]
        //
        // Do *not* catch:
        //    ...
        //    Loc: [Unknown file(0)]
        ErrorFormat(QStringLiteral("   Loc: \\[(.*)\\(([1-9][0-9]*)\\)\\]"), 1, 2, -1),

        // file:///path/to/foo.qml:7:1: Bar is not a type
        // file:///path/to/foo.qml:49:5: QML Row: Binding loop detected for property "height"
        ErrorFormat(QStringLiteral("(file:\\/\\/(?:[^:]+)):([1-9][0-9]*):([1-9][0-9]*): (.*) (?:is not a type|is ambiguous|is instantiated recursively|Binding loop detected)"), 1, 2, -1, 3),

        // file:///path/to/foo.qml:52: TypeError: Cannot read property 'height' of null
        ErrorFormat(QStringLiteral("(file:\\/\\/(?:[^:]+)):([1-9][0-9]*): ([a-zA-Z]+)Error"), 1, 2, -1),

        // END: Qt

        // BEGIN: glib

        // all messages may have an initial entry like "GIO:" in case the log domain was set
        // the function name after the line number may be missing

        // g_assert(0)
        // ERROR:/foo/test.cpp:46:int main(): assertion failed: (0)
        ErrorFormat(QStringLiteral("^(.+:)?ERROR:(.+):([1-9][0-9]*):(.+:)? assertion failed"), 2, 3, -1),

        // g_assert_not_reached()
        // ERROR:/foo/test.cpp:2024:int main(): code should not be reached
        ErrorFormat(QStringLiteral("^(.+:)?ERROR:(.+):([1-9][0-9]*):(.+:)? code should not be reached"), 2, 3, -1),

        // g_assert_null() / g_assert_nonnull()
        // ERROR:/foo/test.cpp:18:int main(): 'bar' should not be nullptr
        // ERROR:/foo/test.c:18:int main(): 'bar' should not be NULL
        ErrorFormat(QStringLiteral("^(.+:)?ERROR:(.+):([1-9][0-9]*):(.+:)? '.+' should (not )?be (nullptr|NULL)"), 2, 3, -1),

        // g_assert_true() / g_assert_false()
        // ERROR:/foo/test.cpp:18:int main(): 'foo' should be TRUE
        // ERROR:/foo/test.cpp:18:int main(): 'foo' should be FALSE
        ErrorFormat(QStringLiteral("^(.+:)?ERROR:(.+):([1-9][0-9]*):(.+:)? '.+' should be (TRUE|FALSE)"), 2, 3, -1),

        // END: glib

        // BEGIN: gtest
        // ../test.cpp:16: Failure
        // c:\foo\test.cpp:16: Failure
        ErrorFormat(QStringLiteral("^(.+):([0-9]+): Failure"), 1, 2, -1)
        // END: gtest
    };

    return match(NATIVE_APPLICATION_ERROR_FILTERS, line);
}

/// --- Static Analysis filter strategy ---

StaticAnalysisFilterStrategy::StaticAnalysisFilterStrategy()
{
}

FilteredItem StaticAnalysisFilterStrategy::actionInLine(const QString& line)
{
    return FilteredItem(line);
}

FilteredItem StaticAnalysisFilterStrategy::errorInLine(const QString& line)
{
    // A list of filters for static analysis tools (krazy2, cppcheck)
    static const ErrorFormat STATIC_ANALYSIS_FILTERS[] = {
        // CppCheck
        ErrorFormat( QStringLiteral("^\\[(.*):([0-9]+)\\]:(.*)"), 1, 2, 3 ),
        // krazy2
        ErrorFormat( QStringLiteral("^\\t([^:]+).*line#([0-9]+).*"), 1, 2, -1 ),
        // krazy2 without line info
        ErrorFormat( QStringLiteral("^\\t(.*): missing license"), 1, -1, -1 )
    };

    return match(STATIC_ANALYSIS_FILTERS, line);
}

}
