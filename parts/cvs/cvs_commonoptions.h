//
//
// C++ Interface: cvs_commonoptions
//
// Description:
// Defines default command line options for CVS commands.
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#define default_cvs 	QString::fromLatin1("-f")
#define default_commit	QString::fromLatin1("")
#define default_update	QString::fromLatin1("-dP")
#define default_add		QString::fromLatin1("")
#define default_remove	QString::fromLatin1("-f")
#define default_replace	QString::fromLatin1("-C -d -P")
#define default_diff	QString::fromLatin1("-u3 -p")
#define default_log		QString::fromLatin1("")
#define default_rsh		QString::fromLatin1("")
