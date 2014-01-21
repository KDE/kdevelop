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

#include "cpputils.h"

#include "setuphelpers.h"
#include "parser/rpp/preprocessor.h"
#include "includepathcomputer.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <language/codegen/coderepresentation.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchain.h>
#include <language/util/includeitem.h>

#include <project/projectmodel.h>

#include <QDirIterator>

template<class T>
static QList<T> makeListUnique(const QList<T>& list)
{
  QList<T> ret;

  QSet<QString> set;
  foreach(const T& item, list)
  {
    if(!set.contains(item))
    {
      ret << item;
      set.insert(item);
    }
  }
  
  return ret;
}

QString addDot(QString ext) {
  if(ext.contains('.')) //We need this check because of the _impl.h thing
    return ext;
  else
    return "." + ext;
}

using namespace KDevelop;

namespace CppUtils
{
  
int findEndOfInclude(QString line)
{
  QString tmp = line;
  tmp = tmp.trimmed();
  if(!tmp.startsWith("#"))
    return -1;
  
  tmp = tmp.mid(1).trimmed();

  if(!tmp.startsWith("include"))
    return -1;
  
  return line.indexOf("include") + 7;
}

QString sourceOrHeaderCandidate( const QString &path_, bool fast )
{
// get the path of the currently active document
  QFileInfo fi( path_ );
  QString path = fi.filePath();
  // extract the exension
  QString ext = fi.suffix();
  if ( ext.isEmpty() )
    return QString();
  // extract the base path (full path without '.' and extension)
  QString base = path.left( path.length() - ext.length() - 1 );
  //kDebug( 9007 ) << "base: " << base << ", ext: " << ext << endl;
  // just the filename without the extension
  QString fileNameWoExt = fi.fileName();
  if ( !ext.isEmpty() )
    fileNameWoExt.replace( "." + ext, "" );
  QStringList possibleExts;
  // depending on the current extension assemble a list of
  // candidate files to look for
  QStringList candidates;
  // special case for template classes created by the new class dialog
  if ( path.endsWith( "_impl.h" ) )
  {
    QString headerpath = path;
    headerpath.replace( "_impl.h", ".h" );
    candidates << headerpath;
    fileNameWoExt.replace( "_impl", "" );
    possibleExts << "h";
  }
  // if file is a header file search for implementation file
  else if ( headerExtensions().contains( ext ) )
  {
    foreach(const QString& ext, sourceExtensions())
      candidates << ( base + addDot(ext) );

    possibleExts = sourceExtensions();
  }
  // if file is an implementation file, search for header file
  else if ( sourceExtensions().contains( ext ) )
  {
    foreach(const QString& ext, headerExtensions())
      candidates << ( base + addDot(ext) );

    possibleExts = headerExtensions();
  }
  // search for files from the assembled candidate lists, return the first
  // candidate file that actually exists or QString::null if nothing is found.
  QStringList::ConstIterator it;
  for ( it = candidates.constBegin(); it != candidates.constEnd(); ++it )
  {
//     kDebug( 9007 ) << "Trying " << ( *it ) << endl;
    if ( QFileInfo( *it ).exists() )
    {
//       kDebug( 9007 ) << "using: " << *it << endl;
      return * it;
    }
  }

  if(fast)
    return QString();

  //kDebug( 9007 ) << "Now searching in project files." << endl;
  // Our last resort: search the project file list for matching files

  QFileInfo candidateFileWoExt;
  QString candidateFileWoExtString;

  const IndexedString file(path);
  foreach (KDevelop::IProject *project, ICore::self()->projectController()->projects()) {
      if (project->inProject(file)) {
        foreach(const IndexedString& otherFile, project->fileSet()) {
          candidateFileWoExt.setFile(otherFile.str());
          //kDebug( 9007 ) << "candidate file: " << otherFile.str() << endl;
          if( !candidateFileWoExt.suffix().isEmpty() )
            candidateFileWoExtString = candidateFileWoExt.fileName().replace( "." + candidateFileWoExt.suffix(), "" );

          if ( candidateFileWoExtString == fileNameWoExt )
          {
            if ( possibleExts.contains( candidateFileWoExt.suffix() ) || candidateFileWoExt.suffix().isEmpty() )
            {
              //kDebug( 9007 ) << "checking if " << url << " exists" << endl;
              return otherFile.str();
            }
          }
        }
      }
  }

  return QString();
}


bool isHeader(const KUrl &url) {
  QFileInfo fi( url.toLocalFile() );
  QString path = fi.filePath();
  // extract the exension
  QString ext = fi.suffix();
  if ( ext.isEmpty() )
    return true;
  
  return headerExtensions().contains(ext);
}

QStringList standardIncludePaths()
{
  static QStringList list = CppTools::setupStandardIncludePaths();
  return list;
}

const Cpp::ReferenceCountedMacroSet& standardMacros()
{
  static Cpp::ReferenceCountedMacroSet macros = CppTools::setupStandardMacros();
  return macros;
}

QPair<KUrl, KUrl> findInclude(const KUrl::List& includePaths, const KUrl& localPath, const QString& includeName, int includeType, const KUrl& skipPath, bool quiet){
    QPair<KUrl, KUrl> ret;
#ifdef DEBUG
    kDebug(9007) << "searching for include-file" << includeName;
    if( !skipPath.isEmpty() )
        kDebug(9007) << "skipping path" << skipPath;
#endif

    if (includeName.startsWith('/')) {
        QFileInfo info(includeName);
        if (info.exists() && info.isReadable() && info.isFile()) {
            //kDebug(9007) << "found include file:" << info.absoluteFilePath();
            ret.first = KUrl(info.canonicalFilePath());
            ret.first.cleanPath();
            ret.second = KUrl("/");
            return ret;
        }
    }

    if (includeType == rpp::Preprocessor::IncludeLocal && localPath != skipPath) {
      QString check = localPath.toLocalFile(KUrl::AddTrailingSlash) + includeName;
        QFileInfo info(check);
        if (info.exists() && info.isReadable() && info.isFile()) {
            //kDebug(9007) << "found include file:" << info.absoluteFilePath();
            ret.first = KUrl(info.canonicalFilePath());
            ret.first.cleanPath();
            ret.second = localPath;
            return ret;
        }
    }

    //When a path is skipped, we will start searching exactly after that path
    bool needSkip = !skipPath.isEmpty();

restart:
    foreach( const KUrl& path, includePaths ) {
        if( needSkip ) {
            if( path == skipPath ) {
                needSkip = false;
                continue;
            }
        }

        QString check = path.toLocalFile(KUrl::AddTrailingSlash) + includeName;
        QFileInfo info(check);

        if (info.exists() && info.isReadable() && info.isFile()) {
            //kDebug(9007) << "found include file:" << info.absoluteFilePath();
            ret.first = KUrl(info.canonicalFilePath());
            ret.first.cleanPath();
            ret.second = path.toLocalFile();
            return ret;
        }
    }

    if( needSkip ) {
        //The path to be skipped was not found, so simply start from the begin, considering any path.
        needSkip = false;
        goto restart;
    }

    if( ret.first.isEmpty())
    {
        //Check if there is an available artificial representation
        if(!includeName.isNull() && artificialCodeRepresentationExists(IndexedString(includeName)))
        {
            ret.first = CodeRepresentation::artificialUrl(includeName);
            kDebug() << "Utilizing Artificial code for include: " << includeName;
        }
        else if(!quiet ) {
            kDebug(9007) << "FAILED to find include-file" << includeName << "in paths:";
            foreach( const KUrl& path, includePaths )
                kDebug(9007) << path;
        }
    }
    
    return ret;
}

bool needsUpdate(const Cpp::EnvironmentFilePointer& file, const KUrl& localPath, const KUrl::List& includePaths)
{
  if(file->needsUpdate())
    return true;

  ///@todo somehow this check should also go into EnvironmentManager
  for( Cpp::ReferenceCountedStringSet::Iterator it = file->missingIncludeFiles().iterator(); it; ++it ) {

    ///@todo store IncludeLocal/IncludeGlobal somewhere
    QPair<KUrl, KUrl> included = findInclude( includePaths, localPath, (*it).str(), rpp::Preprocessor::IncludeLocal, QUrl(), true );
    if(!included.first.isEmpty())
      return true;
  }

  return false;
}

KUrl::List findIncludePaths(const KUrl& source)
{
  IncludePathComputer comp(source);
  comp.computeForeground();
  comp.computeBackground();
  return comp.result();
}

QList<KDevelop::IncludeItem> allFilesInIncludePath(const QString& source, bool local, const QString& addPath,
                                                   const QStringList& addIncludePaths, bool onlyAddedIncludePaths,
                                                   bool prependAddedPathToName, bool allowSourceFiles)
{
    QList<KDevelop::IncludeItem> ret;

    QStringList paths;
    if ( addPath.startsWith('/') ) {
      paths << QString("/");
    } else {
      paths = addIncludePaths;
      if(!onlyAddedIncludePaths) {
        foreach(const KUrl& path, findIncludePaths(source)) {
          paths += path.toLocalFile();
        }

        if(local) {
            KUrl localPath(source);
            localPath.setFileName(QString());
            paths.push_front(localPath.toLocalFile());
        }
      }
    }

    paths = makeListUnique<QString>(paths);
    int pathNumber = 0;

    QSet<QString> hadIncludePaths;
    foreach(const QString& path, paths)
    {
        if(hadIncludePaths.contains(path)) {
          continue;
        }
        hadIncludePaths.insert(path);
        QString searchPath = path;
        if (!addPath.isEmpty() && !addPath.startsWith('/')) {
          if (!searchPath.endsWith('/')) {
            searchPath += '/';
          }
          searchPath += addPath;
        }

        QDirIterator dirContent(searchPath);

        while(dirContent.hasNext()) {
             dirContent.next();
            KDevelop::IncludeItem item;
            item.name = dirContent.fileName();

            if(item.name.startsWith('.') || item.name.endsWith("~")) //This filters out ".", "..", and hidden files, and backups
              continue;
            QString suffix = dirContent.fileInfo().suffix();
            if(!dirContent.fileInfo().suffix().isEmpty() && !headerExtensions().contains(suffix) && (!allowSourceFiles || !sourceExtensions().contains(suffix)))
              continue;
            
            QString fullPath = dirContent.fileInfo().canonicalFilePath();
            if (hadIncludePaths.contains(fullPath)) {
              continue;
            } else {
              hadIncludePaths.insert(fullPath);
            }
            if(prependAddedPathToName) {
              item.name = addPath + item.name;
              item.basePath = path;
            } else {
              item.basePath = searchPath;
            }
            
            item.isDirectory = dirContent.fileInfo().isDir();
            item.pathNumber = pathNumber;

            ret << item;
        }
        ++pathNumber;
    }

    return ret;
}

QStringList headerExtensions()
{
  static const QStringList headerExtensions = QString("h,H,hh,hxx,hpp,tlh,h++").split(',');
  return headerExtensions;
}

QStringList sourceExtensions()
{
  static const QStringList sourceExtensions = QString("c,cc,cpp,c++,cxx,C,m,mm,M,inl,_impl.h").split(',');
  return sourceExtensions;
}

}
