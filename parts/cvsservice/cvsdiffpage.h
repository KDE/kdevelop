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

#ifndef CVSDIFFPAGE_H
#define CVSDIFFPAGE_H

#include <dcopobject.h>
#include <qwidget.h>

class CvsJob_stub;
class CvsService_stub;
class QTextEdit;

/**
Implementation for the form displaying 'cvs diff' output.

@author KDevelop Authors
*/
class CVSDiffPage : public QWidget, public DCOPObject
{
    K_DCOP
    Q_OBJECT
public:
    CVSDiffPage( CvsService_stub *cvsService, QWidget *parent=0, const char *name=0, int flags=0 );
    virtual ~CVSDiffPage();

    void startDiff( const QString &fileName, const QString &v1, const QString &v2 );
    void cancel();

k_dcop:
    void slotDiffJobExited( bool normalExit, int exitStatus );
    void slotReceivedOutput( QString someOutput );
    void slotReceivedErrors( QString someErrors );

private:
    QTextEdit *m_diffText;

    CvsService_stub *m_cvsService;
    CvsJob_stub *m_cvsDiffJob;
};

#endif
