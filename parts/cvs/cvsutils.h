/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVSUTILS_H
#define CVSUTILS_H

class KURL;
class QString;
class KURL::List;

#include "cvspart.h"

/**
* A collection of utility functions for handling cvs repositories.
* @author KDevelop Authors
*/
namespace CvsUtils
{
	bool isRegisteredInRepository( const QString &projectDirectory, const KURL &url );
	// Ideally this function will take a bunch of URLs and validate them (they are valid files,
	// are files registered in CVS, ...). Currently checks only for files belonging to the
	// repository ;)
	void validateURLs( const QString &projectDirectory, KURL::List &urls, CvsOperation op );

	// Add file(s) to their respective ignore list
	void addToIgnoreList( const QString &projectDirectory, const KURL &url );
	void addToIgnoreList( const QString &projectDirectory, const KURL::List &urls );

	// Remove file(s) from their respective .ignore files
	void removeFromIgnoreList( const QString &projectDirectory, const KURL &url );
	void removeFromIgnoreList( const QString &projectDirectory, const KURL::List &urls );
};

#endif // CVSUTILS_H
