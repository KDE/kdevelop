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

#include <KDebug>
#include <KLocalizedString>

#include <QFileInfo>

#include <algorithm>


namespace KDevelop
{

FilteredItem FilteringStrategyUtils::match(const QList<ErrorFormat>& errorFormats, const QString& line)
{
    FilteredItem item(line);
    foreach( const ErrorFormat& curErrFilter, errorFormats ) {
        QRegExp regEx = curErrFilter.expression;
        if( regEx.indexIn( line ) != -1 )
        {
            item.url = regEx.cap( curErrFilter.fileGroup );
            item.lineNo = regEx.cap( curErrFilter.lineGroup ).toInt() - 1;
            if(curErrFilter.columnGroup >= 0) {
                item.columnNo = regEx.cap( curErrFilter.columnGroup ).toInt() - 1;
            } else {
                item.columnNo = 0;
            }

            QString txt = regEx.cap(curErrFilter.textGroup);

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
    CompilerFilterStrategyPrivate(const KUrl& buildDir);
    KUrl urlForFile( const QString& ) const;
    bool isMultiLineCase(ErrorFormat curErrFilter) const;
    void putDirAtEnd(const QString& );

    QVector<QString> m_currentDirs;
    KUrl m_buildDir;

    typedef QMap<QString, int> PositionMap;
    PositionMap m_positionInCurrentDirs;
};

namespace {
// All the possible string that indicate an error if we via Regex have been able to
// extract file and linenumber from a given outputline
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
    // example: AUTOMOC: error: /home/krf/devel/src/foo/src/quick/quickpathitem.cpp The file includes (...),
    ErrorFormat( "^AUTOMOC: error: (.*) (The file includes .*)$", 1, 0, 0 ),
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
                     "g\\+\\+|clang|clang\\+\\+|mpicc|icc|icpc)\\s+.*-c.*[/ '\\\\]+(\\w+\\.(?:cpp|CPP|c|C|cxx|CXX|cs|"
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
                     "linking"), 1, 2, "(gcc|cc|c\\+\\+|g\\+\\+|clang|clang\\+\\+|mpicc|icc|icpc)\\S* (?:\\S* )*-o ([^\\s;]+)"),
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
};

}

CompilerFilterStrategyPrivate::CompilerFilterStrategyPrivate(const KUrl& buildDir)
: m_buildDir(buildDir)
{
}

KUrl CompilerFilterStrategyPrivate::urlForFile(const QString& filename) const
{
    QFileInfo fi( filename );
    KUrl currentUrl;
    if( fi.isRelative() ) {
        if( m_currentDirs.isEmpty() ) {
            currentUrl = m_buildDir;
            currentUrl.addPath( filename );
            return currentUrl;
        }

        QVector<QString>::const_iterator it = m_currentDirs.constEnd() - 1;
        do {
            currentUrl = KUrl( *it );
            currentUrl.addPath( filename );
        } while( (it-- !=  m_currentDirs.constBegin()) && !QFileInfo(currentUrl.toLocalFile()).exists() );

        return currentUrl;
    } else {
        currentUrl = KUrl( filename );
    }
    return currentUrl;
}

bool CompilerFilterStrategyPrivate::isMultiLineCase(KDevelop::ErrorFormat curErrFilter) const
{
    if(curErrFilter.compiler == "gfortran" || curErrFilter.compiler == "cmake") {
        return true;
    }
    return false;
}

void CompilerFilterStrategyPrivate::putDirAtEnd(const QString& dirNameToInsert)
{
    CompilerFilterStrategyPrivate::PositionMap::iterator it = m_positionInCurrentDirs.find( dirNameToInsert );
    // Encountered new build directory?
    if (it == m_positionInCurrentDirs.end()) {
        m_currentDirs.push_back( dirNameToInsert );
        m_positionInCurrentDirs.insert( dirNameToInsert, m_currentDirs.size() - 1 );
    } else {
        // Build dir already in currentDirs, but move it to back of currentDirs list
        // (this gives us most-recently-used semantics in urlForFile)
        std::rotate(m_currentDirs.begin() + it.value(), m_currentDirs.begin() + it.value() + 1, m_currentDirs.end() );
        it.value() = m_currentDirs.size() - 1;
    }
}

CompilerFilterStrategy::CompilerFilterStrategy(const KUrl& buildDir)
: d(new CompilerFilterStrategyPrivate( buildDir ))
{
}

CompilerFilterStrategy::~CompilerFilterStrategy()
{
    delete d;
}

QVector< QString > CompilerFilterStrategy::getCurrentDirs()
{
    return d->m_currentDirs;
}

FilteredItem CompilerFilterStrategy::actionInLine(const QString& line)
{
    const QByteArray cd = "cd";
    const QByteArray compiling = "compiling";
    FilteredItem item(line);
    foreach( const ActionFormat& curActFilter, ACTION_FILTERS ) {
        QRegExp regEx = curActFilter.expression;
        if( regEx.indexIn( line ) != -1 )
        {
            item.type = FilteredItem::ActionItem;
            if( curActFilter.fileGroup != -1 && curActFilter.toolGroup != -1 )
            {
                item.shortenedText = QString( "%1 %2 (%3)").arg(i18nc(curActFilter.context, curActFilter.action)).arg( regEx.cap( curActFilter.fileGroup ) ).arg( regEx.cap( curActFilter.toolGroup ) );
            }
            if( curActFilter.action == cd )
            {
                d->m_currentDirs.push_back( regEx.cap( curActFilter.fileGroup ) );
                d->m_positionInCurrentDirs.insert( regEx.cap( curActFilter.fileGroup ) , d->m_currentDirs.size() - 1 );
            }

            // Special case for cmake: we parse the "Compiling <objectfile>" expression
            // and use it to find out about the build paths encountered during a build.
            // They are later searched by urlForFile to find source files corresponding to
            // compiler errors.
            if ( curActFilter.action == compiling && curActFilter.tool == "cmake") {
                KUrl url = d->m_buildDir;
                url.addPath(regEx.cap( curActFilter.fileGroup ));
                d->putDirAtEnd(url.toLocalFile());
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
        QRegExp regEx = curErrFilter.expression;
        if( regEx.indexIn( line ) != -1 && !( line.contains( "Each undeclared identifier is reported only once" ) || line.contains( "for each function it appears in." ) ) ) {
            if(curErrFilter.fileGroup > 0) {
                if( curErrFilter.compiler == "cmake" ) { // Unfortunately we cannot know if an error or an action comes first in cmake, and therefore we need to do this
                    if( d->m_currentDirs.empty() ) {
                        d->putDirAtEnd( d->m_buildDir.upUrl().toLocalFile() );
                    }
                }
                item.url = d->urlForFile( regEx.cap( curErrFilter.fileGroup ) );
            }
            item.lineNo = regEx.cap( curErrFilter.lineGroup ).toInt() - 1;
            if(curErrFilter.columnGroup >= 0) {
                item.columnNo = regEx.cap( curErrFilter.columnGroup ).toInt() - 1;
            } else {
                item.columnNo = 0;
            }

            QString txt = regEx.cap(curErrFilter.textGroup);

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
    //QRegExp python("^  File \"(.*)\", line (\\d*), in(.*)$");
    << ErrorFormat( "^  File \"(.*)\", line ([0-9]+)(.*$|, in(.*)$)", 1, 2, -1 )
    //QRegExp phpstacktrace("^.*(/.*):(\\d*).*$");
    << ErrorFormat( "^.*(/.*):([0-9]+).*$", 1, 2, -1 )
    //QRegExp phperror("^.* in (/.*) on line (\\d*).*$");
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
