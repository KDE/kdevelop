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

#ifndef CHECKOUTDIALOG_H
#define CHECKOUTDIALOG_H

#include <kdialogbase.h>
#include "cvsservicedcopiface.h"

class CvsService_stub;
class CvsJob_stub;
class CheckoutDialogBase;
class QListViewItem;

/**
* This dialog widget will collect all useful informazion about the module the
* user want to to check-out from a remote repository.
*
* @author Mario Scalas
*/
class CheckoutDialog : public KDialogBase, virtual public CVSServiceDCOPIface
{
    Q_OBJECT
public:
    CheckoutDialog( CvsService_stub *cvsService, QWidget *parent = 0,
        const char *name = 0, WFlags f = 0 );
    virtual ~CheckoutDialog();

    /**
    * @return the value which will be used for setting $CVS_RSH env variable
    */
    QString cvsRsh() const;
    /**
    * @return a server path string (i.e. :pserver:marios@cvs.kde.org:/home/kde)
    */
    QString serverPath() const;
    /**
    * @param aPath a valid path string for "cvs -d" option
    */
    void setServerPath( const QString &aPath );
    /**
    * @return the directory which the user has fetched the module in
    */
    QString workDir() const;
    /**
    * @param aDir directory which fetched modules will be put in
    */
    void setWorkDir( const QString &aDir );
    /**
    * @return the module the user has chosen to check-out from repository
    */
    QString module() const;
    /**
    * @return
    */
    bool pruneDirs() const;
    /**
    * @return
    */
    QString tag() const;

private slots:
    void slotModuleSelected( QListViewItem *item );
    void slotFetchModulesList();
    // DCOP Iface
    virtual void slotJobExited( bool normalExit, int exitStatus );
    virtual void slotReceivedOutput( QString someOutput );
    virtual void slotReceivedErrors( QString someErrors );

private:
    CvsService_stub *m_service;
    CvsJob_stub *m_job;

    CheckoutDialogBase *m_base;
};

#endif
