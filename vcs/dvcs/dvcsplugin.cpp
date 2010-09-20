/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for DVCS (added templates)                                    *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef DVCS_PLUGIN_CC
#define DVCS_PLUGIN_CC

#include "dvcsplugin.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QString>

#include <KDE/KParts/PartManager>
#include <KDE/KParts/Part>

#include <KDE/KActionCollection>
#include <KDE/KMessageBox>
#include <KDE/KAction>

#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>

#include "dvcsjob.h"
#include "ui/dvcsmainview.h"
#include "ui/dvcsgenericoutputview.h"
#include "ui/importdialog.h"
#include "ui/importmetadatawidget.h"
#include "ui/logview.h"
#include "ui/branchmanager.h"
// #include "ui/commitmanager.h"
#include "ui/revhistory/commitlogmodel.h"
#include "ui/revhistory/commitView.h"
#include <language/interfaces/editorcontext.h>
#include <vcs/vcspluginhelper.h>
#include <KMenu>
#include <kparts/mainwindow.h>
#include <interfaces/idocumentcontroller.h>

namespace KDevelop
{

struct DistributedVersionControlPluginPrivate {
    explicit DistributedVersionControlPluginPrivate(DistributedVersionControlPlugin * pThis)
            : m_factory(new KDevDVCSViewFactory(pThis))
            , m_common(new VcsPluginHelper(pThis, pThis)) {}
    KDevDVCSViewFactory* m_factory;
    std::auto_ptr<VcsPluginHelper> m_common;
};

//class DistributedVersionControlPlugin
DistributedVersionControlPlugin::DistributedVersionControlPlugin(QObject *parent, KComponentData compData)
        : IPlugin(compData, parent)
        , d(new DistributedVersionControlPluginPrivate(this))
{}

DistributedVersionControlPlugin::~DistributedVersionControlPlugin()
{
    //TODO: Find out why this crashes on the svn tests delete d->m_factory;
    delete d;
}

// End:  KDevelop::IBasicVersionControl


// Begin:  KDevelop::IDistributedVersionControl

// End:  KDevelop::IDistributedVersionControl


KDevelop::VcsImportMetadataWidget*
DistributedVersionControlPlugin::createImportMetadataWidget(QWidget* parent)
{
    return new ImportMetadataWidget(parent);
}

KDevelop::ContextMenuExtension
DistributedVersionControlPlugin::contextMenuExtension(Context* context)
{
    d->m_common->setupFromContext(context);
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();

    bool isWorkingDirectory = false;
    foreach(const KUrl &url, ctxUrlList) {
        if (isValidDirectory(url)) {
            isWorkingDirectory = true;
            break;
        }
    }

    if (!isWorkingDirectory) {  // Not part of a repository
        return ContextMenuExtension();
    }

    QMenu * menu = d->m_common->commonActions();
    menu->addSeparator();    
    menu->addAction(KIcon("arrow-up-double"), i18n("Push"), this, SLOT(ctxPush()));
    menu->addAction(KIcon("arrow-down-double"), i18n("Pull"), this, SLOT(ctxPull()));
    menu->addAction(i18n("Branches..."), this, SLOT(ctxBranchManager()))->setEnabled(ctxUrlList.count()==1);
    menu->addAction(i18n("Revision History"), this, SLOT(ctxRevHistory()))->setEnabled(ctxUrlList.count()==1);
    additionalMenuEntries(menu, ctxUrlList);

    ContextMenuExtension menuExt;
    menuExt.addAction(ContextMenuExtension::VcsGroup, menu->menuAction());

    return menuExt;

}

void DistributedVersionControlPlugin::additionalMenuEntries(QMenu* menu, const KUrl::List& urls)
{}

void DistributedVersionControlPlugin::slotInit()
{
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();
    Q_ASSERT(!ctxUrlList.isEmpty());

    KUrl url = ctxUrlList.front();
    QFileInfo repoInfo = QFileInfo(url.toLocalFile());
    if (repoInfo.isFile())
        url = repoInfo.path();

    ImportDialog dlg(this, url);
    dlg.exec();
}

void DistributedVersionControlPlugin::ctxPush()
{
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();
    Q_ASSERT(!ctxUrlList.isEmpty());

    VcsJob* job = push(ctxUrlList.front().toLocalFile(), VcsLocation());
    connect(job, SIGNAL(result(KJob*)), this, SIGNAL(jobFinished(KJob*)));
    ICore::self()->runController()->registerJob(job);
}

void DistributedVersionControlPlugin::ctxPull()
{
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();
    Q_ASSERT(!ctxUrlList.isEmpty());

    VcsJob* job = pull(VcsLocation(), ctxUrlList.front().toLocalFile());
    connect(job, SIGNAL(result(KJob*)), this, SIGNAL(jobFinished(KJob*)));
    ICore::self()->runController()->registerJob(job);
}

static QString stripPathToDir(const QString &path)
{
    return QFileInfo(path).absolutePath();
}

void DistributedVersionControlPlugin::ctxBranchManager()
{
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();
    Q_ASSERT(!ctxUrlList.isEmpty());    
    
    ICore::self()->documentController()->saveAllDocuments();
    BranchManager branchManager(stripPathToDir(ctxUrlList.front().toLocalFile()), this, core()->uiController()->activeMainWindow());
    if(branchManager.isValid())
        branchManager.exec();
    else
        KMessageBox::error(0, i18n("Could not show the Branch Manager, current branch is unavailable."));
}

// This is redundant with the normal VCS "history" action
void DistributedVersionControlPlugin::ctxRevHistory()
{
    KUrl::List const & ctxUrlList = d->m_common->contextUrlList();
    Q_ASSERT(!ctxUrlList.isEmpty());
    
    KDialog d;

    CommitLogModel* model = new CommitLogModel(this, ctxUrlList.first().toLocalFile(), &d);
    CommitView *revTree = new CommitView(&d);
    revTree->setModel(model);

    d.setButtons(KDialog::Close);
    d.setMainWidget(revTree);
    d.exec();
}

KDevDVCSViewFactory * DistributedVersionControlPlugin::dvcsViewFactory() const
{
    return d->m_factory;
}

KDevelop::DVcsJob* DistributedVersionControlPlugin::empty_cmd(KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* j = new DVcsJob(QDir(), this, verbosity);
    *j << "echo" << "command not implemented" << "-n";
    return j;
}

}

//-----------------------------------------------------------------------------------


//class KDevDVCSViewFactory
QWidget* KDevDVCSViewFactory::create(QWidget *parent)
{
    return new DVCSmainView(m_plugin, parent);
}

Qt::DockWidgetArea KDevDVCSViewFactory::defaultPosition()
{
    return Qt::BottomDockWidgetArea;
}

QString KDevDVCSViewFactory::id() const
{
    return "org.kdevelop.DVCSview";
}

#endif
