/*
    This file is part of KDevelop
    Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

FilteredItem FilteringStrategyUtils::match(const QList<ErrorFormat>& errorFormats, const QString& line)
{
    FilteredItem item(line);
    foreach( const ErrorFormat& curErrFilter, errorFormats ) {
        const auto match = curErrFilter.expression.match(line);
        if( match.hasMatch() ) {
            item.url = QUrl::fromUserInput(match.captured( curErrFilter.fileGroup ));
            item.lineNo = match.captured( curErrFilter.lineGroup ).toInt() - 1;
            if(curErrFilter.columnGroup >= 0) {
                item.columnNo = match.captured( curErrFilter.columnGroup ).toInt() - 1;
            } else {
                item.columnNo = 0;
            }

            QString txt = match.captured(curErrFilter.textGroup);

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
struct CompilerFilterStrategyPrivate
{
    CompilerFilterStrategyPrivate(const QUrl& buildDir);
    Path pathForFile( const QString& ) const;
    bool isMultiLineCase(ErrorFormat curErrFilter) const;
    void putDirAtEnd(const Path& pathToInsert);

    QVector<Path> m_currentDirs;
    Path m_buildDir;

    using PositionMap = QHash<Path, int>;
    PositionMap m_positionInCurrentDirs;
};

namespace {
// All the possible string that indicate an error if we via Regex have been able to
// extract file and linenumber from a given outputline
// TODO: This seems clumsy -- and requires another scan of the line.
// Merge this information into ErrorFormat? --Kevin
typedef QPair<QString, FilteredItem::FilteredOutputItemType> Indicator;
const QVector<Indicator> INDICATORS {
    // ld
    Indicator("undefined reference", FilteredItem::ErrorItem),
    Indicator("undefined symbol", FilteredItem::ErrorItem),
    Indicator("ld: cannot find", FilteredItem::ErrorItem),
    Indicator("no such file", FilteredItem::ErrorItem),
    // gcc
    Indicator("error", FilteredItem::ErrorItem),
    // generic
    Indicator("warning", FilteredItem::WarningItem),
    Indicator("info", FilteredItem::InformationItem),
    Indicator("note", FilteredItem::InformationItem),
};

// A list of filters for possible compiler, linker, and make errors
const QVector<ErrorFormat> ERROR_FILTERS {
    // GCC - another case, eg. for #include "pixmap.xpm" which does not exists
    ErrorFormat( "^([^:\t]+):([0-9]+):([0-9]+):([^0-9]+)", 1, 2, 4, 3 ),
    // GCC
    ErrorFormat( "^([^:\t]+):([0-9]+):([^0-9]+)", 1, 2, 3 ),
    // GCC
    ErrorFormat( "^(In file included from |[ ]+from )([^: \\t]+):([0-9]+)(:|,)(|[0-9]+)", 2, 3, 5 ),
    // ICC
    ErrorFormat( "^([^: \\t]+)\\(([0-9]+)\\):([^0-9]+)", 1, 2, 3, "intel" ),
    //libtool link
    ErrorFormat( "^(libtool):( link):( warning): ", 0, 0, 0 ),
    // make
    ErrorFormat( "No rule to make target", 0, 0, 0 ),
    // cmake
    ErrorFormat( "^([^: \\t]+):([0-9]+):", 1, 2, 0, "cmake" ),
    // cmake
    ErrorFormat( "CMake (Error|Warning) (|\\([a-zA-Z]+\\) )(in|at) ([^:]+):($|[0-9]+)", 4, 5, 1, "cmake" ),
    // cmake/automoc
    // example: AUTOMOC: error: /foo/bar.cpp The file includes (...),
    // example: AUTOMOC: error: /foo/bar.cpp: The file includes (...)
    // note: ':' after file name isn't always appended, see http://cmake.org/gitweb?p=cmake.git;a=commitdiff;h=317d8498aa02c9f486bf5071963bb2034777cdd6
    // example: AUTOGEN: error: /foo/bar.cpp: The file includes (...)
    // note: AUTOMOC got renamed to AUTOGEN at some point
    ErrorFormat( "^(AUTOMOC|AUTOGEN): error: ([^:]+):? (The file .*)$", 2, 0, 0 ),
    // via qt4_automoc
    // example: automoc4: The file "/foo/bar.cpp" includes the moc file "bar1.moc", but ...
    ErrorFormat( "^automoc4: The file \"([^\"]+)\" includes the moc file", 1, 0, 0 ),
    // Fortran
    ErrorFormat( "\"(.*)\", line ([0-9]+):(.*)", 1, 2, 3 ),
    // GFortran
    ErrorFormat( "^(.*):([0-9]+)\\.([0-9]+):(.*)", 1, 2, 4, "gfortran", 3 ),
    // Jade
    ErrorFormat( "^[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:(.*)", 1, 2, 3 ),
    // ifort
    ErrorFormat( "^fortcom: (.*): (.*), line ([0-9]+):(.*)", 2, 3, 1, "intel" ),
    // PGI
    ErrorFormat( "PGF9(.*)-(.*)-(.*)-(.*) \\((.*): ([0-9]+)\\)", 5, 6, 4, "pgi" ),
    // PGI (2)
    ErrorFormat( "PGF9(.*)-(.*)-(.*)-Symbol, (.*) \\((.*)\\)", 5, 5, 4, "pgi" ),
};

// A list of filters for possible compiler, linker, and make actions
const QVector<ActionFormat> ACTION_FILTERS {
    ActionFormat( I18N_NOOP2_NOSTRIP("", "compiling"), 1, 2, "(?:^|[^=])\\b(gcc|CC|cc|distcc|c\\+\\+|"
                     "g\\+\\+|clang(?:\\+\\+)|mpicc|icc|icpc)\\s+.*-c.*[/ '\\\\]+(\\w+\\.(?:cpp|CPP|c|C|cxx|CXX|cs|"
                     "java|hpf|f|F|f90|F90|f95|F95))"),
    //moc and uic
    ActionFormat( I18N_NOOP2_NOSTRIP("", "generating"), 1, 2, "/(moc|uic)\\b.*\\s-o\\s([^\\s;]+)"),
    //libtool linking
    ActionFormat( I18N_NOOP2_NOSTRIP("Linking object files into a library or executable", "linking"),
                     "libtool", "/bin/sh\\s.*libtool.*--mode=link\\s.*\\s-o\\s([^\\s;]+)", 1 ),
    //unsermake
    ActionFormat( I18N_NOOP2_NOSTRIP("", "compiling"), 1, 1, "^compiling (.*)" ),
    ActionFormat( I18N_NOOP2_NOSTRIP("", "generating"), 1, 2, "^generating (.*)" ),
    ActionFormat( I18N_NOOP2_NOSTRIP("Linking object files into a library or executable",
                     "linking"), 1, 2, "(gcc|cc|c\\+\\+|g\\+\\+|clang(?:\\+\\+)|mpicc|icc|icpc)\\S* (?:\\S* )*-o ([^\\s;]+)"),
    ActionFormat( I18N_NOOP2_NOSTRIP("Linking object files into a library or executable",
                     "linking"), 1, 2, "^linking (.*)" ),
    //cmake
    ActionFormat( I18N_NOOP2_NOSTRIP("", "built"), -1, 1, "\\[.+%\\] Built target (.*)" ),
    ActionFormat( I18N_NOOP2_NOSTRIP("", "compiling"), "cmake", "\\[.+%\\] Building .* object (.*)CMakeFiles/", 1 ),
    ActionFormat( I18N_NOOP2_NOSTRIP("", "generating"), -1, 1, "\\[.+%\\] Generating (.*)" ),
    ActionFormat( I18N_NOOP2_NOSTRIP("Linking object files into a library or executable",
                     "linking"), -1, 1, "^Linking (.*)" ),
    ActionFormat( I18N_NOOP2_NOSTRIP("", "configuring"), "cmake", "(-- Configuring (done|incomplete)|-- Found|-- Adding|-- Enabling)", -1 ),
    ActionFormat( I18N_NOOP2_NOSTRIP("", "installing"), -1, 1, "-- Installing (.*)" ),
    //libtool install
    ActionFormat( I18N_NOOP2_NOSTRIP("", "creating"), "", "/(?:bin/sh\\s.*mkinstalldirs).*\\s([^\\s;]+)", 1 ),
    ActionFormat( I18N_NOOP2_NOSTRIP("", "installing"), "", "/(?:usr/bin/install|bin/sh\\s.*mkinstalldirs"
                     "|bin/sh\\s.*libtool.*--mode=install).*\\s([^\\s;]+)", 1 ),
    //dcop
    ActionFormat( I18N_NOOP2_NOSTRIP("", "generating"), "dcopidl", "dcopidl .* > ([^\\s;]+)", 1 ),
    ActionFormat( I18N_NOOP2_NOSTRIP("", "compiling"), "dcopidl2cpp", "dcopidl2cpp (?:\\S* )*([^\\s;]+)", 1 ),
    // match against Entering directory to update current build dir
    ActionFormat( "", "cd", "", "make\\[\\d+\\]: Entering directory (\\`|\\')(.+)'", 2),
    // waf and scons use the same basic convention as make
    ActionFormat( "", "cd", "", "(Waf|scons): Entering directory (\\`|\\')(.+)'", 3)
};

}

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
        } while( (it-- !=  m_currentDirs.constBegin()) && !QFileInfo(currentPath.toLocalFile()).exists() );

        return currentPath;
    } else {
        currentPath = Path(filename);
    }
    return currentPath;
}

bool CompilerFilterStrategyPrivate::isMultiLineCase(KDevelop::ErrorFormat curErrFilter) const
{
    if(curErrFilter.compiler == "gfortran" || curErrFilter.compiler == "cmake") {
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
: d(new CompilerFilterStrategyPrivate( buildDir ))
{
}

CompilerFilterStrategy::~CompilerFilterStrategy()
{
    delete d;
}

QVector<QString> CompilerFilterStrategy::getCurrentDirs()
{
    QVector<QString> ret;
    ret.reserve(d->m_currentDirs.size());
    for (const auto& path : d->m_currentDirs) {
        ret << path.pathOrUrl();
    }
    return ret;
}

FilteredItem CompilerFilterStrategy::actionInLine(const QString& line)
{
    const QByteArray cd = "cd";
    const QByteArray compiling = "compiling";
    FilteredItem item(line);
    foreach( const ActionFormat& curActFilter, ACTION_FILTERS ) {
        const auto match = curActFilter.expression.match(line);
        if( match.hasMatch() ) {
            item.type = FilteredItem::ActionItem;
            if( curActFilter.fileGroup != -1 && curActFilter.toolGroup != -1 ) {
                item.shortenedText = QStringLiteral( "%1 %2 (%3)")
                    .arg(i18nc(curActFilter.context, curActFilter.action))
                    .arg(match.captured(curActFilter.fileGroup))
                    .arg(match.captured(curActFilter.toolGroup));
            }
            if( curActFilter.action == cd ) {
                const Path path(match.captured(curActFilter.fileGroup));
                d->m_currentDirs.push_back( path );
                d->m_positionInCurrentDirs.insert( path , d->m_currentDirs.size() - 1 );
            }

            // Special case for cmake: we parse the "Compiling <objectfile>" expression
            // and use it to find out about the build paths encountered during a build.
            // They are later searched by pathForFile to find source files corresponding to
            // compiler errors.
            if ( curActFilter.action == compiling && curActFilter.tool == "cmake") {
                d->putDirAtEnd(Path(d->m_buildDir, match.captured(curActFilter.fileGroup).mid(1)));
            }
            break;
        }
    }
    return item;
}

FilteredItem CompilerFilterStrategy::errorInLine(const QString& line)
{
    FilteredItem item(line);
    foreach( const ErrorFormat& curErrFilter, ERROR_FILTERS ) {
        const auto match = curErrFilter.expression.match(line);
        if( match.hasMatch() && !( line.contains( QLatin1String("Each undeclared identifier is reported only once") )
                               || line.contains( QLatin1String("for each function it appears in.") ) ) )
        {
            if(curErrFilter.fileGroup > 0) {
                if( curErrFilter.compiler == "cmake" ) { // Unfortunately we cannot know if an error or an action comes first in cmake, and therefore we need to do this
                    if( d->m_currentDirs.empty() ) {
                        d->putDirAtEnd( d->m_buildDir.parent() );
                    }
                }
                item.url = d->pathForFile( match.captured( curErrFilter.fileGroup ) ).toUrl();
            }
            item.lineNo = match.captured( curErrFilter.lineGroup ).toInt() - 1;
            if(curErrFilter.columnGroup >= 0) {
                item.columnNo = match.captured( curErrFilter.columnGroup ).toInt() - 1;
            } else {
                item.columnNo = 0;
            }

            QString txt = match.captured(curErrFilter.textGroup);

            // Find the indicator which happens most early.
            int earliestIndicatorIdx = txt.length();
            foreach( const Indicator& curIndicator, INDICATORS ) {
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

// A list of filters for possible Python and PHP errors
const QList<ErrorFormat> SCRIPT_ERROR_FILTERS = QList<ErrorFormat>()
    << ErrorFormat( "^  File \"(.*)\", line ([0-9]+)(.*$|, in(.*)$)", 1, 2, -1 )
    << ErrorFormat( "^.*(/.*):([0-9]+).*$", 1, 2, -1 )
    << ErrorFormat( "^.* in (/.*) on line ([0-9]+).*$", 1, 2, -1 );


ScriptErrorFilterStrategy::ScriptErrorFilterStrategy()
{
}

FilteredItem ScriptErrorFilterStrategy::actionInLine(const QString& line)
{
    return FilteredItem(line);
}

FilteredItem ScriptErrorFilterStrategy::errorInLine(const QString& line)
{
    return FilteringStrategyUtils::match(SCRIPT_ERROR_FILTERS, line);
}

/// --- Native application error filter strategy ---

const QList<ErrorFormat> QT_APPLICATION_ERROR_FILTERS = QList<ErrorFormat>()
    // QObject::connect related errors, also see err_method_notfound() in qobject.cpp
    // QObject::connect: No such slot Foo::bar() in /foo/bar.cpp:313
    << ErrorFormat("^QObject::connect: (?:No such|Parentheses expected,) (?:slot|signal) [^ ]* in (.*):([0-9]+)$", 1, 2, -1)
    // ASSERT: "errors().isEmpty()" in file /foo/bar.cpp, line 49
    << ErrorFormat("^ASSERT: \"(.*)\" in file (.*), line ([0-9]+)$", 2, 3, -1)
    // QFATAL : FooTest::testBar() ASSERT: "index.isValid()" in file /foo/bar.cpp, line 32
    << ErrorFormat("^QFATAL : (.*) ASSERT: \"(.*)\" in file (.*), line ([0-9]+)$", 3, 4, -1)
    // Catch:
    // FAIL!  : FooTest::testBar() Compared pointers are not the same
    //    Actual   ...
    //    Expected ...
    //    Loc: [/foo/bar.cpp(33)]
    //
    // Do *not* catch:
    //    ...
    //    Loc: [Unknown file(0)]
    << ErrorFormat("^   Loc: \\[(.*)\\(([1-9][0-9]*)\\)\\]$", 1, 2, -1);

NativeAppErrorFilterStrategy::NativeAppErrorFilterStrategy()
{
}

FilteredItem NativeAppErrorFilterStrategy::actionInLine(const QString& line)
{
    return FilteredItem(line);
}

FilteredItem NativeAppErrorFilterStrategy::errorInLine(const QString& line)
{
    return FilteringStrategyUtils::match(QT_APPLICATION_ERROR_FILTERS, line);
}

/// --- Static Analysis filter strategy ---

// A list of filters for static analysis tools (krazy2, cppcheck)
const QList<ErrorFormat> STATIC_ANALYSIS_FILTERS = QList<ErrorFormat>()
    // CppCheck
    << ErrorFormat( "^\\[(.*):([0-9]+)\\]:(.*)", 1, 2, 3 )
    // krazy2
    << ErrorFormat( "^\\t([^:]+).*line#([0-9]+).*", 1, 2, -1 )
    // krazy2 without line info
    << ErrorFormat( "^\\t(.*): missing license", 1, -1, -1 );

StaticAnalysisFilterStrategy::StaticAnalysisFilterStrategy()
{
}

FilteredItem StaticAnalysisFilterStrategy::actionInLine(const QString& line)
{
    return FilteredItem(line);
}

FilteredItem StaticAnalysisFilterStrategy::errorInLine(const QString& line)
{
    return FilteringStrategyUtils::match(STATIC_ANALYSIS_FILTERS, line);
}

}
