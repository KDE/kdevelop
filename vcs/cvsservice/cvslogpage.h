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

#ifndef CVSLOGPAGE_H
#define CVSLOGPAGE_H

#include "cvsservicedcopIface.h"
#include <qwidget.h>
#include "bufferedstringreader.h"

class CvsJob_stub;
class CvsService_stub;
class QTextBrowser;

/**
Implementation for the form displaying 'cvs log' output.

@author KDevelop Authors
*/
class CVSLogPage : public QWidget, virtual public CVSServiceDCOPIface
{
    Q_OBJECT
public:
    CVSLogPage( CvsService_stub *cvsService, QWidget *parent=0, const char *name=0, int flags=0 );
    virtual ~CVSLogPage();

    void startLog( const QString &workDir, const QString &pathName );
    void cancel();

signals:
    // Emitted when the user click upon a link
    void diffRequested( const QString &pathName, const QString &revA, const QString &revB );

private slots:
    void slotLinkClicked( const QString &link );
    // DCOP Iface
    virtual void slotJobExited( bool normalExit, int exitStatus );
    virtual void slotReceivedOutput( QString someOutput );
    virtual void slotReceivedErrors( QString someErrors );

//private:
//    void parseLogContent( const QString& text );

private:
    QString m_pathName;
    QTextBrowser *m_textBrowser;
    QString m_logTextBackup;
	BufferedStringReader m_outputBuffer;
	QStringList m_diffStrings;

    CvsService_stub *m_cvsService;
    CvsJob_stub *m_cvsLogJob;
};

#endif
