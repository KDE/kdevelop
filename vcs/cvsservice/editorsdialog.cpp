/***************************************************************************
 *   Copyright (C) 2004 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kmessagebox.h>
#include <kcursor.h>
#include <klocale.h>
#include <kdebug.h>
#include <dcopref.h>

#include <qtextbrowser.h>
#include <qregexp.h>

#include "editorsdialog.h"

//dcop connection to cervisia
#include <cvsjob_stub.h>
#include <cvsservice_stub.h>

EditorsDialog::EditorsDialog(CvsService_stub *cvsService, QWidget *parent, const char *name)
    : DCOPObject( "CvsEditorsDCOPIface"), EditorsDialogBase(parent, name, TRUE, Qt::WDestructiveClose),
      m_cvsService(cvsService), m_cvsJob(0)
{
}

EditorsDialog::~EditorsDialog()
{
    kdDebug(9006) << "EditorsDialog::~EditorsDialog"<< endl;

    if (m_cvsJob && m_cvsJob->isRunning()) {
        m_cvsJob->cancel();
    }
    if (m_cvsJob) 
        delete m_cvsJob;
}

void EditorsDialog::startjob(QString strDir)
{
    kdDebug(9006) << "EditorsDialog::start() workDir = " << strDir << endl;

    DCOPRef job = m_cvsService->editors( strDir );
    m_cvsJob = new CvsJob_stub( job.app(), job.obj() );

    // establish connections to the signals of the cvs m_job
    connectDCOPSignal( job.app(), job.obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
    // We'll read the ouput directly from the job ...
    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );

    kdDebug(9006) << "Running: " << m_cvsJob->cvsCommand() << endl;
    m_cvsJob->execute();
}

void EditorsDialog::slotJobExited( bool normalExit, int exitStatus )
{
    if (!normalExit)
    {
        KMessageBox::sorry( this, i18n("Log failed with exitStatus == %1").arg( exitStatus), i18n("Log Failed") );
        return;
    }

    static QRegExp re("([^\\s]+)\\s([^\\s]+)\\s([^\\s]+)\\s([^\\s]+)\\s([^\\s]+)\\s"
                        "([^\\s]+)\\s([^\\s]+)\\s([^\\s]+)\\s([^\\s]+)\\s(.*)");

    QStringList lines = QStringList::split( "\n", m_output );
    int found = 0;
    for (size_t i=0; i<lines.count(); ++i) {
        QString s = lines[i].simplifyWhiteSpace();

        if (re.exactMatch(s)) {
            QString file = re.cap( 1 );
            QString locker = re.cap( 2 );
            QString date = re.cap(5)+" "+re.cap(4)+" "+re.cap(7)+" "+re.cap(6);
            
            m_textBrowser->append( "<b>"+i18n("File")+": <code>"+file+"</code></b>" );
            m_textBrowser->append( "<b>"+i18n("User")+":</b> "+locker );
            m_textBrowser->append( "<b>"+i18n("Date")+":</b> "+date );
            m_textBrowser->append( "<hr>" );
            found++;
        }
    }

    if (!found)
        m_textBrowser->append(i18n("No files from your query are marked as beeing edited!"));

    m_textBrowser->source();

    if (m_cvsJob) {
        disconnectDCOPSignal( m_cvsJob->app(), m_cvsJob->obj(), "jobExited(bool, int)", "slotJobExited(bool, int)" );
        delete m_cvsJob;
        m_cvsJob=NULL;
    }
}

void EditorsDialog::slotReceivedOutput( QString someOutput )
{
    kdDebug(9006) << "OUTPUT: " << someOutput << endl;

    m_output += someOutput; //append the whole output into one large QStrin
}

///////////////////////////////////////////////////////////////////////////////

void EditorsDialog::slotReceivedErrors( QString someErrors )
{
    kdDebug(9006) << "ERRORS: " << someErrors << endl;
}

#include "editorsdialog.moc"
