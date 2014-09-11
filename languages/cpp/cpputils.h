/*
   Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CPPUTILS_H
#define CPPUTILS_H

#include "environmentmanager.h"
#include <util/path.h>

namespace KDevelop
{
class IncludeItem;
class Declaration;
class ParsingEnvironment;
}

class QUrl;

namespace CppUtils
{
  
///Returns -1 if there is no #include in this line, else the first position behind the "#include" string, which may also look like " # include" or similar
int findEndOfInclude(QString line);

///If @param fast is true, no exhaustive search is done as fallback.
QString sourceOrHeaderCandidate( const QString &path, bool fast = false );

///Returns true if the given url is a header, looking at he known file extensions
bool isHeader(const QUrl &url);

const Cpp::ReferenceCountedMacroSet& standardMacros();

/// Get the full path for a file based on a search through the project's
/// include directories
/// @param localPath the path from which this findInclude is issued
/// @param skipPath this path will be skipped while searching, as needed for gcc extension #include_next
/// @return first: The found file(not a canonical path, starts with the directory it was found in)
///         second: The include-path the file was found in(can be used to skip that path on #include_next)
QPair<KDevelop::Path, KDevelop::Path> findInclude(const KDevelop::Path::List& includePaths, const KDevelop::Path& localPath,
                                                  const QString& includeName, int includeType, const KDevelop::Path& skipPath,
                                                  bool quiet=false);
    
///Thread-safe
bool needsUpdate(const Cpp::EnvironmentFilePointer& file, const KDevelop::Path& localPath, const KDevelop::Path::List& includePaths );

///Returns the include-path. Each dir has a trailing slash. Search should be iterated forward through the list
// NOTE: _no_ lock must be held when calling this, otherwise you might deadlock since this is a very ugly hacky function
// which must call stuff from the foreground thread...
KDevelop::Path::List findIncludePaths(const QString& source);

/**
  * Returns a list of all files within the include-path of the given file
  * @param addPath This path is added behind each include-path, and the content of the resulting directory used.
  * @param addIncludePaths A list of include-paths that should be used for listing, additionally to the known ones
  * @param onlyAddedIncludePaths If this is true, only the include-paths given in @p addIncludePaths will be used
  * @param prependAddedPathToName If this is true, @p addPath is prepended to each of the returned items paths
  * */
QList<KDevelop::IncludeItem> allFilesInIncludePath(const QString& source, bool local, const QString& addPath,
                                                   const QStringList& addIncludePaths = QStringList(), bool onlyAddedIncludePaths = false,
                                                   bool prependAddedPathToName = false, bool allowSourceFiles = false );

/// @return List of possible header extensions used for definition/declaration fallback switching
QStringList headerExtensions();
/// @return List of possible source extensions used for definition/declaration fallback switching
QStringList sourceExtensions();

}


#endif	//CPPUTILS_H
