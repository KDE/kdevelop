/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnlogviewwidgets.h"
#include "ui_logviewoptiondlg.h"
#include "svnblamewidgets.h"
#include "svncore.h"
#include "svnthreads.h"
#include "svnrevision.h"
// #include "svnmodels.h" // included int blamewidget
#include "svnjobbase.h"
#include <kaction.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kurl.h>
#include <kdebug.h>
#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QContextMenuEvent>
#include <QCursor>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QListWidget>

SvnLogviewWidget::SvnLogviewWidget( const KUrl &url, KDevSubversionPart *part, QWidget *parent )
    :QWidget(parent), Ui::SvnLogviewWidget(), m_part(part)
    ,m_url(url)
{
    Ui::SvnLogviewWidget::setupUi(this);

    m_logviewModel= new SvnLogModel(this);
    treeView->setModel( m_logviewModel );
    treeView->sortByColumn(0, Qt::DescendingOrder);
    treeView->setContextMenuPolicy( Qt::CustomContextMenu );

    m_logviewDetailedModel = new SvnChangedPathModel(this);
    listView->setModel( m_logviewDetailedModel );

    connect( treeView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(treeViewClicked(const QModelIndex &)) );
    connect( treeView, SIGNAL(customContextMenuRequested( const QPoint & )),
             this, SLOT( customContextMenuEvent( const QPoint & ) ) );
    connect( listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(listViewClicked(const QModelIndex &)) );
}
SvnLogviewWidget::~SvnLogviewWidget()
{}
void SvnLogviewWidget::refreshWithNewData( const QList<SvnLogHolder>& datalist )
{
    m_logviewModel->setLogList( datalist );
    treeView->resizeColumnToContents(0);
    treeView->resizeColumnToContents(1);
}

void SvnLogviewWidget::customContextMenuEvent( const QPoint &point )
{
    m_contextIndex = treeView->indexAt( point );
    if( !m_contextIndex.isValid() ){
        kDebug(9500) << "contextMenu is not in TreeView";
        return;
    }

    KMenu menu( this );

    QAction *action = menu.addAction(i18n("Blame this Revision"));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(blameRev()) );

    action = menu.addAction(i18n("Diff to previous revision"));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(diffToPrev()) );

    action = menu.addAction(i18n("Cat this revision"));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(catThisRev()) );

    menu.exec( treeView->viewport()->mapToGlobal(point) );
}

void SvnLogviewWidget::blameRev()
{
    qlonglong rev = m_logviewModel->logEntryForIndex( m_contextIndex ).rev;
    if( rev == -1 ){ //error
        return;
    }
    // note that blame is done on single file.
    SvnChangedPathModel* modifies = new SvnChangedPathModel(this);
    modifies->setChangedPaths( m_logviewModel->logEntryForIndex( m_contextIndex ).pathlist );
    SvnChangedPath selectedPath;
    if( modifies->rowCount() > 1 ){
        SvnFileSelectFromLogviewDlg dlg(i18n("Select one file to view annotation"),this);
        dlg.setCandidate( modifies );
        if( dlg.exec() == QDialog::Accepted ){
            selectedPath = dlg.selected();
        } else{
            return;
        }

    } else if( modifies->rowCount() == 1 ){
        selectedPath = m_logviewModel->logEntryForIndex( m_contextIndex ).pathlist[0];
    } else {
        return;
    }

    delete modifies;
    modifies = 0;

    QString relPath = selectedPath.path.section( '/', 1 );
    // get repository root URL
    SvnRevision nullRev;
    SvnJobBase* job = m_part->svncore()->createInfoJob( m_url, nullRev, nullRev, false );
    if( job->exec() != KDevelop::VcsJob::JobSucceeded ) return;
    SvnInfoJob *thread = dynamic_cast<SvnInfoJob*>(job->svnThread());
    QMap<KUrl, SvnInfoHolder> &infoMap = thread->m_holderMap;

    QList< SvnInfoHolder > holderList = infoMap.values();
    if( holderList.count() > 0 ){
        // get full Url
        SvnInfoHolder holder = holderList.first();
        KUrl absUrl(holder.repoUrl);
        absUrl.addPath( relPath );
        kDebug(9500) << "Blame requested on path" << absUrl;
        // final request
        SvnRevision rev1, rev2;
        rev1.setNumber( 0 );
        rev2.setNumber( rev );
        m_part->svncore()->spawnBlameThread( absUrl, rev1, rev2 );
    }
    else{
        return;
    }
}

void SvnLogviewWidget::diffToPrev()
{
    qlonglong rev = m_logviewModel->logEntryForIndex( m_contextIndex ).rev;
    if( rev == -1 ){ //error
        return;
    }
    SvnRevision rev1, rev2, peg_rev;
    rev1.setNumber( rev - 1 );
    rev2.setNumber( rev );
    if( m_url.isLocalFile() ){
        // peg revision is local.
        peg_rev.setKey( SvnRevision::Base );
    }

    m_part->svncore()->spawnDiffThread( m_url, m_url, peg_rev, rev1, rev2, true, false, false, false );
}

void SvnLogviewWidget::catThisRev()
{
    qlonglong rev = m_logviewModel->logEntryForIndex( m_contextIndex ).rev;
    if( rev == -1 ){ //error
        return;
    }
    SvnRevision rev1, peg_rev;
    peg_rev.setNumber( rev );
    rev1.setNumber( rev );

    // note that cat is done on single file.

    SvnChangedPathModel* modifies = new SvnChangedPathModel(this);
    modifies->setChangedPaths( m_logviewModel->logEntryForIndex( m_contextIndex ).pathlist );
    SvnChangedPath selectedPath;
    if( modifies->rowCount() > 1 ){
        SvnFileSelectFromLogviewDlg dlg("Select one file to cat", this);
        dlg.setCandidate( modifies );
        if( dlg.exec() == QDialog::Accepted ){
            selectedPath = dlg.selected();
        } else{
            return;
        }

    } else if( modifies->rowCount() == 1 ){
        selectedPath = m_logviewModel->logEntryForIndex( m_contextIndex ).pathlist[0];
    } else {
        return;
    }

    delete modifies;
    modifies = 0;

    QString relPath = selectedPath.path.section( '/', 1 );
    // get repository root URL
    SvnRevision nullRev;
    SvnJobBase* job = m_part->svncore()->createInfoJob( m_url, nullRev, nullRev, false );
    if( job->exec() != KDevelop::VcsJob::JobSucceeded ) return;
    SvnInfoJob *thread = dynamic_cast<SvnInfoJob*>(job->svnThread());
    QMap<KUrl, SvnInfoHolder> &infoMap = thread->m_holderMap;

    QList< SvnInfoHolder > holderList = infoMap.values();
    if( holderList.count() > 0 ){
        // get full Url
        SvnInfoHolder holder = holderList.first();
        KUrl absUrl(holder.repoUrl);
        absUrl.addPath( relPath );
        kDebug(9500) << "Cat requested on path" << absUrl;
        // final request
        m_part->svncore()->spawnCatThread( absUrl, peg_rev, rev1 );
    }
    else{
        return;
    }
}

void SvnLogviewWidget::treeViewClicked( const QModelIndex &index )
{
    m_logviewDetailedModel->setChangedPaths( m_logviewModel->logEntryForIndex( index ).pathlist );

//     QMenu menu( this );
//     QAction *action = menu.addAction( i18n( "Blame this revision" ) );
//     connect( action, SIGNAL(triggered(bool)), this, SLOT(blameRev()) );
//     menu.exec(QCursor::pos());
}
void SvnLogviewWidget::listViewClicked( const QModelIndex &/*index*/ )
{
}

/////////////////////////////////////////////////////////////////////////

class SvnLogviewOptionDialogPrivate
{
public:
    Ui::SvnLogViewOptionDlg ui;
    KUrl m_url;
};

SvnLogviewOptionDialog::SvnLogviewOptionDialog( const KUrl &url, QWidget *parent )
    : KDialog( parent )
    , d( new SvnLogviewOptionDialogPrivate )
{
    d->m_url = url;
    QWidget *widget = new QWidget( this );
    d->ui.setupUi( widget );
    setMainWidget( widget );
    setCaption( QString("LogView for %1").arg(d->m_url.toLocalFile()) );
    setButtons( KDialog::Ok | KDialog::Cancel );

    d->ui.startRevWidget->setKey( SvnRevision::Base );
    d->ui.startRevWidget->enableType( SvnRevision::Kind );
    d->ui.endRevWidget->setNumber(1);
    d->ui.endRevWidget->enableType( SvnRevision::Number );
}

SvnLogviewOptionDialog::~SvnLogviewOptionDialog()
{
    delete d;
}

SvnRevision SvnLogviewOptionDialog::startRev()
{
    return d->ui.startRevWidget->revision();
}

SvnRevision SvnLogviewOptionDialog::endRev()
{
    return d->ui.endRevWidget->revision();
}

int SvnLogviewOptionDialog::limit()
{
    return d->ui.limitNumber->value();
}

bool SvnLogviewOptionDialog::strictNode()
{
    return d->ui.checkBox1->isChecked();
}

SvnFileSelectFromLogviewDlg::SvnFileSelectFromLogviewDlg( const QString &title, QWidget *parent )
    : KDialog( parent )
{
    setWindowTitle( title );

    widget = new QListView( this );
    widget->setSelectionBehavior( QAbstractItemView::SelectRows );
    widget->setSelectionMode( QAbstractItemView::SingleSelection );
    setMainWidget( widget );

    setButtons( KDialog::Ok | KDialog::Cancel );
}

SvnFileSelectFromLogviewDlg::~SvnFileSelectFromLogviewDlg()
{
}

void SvnFileSelectFromLogviewDlg::setCandidate( SvnChangedPathModel* model )
{
    m_candidates = model;
    widget->setModel( model );
}

SvnChangedPath SvnFileSelectFromLogviewDlg::selected()
{
    return m_selected;
}

void SvnFileSelectFromLogviewDlg::accept()
{
    while( true ){
        QModelIndexList index = widget->selectionModel()->selectedIndexes();
        if( !index.isEmpty() ){
            m_selected = m_candidates->changedPathForIndex( index[0] );
            break;
        }
        else{
            KMessageBox::error( this, i18n("Select file from list") );
        }
    }
    KDialog::accept();
}

#include "svnlogviewwidgets.moc"
