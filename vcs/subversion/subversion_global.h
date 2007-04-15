/*
 *  Copyright (C) 2007 Dukju Ahn (dukjuahn@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 */

#ifndef SUBVERSION_GLOBAL_H
#define SUBVERSION_GLOBAL_H

#include <kurl.h>
#include <qdatetime.h>

namespace SvnGlobal
{

typedef enum {
    path_to_reposit = 0,
    path_to_path = 1,
    dont_touch = 2
} UrlMode;

/// A structure which describes various system-generated metadata about
/// a working-copy path or URL.
class SvnInfoHolder {
public:
    // the requested path
    KURL path;
    /* Where the item lives in the repository. */
    KURL url;
    // The revision of the object.  If path_or_url is a working-copy
    // path, then this is its current working revnum.  If path_or_url
    // is a URL, then this is the repos revision that path_or_url lives in. */
    int rev;
    int kind;
    /* The root URL of the repository. */
    KURL reposRootUrl;
    QString reposUuid;
};

class SvnRevision{
public:
	int revNum;
	QString revKind;
	QDateTime revDate;
};

} // end of namespace SvnGlobal
#endif
