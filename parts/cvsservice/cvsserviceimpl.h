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

#ifndef CVSSERVICEIMPL_H
#define CVSSERVICEIMPL_H

#include "cvspartimpl.h"

class CvsProcessWidget;
class CvsForm;
class CheckoutDialog;
class JobScheduler;
class CvsService_stub;
class Repository_stub;

/**
* Implementation using Cervisia HEAD's DCOP CvsService.
*
* @author Mario Scalas
*/
class CvsServiceImpl : public CvsServicePartImpl
{
    Q_OBJECT
public:
    CvsServiceImpl( CvsServicePart *part, const char *name=0 );
    virtual ~CvsServiceImpl();

    // Implementation of CvsOperations
    virtual void login();
    virtual void logout();
    virtual void checkout();
    virtual void commit( const KURL::List& urlList );
    virtual void update( const KURL::List& urlList );
    virtual void add( const KURL::List& urlList, bool binary = false );
    virtual void remove( const KURL::List& urlList );
    virtual void log( const KURL::List& urlList );
    virtual void diff( const KURL::List& urlList );
    virtual void tag( const KURL::List& urlList );
    virtual void unTag( const KURL::List& urlList );
    virtual void removeStickyFlag( const KURL::List& urlList );
    virtual void addToIgnoreList( const KURL::List& urlList );
    virtual void removeFromIgnoreList( const KURL::List& urlList );
    virtual void createNewProject( const QString &dirName,
        const QString &cvsRsh, const QString &location,
        const QString &message, const QString &module, const QString &vendor,
        const QString &release, bool mustInitRoot );

protected:
    // Reimplemented so we can test for correctly setup DCOP CvsService :-)
    virtual bool prepareOperation( const KURL::List &someUrls, CvsOperation op );

private slots:
    void slotJobFinished( bool normalExit, int exitStatus );
    void slotDiffFinished( bool normalExit, int exitStatus );
    void slotCheckoutFinished( bool normalExit, int exitStatus );
    void slotProjectOpened();

private:
    bool requestCvsService();
    void releaseCvsService();

    CvsService_stub *m_cvsService;
    Repository_stub *m_repository;

    // Used for storing module path between start and ending of check-out
    QString modulePath;

    JobScheduler *m_scheduler;
};

#endif
