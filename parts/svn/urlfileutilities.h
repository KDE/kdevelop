//
//
// C++ Interface: urlfileutilies
//
// Description: A collection of common used functions, so they can be used by any part dealing with KURLs which needs to convert
// to unix filesystem path.
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef __URLFILEUTILIES_H_
#define __URLFILEUTILIES_H_

#include <kurl.h>

namespace UrlFileUtilities
{
	/**
	* Given a base directory url in @p baseDirUrl and the url referring to a date sub-directory or file,
	* it will return the path relative to @p baseDirUrl.
	* <code>
	* KURL baseUrl, dirUrl;
	* baseUrl.setPath( "/home/mario/src/kdevelop/" );
	* dirUrl.setPath( "/home/mario/src/kdevelop/parts/svn/" );
	* QString relPathName = extractDirPathRelative( baseUrl, url ); // == "parts/svn/"
	* QString absPathName = extractDirPathAbsolute( url ); // == "/home/mario/src/kdevelop/parts/svn/"
	* </code>
	* Note that if you pass a file name in @p url (instead of a directory) or the @p baseUrl is not contained
	* in @p url then the function will return "" (void string).
	**/
	QString extractPathNameRelative(const KURL &baseDirUrl, const KURL &url );
	QString extractPathNameRelative(const QString &basePath, const KURL &url );

	/**
	* Will return the absolute path name referred in @p url.
	* Look at above as an example.
	**/
	QString extractPathNameAbsolute( const KURL &url );

	/**
	* If @p url is a directory will return true, false otherwise.
	**/
	bool isDirectory( const KURL &url );
};

#endif // __URLFILEUTILIES_H_
