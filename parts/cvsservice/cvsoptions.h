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

#include <qstring.h>
#include <qdom.h>

class CvsPart;
class KConfig;

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

    void setRevert( const QString &p );
    QString revert();

    void setDiff( const QString &p );
    QString diff();

    void setLog( const QString &p );
    QString log();

    void setRsh( const QString &p );
    QString rsh();

    /**
    * Set server path string
    * @param p (i.e. :pserver:marios@cvs.kde.org:/home/kde)
    */
    void setLocation( const QString &p );
    /**
    * @result remote path (i.e. :pserver:marios@cvs.kde.org:/home/kde)
    */
    QString location();

    void setContextLines( unsigned int contextLines );
    unsigned int contextLines() const;

    void setCompressionLevel( unsigned int compressionLevel = 0 );
    unsigned int compressionLevel() const;

private:
    // Cache
    QString m_cvs;
    QString m_commit;
    QString m_update;
    QString m_add;
    QString m_remove;
    QString m_revert;
    QString m_diff;
    QString m_log;
    QString m_rsh;
    QString m_location;
    unsigned int m_compressionLevel;
    unsigned int m_contextLines;
    // So we can access cvssservice configuration (repositories first of all)
    KConfig *m_serviceConfig;

    static CvsOptions *m_instance;
    CvsOptions();
};
