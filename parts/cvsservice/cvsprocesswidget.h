/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                                *
 *   www.kdevelop.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CVSPROCESSWIDGET_H_
#define _CVSPROCESSWIDGET_H_

#include <klistbox.h>
#include <dcopobject.h>

class CvsPart;
class CvsJob_stub;

class CvsProcessWidget : public KListBox, public DCOPObject
{
    K_DCOP
    Q_OBJECT
public:
    CvsProcessWidget( QCString appId, CvsPart *part, QWidget *parent, const char *name );
    ~CvsProcessWidget();

    bool startJob();
    bool startJob( const QCString appId, const QCString objId );

    void cancelJob();

    QString output() const { return m_output; }
    QString errors() const { return m_errors; }

k_dcop:
    // Connect this part with job's DCOP reference
    void slotJobExited( bool normalExit, int exitStatus );
    void slotReceivedOutput( QString someOutput );
    void slotReceivedErrors( QString someErrors );

signals:
    void jobFinished( bool normalExit, int exitStatus );

private slots:
    void slotLineHighlighted( int line );

private:
    CvsPart *m_part;
    CvsJob_stub *m_job;

    QString m_output,
        m_errors;
};

#endif

