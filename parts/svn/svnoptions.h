//
//
// C++ Interface: svnoptions
//
// Description:
// Defines default command line options for SVN commands.
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qstring.h>
#include <qdom.h>

class SvnPart;

/* This class represents the command line options for the used svn commands.
 * It uses the singleton pattern.
 * @author Mario Scalas <mario.scalas@libero.it>
*/
class SvnOptions
{
public:
	virtual ~SvnOptions();

	static SvnOptions *instance();

	void save( QDomDocument &dom );
	void load( const QDomDocument &dom );

	void setSvn( const QString &p );
	QString svn();

	void setCommit( const QString &p );
	QString commit();

	void setUpdate( const QString &p );
	QString update();

	void setAdd( const QString &p );
	QString add();

	void setRemove( const QString &p );
	QString remove();

	void setRevert( const QString &p );
	QString revert();

	void setDiff( const QString &p );
	QString diff();

	void setLog( const QString &p );
	QString log();

	void setRsh( const QString &p );
	QString rsh();

private:
	// Cache
	QString m_svn;
	QString m_commit;
	QString m_update;
	QString m_add;
	QString m_remove;
	QString m_revert;
	QString m_diff;
	QString m_log;
	QString m_rsh;

	static SvnOptions *m_instance;
	SvnOptions();
};

