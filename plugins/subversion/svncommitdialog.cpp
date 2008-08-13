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
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

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
    ui.message->selectAll();
    connect(this, SIGNAL( okClicked() ), SLOT( ok() ) );
    connect(this, SIGNAL( cancelClicked() ), SLOT( cancel() ) );
}

SvnCommitDialog::~SvnCommitDialog()
{}

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
            QString path = info.url().pathOrUrl();
            KDevelop::IProject* project = m_part->core()->projectController()->findProjectForUrl( info.url() );
            if( project )
            {
                path = project->relativeUrl( info.url() ).pathOrUrl();
            }
            statusInfos.insert(path, info);
            switch( info.state() )
            {
                case KDevelop::VcsStatusInfo::ItemAdded:
                    insertRow( i18nc("file was added to subversion", "Added"), path );
                    break;
                case KDevelop::VcsStatusInfo::ItemDeleted:
                    insertRow( i18nc("file was deleted to subversion", "Deleted"), path );
                    break;
                case KDevelop::VcsStatusInfo::ItemModified:
                    insertRow( i18nc("subversion controlled file was modified", "Modified"), path );
                    break;
                case KDevelop::VcsStatusInfo::ItemUnknown:
                    insertRow( i18nc("File not known to subversion", "Unknown"), path, Qt::Unchecked );
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
    KUrl::List addItems;
    QTreeWidgetItemIterator it( ui.files, QTreeWidgetItemIterator::Checked );
    for( ; *it; ++it ){
        KUrl path;
        KDevelop::VcsStatusInfo info = statusInfos.value((*it)->text(2));
        if( info.state() == KDevelop::VcsStatusInfo::ItemUnknown ) {
            kDebug() << "adding" << info.url() << "to additems";
            addItems << info.url();
        }
        list << info.url();
    }
    if( !addItems.isEmpty() ) {
        kDebug() << "Adding new files" << addItems;
        KDevelop::VcsJob* job = m_part->add( addItems, KDevelop::IBasicVersionControl::NonRecursive );
        job->exec();
    } else {
        kDebug() << "oops no files to add";
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

void SvnCommitDialog::insertRow( const QString& state, const KUrl& url, Qt::CheckState checkstate )
{
    QStringList strings;
    strings << "" << state << url.pathOrUrl();
    QTreeWidgetItem *item = new QTreeWidgetItem( ui.files, strings );
    item->setCheckState(0, checkstate);
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
