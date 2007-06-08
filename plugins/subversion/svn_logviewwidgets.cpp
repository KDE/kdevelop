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

#include "ui_uilogview_option_dlg.h"
#include "svn_logviewwidgets.h"
#include "svn_blamewidgets.h"
#include "subversion_core.h"
#include "subversionthreads.h"
#include "subversion_utils.h"
// #include "svn_models.h" // included int blamewidget
#include "svnkjobbase.h"
#include <kaction.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kurl.h>
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

SvnLogviewWidget::SvnLogviewWidget( KUrl &url, KDevSubversionPart *part, QWidget *parent )
    :QWidget(parent), Ui::SvnLogviewWidget()
    ,m_url(url), m_part(part)
{
    Ui::SvnLogviewWidget::setupUi(this);

    m_item = new LogItem();
    m_logviewModel= new LogviewTreeModel(m_item);
    treeView->setModel( m_logviewModel );
    treeView->sortByColumn(0, Qt::DescendingOrder);
    treeView->setContextMenuPolicy( Qt::CustomContextMenu );

    m_logviewDetailedModel = new LogviewDetailedModel(m_item);
    listView->setModel( m_logviewDetailedModel );

    connect( treeView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(treeViewClicked(const QModelIndex &)) );
    connect( treeView, SIGNAL(customContextMenuRequested( const QPoint & )),
             this, SLOT( customContextMenuEvent( const QPoint & ) ) );
    connect( listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(listViewClicked(const QModelIndex &)) );
}
SvnLogviewWidget::~SvnLogviewWidget()
{}
void SvnLogviewWidget::refreshWithNewData( QList<SvnLogHolder> datalist )
{
    m_logviewModel->prepareItemUpdate();
    m_item->setHolderList( datalist );
    m_logviewModel->finishedItemUpdate();
    treeView->resizeColumnToContents(0);
    treeView->resizeColumnToContents(1);
}

void SvnLogviewWidget::customContextMenuEvent( const QPoint &point )
{
    m_contextIndex = treeView->indexAt( point );
    if( !m_contextIndex.isValid() ){
        kDebug() << " contextMenu is not in TreeView " << endl;
        return;
    }

    KMenu menu( this );

    QAction *action = menu.addAction(i18n("Blame this Revision"));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(blameRev()) );

    action = menu.addAction(i18n("Diff to previous revision"));
    connect( action, SIGNAL(triggered(bool)), this, SLOT(diffToPrev()) );

    menu.exec( treeView->viewport()->mapToGlobal(point) );
}

void SvnLogviewWidget::contextMenuEvent( QContextMenuEvent * event )
{
    kDebug() << " SvnLogviewWidget::contextMenuEvent " << endl;
//     m_contextIndex = treeView->indexAt( event->pos() );
//     if( !m_contextIndex.isValid() ){
//         kDebug() << " contextMenu is not in TreeView " << endl;
//         return;
//     }
//
//     KMenu menu( this );
//
//     QAction *action = menu.addAction(i18n("Blame this Revision"));
//     connect( action, SIGNAL(triggered(bool)), this, SLOT(blameRev()) );
//     menu.exec(event->globalPos());
//
    QWidget::contextMenuEvent( event );
}

void SvnLogviewWidget::blameRev()
{
    long rev = m_logviewModel->revision( m_contextIndex );
    if( rev == -1 ){ //error
        return;
    }
    // note that blame is done on single file.
    QStringList modifies = m_logviewModel->modifiedLists( m_contextIndex );
    QString selectedPath;
    if( modifies.count() > 1 ){
        SvnBlameFileSelectDlg dlg(this);
        dlg.setCandidate( &modifies );
        if( dlg.exec() == QDialog::Accepted ){
            selectedPath = dlg.selected();
        } else{
            return;
        }

    } else if( modifies.count() == 1 ){
        selectedPath = modifies.at(0);
    } else {
        return;
    }

    QString relPath = selectedPath.section( '/', 1 );
    // get repository root URL
    SvnUtils::SvnRevision nullRev;
    SvnKJobBase* job = m_part->svncore()->createInfoJob( m_url, nullRev, nullRev, false );
    if( !job->exec() ) return;
    SvnInfoJob *thread = dynamic_cast<SvnInfoJob*>(job->svnThread());
    QMap<KUrl, SvnInfoHolder> &infoMap = thread->m_holderMap;

    QList< SvnInfoHolder > holderList = infoMap.values();
    if( holderList.count() > 0 ){
        // get full Url
        SvnInfoHolder holder = holderList.first();
        KUrl absUrl =  holder.reposRootUrl;
        absUrl.addPath( relPath );
        kDebug() << " Blame requested on path " << absUrl << endl;
        // final request
        SvnRevision rev1, rev2;
        rev1.setNumber( 0 );
        rev2.setNumber( rev );
        m_part->svncore()->spawnBlameThread( absUrl, true,  rev1, rev2 );
    }
    else{
        return;
    }
}

void SvnLogviewWidget::diffToPrev()
{
    long rev = m_logviewModel->revision( m_contextIndex );
    if( rev == -1 ){ //error
        return;
    }
    SvnUtils::SvnRevision rev1, rev2;
    rev1.setNumber( rev - 1 );
    rev2.setNumber( rev );
    m_part->svncore()->spawnDiffThread( m_url, m_url, rev1, rev2, true, false, false, false );
}
void SvnLogviewWidget::treeViewClicked( const QModelIndex &index )
{
    m_logviewDetailedModel->setNewRevision( index );

//     QMenu menu( this );
//     QAction *action = menu.addAction( i18n( "Blame this revision" ) );
//     connect( action, SIGNAL(triggered(bool)), this, SLOT(blameRev()) );
//     menu.exec(QCursor::pos());
}
void SvnLogviewWidget::listViewClicked( const QModelIndex &index )
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

    d->ui.startDateEdit->setDateTime( QDateTime::currentDateTime() );
    d->ui.endDateEdit->setDateTime( QDateTime::currentDateTime() );
}

SvnLogviewOptionDialog::~SvnLogviewOptionDialog()
{
    delete d;
}

bool SvnLogviewOptionDialog::repositLog()
{
    return d->ui.reposit->isChecked();
}

SvnRevision SvnLogviewOptionDialog::startRev()
{
    SvnUtils::SvnRevision rev;
    if( d->ui.startNum->isChecked() ){
        rev.setNumber( d->ui.startNumEdit->value() );
        return rev;
    }
    else if( d->ui.startKind->isChecked() ){
        // note. If you add more keywords in .ui, you should update below.
        if( d->ui.startKindEdit->currentText() == "HEAD" ){
            rev.setKey( SvnUtils::SvnRevision::HEAD );
        }
        else if( d->ui.startKindEdit->currentText() == "BASE" ){
            rev.setKey( SvnUtils::SvnRevision::BASE );
        }
        else if( d->ui.startKindEdit->currentText() == "PREV" ){
            rev.setKey( SvnUtils::SvnRevision::PREV );
        }
        else if( d->ui.startKindEdit->currentText() == "COMMITTED" ){
            rev.setKey( SvnUtils::SvnRevision::COMMITTED );
        }
        return rev;
    }
    else if( d->ui.startDate->isChecked() ){
        rev.setDate( d->ui.startDateEdit->dateTime() );
        return rev;
    }
    else{
        // should not reach here
        return rev;
    }
}

SvnRevision SvnLogviewOptionDialog::endRev()
{
    SvnUtils::SvnRevision rev;
    if( d->ui.endNum->isChecked() ){
        rev.setNumber( d->ui.endNumEdit->value() );
        return rev;
    }
    else if( d->ui.endKind->isChecked() ){
        // note. If you add more keywords in .ui, you should update below.
        if( d->ui.endKindEdit->currentText() == "HEAD" ){
            rev.setKey( SvnUtils::SvnRevision::HEAD );
        }
        else if( d->ui.endKindEdit->currentText() == "BASE" ){
            rev.setKey( SvnUtils::SvnRevision::BASE );
        }
        else if( d->ui.endKindEdit->currentText() == "PREV" ){
            rev.setKey( SvnUtils::SvnRevision::PREV );
        }
        else if( d->ui.endKindEdit->currentText() == "COMMITTED" ){
            rev.setKey( SvnUtils::SvnRevision::COMMITTED );
        }
        return rev;
    }
    else if( d->ui.endDate->isChecked() ){
        rev.setDate( d->ui.endDateEdit->dateTime() );
        return rev;
    }
    else{
        // should not reach here
        return rev;
    }
}

int SvnLogviewOptionDialog::limit()
{
    return d->ui.limitNumber->value();
}

bool SvnLogviewOptionDialog::strictNode()
{
    return d->ui.checkBox1->isChecked();
}

#include "svn_logviewwidgets.moc"
