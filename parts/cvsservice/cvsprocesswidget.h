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
#include <qstringlist.h>

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

    QString outputString() const { return m_output.join("\n"); }
    QString errorsString() const { return m_errors.join("\n"); }

k_dcop:
    // Connect this part with job's DCOP reference
    void slotJobExited( bool normalExit, int exitStatus );
    void slotReceivedOutput( QString someOutput );
    void slotReceivedErrors( QString someErrors );

signals:
    void jobFinished( bool normalExit, int exitStatus );

private:
    void showInfo( const QStringList &msg );
    void showError( const QStringList &msg  );
    void showOutput( const QStringList &msg  );

    CvsPart *m_part;
    CvsService_stub *m_service;
    CvsJob_stub *m_job;

    QStyleSheetItem *m_goodStyle,
        *m_errorStyle,
        *m_infoStyle;

    // A temporary buffer for storing characters as they arrive. CVS is not
    // kind enough to present us whole strings :-(
    QString m_outputBuffer,
        m_errorBuffer;

    QStringList m_output,
        m_errors;

    QStringList processBuffer( QString &buffer );
};

#endif

