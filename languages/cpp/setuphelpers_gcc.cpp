/*
* This file is part of KDevelop
*
* Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
* Copyright 2007 Kris Wong <kris.p.wong@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "setuphelpers.h"

#include <QString>
#include <QStringList>
#include <QDir>

#include <kprocess.h>
#include <kdebug.h>
#include <parser/rpp/chartools.h>
#include <parser/rpp/macrorepository.h>

using namespace KDevelop;

#ifdef _WIN32
#define NULL_DEVICE "NUL"
#else
#define NULL_DEVICE "/dev/null"
#endif

namespace CppTools {

QStringList gccSetupStandardIncludePaths(bool withStdCpp0x)
{
    QStringList includePaths;
    
    KProcess proc;
    proc.setOutputChannelMode(KProcess::MergedChannels);

    // The following command will spit out a bnuch of information we don't care
    // about before spitting out the include paths.  The parts we care about
    // look like this:
    // #include "..." search starts here:
    // #include <...> search starts here:
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/i486-linux-gnu
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/backward
    //  /usr/local/include
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/include
    //  /usr/include
    // End of search list.
    proc << "gcc";
    if (withStdCpp0x) {
        // see also: https://bugs.kde.org/show_bug.cgi?id=298252
        proc << "-std=c++0x";
    }
    proc << "-xc++" << "-E" << "-v" << NULL_DEVICE;

    // We'll use the following constants to know what we're currently parsing.
    const short parsingInitial = 0;
    const short parsedFirstSearch = 1;
    const short parsingIncludes = 2;
    const short parsingFinished = 3;
    short parsingMode = parsingInitial;

    if (proc.execute(5000) == 0) {
        QString line;
        while (proc.canReadLine() && parsingMode != parsingFinished) {
            QByteArray buff = proc.readLine();
            if (!buff.isEmpty()) {
                line = buff;
                switch (parsingMode) {
                case parsingInitial:
                    if (line.indexOf("#include \"...\"") != -1) {
                        parsingMode = parsedFirstSearch;
                    }
                    break;
                case parsedFirstSearch:
                    if (line.indexOf("#include <...>") != -1) {
                        parsingMode = parsingIncludes;
                        break;
                    }
                case parsingIncludes:
                    //if (!line.indexOf(QDir::separator()) == -1 && line != "." ) {
                    //Detect the include-paths by the first space that is prepended. Reason: The list may contain relative paths like "."
                    if (!line.startsWith(" ") ) {
                        // We've reached the end of the list.
                        parsingMode = parsingFinished;
                    } else {
                        line = line.trimmed();
                        // This is an include path, add it to the list.
                        includePaths << QDir::cleanPath(line);
                    }
                    break;
                }
            }
        }
    } else if (withStdCpp0x) {
        // fallback to include-path computation without -std=c++0x arg for old gcc versions
        return gccSetupStandardIncludePaths(false);
    } else {
        kDebug(9007) <<"Unable to read standard c++ macro definitions from gcc:" <<QString(proc.readAll()) ;
    }
    
    return includePaths;
}

QStringList gccSetupStandardIncludePaths()
{
  return gccSetupStandardIncludePaths(true);
}

QVector<rpp::pp_macro*> computeGccStandardMacros(bool withStdCpp0x = true)
{
    QVector<rpp::pp_macro*> ret;
    //Get standard macros from gcc
    KProcess proc;
    proc.setOutputChannelMode(KProcess::MergedChannels);

    // The output of the following gcc commands is several line in the format:
    // "#define MACRO [definition]", where definition may or may not be present.
    // Parsing each line sequentially, we can easily build the macro set.
    proc << "gcc";
    if (withStdCpp0x) {
        // see also: https://bugs.kde.org/show_bug.cgi?id=298252
        proc << "-std=c++0x";
    }
    proc << "-xc++" << "-E" << "-dM" <<NULL_DEVICE;

    if (proc.execute(5000) == 0) {
        QString line;
        while (proc.canReadLine()) {
            QByteArray buff = proc.readLine();
            if (!buff.isEmpty()) {
                line = buff;
                if (line.startsWith("#define ")) {
                    line = line.right(line.length() - 8).trimmed();
                    int pos = line.indexOf(' ');
                    
                    ret.append(new rpp::pp_macro);
                    
                    rpp::pp_macro& macro(*ret.back());
                    if (pos != -1) {
                        macro.name = IndexedString( line.left(pos) );
                        macro.setDefinitionText( line.right(line.length() - pos - 1).toUtf8() );
                    } else {
                        macro.name = IndexedString( line );
                    }
                }
            }
        }
    } else if (withStdCpp0x) {
        // fallback to macro computation without -std=c++0x arg for old gcc versions
        return computeGccStandardMacros(false);
    } else {
        kDebug(9007) <<"Unable to read standard c++ macro definitions from gcc:" <<QString(proc.readAll()) ;
    }
    return ret;
}

const QVector<rpp::pp_macro*>& gccStandardMacros()
{
  static QVector<rpp::pp_macro*> macros = computeGccStandardMacros();
  return macros;
}

}

