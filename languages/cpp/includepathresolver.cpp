/***************************************************************************
   copyright            : (C) 2007 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** Unfortunately this doesn't work with unsermake, but with make and cmake.
/**
 * */
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include "kurl.h" /* defines KURL */
#include "qdir.h" /* defines QDir */
#include "qregexp.h" /* defines QRegExp */
#include "klocale.h" /* defines [function] i18n */
#include "lib/util/blockingkprocess.h" /* defines BlockingKProcess */
#include "includepathresolver.h"

#ifdef TEST
#include "lib/util/blockingkprocess.cpp"

#include <iostream>
using namespace std;
#endif

///After how many seconds should we retry?
#define CACHE_FAIL_FOR_SECONDS 120

using namespace CppTools;

bool IncludePathResolver::executeCommand ( const QString& command, QString& result )
{
  FILE* fp;
  const int BUFSIZE = 2048;
  char buf [BUFSIZE];

  result = QString();

  if ((fp = popen(command.local8Bit(), "r")) == NULL)
    return false;

  while (fgets(buf, sizeof (buf), fp))
    result += QString(buf);

  int stat = pclose(fp);

  return stat == 0;
} 

IncludePathResolver::IncludePathResolver( bool continueEventLoop ) : m_isResolving(false), m_outOfSource(false), m_continueEventLoop(continueEventLoop)  {
}

///More efficient solution: Only do exactly one call for each directory. During that call, mark all source-files as changed, and make all targets for those files.
PathResolutionResult IncludePathResolver::resolveIncludePath( const QString& file ) {
  QFileInfo fi( file );
  return resolveIncludePath( fi.fileName(), fi.dirPath(true) );
}

PathResolutionResult IncludePathResolver::resolveIncludePath( const QString& file, const QString& workingDirectory ) {
  
  struct Enabler {
    bool& b;
    Enabler( bool& bb ) : b(bb) {
      b = true;
    }
    ~Enabler() {
      b = false;
    }
  };

  if( m_isResolving )
    return PathResolutionResult(false, i18n("tried include-path-resolution while another resolution-process was still running") );
  
  Enabler e( m_isResolving );
  ///First check the cache
  QDir dir( workingDirectory );
  dir = QDir( dir.absPath() );
  QFileInfo makeFile( dir, "Makefile" );

  QStringList cachedPath; ///If the call doesn't succeed, use the cached not up-to-date version
  QDateTime makeFileModification = makeFile.lastModified();
  Cache::iterator it = m_cache.find( dir.path() );
  if( it != m_cache.end() ) {
    cachedPath = (*it).path;
    if( makeFileModification == (*it).modificationTime ) {
      if( !(*it).failed ) {
        ///We have a valid cached result
        PathResolutionResult ret(true);
        ret.path = (*it).path;
        return ret;
      } else {
        ///We have a cached failed result. We should use that for some time but then try again. Return the failed result if: ( there were too many tries within this folder OR this file was already tried ) AND The last tries have not expired yet
        if( ((*it).failedFiles.size() > 10 || (*it).failedFiles.find( file ) != (*it).failedFiles.end()) && (*it).failTime.secsTo( QDateTime::currentDateTime() ) < CACHE_FAIL_FOR_SECONDS ) {
          PathResolutionResult ret(true); ///Fake that the result is ok
          ret.path = (*it).path;
          return ret;
        } else {
          ///Try getting a correct result again
        }
      }
    }
  }
  
  QString targetName;
  QFileInfo fi( file );
  
  QString absoluteFile = file;
  if( !file.startsWith("/") )
    absoluteFile = dir.path() + "/" + file;
  KURL u( absoluteFile );
  u.cleanPath();
  absoluteFile = u.path();
  
  int dot;
  if( (dot = file.findRev( '.' )) == -1 )
    return PathResolutionResult( false, i18n( "Filename %1 seems to be malformed" ).arg(file) );

  targetName = file.left( dot ) + ".o";

  QString wd = dir.path();
  if( !wd.startsWith("/") ) {
    wd = QDir::currentDirPath() + "/" + wd;
    KURL u( wd );
    u.cleanPath();
    wd = u.path();
  }
  if( m_outOfSource ) {
    if( wd.startsWith( m_source ) ) {
      ///Move the current working-directory out of source, into the build-system
      wd = m_build + "/" + wd.mid( m_source.length() );
      KURL u( wd );
      u.cleanPath();
      wd = u.path();
    }
  }

  PathResolutionResult res = resolveIncludePathInternal( absoluteFile, wd, targetName );
  if( res ) {
    CacheEntry ce;
    ce.modificationTime = makeFileModification;
    ce.path = res.path;
    m_cache[dir.path()] = ce;

    return res;
  }

  //Try it using a relative path. Which kind is required differs by setup.
  QString relativeFile = KURL::relativePath(wd, absoluteFile);
  
  res = resolveIncludePathInternal( relativeFile, wd, targetName );
    
  if( res.path.isEmpty() )
      res.path = cachedPath; //We failed, maybe there is an old cached result, use that.
  
  if( it == m_cache.end() )
    it = m_cache.insert( dir.path(), CacheEntry() );
  
  CacheEntry& ce(*it);
  ce.modificationTime = makeFileModification;
  ce.path = res.path;
  if( !res ) {
    ce.failed = true;
    ce.failTime = QDateTime::currentDateTime();
    ce.failedFiles[file] = true;
  } else {
    ce.failed = false;
    ce.failedFiles.clear();
  }
  
  return res;
}

PathResolutionResult IncludePathResolver::resolveIncludePathInternal( const QString& file, const QString& workingDirectory, const QString& makeParameters ) {

  QString processStdout;
  
  QString command = "make --no-print-directory -W \'" + file + "\' -n " + makeParameters;
  
  QString fullOutput;
  
  if( m_continueEventLoop ) {
    BlockingKProcess proc;
    proc.setWorkingDirectory( workingDirectory );
    proc.setUseShell( true );
    proc << command;
    if ( !proc.start(KProcess::NotifyOnExit, KProcess::Stdout) ) {
      return PathResolutionResult( false, i18n("Couldn't start the make-process") );
    }
    
    fullOutput = proc.stdOut();
    if( proc.exitStatus() != 0 )
      return PathResolutionResult( false, i18n("make-process finished with nonzero exit-status"), i18n("output: %1").arg( fullOutput ) );
  } else {
    char* oldWd = getcwd(0,0);
    chdir( workingDirectory.local8Bit() );
    bool ret = executeCommand(command, fullOutput);
    if( oldWd ) {
      chdir( oldWd );
      free( oldWd );
    }
    if( !ret )
      return PathResolutionResult( false, i18n("make-process failed"), i18n("output: %1").arg( fullOutput ) );
  }
  

  QRegExp newLineRx("\\\\\\n");
  fullOutput.replace( newLineRx, "" );
  ///@todo collect multiple outputs at the same time for performance-reasons
  QString firstLine = fullOutput;
  int lineEnd;
  if( (lineEnd = fullOutput.find('\n')) != -1 )
    firstLine.truncate( lineEnd ); //Only look at the first line of output
  
  /**
   * There's two possible cases this can currently handle.
   * 1.: gcc is called, with the parameters we are searching for(so we parse the parameters)
   * 2.: A recursive make is called, within another directory(so we follow the recursion and try again) "cd /foo/bar && make -f pi/pa/build.make pi/pa/po.o
   * */


  ///STEP 1: Test if it is a recursive make-call
  QRegExp makeRx( "\\bmake\\s" );
  int offset = 0;
  while( (offset = makeRx.search( firstLine, offset )) != -1 )
  {
    QString prefix = firstLine.left( offset ).stripWhiteSpace();
    if( prefix.endsWith( "&&") || prefix.endsWith( ";" ) || prefix.isEmpty() )
    {
      QString newWorkingDirectory = workingDirectory;
      ///Extract the new working-directory
      if( !prefix.isEmpty() ) {
        if( prefix.endsWith( "&&" ) )
          prefix.truncate( prefix.length() - 2 );
        else if( prefix.endsWith( ";" ) )
          prefix.truncate( prefix.length() - 1 );
        ///Now test if what we have as prefix is a simple "cd /foo/bar" call.
        if( prefix.startsWith( "cd ") && !prefix.contains( ";") && !prefix.contains("&&") ) {
          newWorkingDirectory = prefix.right( prefix.length() - 3 ).stripWhiteSpace();
          if( !newWorkingDirectory.startsWith("/") )
            newWorkingDirectory = workingDirectory + "/" + newWorkingDirectory;
          KURL u( newWorkingDirectory );
          u.cleanPath();
          newWorkingDirectory = u.path();
        }
      }
      QFileInfo d( newWorkingDirectory );
      if( d.exists() ) {
        ///The recursive working-directory exists.
        QString makeParams = firstLine.mid( offset+5 );
        if( !makeParams.contains( ";" ) && !makeParams.contains( "&&" ) ) {
          ///Looks like valid parameters
          ///Make the file-name absolute, so it can be referenced from any directory
          QString absoluteFile = file;
          if( !absoluteFile.startsWith("/") )
            absoluteFile = workingDirectory +  "/" + file;
          KURL u( absoluteFile );
          u.cleanPath();
          ///Try once with relative, and once with absolute path
          PathResolutionResult res = resolveIncludePathInternal( u.path(), newWorkingDirectory, makeParams );
          if( res )
            return res;
          return resolveIncludePathInternal( KURL::relativePath(newWorkingDirectory,u.path()), newWorkingDirectory, makeParams );
        }else{
          return PathResolutionResult( false, i18n("Recursive make-call failed"), i18n("The parameter-string \"%1\" does not seem to be valid. Output was: %2").arg(makeParams).arg(fullOutput) );
        }
      } else {
        return PathResolutionResult( false, i18n("Recursive make-call failed"), i18n("The directory \"%1\" does not exist. Output was: %2").arg(newWorkingDirectory).arg(fullOutput) );
      }

    } else {
      return PathResolutionResult( false, i18n("Recursive make-call malformed"), i18n("Output was: %2").arg(fullOutput) );
    }

    ++offset;
    if( offset >= firstLine.length() ) break;
  }

  ///STEP 2: Search the output for include-paths
  QRegExp validRx( "\\b([cg]\\+\\+|gcc)\\s" );
  ///@todo fix this regular expression(it must not break on escaped spaces)
  QRegExp pathEndRx( "\\s");//( [^\\](\\\\\\\\)*)[\\s]" ); ///Regular expression to find the end of an include-path without triggering at an escaped white-space
  if( validRx.search( fullOutput ) == -1 )
    return PathResolutionResult( false, i18n("Output seems not to be a valid gcc or g++ call"), i18n("Folder: \"%1\"  Command: \"%2\"Output: \"%3\"").arg(workingDirectory).arg(command).arg(fullOutput) );

  PathResolutionResult ret( true );
  ret.longErrorMessage = fullOutput;
  
  QRegExp includeRx("\\s(-I|--include-dir=)");
  offset = 0;
  while( (offset = includeRx.search( fullOutput, offset )) != -1 ) {
    offset += 1;
    int pathOffset = 2;
    if( fullOutput[offset+1] == '-' ) {
      ///Must be --include-dir=, with a length of 14 characters
      pathOffset = 14;
    }
    if( fullOutput.length() <= offset + pathOffset )
      break;
    
    if( fullOutput[offset+pathOffset].isSpace() )
      pathOffset++;
    
    

    int start = offset + pathOffset;
    int end = pathEndRx.search( fullOutput, start );
    if( end == -1 ) break; //Maybe a warning? Something went wrong
    end+= pathEndRx.matchedLength();
    QString path = fullOutput.mid( start, end-start ).stripWhiteSpace();;
    if( !path.startsWith("/") )
      path = workingDirectory + (workingDirectory.endsWith("/") ? "" : "/") + path;
    
    KURL u( path );
    u.cleanPath();
      
    ret.path << u.path();

    offset = end-1;
  }

  
  return ret;
}

void IncludePathResolver::setOutOfSourceBuildSystem( const QString& source, const QString& build ) {
  m_outOfSource = true;
  m_source = source;
  m_build = build;
}

#ifdef TEST
/** This can be used for testing and debugging the system. To compile it use
 * gcc includepathresolver.cpp -I /usr/share/qt3/include -I /usr/include/kde -I ../.. -DTEST -lkdecore -g -o includepathresolver
 * */
         
int main(int argc, char **argv) {
  QApplication app(argc,argv);
  IncludePathResolver resolver;
  if( argc < 3 ) {
    cout << "params: 1. file-name, 2. working-directory [3. source-directory 4. build-directory]" << endl;
    return 1;
  }
  if( argc >= 5 ) {
    cout << "mapping " << argv[3] << " -> " << argv[4] << endl;
    resolver.setOutOfSourceBuildSystem( argv[3], argv[4] );
  }
  PathResolutionResult res = resolver.resolveIncludePath( argv[1], argv[2] );
  cout << "success: " << res.success << "\n";
  if( !res.success ) {
    cout << "error-message: \n" << res.errorMessage << "\n";
    cout << "long error-message: \n" << res.longErrorMessage << "\n";
  }
  cout << "path: \n" << res.path.join("\n");
  return res.success;
}

#endif
