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
#include "debug.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <language/codegen/coderepresentation.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/util/includeitem.h>

#include <util/foregroundlock.h>

#include <project/projectmodel.h>

#include <QDirIterator>
#include <QThread>
#include <QCoreApplication>

Q_LOGGING_CATEGORY(CPP, "kdevelop.languages.cpp")

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
  //qCDebug(CPP) << "base: " << base << ", ext: " << ext << endl;
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
//     qCDebug(CPP) << "Trying " << ( *it ) << endl;
    if ( QFileInfo( *it ).exists() )
    {
//       qCDebug(CPP) << "using: " << *it << endl;
      return * it;
    }
  }

  if(fast)
    return QString();

  //qCDebug(CPP) << "Now searching in project files." << endl;
  // Our last resort: search the project file list for matching files

  QFileInfo candidateFileWoExt;
  QString candidateFileWoExtString;

  const IndexedString file(path);
  foreach (KDevelop::IProject *project, ICore::self()->projectController()->projects()) {
      if (project->inProject(file)) {
        foreach(const IndexedString& otherFile, project->fileSet()) {
          candidateFileWoExt.setFile(otherFile.str());
          //qCDebug(CPP) << "candidate file: " << otherFile.str() << endl;
          if( !candidateFileWoExt.suffix().isEmpty() )
            candidateFileWoExtString = candidateFileWoExt.fileName().replace( "." + candidateFileWoExt.suffix(), "" );

          if ( candidateFileWoExtString == fileNameWoExt )
          {
            if ( possibleExts.contains( candidateFileWoExt.suffix() ) || candidateFileWoExt.suffix().isEmpty() )
            {
              //qCDebug(CPP) << "checking if " << url << " exists" << endl;
              return otherFile.str();
            }
          }
        }
      }
  }

  return QString();
}


bool isHeader(const QUrl &url) {
  QFileInfo fi( url.toLocalFile() );
  QString path = fi.filePath();
  // extract the exension
  QString ext = fi.suffix();
  if ( ext.isEmpty() )
    return true;

  return headerExtensions().contains(ext);
}

const Cpp::ReferenceCountedMacroSet& standardMacros()
{
  static Cpp::ReferenceCountedMacroSet macros = CppTools::setupStandardMacros();
  return macros;
}

QPair<Path, Path> findInclude(const Path::List& includePaths, const Path& localPath,
                              const QString& includeName, int includeType,
                              const Path& skipPath, bool quiet){
    QPair<Path, Path> ret;
#ifdef DEBUG
    qCDebug(CPP) << "searching for include-file" << includeName;
    if( !skipPath.isEmpty() )
        qCDebug(CPP) << "skipping path" << skipPath;
#endif

    if (includeName.startsWith('/')) {
        QFileInfo info(includeName);
        if (info.exists() && info.isReadable() && info.isFile()) {
            //qCDebug(CPP) << "found include file:" << info.absoluteFilePath();
            ret.first = Path(info.canonicalFilePath());
            ret.second = Path("/");
            return ret;
        }
    }

    if (includeType == rpp::Preprocessor::IncludeLocal && localPath != skipPath) {
        Path check(localPath, includeName);
        QFileInfo info(check.toLocalFile());
        if (info.exists() && info.isReadable() && info.isFile()) {
            //qCDebug(CPP) << "found include file:" << info.absoluteFilePath();
            ret.first = check;
            ret.second = localPath;
            return ret;
        }
    }

    //When a path is skipped, we will start searching exactly after that path
    bool needSkip = skipPath.isValid();

restart:
    foreach( const Path& path, includePaths ) {
        if( needSkip ) {
            if( path == skipPath ) {
                needSkip = false;
                continue;
            }
        }

        Path check(path, includeName);
        QFileInfo info(check.toLocalFile());

        if (info.exists() && info.isReadable() && info.isFile()) {
            //qCDebug(CPP) << "found include file:" << info.absoluteFilePath();
            ret.first = check;
            ret.second = path;
            return ret;
        }
    }

    if( needSkip ) {
        //The path to be skipped was not found, so simply start from the begin, considering any path.
        needSkip = false;
        goto restart;
    }

    const int idx = includeName.indexOf('/');
    if ( idx != -1 ) {
      // HACK: parse Qt4 includes and similar even without the full include paths from the project manager
      // there, a file in /usr/include/qt4/QtCore/ tries to include sibling files via QtCore/file
      ret = findInclude(includePaths, localPath, includeName.mid(idx + 1), rpp::Preprocessor::IncludeLocal, skipPath, false);
    }

    if( !ret.first.isValid())
    {
        //Check if there is an available artificial representation
        if(!includeName.isNull() && artificialCodeRepresentationExists(IndexedString(includeName)))
        {
            ret.first = Path(CodeRepresentation::artificialPath(includeName));
            qCDebug(CPP) << "Utilizing Artificial code for include: " << includeName;
        }
        else if(!quiet ) {
            qCDebug(CPP) << "FAILED to find include-file" << includeName << "in paths:" << includePaths;
        }
    }

    return ret;
}

bool needsUpdate(const Cpp::EnvironmentFilePointer& file, const Path& localPath, const Path::List& includePaths)
{
  if(file->needsUpdate())
    return true;

  ///@todo somehow this check should also go into EnvironmentManager
  for( Cpp::ReferenceCountedStringSet::Iterator it = file->missingIncludeFiles().iterator(); it; ++it ) {

    ///@todo store IncludeLocal/IncludeGlobal somewhere
    auto included = findInclude( includePaths, localPath, (*it).str(), rpp::Preprocessor::IncludeLocal, Path(), true );
    if(included.first.isValid())
      return true;
  }

  return false;
}

/**
 * This is an ugly HACK but well... it works and doesn't require an extremely big API refactoring.
 *
 * Thing is, the findIncludePaths was potentially called from background threads, mostly during
 * code completion. From there, it is not safe to call IncludePathComputer::computeForeground.
 * To fix this, we now add another eventloop roundtrip, execute that function in the foreground
 * and then afterwards compute the rest in the background. This does mean that the background thread
 * is _blocked_ while we compute stuff in the foreground.
 *
 * This must be properly fixed when we port the MissingInclude feature to clang. There, the
 * include paths must be queried in the foreground thread, before invoking code completion.
 */
class IncludePathForegroundComputer : public DoInForeground
{
public:
    IncludePathForegroundComputer(IncludePathComputer* includePathComputer)
      : m_includePathComputer(includePathComputer)
    {}

private:
    void doInternal() override final
    {
      m_includePathComputer->computeForeground();
    }

    IncludePathComputer* m_includePathComputer;
};

Path::List findIncludePaths(const QString& source)
{
  Q_ASSERT(QThread::currentThread() == qApp->thread() ||
           (!DUChain::lock()->currentThreadHasReadLock() && !DUChain::lock()->currentThreadHasWriteLock()));

  IncludePathComputer comp(source);
  IncludePathForegroundComputer foreground(&comp);
  foreground.doIt();
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
        foreach(const Path& path, findIncludePaths(source)) {
          paths += path.toLocalFile();
        }

        if(local) {
            QUrl localPath = QUrl::fromLocalFile(source);
            localPath = localPath.adjusted(QUrl::RemoveFilename);
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
              item.basePath = QUrl::fromLocalFile(path);
            } else {
              item.basePath = QUrl::fromLocalFile(searchPath);
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
