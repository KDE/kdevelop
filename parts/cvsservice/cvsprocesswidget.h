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

#include <qtextedit.h>
#include <dcopobject.h>

class CvsPart;
class DCOPRef;
class CvsJob_stub;
class CvsService_stub;
class QStyleSheetItem;

class CvsProcessWidget : public QTextEdit, public DCOPObject
{
    K_DCOP
    Q_OBJECT
public:
    CvsProcessWidget( CvsService_stub *service, CvsPart *part,
        QWidget *parent, const char *name );
    virtual ~CvsProcessWidget();

    bool startJob( const DCOPRef &aJob );

    /**
    * @return true if there is already a job pending, false otherwise
    * (another job can be requested)
    */
    bool isAlreadyWorking() const;
    void cancelJob();

    virtual void clear();

    QString output() const { return m_output; }
    QString errors() const { return m_errors; }

k_dcop:
    // Connect this part with job's DCOP reference
    void slotJobExited( bool normalExit, int exitStatus );
    void slotReceivedOutput( QString someOutput );
    void slotReceivedErrors( QString someErrors );

signals:
    void jobFinished( bool normalExit, int exitStatus );

private:
    void showInfo( const QString &msg );
    void showError( const QString &msg );
    void showOutput( const QString &msg );

    CvsPart *m_part;
    CvsService_stub *m_service;
    CvsJob_stub *m_job;

    QStyleSheetItem *m_goodStyle,
        *m_errorStyle,
        *m_infoStyle;

    QString m_output,
        m_errors;
};

#endif

