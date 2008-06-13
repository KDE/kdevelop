/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svncommitdialog.h"
#include "kdevsvnplugin.h"

#include <QKeyEvent>

#include <ktextedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>

#include <vcs/vcsjob.h>
#include <vcs/vcsstatusinfo.h>

SvnCommitDialog::SvnCommitDialog( KDevSvnPlugin *part, QWidget *parent )
    : KDialog( parent )
{
    m_part = part;

    ui.setupUi( mainWidget() );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setWindowTitle( i18n("Commit Dialog") );

    ui.files->resizeColumnToContents(0);
    ui.files->resizeColumnToContents(1);
    ui.message->installEventFilter(this);
    connect(this, SIGNAL( okClicked() ), SLOT( ok() ) );
    connect(this, SIGNAL( cancelClicked() ), SLOT( cancel() ) );
}

SvnCommitDialog::~SvnCommitDialog()
{}

bool SvnCommitDialog::eventFilter( QObject* o, QEvent* e )
{
    if( e->type() == QEvent::KeyPress )
    {
        QKeyEvent* k = static_cast<QKeyEvent*>(e);
        if( ( k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter ) && (k->modifiers() & Qt::ControlModifier) == Qt::ControlModifier )
        {
            ok();
            return true;
        }
    }
    return false;
}


void SvnCommitDialog::setKeepLocks( bool keeplock )
{
    ui.keepLocksChk->setChecked( keeplock );
}
void SvnCommitDialog::setRecursive( bool recursive )
{
    ui.recursiveChk->setChecked( recursive );
}

void SvnCommitDialog::setMessage( const QString& msg )
{
    ui.message->setPlainText( msg );
}
void SvnCommitDialog::setOldMessages( const QStringList& list )
{
    ui.lastmessages->clear();
    ui.lastmessages->addItems( list );
}


QString SvnCommitDialog::message() const
{
    return ui.message->toPlainText();
}

void SvnCommitDialog::setCommitCandidates( const KUrl::List &urls )
{
    kDebug(9510) << "Fetching status for urls:" << urls;
    KDevelop::VcsJob *job = m_part->status( urls );
    job->exec();
    kDebug(9510) << "Job done with status:" << job->status();
    if( job->status() != KDevelop::VcsJob::JobSucceeded )
    {
        kDebug(9510) << "Couldn't get status for urls: " << urls;
    }else
    {
        QVariant varlist = job->fetchResults();
        kDebug(9510) << "jobs result:" << varlist;
        foreach( QVariant var, varlist.toList() )
        {
            kDebug(9510) << "converting info:" << var;
            KDevelop::VcsStatusInfo info = qVariantValue<KDevelop::VcsStatusInfo>( var );

            kDebug(9510) << "converted info:"  << info.state() << info.url();
            QString state;
            switch( info.state() )
            {
                case KDevelop::VcsStatusInfo::ItemAdded:
                    insertRow( "A", info.url() );
                    break;
                case KDevelop::VcsStatusInfo::ItemDeleted:
                    insertRow( "D", info.url() );
                    break;
                case KDevelop::VcsStatusInfo::ItemModified:
                    insertRow( "M", info.url() );
                    break;
                default:
                    break;
            }
        }
    }
    if( ui.files->topLevelItemCount() == 0 )
    {
        reject();
    }
}
KUrl::List SvnCommitDialog::checkedUrls() const
{
    KUrl::List list;

    QTreeWidgetItemIterator it( ui.files, QTreeWidgetItemIterator::Checked );
    for( ; *it; ++it ){
        QString path = (*it)->text( 2 );
        list << KUrl( path );
    }
    return list;
}

bool SvnCommitDialog::recursive() const
{
    return ui.recursiveChk->isChecked();
}

bool SvnCommitDialog::keepLocks() const
{
    return ui.keepLocksChk->isChecked();
}

void SvnCommitDialog::insertRow( const QString& state, const KUrl& url )
{
    QStringList strings;
    strings << "" << state << url.prettyUrl();
    QTreeWidgetItem *item = new QTreeWidgetItem( ui.files, strings );
    item->setCheckState(0, Qt::Checked);
}

void SvnCommitDialog::ok()
{
    emit okClicked( this );
}

void SvnCommitDialog::cancel()
{
    emit cancelClicked( this );
}

#include "svncommitdialog.moc"
