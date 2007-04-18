/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include "subversion_view.h"
#include "subversion_core.h"
#include "svnkjobbase.h"
#include "subversionthreads.h"
#include "interthreadevents.h"
#include "svn_models.h"
#include "svn_logviewwidgets.h"
#include "svn_blamewidgets.h"
#include "subversion_utils.h"
// #include "ui_svnlogviewwidget.h"
#include <kmessagebox.h>
#include <ktabwidget.h>
#include <ktextedit.h>
#include <ktextbrowser.h>
#include <klineedit.h>

#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QPushButton>
#include <QTextStream>
#include <QFile>
#include <QDialog>
#include <QLabel>
#include <QProgressBar>

class KDevSubversionViewPrivate{
public:
    KDevSubversionPart *m_part;
    KDevSubversionView *m_view;
    QVBoxLayout *m_layout;
    KTabWidget *m_tab;
    KTextEdit *m_edit;
    QPushButton *m_closeButton;
};
// TODO first make empty widget by factory. This host container widget is parent
// of every other subwidgets, including logviewer, blame, notifier, ...
KDevSubversionView::KDevSubversionView( KDevSubversionPart *part, QWidget* parent )
    : QWidget( parent ), d(new KDevSubversionViewPrivate)
{
    d->m_part = part;
    d->m_view = this;
    
    d->m_tab = new KTabWidget(this);
    d->m_layout = new QVBoxLayout( this );
    d->m_layout->setMargin(0);
    d->m_layout->addWidget( tab() );
    
    d->m_edit = new KTextEdit( tab() );
    d->m_edit->setReadOnly( TRUE );
    tab()->addTab( d->m_edit, i18n("Notification") );
    
    d->m_closeButton = new QPushButton( tab() );
    d->m_closeButton->setText( i18n("Close") );
    tab()->setCornerWidget( d->m_closeButton );
    connect( d->m_closeButton, SIGNAL(clicked()), this, SLOT(closeCurrentTab()) );
    
    setObjectName( i18n( "Subversion" ) );
    setWhatsThis( i18n( "<b>Subversion</b><p>"
            "This window contains an embedded Subversion View Reusults." ) );
    setWindowIcon( KIcon( "subversion" ) );
    setWindowTitle( i18n( "subversion" ) );
    
    connect( d->m_part->svncore(), SIGNAL(svnNotify(QString)),
            this, SLOT(printNotification(QString)) );
    connect( d->m_part->svncore(), SIGNAL(logFetched(SvnKJobBase*)),
             this, SLOT(printLog(SvnKJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(blameFetched(SvnKJobBase*)),
             this, SLOT(printBlame(SvnKJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(diffFetched(SvnKJobBase *)),
             this, SLOT(printDiff(SvnKJobBase *)) );
    connect( d->m_part->svncore(), SIGNAL(jobFinished(SvnKJobBase*)),
             this, SLOT(slotJobFinished(SvnKJobBase*)) );

}

KDevSubversionView::~KDevSubversionView()
{
//     delete d->ui;
    delete d;
}

void KDevSubversionView::printNotification(QString msg)
{
    kDebug() << " KDevSubversionView::printNotification: " << msg << endl;
    if( !d->m_edit ){
        // should not happen
        d->m_edit = new KTextEdit(this);
    }
    d->m_edit->append( msg );
    tab()->setCurrentIndex( tab()->indexOf(d->m_edit) );
}
void KDevSubversionView::printLog(SvnKJobBase *j)
{
    if( j->error() ){
        KMessageBox::error( this, j->smartError(), "error" );
        return;
    }
    SvnLogviewJob *th = (SvnLogviewJob*) ( j->svnThread() );
    SvnLogviewWidget *widget= new SvnLogviewWidget( th->urlList()[0], d->m_part, tab() );

    widget->refreshWithNewData( th->m_loglist );

    tab()->addTab( widget, i18n("Log History") );
    tab()->setTabEnabled( tab()->indexOf(widget), true );
    tab()->setCurrentIndex( tab()->indexOf(widget) );
}
void KDevSubversionView::printBlame( SvnKJobBase* job )
{
    if( job->error() ){
        KMessageBox::error( this, job->errorText(), "error" );
        return;
    }
    SvnBlameWidget *widget = new SvnBlameWidget( tab() );
    SvnBlameJob *th = dynamic_cast< SvnBlameJob*>( job->svnThread() );
    widget->refreshWithNewData( th->m_blameList);
    tab()->addTab( widget, i18n("Blame") );
    tab()->setTabEnabled( tab()->indexOf(widget), true );
    tab()->setCurrentIndex( tab()->indexOf(widget) );
}

void KDevSubversionView::printDiff( SvnKJobBase *job )
{
    if( job->error() ){
        KMessageBox::error( this, job->errorText(), "error" );
        return;
    }
    
    SvnDiffJob *th = dynamic_cast<SvnDiffJob*>( job->svnThread() );
    if( !job ) return;
    
    QFile file( th->out_name );
    if( !file.exists() ){
        KMessageBox::error( this, i18n("The output diff file cannot be located") );
        return;
    }
    // end of error check
    
    KTextBrowser *widget = new KTextBrowser( tab() );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &file );
        QString line;
        while ( !stream.atEnd() ) {
            line = stream.readLine();
            widget->append( line );
        }
        file.close();
    }
    
    widget->setReadOnly( true );
    tab()->addTab( widget, i18n("Diff") );
    tab()->setCurrentIndex( tab()->indexOf(widget) );

}

void KDevSubversionView::slotJobFinished( SvnKJobBase *job )
{
    if( job->error() ){
        KMessageBox::error( this, job->errorText(), "error" );
        return;
    }
    switch( job->type() ){
        case SVN_LOGVIEW:
            break; // not used for logview
        case SVN_BLAME: // not used for blame
            break;
        case SVN_CHECKOUT:
            // not implemented yet
            break;
        case SVN_ADD:
            // note: these messagebox are just temporary ones.
            // don't try to i18n(), because it will be removed
            break;
        case SVN_DELETE:
            break;
        case SVN_COMMIT:
            break;
        case SVN_UPDATE:
            break;
        case SVN_STATUS:
            break;
    }
}

void KDevSubversionView::closeCurrentTab()
{
    QWidget *current = tab()->currentWidget();
    KTextEdit *edit = static_cast<KTextEdit*>(current);
    if( edit ){
        if( edit == d->m_edit ) // main notification output should not be deleted
            return;
    }
    tab()->removePage( current );
    delete current;
}

KTabWidget* KDevSubversionView::tab()
{
    return d->m_tab;
}

//////////////////////////////////////////////////////////////////

class SvnProgressDialog::SvnProgressDialogPrivate
{
public:
	QLabel *srcLabel;
	QLabel *destLabel;
	QLabel *progressLabel;
	KLineEdit *sourceEdit;
	KLineEdit *destEdit;
	QProgressBar *m_progressBar;
};

SvnProgressDialog::SvnProgressDialog( QWidget *parent, const QString &caption )
	: KDialog( parent ), d( new SvnProgressDialogPrivate )
{
	setCaption( caption );
	setButtons( KDialog::Cancel );
	setButtonText( KDialog::Cancel, i18n( " Abort" ) );
	setModal( false );
	setMinimumWidth( 360 );

	QWidget *mainWidget = new QWidget( this );
	QGridLayout *layout = new QGridLayout( mainWidget );

	d->srcLabel = new QLabel( i18n("Source:"), mainWidget );
	layout->addWidget( d->srcLabel, 0, 0 );
	d->destLabel = new QLabel( i18n("Destination:"), mainWidget );
	layout->addWidget( d->destLabel, 1, 0 );
	d->progressLabel = new QLabel( i18n("Progress:"), mainWidget );
	layout->addWidget( d->progressLabel, 2, 0 );

	d->sourceEdit = new KLineEdit( mainWidget );
	d->sourceEdit->setReadOnly( true );
	layout->addWidget( d->sourceEdit, 0, 1 );
	d->destEdit = new KLineEdit( mainWidget );
	d->destEdit->setReadOnly( true );
	layout->addWidget( d->destEdit, 1, 1 );
	d->m_progressBar = new QProgressBar( mainWidget );
	layout->addWidget( d->m_progressBar, 2, 1 );

	QSizePolicy policy( QSizePolicy::Expanding, QSizePolicy::Fixed );
	policy.setHorizontalStretch(1);
	d->sourceEdit->setSizePolicy( policy );
	d->destEdit->setSizePolicy( policy );
	d->m_progressBar->setSizePolicy( policy );
	
	setMainWidget(mainWidget);
}

SvnProgressDialog::~SvnProgressDialog()
{
    kDebug() << "SvnProgressDialog:: destructor " << endl;
	delete d;
}

QProgressBar* SvnProgressDialog::progressBar()
{
    return d->m_progressBar;
}

void SvnProgressDialog::setSource( const QString &src )
{
    d->sourceEdit->setText( src );
}

void SvnProgressDialog::setDestination( const QString &dest )
{
    d->destEdit->setText( dest );
}

#include "subversion_view.moc"
