/* This file is part of the KDE project
   Copyright (C) 2003 Julian Rockey <linux@jrockey.com>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _URLUTIL_H_
#define _URLUTIL_H_

#include <qstring.h>
#include <qvaluelist.h>
#include <kurl.h>

/**
@file urlutil.h
Utility functions to operate on URLs.
*/

/**Utility functions to operate on URLs.*/
namespace URLUtil
{
  /**Position of a slash in the URL.*/
  enum SlashesPosition { 
      SLASH_PREFIX = 1    /**<URL has slash as a prefix.*/,
      SLASH_SUFFIX = 2    /**<URL has slash as a suffix.*/ 
  };

  /**
   * @return The filename part of a pathname (i.e. everything past the last slash).
   * @param pathName The absolute path to a file.
   */
  QString filename(const QString & pathName);
  /**
   * @return The directory part of a path (i.e. everything up to but not including the last slash)
   * @param pathName The absolute path to a directory.
   */
  QString directory(const QString & pathName);
  /**
   * @return The relative path between a parent and child URL, or blank if the specified 
   * child is not a child of parent.
   * @param parent The parent URL.
   * @param child The child URL.
   * @param slashPolicy If parent and child are equal then the function returns "/" if 
   * slashPolicy contains SLASH_PREFIX and otherwise "".\n"/" is appended to a result
   * if slashPolicy contains SLASH_SUFFIX.\n"/" is prepended to a result if 
   * slashPolicy contains SLASH_PREFIX.
   */
  QString relativePath(const KURL & parent, const KURL & child, uint slashPolicy = SLASH_PREFIX);
  /**
   * @return The relative path between a parent and child URL, or blank if the specified 
   * child is not a child of parent.
   * @param parent The parent URL.
   * @param child The child URL.
   * @param slashPolicy If parent and child are equal then the function returns "/" if 
   * slashPolicy contains SLASH_PREFIX and otherwise "".\n"/" is appended to a result
   * if slashPolicy contains SLASH_SUFFIX.\n"/" is prepended to a result if 
   * slashPolicy contains SLASH_PREFIX.
   */
  QString relativePath(const QString & parent, const QString & child, uint slashPolicy = SLASH_PREFIX);
  /**
   * @param dirUrl An URL of a directory.
   * @param fileUrl An URL of a file.
   * @return The relative path between a directory and file. Should never return empty path.\n
   *  <pre>Example:
   *   dirUrl:  /home/test/src
   *   fileUrl: /home/test/lib/mylib.cpp
   *  returns:  ../lib/mylib.cpp</pre>
   */
  QString relativePathToFile( const QString & dirUrl, const QString & fileUrl );
  /**
   *@param path A path (absolute or relative).
   *@param slashSuffix if true then "/" is appended to a path.
   *@returns The path 'up one level' - the opposite of what filename returns.
   */
  QString upDir(const QString & path, bool slashSuffix = false);
  /**
   * 'Merges' URLs - changes a URL that starts with dest to start with source instead.\n
   *   <pre>Example:
   *     source is /home/me/
   *     dest is /home/you/
   *     child is /home/you/dir1/file1
   *   returns /home/me/dir1/fil1</pre>
   * @param source An URL of a source.
   * @param dest An URL of a destination.
   * @param child An URL to change.
   * @return The result of merge.
   */
  KURL mergeURL(const KURL & source, const KURL & dest, const KURL & child);
  /**
   * @return The file extension for a filename or path.
   * @param path Absolute or relative path.
   */
  QString getExtension(const QString & path);

  /**
  * Given a base directory url in @p baseDirUrl and the url referring to the
  * sub-directory or file, it will return the path relative to @p baseDirUrl.
  * If baseDirUrl == url.path() then it will return ".".
  * @code
  * KURL baseUrl, dirUrl;
  * baseUrl.setPath( "/home/mario/src/kdevelop/" );
  * dirUrl.setPath( "/home/mario/src/kdevelop/parts/cvs/" );
  * QString relPathName = extractDirPathRelative( baseUrl, url ); // == "parts/cvs/"
  * QString absPathName = extractDirPathAbsolute( url ); // == "/home/mario/src/kdevelop/parts/cvs/"
  * @endcode
  * Note that if you pass a file name in @p url (instead of a directory) or the 
  * @p baseUrl is not contained in @p url then the function will return "" (void string).
  *
  * @param baseDirUrl Base directory URL.  
  * @param url Base directory URL.
  * @return The relative path between @p url and @p baseDirUrl.
  */
  QString extractPathNameRelative(const KURL &baseDirUrl, const KURL &url );
  /**Same as above. @p basePath is QString.*/
  QString extractPathNameRelative(const QString &basePath, const KURL &url );
  /**Same as above. Both @p basePath and @p absFilePath are QStrings.*/
  QString extractPathNameRelative(const QString &basePath, const QString &absFilePath );

  /**
  * @param url The url to extract the absolute path from.
  * @return The absolute path name referred in @p url.
  * Look at @ref extractPathNameRelative documentation for an example.
  */
  QString extractPathNameAbsolute( const KURL &url );

  /**
  * @param baseDir Base directory for relative URLs.
  * @param urls The list of urls to extract the relative paths from.
  * @return A QStringList of relative (to @p baseDir) paths from a list of KURLs in @p urls.
  */
  QStringList toRelativePaths( const QString &baseDir, const KURL::List &urls);

  /**
  * @param url The absolute URL.
  * @return true if @p url is a directory, false otherwise.
  */
  bool isDirectory( const KURL &url );
  /**
  * @param absFilePath The absolute path.
  * @return true if @p url is a directory, false otherwise.
  */
  bool isDirectory( const QString &absFilePath );

  /**
  * Dumps the list of KURL @p urls on standard output, eventually printing @p aMessage if it
  * is not null.
  * @param urls URLs to dump.
  * @param aMessage Message to be written onto a stdout.
  */
  void dump( const KURL::List &urls, const QString &aMessage = QString::null );

  /**
   * Same as QDir::canonicalPath in later versions of Qt. Earlier versions of Qt
   * had this broken, so it's reproduced here.
   */
  QString canonicalPath( const QString & path );

    /**
     * Performs environment variable expansion on @p variable.
     *
     * @param variable The string with the environment variable to expand.
     * @return The expanded environment variable value. if the variable
     *         cannot be expanded, @p variable itself is returned.
     */
    QString envExpand ( const QString &variable );

}

#endif
