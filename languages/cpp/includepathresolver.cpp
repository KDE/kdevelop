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

using namespace CppTools;

IncludePathResolver::IncludePathResolver() : m_outOfSource(false) {
}

PathResolutionResult IncludePathResolver::resolveIncludePath( const QString& file, const QString& workingDirectory ) {
  QString targetName;
  QFileInfo fi( file );
  
  QString absoluteFile = file;
  if( !file.startsWith("/") )
    absoluteFile = workingDirectory + "/" + file;
  KURL u( absoluteFile );
  u.cleanPath();
  absoluteFile = u.path();
  
  int dot;
  if( (dot = file.findRev( '.' )) == -1 )
    return PathResolutionResult( false, i18n( "Filename %1 seems to be malformed" ).arg(file) );

  targetName = file.left( dot ) + ".o";

  QString wd = workingDirectory;
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
  if( res )
    return res;

  //Try it using a relative path. Unfortunately, which is required differs by setup.
  QString relativeFile = absoluteFile;
  if( relativeFile.startsWith( wd ) )
    relativeFile = relativeFile.mid( wd.length() );
  while( relativeFile.startsWith("/") )
    relativeFile = relativeFile.mid(1);
  
  return resolveIncludePathInternal( relativeFile, wd, targetName );
}

PathResolutionResult IncludePathResolver::resolveIncludePathInternal( const QString& file, const QString& workingDirectory, const QString& makeParameters ) {

  QString processStdout;
  BlockingKProcess proc;

  proc.setWorkingDirectory( workingDirectory );
  proc.setUseShell( true );
  QString command = "make --no-print-directory -W " + proc.quote(file) + " -n " + makeParameters;
  cout << "calling " << command << endl;
  proc << command;
  if ( !proc.start(KProcess::NotifyOnExit, KProcess::Stdout) ) {
    return PathResolutionResult( false, i18n("Couldn't start the make-process") );
  }
  
  QString fullOutput = proc.stdOut();
  
  if( proc.exitStatus() != 0 )
    return PathResolutionResult( false, i18n("make-process finished with nonzero exit-status"), i18n("output: %1").arg( fullOutput ) );

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
          return resolveIncludePathInternal( u.path(), newWorkingDirectory, makeParams );
          
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
  QRegExp pathEndRx( "\\s");//( [^\\](\\\\\\\\)*)[\\s]" ); ///Regular expression to find the end of an include-path without triggering at an escaped white-space
  if( validRx.search( fullOutput ) == -1 )
    return PathResolutionResult( false, i18n("Output seems not to be a valid gcc or g++ call"), i18n("Folder: %1\nCommand: \"%2\"\nOutput: \"%3\"").arg(workingDirectory).arg(command).arg(fullOutput) );

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