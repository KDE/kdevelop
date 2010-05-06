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

namespace CppTools {

QStringList setupStandardIncludePaths()
{
    QStringList includePaths;
    
    KProcess proc;
    proc.setOutputChannelMode(KProcess::MergedChannels);
    proc.setTextModeEnabled(true);

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
    proc <<"gcc" <<"-xc++" <<"-E" <<"-v" <<"/dev/null";

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
    } else {
        kDebug(9007) <<"Unable to read standard c++ macro definitions from gcc:" <<QString(proc.readAll()) ;
    }
    
    return includePaths;
}

void insertMacro(Cpp::ReferenceCountedMacroSet& macros, const rpp::pp_macro& macro)
{
  macros.insert(macro);
}

QVector<rpp::pp_macro*> computeGccStandardMacros()
{
    QVector<rpp::pp_macro*> ret;
    //Get standard macros from gcc
    KProcess proc;
    proc.setOutputChannelMode(KProcess::MergedChannels);
    proc.setTextModeEnabled(true);

    // The output of the following gcc commands is several line in the format:
    // "#define MACRO [definition]", where definition may or may not be present.
    // Parsing each line sequentially, we can easily build the macro set.
    proc <<"gcc" <<"-xc++" <<"-E" <<"-dM" <<"/dev/null";

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

Cpp::ReferenceCountedMacroSet setupStandardMacros()
{
    Cpp::ReferenceCountedMacroSet macros;
    //Add some macros to be compatible with the gnu c++ compiler
    //Used in several headers like sys/time.h
    insertMacro( macros, rpp::pp_macro("__restrict") );
    insertMacro( macros, rpp::pp_macro("__extension__") );
    //Add macros that allow special treatment when within kdevelop
    insertMacro( macros, rpp::pp_macro("IN_KDEVELOP_PARSER") );
    insertMacro( macros, rpp::pp_macro("IN_IDE_PARSER") );
    {
      //Used in several headers like sys/time.h
      rpp::pp_macro m("__const");
      m.setDefinitionText( "const" );
      insertMacro( macros, m );
    }
    {
      rpp::pp_macro m("__null");
      m.setDefinitionText( "0" );
      insertMacro( macros, m );
    }

    {
      //Used in several gcc headers
      rpp::pp_macro m("__inline");
      m.setDefinitionText( "inline" );
      insertMacro( macros, m );
      m.name = IndexedString("__always_inline");
      insertMacro( macros, m );
    }

    {
      //It would be better if the parser could deal with it, for example in class declarations. However it cannot.
      //If we wouldn't need this, macros could be more transparent.
      rpp::pp_macro m("__attribute__");
      m.function_like = true;
      m.formalsList().append(IndexedString("param"));
      insertMacro( macros, m );
    }
    
    /// The following macros are required for qt only. That's why we set them to become active only when their
    /// 'real' versions become defined in qobjectdefs.h. A slight problem is that they are 'fixed', so they will block
    /// any other macros with the same names.
    {
      rpp::pp_macro m("Q_SLOTS");
      m.setDefinitionText( "__qt_slots__" );
      
      m.defined = false;
      m.defineOnOverride = true;
      m.file = IndexedString("/qobjectdefs.h"); // Only define the macro if it is overriden in this file
      m.fixed = true;
      
      insertMacro( macros, m );

      m.name = IndexedString("Q_PRIVATE_SLOT");
      m.formalsList().append(IndexedString("d"));
      m.formalsList().append(IndexedString("sig"));
      m.function_like = true;
      m.setDefinitionText( "private slots: sig{ d; }; private:" );
      insertMacro( macros, m );
      
      m.name = IndexedString("slots");
      m.setDefinitionText("__qt_slots__");
      m.formalsList().clear();
      m.function_like = false;
      insertMacro( macros, m );

      m.name = IndexedString("Q_SIGNALS");
      m.setDefinitionText( "__qt_signals__" );
      m.formalsList().clear();
      insertMacro( macros, m );

      m.name = IndexedString("signals");
      m.setDefinitionText("__qt_signals__");
      m.formalsList().clear();
      insertMacro( macros, m );

      m.name = IndexedString("SIGNAL");
      m.setDefinitionText("__qt_signal__");
      m.formalsList().clear();
      insertMacro( macros, m );
      
      m.name = IndexedString("SLOT");
      m.setDefinitionText("__qt_slot__");
      m.formalsList().clear();
      insertMacro( macros, m );

      m.name = IndexedString("Q_PROPERTY");
      m.setDefinitionText("__qt_property__");
      m.formalsList().clear();
      insertMacro( macros, m );
    }
    
    {
      // We don't provide a real implementation of offsetof, but at least provide a stub that allows correct use-building for the member.
      rpp::pp_macro m("__builtin_offsetof");
      m.function_like = true;
      m.formalsList().append(IndexedString("TYPE"));
      m.formalsList().append(IndexedString("MEMBER"));
      m.setDefinitionText("(size_t)((void)TYPE::MEMBER)");
      insertMacro( macros, m );
    }
    
    foreach(const rpp::pp_macro* macro, gccStandardMacros())
      insertMacro(macros, *macro);
    
    return macros;
}

}

