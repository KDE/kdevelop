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

#ifndef CVSOPTIONS_H
#define CVSOPTIONS_H

#include <qstring.h>
#include <qdom.h>

class CvsServicePart;
class KConfig;
class KDevProject;

/* This class represents the command line options for the used cvs commands.
 * It uses the singleton pattern.
 * @author Mario Scalas <mario.scalas@libero.it>
*/
class CvsOptions
{
public:
    static CvsOptions *instance();

    static QString invalidLocation;

    virtual ~CvsOptions();

    void save( KDevProject *project );
    /// \FIXME parameter should be const!!
    void load( KDevProject *project );

    void setRecursiveWhenCommitRemove( bool b );
    bool recursiveWhenCommitRemove() const;

    void setPruneEmptyDirsWhenUpdate( bool b );
    bool pruneEmptyDirsWhenUpdate() const;

    void setRecursiveWhenUpdate( bool b );
    bool recursiveWhenUpdate() const;

    void setCreateDirsWhenUpdate( bool b );
    bool createDirsWhenUpdate() const;

    void setDiffOptions( const QString &p );
    QString diffOptions();

    void setRevertOptions( const QString &p );
    QString revertOptions();

    void setCvsRshEnvVar( const QString &p );
    QString cvsRshEnvVar();

    /**
    * Will try to determine location by using CVS/Root file
    */
    QString guessLocation( const QString &projectDir ) const;

    /**
    * Set server path string (this should be called by the part when a new project
    * is created or imported)
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
    bool m_recursiveWhenCommitRemove;
    bool m_pruneEmptyDirsWhenUpdate;
    bool m_recursiveWhenUpdate;
    bool m_createDirsWhenUpdate;
    QString m_revertOptions;
    QString m_diffOptions;
    QString m_cvsRshEnvVar;
    QString m_location;
    unsigned int m_compressionLevel;
    unsigned int m_contextLines;
    //! So we can access cvssservice configuration (repositories first of all)
    KConfig *m_serviceConfig;

    static CvsOptions *m_instance;
    CvsOptions();
};

#endif  // CVSOPTIONS_H

