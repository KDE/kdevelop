/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
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

class URLUtil {

public:

  enum SlashesPosition { SLASH_PREFIX = 1, SLASH_SUFFIX = 2 };

  /**
   * Returns the filename part of a path (i.e. everything past the last slash)
   */
  static QString filename(const QString & name);
  /**
   * Returns the directory part of a path (i.e. everything up to but not including the last slash)
   */
  static QString directory(const QString & name);
  /**
   * Returns the relative path between a parent and child URL, or blank if the specified child is not a child of parent
   */
  static QString relativePath(const KURL & parent, const KURL & child, uint slashPolicy = SLASH_PREFIX);
  /**
   * Returns the relative path between a parent and child URL, or blank if the specified child is not a child of parent
   */
  static QString relativePath(const QString & parent, const QString & child, uint slashPolicy = SLASH_PREFIX);
  /**
   * Returns the relative path between a directory and file. Should never return empty path.
   *  Example:
   *   dirUrl:  /home/test/src
   *   fileUrl: /home/test/lib/mylib.cpp
   *  returns:  ../lib/mylib.cpp
   */
  static QString relativePathToFile( const QString & dirUrl, const QString & fileUrl );
  /**
   *Returns the path 'up one level' - the opposite of what filename returns
   */
  static QString upDir(const QString & path, bool slashSuffix = false);
  /**
   * 'Merges' URLs - changes a URL that starts with dest to start with source instead
   *   Example:
   *     source is /home/me/
   *     dest is /home/you/
   *     child is /home/you/dir1/file1
   *   returns /home/me/dir1/fil1
   */
  static KURL mergeURL(const KURL & source, const KURL & dest, const KURL & child);
  /**
   * Returns the file extension for a filename or path
   */
  static QString getExtension(const QString & path);

  /**
  * Given a base directory url in @p baseDirUrl and the url referring to a date sub-directory or file,
  * it will return the path relative to @p baseDirUrl.
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
  static QString extractPathNameRelative(const KURL &baseDirUrl, const KURL &url );
  static QString extractPathNameRelative(const QString &basePath, const KURL &url );

  /**
  * Will return the absolute path name referred in @p url.
  * Look at above as an example.
  */
  static QString extractPathNameAbsolute( const KURL &url );

  /**
  * Returns a QStringList of relative (to @p baseDir) paths from a list of KURLs in @p urls
  */
  static QStringList toRelativePaths( const QString &baseDir, const KURL::List &urls);

  /**
  * If @p url is a directory will return true, false otherwise.
  */
  static bool isDirectory( const KURL &url );

  /**
  *
  */
  static void dump( const KURL::List &urls );


};

#endif
