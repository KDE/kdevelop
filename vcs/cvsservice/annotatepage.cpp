/***************************************************************************
 *   Copyright (C) 2005 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <kmessagebox.h>
#include <kcursor.h>
#include <klocale.h>
#include <kdebug.h>
#include <krfcdate.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kdialogbase.h>
#include <kmessagebox.h>

#include <dcopref.h>
#include <cvsjob_stub.h>
#include <cvsservice_stub.h>

#include "annotatepage.h"
#include "annotateview.h"

AnnotatePage::AnnotatePage( CvsService_stub *cvsService, QWidget *parent, const char *name, int )
    : DCOPObject( "CvsAnnotatePageDCOPIface" ),
    QWidget( parent, name? name : "annotateformpage" ),
    m_cvsService( cvsService ), m_cvsAnnotateJob( 0 )
{
    kdDebug(9006) << "AnnotatePage::AnnotatePage()" << endl;

    QLayout *dialogLayout = new QVBoxLayout( this );

    //First create the top-line where user can choose a revision
    QWidget * LayoutWidget = new QWidget( this );
    QHBoxLayout * AnnotateLayout = new QHBoxLayout( LayoutWidget ); 

    QLabel * lblRevision = new QLabel( LayoutWidget );
    AnnotateLayout->addWidget( lblRevision );
    lblRevision->setText( tr( "Revision:" ) );

    m_leRevision = new KLineEdit( LayoutWidget );
    AnnotateLayout->addWidget( m_leRevision );

    m_btnAnnotate = new KPushButton( LayoutWidget );
    AnnotateLayout->addWidget( m_btnAnnotate );
    m_btnAnnotate->setText( tr( "&Annotate" ) );
    m_btnAnnotate->setAccel( QKeySequence( tr( "Alt+A" ) ) );

    dialogLayout->add( LayoutWidget );

    connect( m_btnAnnotate, SIGNAL(clicked()),
             this, SLOT(slotNewAnnotate()) );
    connect( m_leRevision, SIGNAL( returnPressed() ), 
             m_btnAnnotate, SLOT( setFocus() ) );

    //Nest create the AnnotateView; it will do the actual displaying
    m_annotateView = new AnnotateView(this, "annotateview");
    dialogLayout->add( m_annotateView );
}

AnnotatePage::~AnnotatePage()
{
    kdDebug(9006) << "AnnotatePage::~Annotate()" << endl;
    cancel();
    delete m_cvsAnnotateJob;
}

void AnnotatePage::startAnnotate( const QString pathName, const QString revision )
{
    kdDebug(9006) << "AnnotatePage::startAnnotate() pathName = " << pathName << 
        "revision = " << revision << endl;

    m_leRevision->setText(revision);

    m_pathName = pathName;

    DCOPRef job = m_cvsService->annotate( pathName, revision );
    m_cvsAnnotateJob = new CvsJob_stub( job.app(), job.obj() );

    // establish connections to the signals of the cvs m_job
    connectDCOPSignal( job.app(), job.obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );

    //clear both the outputbuffer and the AnnotateView
    m_output = "";
    ((KListView*)m_annotateView)->clear();

    kdDebug(9006) << "Running: " << m_cvsAnnotateJob->cvsCommand() << endl;
    m_cvsAnnotateJob->execute();
}

void AnnotatePage::slotJobExited( bool normalExit, int exitStatus )
{
    kdDebug(9006) << "AnnotatePage::slotJobExited(bool, int)" << endl;

    if (!normalExit)
    {
        KMessageBox::sorry( this, i18n("Annotate failed with exitStatus == %1").arg( exitStatus), i18n("Annotate Failed") );
        return;
    }

    //split the collected output and pass the lines to the parser function
    QStringList lines = QStringList::split("\n", m_output);
    parseAnnotateOutput(lines);
}

void AnnotatePage::slotReceivedOutput( QString someOutput )
{
    kdDebug(9006) << "AnnotatePage::slotReceivedOutput(QString)" << endl;
    m_output += someOutput;
}

void AnnotatePage::slotReceivedErrors( QString )
{
    kdDebug(9006) << "AnnotatePage::slotReceivedErrors(QString)" << endl;
}

void AnnotatePage::cancel()
{
    if (m_cvsAnnotateJob && m_cvsAnnotateJob->isRunning())
        m_cvsAnnotateJob->cancel();
}

void AnnotatePage::parseAnnotateOutput(QStringList& lines)
{
    kdDebug(9006) << "AnnotatePage::parseAnnotateOutput(QStringList)" << endl;

    /**
     * First we need to parse the output of "cvs log" which the dcop-interface delivers
     * everytime annotate is requested.
     * The QMap m_comments stores the revisions together with the matching comments.
     * The comments will be passed to the AnnotateView in order to display them as QToolTip
     */
    QString line, comment, rev;

    enum { Begin, Tags, Admin, Revision,
           Author, Branches, Comment, Finished } state;

    QStringList::Iterator it = lines.begin();
    state = Begin;
    do
    {
        line  = *it;

        switch( state )
        {
            case Begin:
                if( line == "symbolic names:" )
                    state = Tags;
                break;
            case Tags:
                if( line[0] != '\t' )
                    state = Admin;
                break;
            case Admin:
                if( line == "----------------------------" )
                    state = Revision;
                break;
            case Revision:
                rev = line.section(' ', 1, 1);
                state = Author;
                break;
            case Author:
                state = Branches;
                break;
            case Branches:
                if( !line.startsWith("branches:") )
                {
                    state = Comment;
                    comment = line;
                }
                break;
            case Comment:
                if( line == "----------------------------" )
                    state = Revision;
                else if( line == "=============================================================================" )
                    state = Finished;
                if( state == Comment )
                    comment += QString("\n") + line;
                else
                    m_comments[rev] = comment;
                break;
            case Finished:
                    ;
        }

        if (state == Finished)
            break;
    } while( ++it != lines.end());

    // move forward until we get to the actual output of "cvs annotate"
    bool notEof = true;
    while( notEof && !(*it).startsWith("*****") ) {
        notEof = (++it != lines.end());
    }

    //if the upper loop hit the ent of the list, this can only mean, that
    //the selected revision is unknown to CVS
    if (!notEof) {
        KMessageBox::error(this, i18n("The selected revision does not exist."));
        ((KListView*)m_annotateView)->clear();
        return;
    }
    ++it;

    QString author, content;
    QString oldRevision = ""; //we always store the last revision to recognice...
    bool changeColor = false; //...when the AnnotateView needs to change the coloring
    QDateTime logDate;

    do
    {
        line  = *it;

        //the log date should be printed according to the user's global setting
        //so we pass it as QDateTime to the AnnotateView below
        QString dateString = line.mid(23, 9);
        if( !dateString.isEmpty() )
            logDate.setTime_t(KRFCDate::parseDate(dateString), Qt::UTC);

        rev     = line.left(13).stripWhiteSpace();
        author  = line.mid(14, 8).stripWhiteSpace();
        content = line.mid(35, line.length()-35);

        comment = m_comments[rev];
        if( comment.isNull() )
            comment = "";

        if( rev != oldRevision )
        {
            oldRevision = rev;
            changeColor = !changeColor;
        }

        //finished parsing the annotate line
        //We pass the needed data to the AnnotateView
        m_annotateView->addLine(rev, author, logDate, content, m_comments[rev], changeColor);
    } while (++it != lines.end());
}

void AnnotatePage::slotNewAnnotate()
{
    startAnnotate(m_pathName, m_leRevision->text());
}

#include "annotatepage.moc"
