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

#include <QtCore/QHash>
#include <QtGui/QBrush>
#include <QtCore/QRegExp>

#include <KDE/KTextEdit>
#include <KDE/KComboBox>
#include <KDE/KLocale>
#include <KDE/KDebug>
#include <KDE/KColorScheme>
#include <KDE/KMessageBox>
#include <kparts/mainwindow.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugin.h>
#include <interfaces/iuicontroller.h>

#include "../vcsjob.h"
#include "../interfaces/ibasicversioncontrol.h"
#include "../interfaces/idistributedversioncontrol.h"
#include "../interfaces/icentralizedversioncontrol.h"
#include "../vcsstatusinfo.h"

#include "ui_vcscommitdialog.h"
#include "vcsdiffpatchsources.h"
#include <KComponentData>
#include <vcspluginhelper.h>

namespace KDevelop
{

class VcsCommitDialogPrivate
{
public:

    VcsCommitDialogPrivate(VcsCommitDialog* dialog)
        : dlg(dialog)
    {}

    void insertRow( const QString& state, const KUrl& url,
                    const KStatefulBrush &foregroundColor = KStatefulBrush(KColorScheme::View, KColorScheme::NormalText),
                    Qt::CheckState checkstate = Qt::Checked)
    {
        QStringList strings;
        strings << "" << state << ICore::self()->projectController()->prettyFileName(url, KDevelop::IProjectController::FormatPlain);
        QTreeWidgetItem *item = new QTreeWidgetItem( ui.files, strings );
        item->setData(0, Qt::UserRole, url);
        item->setForeground(2,  foregroundColor.brush(dlg));
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
    
    QList< KUrl > selection() {
        if(!m_selection.isEmpty())
            return m_selection;
        
        QList< KUrl > ret;
        
        QTreeWidgetItemIterator it( ui.files, QTreeWidgetItemIterator::Checked );
        for( ; *it; ++it ){
            QVariant v = (*it)->data(0, Qt::UserRole);
            Q_ASSERT(v.canConvert<KUrl>());
            ret << v.value<KUrl>();
        }
        
        return ret;
    }

    QMap<KUrl, VcsStatusInfo::State> urls;
    IPlugin *plugin;
    VcsCommitDialog* dlg;
    QHash<KUrl, KDevelop::VcsStatusInfo> statusInfos;
    Ui::VcsCommitDialog ui;
    KDevelop::VcsDiff diff;
    QList< KUrl > m_selection;
    QStringList m_oldMessages;
};

VcsCommitDialog::VcsCommitDialog( KDevelop::IPlugin *plugin, QWidget *parent )
    : KDialog( parent ), d(new VcsCommitDialogPrivate(this))
{
    d->plugin = plugin;

    d->m_oldMessages = retrieveOldCommitMessages();
    
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
    addOldCommitMessage(d->ui.message->toPlainText());
    delete d;
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

IPlugin* VcsCommitDialog::versionControlPlugin()
{
    return d->plugin;
}

void VcsCommitDialog::setCommitCandidatesAndShow( const KUrl &url )
{
    kDebug() << "Fetching status for urls:" << url;
    KDevelop::IBasicVersionControl *vcsiface = d->plugin->extension<KDevelop::IBasicVersionControl>();
    if( !vcsiface )
    {
        kDebug() << "oops, no vcsiface";
        return;
    }
    
    d->urls.clear();
    d->diff.clear();
    
    //DVCS uses some "hack", see DistributedVersionControlPlugin::status()
    //Thus DVCS gets statuses for all files in the repo. But project->relativeUrl() below helps us
    VcsJob *job = vcsiface->status( url );
    job->exec();
    if( job->status() != VcsJob::JobSucceeded )
    {
        kDebug() << "Couldn't get status for urls: " << url;
    }else
    {
        QVariant varlist = job->fetchResults();

        KStatefulBrush deletedRed(KColorScheme::View, KColorScheme::NegativeText);
        KStatefulBrush newGreen(KColorScheme::View, KColorScheme::ActiveText);

        foreach( const QVariant &var, varlist.toList() )
        {
            VcsStatusInfo info = qVariantValue<KDevelop::VcsStatusInfo>( var );

            d->statusInfos.insert(info.url(), info);
            
            QString state;
            KStatefulBrush brush(KColorScheme::View, KColorScheme::NormalText);
            Qt::CheckState checked = Qt::Checked;
            
            switch( info.state() )
            {
                case VcsStatusInfo::ItemAdded:
                    state = i18nc("file was added to versioncontrolsystem", "Added");
                    brush = newGreen;
                    break;
                case VcsStatusInfo::ItemDeleted:
                    state = i18nc("file was deleted from versioncontrolsystem", "Deleted");
                    brush = deletedRed;
                    break;
                case VcsStatusInfo::ItemModified:
                    state = i18nc("version controlled file was modified", "Modified");
                    break;
                case VcsStatusInfo::ItemUnknown:
                    state = i18nc("file is not known to versioncontrolsystem", "Unknown");
                    brush = newGreen;
                    checked = Qt::Unchecked;
                    break;
                default:
                    break;
            }
            
            if(!state.isEmpty())
            {
                d->insertRow(state, info.url(), brush, checked);
                d->urls[info.url()] = info.state();
            }
        }
    }
    if( d->ui.files->topLevelItemCount() == 0 )
    {
        reject();
    }
    
    {
    KDevelop::VcsJob* job = vcsiface->diff(url,
                                        KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Base),
                                        KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Working));

    connect(job, SIGNAL(finished(KJob*)), this, SLOT(commitDiffJobFinished(KJob*)));
    job->exec();
    }
    
    VCSCommitDiffPatchSource* patchSource = new VCSCommitDiffPatchSource(d->diff, d->urls, vcsiface, d->m_oldMessages);
    
    if(showVcsDiff(patchSource))
    {
        connect(patchSource, SIGNAL(reviewFinished(QString,QList<KUrl>)), this, SLOT(reviewFinished(QString,QList<KUrl>)));
        connect(patchSource, SIGNAL(reviewCancelled(QString)), this, SLOT(reviewCancalled(QString)));
        connect(patchSource, SIGNAL(destroyed(QObject*)), SLOT(deleteLater()));
    }else{
        delete patchSource;
        show();
    }
}

void VcsCommitDialog::reviewCancalled(QString message)
{
    d->ui.message->setPlainText(message);
}

void VcsCommitDialog::reviewFinished(QString message, QList< KUrl > selection)
{
    d->ui.message->setPlainText(message);
    d->m_selection = selection;
    emit doCommit(this);
}

KUrl::List VcsCommitDialog::determineUrlsForCheckin()
{
    KUrl::List list;
    KUrl::List addItems;

    foreach(const KUrl& url, d->selection()) {
        VcsStatusInfo info = d->statusInfos[url];
        
        if( info.state() == VcsStatusInfo::ItemUnknown ) {
            addItems << info.url();
        }
        list << info.url();
    }

    if (!addItems.isEmpty()) {
        IBasicVersionControl* iface = d->plugin->extension<IBasicVersionControl>();
        VcsJob* job = iface->add( addItems, IBasicVersionControl::NonRecursive );
        job->exec();
    }

    return list;
}

bool VcsCommitDialog::recursive() const
{
    return d->ui.recursiveChk->isChecked();
}

void VcsCommitDialog::commitDiffJobFinished(KJob* job)
{
    KDevelop::VcsJob* vcsjob = dynamic_cast<KDevelop::VcsJob*>(job);
    Q_ASSERT(vcsjob);

    if (vcsjob) {
        if (vcsjob->status() == KDevelop::VcsJob::JobSucceeded) {
            d->diff = vcsjob->fetchResults().value<KDevelop::VcsDiff>();
        } else {
            KMessageBox::error(ICore::self()->uiController()->activeMainWindow(), vcsjob->errorString(), i18n("Unable to get difference."));
        }

        vcsjob->disconnect(this);
    }
}

}

#include "vcscommitdialog.moc"
