//
//
// C++ Interface: cvsoptions
//
// Description:
// Defines default command line options for CVS commands.
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qstring.h>
#include <qdom.h>

class CvsPart;

/* This class represents the command line options for the used cvs commands.
 * It uses the singleton pattern.
 * @author Mario Scalas <mario.scalas@libero.it>
*/
class CvsOptions
{
public:
	virtual ~CvsOptions();

	static CvsOptions *instance();

	void save( QDomDocument &dom );
	void load( const QDomDocument &dom );

	void setCvs( const QString &p );
	QString cvs();

	void setCommit( const QString &p );
	QString commit();

	void setUpdate( const QString &p );
	QString update();

	void setAdd( const QString &p );
	QString add();

	void setRemove( const QString &p );
	QString remove();

	void setReplace( const QString &p );
	QString replace();

	void setDiff( const QString &p );
	QString diff();

	void setLog( const QString &p );
	QString log();

	void setRsh( const QString &p );
	QString rsh();

private:
	// Cache
	QString m_cvs;
	QString m_commit;
	QString m_update;
	QString m_add;
	QString m_remove;
	QString m_replace;
	QString m_diff;
	QString m_log;
	QString m_rsh;

	static CvsOptions *m_instance;
	CvsOptions();
};

#define default_cvs 	QString::fromLatin1("-f")
#define default_commit	QString::fromLatin1("")
#define default_update	QString::fromLatin1("-dP")
#define default_add		QString::fromLatin1("")
#define default_remove	QString::fromLatin1("-f")
#define default_replace	QString::fromLatin1("-C -d -P")
#define default_diff	QString::fromLatin1("-u3 -p")
#define default_log		QString::fromLatin1("")
#define default_rsh		QString::fromLatin1("")
