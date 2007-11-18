/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnview.h"
#include "svncore.h"
#include "svnjobbase.h"
#include "svnthreads.h"
#include "interthreadevents.h"
#include "svnmodels.h"
#include "svnoutputmodel.h"
#include "svnlogviewwidgets.h"
#include "svnblamewidgets.h"
#include "svnrevision.h"
#include "svnstatuswidgets.h"
#include "svnoutputdelegate.h"
// #include "ui_logviewwidget.h"
#include "ioutputview.h"
#include <icore.h>
#include <iplugincontroller.h>
#include <idocumentcontroller.h>
#include <kmessagebox.h>
#include <ktabwidget.h>
#include <ktextedit.h>
#include <klineedit.h>
#include <ktemporaryfile.h>
#include <ktempdir.h>

#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QPushButton>
#include <QTextStream>
#include <QFile>
#include <QDialog>
#include <QLabel>
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
    SvnOutputDelegate* m_delegate;
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
    d->m_closeButton->setText( i18nc("Close the current tab", "Close") );
    tab()->setCornerWidget( d->m_closeButton );
    connect( d->m_closeButton, SIGNAL(clicked()), this, SLOT(closeCurrentTab()) );

    setObjectName( i18n( "Subversion" ) );
    setWhatsThis( i18n( "<b>Subversion</b><p>"
            "This window contains an embedded Subversion View Reusults.</p>" ) );
    setWindowIcon( KIcon( "subversion" ) );
    setWindowTitle( i18n( "subversion" ) );

    connect( d->m_part->svncore(), SIGNAL(logFetched(SvnJobBase*)),
             this, SLOT(printLog(SvnJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(blameFetched(SvnJobBase*)),
             this, SLOT(printBlame(SvnJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(diffFetched(SvnJobBase *)),
             this, SLOT(printDiff(SvnJobBase *)) );
    connect( d->m_part->svncore(), SIGNAL(infoFetched(SvnJobBase*)),
             this, SLOT(printInfo(SvnJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(statusFetched(SvnJobBase*)),
             this, SLOT(printStatus(SvnJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(catFetched(SvnJobBase*)),
             this, SLOT(openCat(SvnJobBase*)) );
    connect( d->m_part->svncore(), SIGNAL(jobFinished(SvnJobBase*)),
             this, SLOT(slotJobFinished(SvnJobBase*)) );

    KDevelop::IPlugin* plugin =
        d->m_part->core()->pluginController()->pluginForExtension( "org.kdevelop.IOutputView" );
    if( plugin ){
        d->m_outview = plugin->extension<KDevelop::IOutputView>();
        if( d->m_outview ){
            d->m_outputViewId = d->m_outview->registerView( i18n("Subversion Notification"),
                              KDevelop::IOutputView::AlwaysShowView );
            d->m_outputModel = new SvnOutputModel( d->m_part, this );
            d->m_outview->setModel( d->m_outputViewId, d->m_outputModel );
	    d->m_delegate = new SvnOutputDelegate(this);
	    d->m_outview->setDelegate( d->m_outputViewId, d->m_delegate );
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

void KDevSubversionView::printLog(SvnJobBase *j)
{
    if( j->error() ){
        KMessageBox::error( this, j->smartError(), "error" );
        return;
    }
    SvnLogviewJob *th = static_cast<SvnLogviewJob*>( j->svnThread() );
    SvnLogviewWidget *widget= new SvnLogviewWidget( th->urlList()[0], d->m_part, tab() );

    widget->refreshWithNewData( th->m_loglist );

    tab()->addTab( widget, i18n("Log History") );
    tab()->setTabEnabled( tab()->indexOf(widget), true );
    tab()->setCurrentIndex( tab()->indexOf(widget) );
}
void KDevSubversionView::printBlame( SvnJobBase* job )
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

void KDevSubversionView::printDiff( SvnJobBase *job )
{
    if( job->error() ){
        KMessageBox::error( this, job->errorText(), "error" );
        return;
    }

    SvnDiffJob *th = dynamic_cast<SvnDiffJob*>( job->svnThread() );
    if( !th ) return;

    QFile file( th->out_name );
    if( !file.exists() ){
        KMessageBox::error( this, i18n("The output diff file cannot be located") );
        return;
    }
    if( file.size() == 0 ){
        KMessageBox::information( this, i18n("No subversion differences") );
        return;
    }
    // end of error check

    // Below code prints diff into dockwidget, which has no advantage over katepart
    // because printing area is small and no syntax highlighting is available.

//     KTextBrowser *widget = new KTextBrowser( tab() );
//     if ( file.open( QIODevice::ReadOnly ) ) {
//         QTextStream stream( &file );
//         QString line;
//         while ( !stream.atEnd() ) {
//             line = stream.readLine();
//             widget->append( line );
//         }
//         file.close();
//     }
//
//     widget->setReadOnly( true );
//     tab()->addTab( widget, i18n("Diff") );
//     tab()->setCurrentIndex( tab()->indexOf(widget) );

    openDiff( job );

}

void KDevSubversionView::openDiff( SvnJobBase *job )
{
    SvnDiffJob *th = dynamic_cast<SvnDiffJob*>( job->svnThread() );
    d->m_part->core()->documentController()->openDocument( KUrl(th->out_name) );
}

void KDevSubversionView::printInfo( SvnJobBase* job )
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
        widget->append( QString("Repository URL: %1").arg(holder.url.prettyUrl() ) );
        widget->append( QString("Repository Root: %1").arg(holder.repoUrl.prettyUrl()) );
        widget->append( QString("Repository UUID: %1").arg(holder.repouuid) );
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
        if( !holder.copyFromUrl.isEmpty() ){
            widget->append( QString("Copied from: %1").arg(holder.copyFromUrl.prettyUrl()) );
            widget->append( QString("Copied from revision: %1").arg(holder.copyFromRevision) );
        }
        // todo have a room to add some more fields.
    }

    widget->setReadOnly( true );
    tab()->addTab( widget, "Information" );
    tab()->setCurrentIndex( tab()->indexOf(widget) );
}

void KDevSubversionView::printStatus( SvnJobBase* job )
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

void KDevSubversionView::openCat( SvnJobBase *job )
{
    if( job->error() ){
        KMessageBox::error( this, job->errorText(), "error" );
        return;
    }
    SvnCatJob *thread = dynamic_cast<SvnCatJob*>( job->svnThread() );
    if( !thread ) return;

    KTemporaryFile tmpFile;
    QString prefix = d->m_part->outputTmpDir()->name();
    tmpFile.setPrefix( prefix );
    tmpFile.setAutoRemove( false );
    tmpFile.setFileTemplate( prefix + QString("SVNCAT_") + thread->url().fileName() );

    if( !tmpFile.open() ){
        KMessageBox::error( this, "Fail to create temp file", "error" );
        return;
    }

    QByteArray ba( thread->m_total_string );
    tmpFile.write( ba );
    QString outputFile = tmpFile.fileName();
    tmpFile.close();

    KUrl url( outputFile );
    if( ! d->m_part->core()->documentController()->openDocument( url ) )
        KMessageBox::error( this, "Fail to open file", "error" );
}

void KDevSubversionView::slotJobFinished( SvnJobBase *job )
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

#include "svnview.moc"
