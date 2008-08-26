/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "vcscommitdialog.h"

#include <KDE/KTextEdit>
#include <KDE/KComboBox>
#include <KDE/KLocale>
#include <KDE/KDebug>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugin.h>

#include "../vcsjob.h"
#include "../interfaces/ibasicversioncontrol.h"
#include "../vcsstatusinfo.h"

#include <QtCore/QHash>
#include <QtGui/QColor>
#include <QtCore/QRegExp>

#include "ui_vcscommitdialog.h"

namespace KDevelop
{

class VcsCommitDialogPrivate
{
public:

    VcsCommitDialogPrivate(VcsCommitDialog* dialog)
        : dlg(dialog)
    {}

    void insertRow( const QString& state, const KUrl& url,
                    const QColor &foregroundColor = Qt::black,
                    Qt::CheckState checkstate = Qt::Checked)
    {
        QStringList strings;
        strings << "" << state << url.pathOrUrl();
        QTreeWidgetItem *item = new QTreeWidgetItem( ui.files, strings );
        item->setForeground(2, foregroundColor);
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
    IPlugin *plugin;
    VcsCommitDialog* dlg;
    QHash<QString, KDevelop::VcsStatusInfo> statusInfos;
    Ui::VcsCommitDialog ui;
};

VcsCommitDialog::VcsCommitDialog( KDevelop::IPlugin *plugin, QWidget *parent )
    : KDialog( parent ), d(new VcsCommitDialogPrivate(this))
{
    d->vcsiface = plugin->extension<KDevelop::IBasicVersionControl>();
    d->plugin = plugin;

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

IPlugin* VcsCommitDialog::pluginToGetVCiface()
{
    return d->plugin;
}

void VcsCommitDialog::setCommitCandidates( const KUrl::List &urls )
{
    kDebug() << "Fetching status for urls:" << urls;
    if( !d->vcsiface )
    {
        kDebug() << "oops, no vcsiface";
        return;
    }
    //DVCS uses some "hack", see DistributedVersionControlPlugin::status
    //Thus DVCS gets statuses for all files in the repo. But project->relativeUrl() below helps us
    VcsJob *job = d->vcsiface->status( urls );
    job->exec();
    if( job->status() != VcsJob::JobSucceeded )
    {
        kDebug() << "Couldn't get status for urls: " << urls;
    }else
    {
        QVariant varlist = job->fetchResults();
        foreach( QVariant var, varlist.toList() )
        {
            VcsStatusInfo info = qVariantValue<KDevelop::VcsStatusInfo>( var );

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
                    d->insertRow( i18nc("file was added to versioncontrolsystem", "Added"), path, Qt::green );
                    break;
                case VcsStatusInfo::ItemDeleted:
                    d->insertRow( i18nc("file was deleted from versioncontrolsystem", "Deleted"), path, Qt::red );
                    break;
                case VcsStatusInfo::ItemModified:
                    d->insertRow( i18nc("version controlled file was modified", "Modified"), path );
                    break;
                case VcsStatusInfo::ItemUnknown:
                    d->insertRow( i18nc("file is not known to versioncontrolsystem", "Unknown"), 
                                  path, Qt::green, Qt::Unchecked );
                    break;
                //DVCS part
                //TODO: find better names, than "C <something>"
                //NOTE: KDevVcsCommonPlugin::commit() will require fix!!!
                case VcsStatusInfo::ItemAddedIndex:
                    d->insertRow( i18nc("file was added to index", "C Added"), path, Qt::green );
                    break;
                case VcsStatusInfo::ItemDeletedIndex:
                    d->insertRow( i18nc("file was deleted from index", "C Deleted"), path, Qt::red );
                    break;
                case VcsStatusInfo::ItemModifiedIndex:
                    d->insertRow( i18nc("file was modified in index", "C Modified"), path);
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
            addItems << info.url();
        }
        list << info.url();
    }
    if( !addItems.isEmpty() ) {
        KDevelop::VcsJob* job = d->vcsiface->add( addItems, IBasicVersionControl::NonRecursive );
        job->exec();
    }
    return list;
}

void VcsCommitDialog::getDVCSlists(KUrl::List &resetFiles, KUrl::List &addFiles, KUrl::List &rmFiles) const
{
    QTreeWidgetItemIterator it(d->ui.files);
    for( ; *it; ++it )
    {
        bool indexed = (*it)->text(1).contains(QRegExp("C *"));
        bool deleted = (*it)->text(1).contains(QString("Deleted"));
        bool unchecked = (*it)->checkState(0) == Qt::Unchecked;

        KUrl path;
        VcsStatusInfo info = d->statusInfos.value((*it)->text(2));
        if (indexed && unchecked)
            resetFiles << info.url();
        if (!indexed && !unchecked)
        {
            if (deleted)
                rmFiles << info.url();
            else
                addFiles << info.url();
        }
    }
}

bool VcsCommitDialog::recursive() const
{
    return d->ui.recursiveChk->isChecked();
}


}

#include "vcscommitdialog.moc"
