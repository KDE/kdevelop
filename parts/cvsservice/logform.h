/***************************************************************************
 *   Copyright (C) 200?-2003 by KDevelop Authors                           *
 *   www.kdevelop.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LOGFORM_H
#define LOGFORM_H

#include <dcopobject.h>
#include <kdialogbase.h>

class LogFormBase;
class CvsJob_stub;
class CvsService_stub;

/**
Implementation for the form displaying 'cvs log' output.

@author KDevelop Authors
*/
class LogForm : public KDialogBase, public DCOPObject
{
    K_DCOP
    Q_OBJECT
public:
    LogForm( CvsService_stub *cvsService, QWidget *parent=0, const char *name=0, int flags=0 );
    virtual ~LogForm();

    void start( const QString &workDir, const QString &pathName );

k_dcop:
    // Connect this part with job's DCOP reference
    void slotJobExited( bool normalExit, int exitStatus );
    void slotReceivedOutput( QString someOutput );
    void slotReceivedErrors( QString someErrors );
    // This is called by the cvs job when requested diff job is finished
    void slotDiffFinished( bool normalExit, int exitStatus );

public slots:
    void slotLinkClicked( const QString & link );

private:
    void setText( const QString& text );
    void startDiff();

private:
    QString m_pathName;
    LogFormBase *m_base;

    CvsService_stub *m_cvsService;
    CvsJob_stub *m_cvsLogJob,
        *m_cvsDiffJob;

    QString m_diffText;
};

#endif
