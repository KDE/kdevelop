/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _URLUTIL_H_
#define _URLUTIL_H_

#include <qstring.h>
#include <qvaluelist.h>
#include <kurl.h>

namespace URLUtil
{
  enum SlashesPosition { SLASH_PREFIX = 1, SLASH_SUFFIX = 2 };

  /**
   * Returns the filename part of a pathname (i.e. everything past the last slash)
   */
  QString filename(const QString & pathName);
  /**
   * Returns the directory part of a path (i.e. everything up to but not including the last slash)
   */
  QString directory(const QString & pathName);
  /**
   * Returns the relative path between a parent and child URL, or blank if the specified child is not a child of parent
   */
  QString relativePath(const KURL & parent, const KURL & child, uint slashPolicy = SLASH_PREFIX);
  /**
   * Returns the relative path between a parent and child URL, or blank if the specified child is not a child of parent
   */
  QString relativePath(const QString & parent, const QString & child, uint slashPolicy = SLASH_PREFIX);
  /**
   * Returns the relative path between a directory and file. Should never return empty path.
   *  Example:
   *   dirUrl:  /home/test/src
   *   fileUrl: /home/test/lib/mylib.cpp
   *  returns:  ../lib/mylib.cpp
   */
  QString relativePathToFile( const QString & dirUrl, const QString & fileUrl );
  /**
   *Returns the path 'up one level' - the opposite of what filename returns
   */
  QString upDir(const QString & path, bool slashSuffix = false);
  /**
   * 'Merges' URLs - changes a URL that starts with dest to start with source instead
   *   Example:
   *     source is /home/me/
   *     dest is /home/you/
   *     child is /home/you/dir1/file1
   *   returns /home/me/dir1/fil1
   */
  KURL mergeURL(const KURL & source, const KURL & dest, const KURL & child);
  /**
   * Returns the file extension for a filename or path
   */
  QString getExtension(const QString & path);

  /**
  * Given a base directory url in @p baseDirUrl and the url referring to a date sub-directory or file,
  * it will return the path relative to @p baseDirUrl.
  * If baseDirUrl == url.path() then it will return ".".
  * <code>
  * KURL baseUrl, dirUrl;
  * baseUrl.setPath( "/home/mario/src/kdevelop/" );
  * dirUrl.setPath( "/home/mario/src/kdevelop/parts/cvs/" );
  * QString relPathName = extractDirPathRelative( baseUrl, url ); // == "parts/cvs/"
  * QString absPathName = extractDirPathAbsolute( url ); // == "/home/mario/src/kdevelop/parts/cvs/"
  * </code>
  * Note that if you pass a file name in @p url (instead of a directory) or the @p baseUrl is not contained
  * in @p url then the function will return "" (void string).
  */
  QString extractPathNameRelative(const KURL &baseDirUrl, const KURL &url );
  QString extractPathNameRelative(const QString &basePath, const KURL &url );

  /**
  * Will return the absolute path name referred in @p url.
  * Look at above for an example.
  */
  QString extractPathNameAbsolute( const KURL &url );

  /**
  * Returns a QStringList of relative (to @p baseDir) paths from a list of KURLs in @p urls
  */
  QStringList toRelativePaths( const QString &baseDir, const KURL::List &urls);

  /**
  * If @p url is a directory will return true, false otherwise.
  */
  bool isDirectory( const KURL &url );

  /**
  * Will dump the list of KURL @p urls on standard output, eventually printing @ aMessage if it
  * is not null.
  */
  void dump( const KURL::List &urls, const QString &aMessage = QString::null );

  /**
   * Same as QDir::canonicalPath in later versions of QT. Earlier versions of QT
   * had this broken, so it's reproduced here.
   */
  QString canonicalPath( const QString & path );


};

#endif
