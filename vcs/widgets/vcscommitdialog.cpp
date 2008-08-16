/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "vcscommitdialog.h"

#include <ktextedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include "../vcsjob.h"
#include "../interfaces/ibasicversioncontrol.h"
#include "../vcsstatusinfo.h"

#include <QtCore/QHash>

#include "ui_vcscommitdialog.h"

namespace KDevelop
{

class VcsCommitDialogPrivate
{
public:

    VcsCommitDialogPrivate(VcsCommitDialog* dialog)
        : dlg(dialog)
    {}

    void insertRow( const QString& state, const KUrl& url, Qt::CheckState checkstate = Qt::Checked )
    {
        QStringList strings;
        strings << "" << state << url.pathOrUrl();
        QTreeWidgetItem *item = new QTreeWidgetItem( ui.files, strings );
        item->setCheckState(0, checkstate);
    }

    void ok()
    {
        emit dlg->doCommit(dlg);
    }

    void cancel()
    {
        emit dlg->cancelCommit(dlg);
    }

    IBasicVersionControl *vcsiface;
    VcsCommitDialog* dlg;
    QHash<QString, KDevelop::VcsStatusInfo> statusInfos;
    Ui::VcsCommitDialog ui;
};

VcsCommitDialog::VcsCommitDialog( IBasicVersionControl* iface, QWidget *parent )
    : KDialog( parent ), d(new VcsCommitDialogPrivate(this))
{
    d->vcsiface = iface;

    d->ui.setupUi( mainWidget() );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setWindowTitle( i18n("Commit Message") );

    d->ui.files->resizeColumnToContents(0);
    d->ui.files->resizeColumnToContents(1);
    d->ui.message->selectAll();
    connect(this, SIGNAL( okClicked() ), SLOT( ok() ) );
    connect(this, SIGNAL( cancelClicked() ), SLOT( cancel() ) );
}

VcsCommitDialog::~VcsCommitDialog()
{
}

void VcsCommitDialog::setRecursive( bool recursive )
{
    d->ui.recursiveChk->setChecked( recursive );
}

void VcsCommitDialog::setMessage( const QString& msg )
{
    d->ui.message->setPlainText( msg );
}
void VcsCommitDialog::setOldMessages( const QStringList& list )
{
    d->ui.lastmessages->clear();
    d->ui.lastmessages->addItems( list );
}


QString VcsCommitDialog::message() const
{
    return d->ui.message->toPlainText();
}

void VcsCommitDialog::setCommitCandidates( const KUrl::List &urls )
{
    kDebug() << "Fetching status for urls:" << urls;
    if( !d->vcsiface )
    {
        kDebug() << "oops, no vcsiface";
        return;
    }
    VcsJob *job = d->vcsiface->status( urls );
    job->exec();
    kDebug() << "Job done with status:" << job->status();
    if( job->status() != VcsJob::JobSucceeded )
    {
        kDebug() << "Couldn't get status for urls: " << urls;
    }else
    {
        QVariant varlist = job->fetchResults();
        kDebug(9510) << "jobs result:" << varlist;
        foreach( QVariant var, varlist.toList() )
        {
            kDebug(9510) << "converting info:" << var;
            VcsStatusInfo info = qVariantValue<KDevelop::VcsStatusInfo>( var );

            kDebug(9510) << "converted info:"  << info.state() << info.url();
            QString state;
            QString path = info.url().pathOrUrl();
            IProject* project = ICore::self()->projectController()->findProjectForUrl( info.url() );
            if( project )
            {
                path = project->relativeUrl( info.url() ).pathOrUrl();
            }
            d->statusInfos.insert(path, info);
            switch( info.state() )
            {
                case VcsStatusInfo::ItemAdded:
                    d->insertRow( i18nc("file was added to versioncontrolsystem", "Added"), path );
                    break;
                case VcsStatusInfo::ItemDeleted:
                    d->insertRow( i18nc("file was deleted to versioncontrolsystem", "Deleted"), path );
                    break;
                case VcsStatusInfo::ItemModified:
                    d->insertRow( i18nc("version controlled file was modified", "Modified"), path );
                    break;
                case VcsStatusInfo::ItemUnknown:
                    d->insertRow( i18nc("File not known to versioncontrolsystem", "Unknown"), path, Qt::Unchecked );
                    break;
                default:
                    break;
            }
        }
    }
    if( d->ui.files->topLevelItemCount() == 0 )
    {
        reject();
    }
}
KUrl::List VcsCommitDialog::checkedUrls() const
{
    KUrl::List list;
    KUrl::List addItems;
    QTreeWidgetItemIterator it( d->ui.files, QTreeWidgetItemIterator::Checked );
    for( ; *it; ++it ){
        KUrl path;
        VcsStatusInfo info = d->statusInfos.value((*it)->text(2));
        if( info.state() == VcsStatusInfo::ItemUnknown ) {
            kDebug() << "adding" << info.url() << "to additems";
            addItems << info.url();
        }
        list << info.url();
    }
    if( !addItems.isEmpty() ) {
        kDebug() << "Adding new files" << addItems;
        KDevelop::VcsJob* job = d->vcsiface->add( addItems, IBasicVersionControl::NonRecursive );
        job->exec();
    } else {
        kDebug() << "oops no files to add";
    }
    return list;
}

bool VcsCommitDialog::recursive() const
{
    return d->ui.recursiveChk->isChecked();
}


}

#include "vcscommitdialog.moc"
