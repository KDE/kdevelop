/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                           *
 *   www.kdevelop.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVSDIFFPAGE_H
#define CVSDIFFPAGE_H

#include "cvsservicedcopIface.h"
#include <qwidget.h>
#include "bufferedstringreader.h"

class CvsJob_stub;
class CvsService_stub;
class Q3TextEdit;
class DiffWidget;

/**
Implementation for the form displaying 'cvs diff' output.

@author KDevelop Authors
*/
class CVSDiffPage : public QWidget, virtual public CVSServiceDCOPIface
{
    Q_OBJECT
public:
    CVSDiffPage( CvsService_stub *cvsService, QWidget *parent=0, const char *name=0, int flags=0 );
    virtual ~CVSDiffPage();

    void startDiff( const QString &fileName, const QString &v1, const QString &v2 );
    void cancel();

//private slots:
    // DCOP Iface
    virtual void slotJobExited( bool normalExit, int exitStatus );
    virtual void slotReceivedOutput( QString someOutput );
    virtual void slotReceivedErrors( QString someErrors );

private:
    //QTextEdit *m_diffText;
    DiffWidget *m_diffText;
    BufferedStringReader m_outputBuffer;
    QString m_diffString;

    CvsService_stub *m_cvsService;
    CvsJob_stub *m_cvsDiffJob;
};

#endif
