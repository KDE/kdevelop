/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
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
#include "svn_revision.h"
#include "svn_statuswidgets.h"
// #include "ui_svnlogviewwidget.h"
#include "ioutputview.h"
#include <icore.h>
#include <iplugincontroller.h>
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
#include <QtGui/QStandardItemModel>

class KDevSubversionViewPrivate{
public:
    KDevSubversionPart *m_part;
    KDevSubversionView *m_view;
    QVBoxLayout *m_layout;
    KTabWidget *m_tab;
    QPushButton *m_closeButton;
    KDevelop::IOutputView *m_outview;
    int m_outputViewId;
    QStandardItemModel* m_outputModel;
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

    d->m_closeButton = new QPushButton( tab() );
    d->m_closeButton->setText( i18n("Close") );
    tab()->setCornerWidget( d->m_closeButton );
    connect( d->m_closeButton, SIGNAL(clicked()), this, SLOT(closeCurrentTab()) );

    setObjectName( i18n( "Subversion" ) );
    setWhatsThis( i18n( "<b>Subversion</b><p>"
            "This window contains an embedded Subversion View Reusults.</p>" ) );
    setWindowIcon( KIcon( "subversion" ) );
    setWindowTitle( i18n( "subversion" ) );

    connect( d->m_part->svncore(), SIGNAL(logFetched(SvnKJobBase*)),
             this, SLOT(printLog(SvnKJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(blameFetched(SvnKJobBase*)),
             this, SLOT(printBlame(SvnKJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(diffFetched(SvnKJobBase *)),
             this, SLOT(printDiff(SvnKJobBase *)) );
    connect( d->m_part->svncore(), SIGNAL(infoFetched(SvnKJobBase*)),
             this, SLOT(printInfo(SvnKJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(statusFetched(SvnKJobBase*)),
             this, SLOT(printStatus(SvnKJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(jobFinished(SvnKJobBase*)),
             this, SLOT(slotJobFinished(SvnKJobBase*)) );

    KDevelop::IPlugin* plugin =
        d->m_part->core()->pluginController()->pluginForExtension( "org.kdevelop.IOutputView" );
    if( plugin ){
        d->m_outview = plugin->extension<KDevelop::IOutputView>();
        if( d->m_outview ){
            d->m_outputViewId = d->m_outview->registerView( i18n("Subversion Notification"),
                              KDevelop::IOutputView::AlwaysShowView );
            d->m_outputModel = new SvnOutputModel( d->m_part, this );
            d->m_outview->setModel( d->m_outputViewId, d->m_outputModel );
            connect( d->m_part->svncore(), SIGNAL(svnNotify(QString, QString)),
                    this, SLOT(printNotification(QString, QString)) );
        }

    } else{
        d->m_outview = 0;
    }

}

KDevSubversionView::~KDevSubversionView()
{
//     delete d->ui;
    delete d;
}

void KDevSubversionView::printNotification( const QString &path, const QString& msg )
{
    if(! d->m_outputModel )
        return;
    d->m_outputModel->appendRow(new SvnOutputItem( path, msg ));
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

void KDevSubversionView::printInfo( SvnKJobBase* job )
{
    if( job->error() ){
        KMessageBox::error( this, job->smartError() );
        return;
    }
    SvnInfoJob *thread = dynamic_cast<SvnInfoJob*>( job->svnThread() );
    if( !thread ) return;
    QMap<KUrl, SvnInfoHolder> map = thread->m_holderMap;

    KTextEdit *widget = new KTextEdit( tab() );
    for( QMap<KUrl, SvnInfoHolder>::iterator it = map.begin(); it != map.end(); ++it ){
        SvnInfoHolder holder = it.value();
        widget->append( "========================" );
        widget->append( QString("Information for: %1").arg(it.key().toLocalFile()) );
        widget->append( QString("Repository URL: %1").arg(holder.URL) );
        widget->append( QString("Repository Root: %1").arg(holder.repos_root_URL) );
        widget->append( QString("Repository UUID: %1").arg(holder.repos_UUID) );
        widget->append( QString("Revision: %1").arg(holder.rev) );

        QString scheduleStr;
        switch( holder.schedule ){
            case svn_wc_schedule_normal:
                scheduleStr = QString("Normal");
                break;
            case svn_wc_schedule_add:
                scheduleStr = QString("Add");
                break;
            case svn_wc_schedule_delete:
                scheduleStr = QString("Delete");
                break;
            case svn_wc_schedule_replace:
                scheduleStr = QString("Replace");
                break;
        }
        // from belows are for working-copy only fields.
        if( !scheduleStr.isEmpty() )
            widget->append( QString("Schedule: %1").arg(scheduleStr) );
        if( holder.copyfrom_url ){
            widget->append( QString("Copied from: %1").arg(holder.copyfrom_url) );
            widget->append( QString("Copied from revision: %1").arg(holder.copyfrom_rev) );
        }
        // todo have a room to add some more fields.
    }

    widget->setReadOnly( true );
    tab()->addTab( widget, "Information" );
    tab()->setCurrentIndex( tab()->indexOf(widget) );
}

void KDevSubversionView::printStatus( SvnKJobBase* job )
{
    if( job->error() ){
        KMessageBox::error( this, job->smartError() );
        return;
    }
    SvnStatusJob *thread = dynamic_cast<SvnStatusJob*>( job->svnThread() );
    if( !thread ) return;
    QMap< KUrl, SvnStatusHolder > map = thread->m_holderMap;

    SvnStatusDisplayWidget *display =
            new SvnStatusDisplayWidget( thread->requestedUrl(),
                                        thread->contactRepository(),
                                        tab() );
    tab()->addTab( display, "Status" );
    tab()->setCurrentIndex( tab()->indexOf(display) );

    display->setResults( map );

}

void KDevSubversionView::slotJobFinished( SvnKJobBase *job )
{
    if( job->error() ){
        KMessageBox::error( this, job->errorText(), "error" );
        return;
    }
}

void KDevSubversionView::closeCurrentTab()
{
    QWidget *current = tab()->currentWidget();
//     KTextEdit *edit = static_cast<KTextEdit*>(current);
//     if( edit ){
//         if( edit == d->m_edit ) // main notification output should not be deleted
//             return;
//     }
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
