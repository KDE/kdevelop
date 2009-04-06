/***************************************************************************
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "vcspluginhelper.h"

#include <QtDesigner/QExtensionFactory>
#include <QAction>
#include <QVariant>
#include <QMenu>

#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kaction.h>

#include <ktexteditor/annotationinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include <kparts/mainwindow.h>

#include <interfaces/context.h>
#include <interfaces/icore.h>
#include <interfaces/iplugin.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <language/interfaces/codecontext.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/widgets/vcscommitdialog.h>
#include <vcs/models/vcsannotationmodel.h>
#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcsdiff.h>
#include <vcs/widgets/vcseventwidget.h>
#include <language/duchain/duchainbase.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/interfaces/editorcontext.h>

namespace KDevelop
{

struct VcsPluginHelper::VcsPluginHelperPrivate {
    IPlugin * plugin;
    IBasicVersionControl * vcs;
    KUrl::List ctxUrls;
    QList<QAction*> actions;
    KAction * commitAction;
    KAction * addAction;
    KAction * removeAction;
    KAction * updateAction;
    KAction * historyAction;
    KAction * annotationAction;
    KAction * diffToHeadAction;
    KAction * diffToBaseAction;
    KAction* revertAction;
    void createActions(QObject * parent) {
        commitAction = new KAction(i18n("Commit..."), parent);
        actions.push_back(commitAction);
        addAction = new KAction(i18n("Add"), parent);
        actions.push_back(addAction);
        removeAction = new KAction(i18n("Remove"), parent);
        actions.push_back(removeAction);
        updateAction = new KAction(i18n("Update"), parent);
        actions.push_back(updateAction);
        diffToHeadAction = new KAction(i18n("Compare to Head..."), parent);
        actions.push_back(diffToHeadAction);
        diffToBaseAction = new KAction(i18n("Compare to Base..."), parent);
        actions.push_back(diffToBaseAction);
        revertAction = new KAction(i18n("Revert"), parent);
        actions.push_back(revertAction);
        historyAction = new KAction(i18n("History..."), parent);
        actions.push_back(historyAction);
        annotationAction = new KAction(i18n("Annotation..."), parent);
        actions.push_back(annotationAction);
    }
};


VcsPluginHelper::VcsPluginHelper(KDevelop::IPlugin* parent, KDevelop::IBasicVersionControl* vcs)
        : QObject(parent)
        , d(new VcsPluginHelperPrivate())
{
    Q_ASSERT(0 != vcs);
    Q_ASSERT(0 != parent);
    d->plugin = parent;
    d->vcs = vcs;
    d->createActions(this);
    connect(d->commitAction, SIGNAL(triggered()), this, SLOT(commit()));
    connect(d->addAction, SIGNAL(triggered()), this, SLOT(add()));
    connect(d->removeAction, SIGNAL(triggered()), this, SLOT(remove()));
    connect(d->updateAction, SIGNAL(triggered()), this, SLOT(update()));
    connect(d->diffToHeadAction, SIGNAL(triggered()), this, SLOT(diffToHead()));
    connect(d->diffToBaseAction, SIGNAL(triggered()), this, SLOT(diffToBase()));
    connect(d->revertAction, SIGNAL(triggered()), this, SLOT(revert()));
    connect(d->historyAction, SIGNAL(triggered()), this, SLOT(history()));
    connect(d->annotationAction, SIGNAL(triggered()), this, SLOT(annotation()));
}

VcsPluginHelper::~VcsPluginHelper()
{
}

KUrl VcsPluginHelper::urlForItem(KDevelop::ProjectBaseItem* item) const
{
    if (item->file()) {
        return item->file()->url();
    }

    if (item->folder()) {
        return item->folder()->url();
    }

    KDevelop::ProjectBaseItem* parentitem = dynamic_cast<KDevelop::ProjectBaseItem*>(item->parent());

    if (parentitem) {
        return parentitem->folder()->url();
    }

    return KUrl();
}

void VcsPluginHelper::setupFromContext(Context* context)
{
    d->ctxUrls.clear();
    {
        KDevelop::ProjectItemContext* prjctx = dynamic_cast<KDevelop::ProjectItemContext*>(context);

        if (prjctx) {
            foreach(KDevelop::ProjectBaseItem* item, prjctx->items()) {
                d->ctxUrls.append(urlForItem(item));
            }
        }
    }

    {
        KDevelop::EditorContext* editctx = dynamic_cast<KDevelop::EditorContext*>(context);

        if (editctx) {
            d->ctxUrls.append(editctx->url());
        }
    }

    {
        KDevelop::FileContext* filectx = dynamic_cast<KDevelop::FileContext*>(context);

        if (filectx) {
            d->ctxUrls = filectx->urls();
        }
    }
}

KUrl::List const & VcsPluginHelper::contextUrlList()
{
    return d->ctxUrls;
}

QList<QAction*> VcsPluginHelper::commonActions()
{
    /* TODO: the following logic to determine which actions need to be enabled
     * or disabled does not work properly. What needs to be implemented is that
     * project items that are vc-controlled enable all except add, project
     * items that are not vc-controlled enable add action. For urls that cannot
     * be made into a project item, or if the project has no associated VC
     * plugin we need to check whether a VC controls the parent dir, if we have
     * one we assume the urls can be added but are not currently controlled. If
     * the url is already version controlled then just enable all except add
     */

    if (d->ctxUrls.isEmpty())
        return QList<QAction*>();

    const bool singleFile = d->ctxUrls.count() == 1;
    d->historyAction->setEnabled(singleFile);
    d->annotationAction->setEnabled(singleFile);
    d->diffToHeadAction->setEnabled(singleFile);
    d->diffToBaseAction->setEnabled(singleFile);

    return d->actions;
}

#define EXECUTE_VCS_METHOD( method ) \
    d->plugin->core()->runController()->registerJob( d->vcs-> method ( d->ctxUrls ) )

#define SINGLEURL_SETUP_VARS \
    KDevelop::IBasicVersionControl* iface = d->vcs;\
    const KUrl & url = d->ctxUrls.front();


void VcsPluginHelper::revert()
{
    EXECUTE_VCS_METHOD(revert);
}

void VcsPluginHelper::diffToHead()
{
    SINGLEURL_SETUP_VARS
    KDevelop::VcsJob* job = iface->diff(url,
                                        KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Head),
                                        KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Working));

    connect(job, SIGNAL(finished(KJob*)), this, SLOT(diffJobFinished(KJob*)));
    d->plugin->core()->runController()->registerJob(job);
}

void VcsPluginHelper::diffJobFinished(KJob* job)
{
    KDevelop::VcsJob* vcsjob = dynamic_cast<KDevelop::VcsJob*>(job);
    Q_ASSERT(vcsjob);

    if (vcsjob) {
        if (vcsjob->status() == KDevelop::VcsJob::JobSucceeded) {
            KDevelop::VcsDiff d = vcsjob->fetchResults().value<KDevelop::VcsDiff>();
            QString diff = d.diff();
            ICore::self()->documentController()->openDocumentFromText(diff);
        } else {
            KMessageBox::error(ICore::self()->uiController()->activeMainWindow(), vcsjob->errorString(), i18n("Unable to get difference."));
        }

        vcsjob->disconnect(this);
    }
}

void VcsPluginHelper::diffToBase()
{
    SINGLEURL_SETUP_VARS
    KDevelop::VcsJob* job = iface->diff(url,
                                        KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Base),
                                        KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Working));

    connect(job, SIGNAL(finished(KJob*)), this, SLOT(diffJobFinished(KJob*)));
    ICore::self()->runController()->registerJob(job);
}

void VcsPluginHelper::history()
{
    SINGLEURL_SETUP_VARS
    KDevelop::VcsJob *job = iface->log(url);
    KDialog* dlg = new KDialog();
    dlg->setButtons(KDialog::Close);
    dlg->setCaption(i18n("%2 History (%1)", url.pathOrUrl(), iface->name()));
    KDevelop::VcsEventWidget* logWidget = new KDevelop::VcsEventWidget(url, job, dlg);
    dlg->setMainWidget(logWidget);
    connect(dlg, SIGNAL(closeClicked()), job, SLOT(deleteLater()));
    dlg->show();
}

void VcsPluginHelper::annotation()
{
    SINGLEURL_SETUP_VARS
    KDevelop::IDocument* doc = ICore::self()->documentController()->documentForUrl(url);

    if (!doc)
        doc = ICore::self()->documentController()->openDocument(url);

    if (doc && doc->textDocument()) {
        KDevelop::VcsJob* job = iface->annotate(url);
        KTextEditor::AnnotationInterface* annotateiface = qobject_cast<KTextEditor::AnnotationInterface*>(doc->textDocument());
        KTextEditor::AnnotationViewInterface* viewiface = qobject_cast<KTextEditor::AnnotationViewInterface*>(doc->textDocument()->activeView());

        if (annotateiface && viewiface) {
            KDevelop::VcsAnnotationModel* model = new KDevelop::VcsAnnotationModel(job, url, doc->textDocument());
            annotateiface->setAnnotationModel(model);
            viewiface->setAnnotationBorderVisible(true);
        } else {
            KMessageBox::error(0, i18n("Cannot display annotations, missing interface KTextEditor::AnnotationInterface for the editor."));
            delete job;
        }
    } else {
        KMessageBox::error(0, i18n("Cannot execute annotate action because the "
                                   "document was not found, or was not a text document:\n%1", url.pathOrUrl()));
    }
}

void VcsPluginHelper::update()
{
    EXECUTE_VCS_METHOD(update);
}

void VcsPluginHelper::remove()
{
    EXECUTE_VCS_METHOD(remove);
}

void VcsPluginHelper::add()
{
    EXECUTE_VCS_METHOD(add);
}

void VcsPluginHelper::commit()
{
    Q_ASSERT(!d->ctxUrls.isEmpty());
    KDevelop::VcsCommitDialog* dlg = new KDevelop::VcsCommitDialog(d->plugin, d->plugin->core()->uiController()->activeMainWindow());
    dlg->setCommitCandidates(d->ctxUrls);
//     KConfigGroup vcsGroup(KSharedConfig::openConfig(componentData()), "VcsCommon");
//     dlg->setOldMessages(vcsGroup.readEntry("OldCommitMessages", QStringList()));
//     dlg->setRecursive(true);
    connect(dlg, SIGNAL(doCommit(KDevelop::VcsCommitDialog*)), this, SLOT(executeCommit(KDevelop::VcsCommitDialog*)));
    connect(dlg, SIGNAL(cancelCommit(KDevelop::VcsCommitDialog*)), this, SLOT(cancelCommit(KDevelop::VcsCommitDialog*)));
    dlg->show();
}

void VcsPluginHelper::executeCommit(KDevelop::VcsCommitDialog* dlg)
{
    KConfigGroup vcsGroup(KSharedConfig::openConfig(d->plugin->componentData()), "VcsCommon");
    QStringList oldMessages = vcsGroup.readEntry("OldCommitMessages", QStringList());
    oldMessages << dlg->message();
    vcsGroup.writeEntry("OldCommitMessages", oldMessages);

    KDevelop::IBasicVersionControl* iface = dlg->versionControlPlugin()->extension<KDevelop::IBasicVersionControl>();
    d->plugin->core()->runController()->registerJob(iface->commit(dlg->message(), dlg->checkedUrls(),
            dlg->recursive() ?  KDevelop::IBasicVersionControl::Recursive : KDevelop::IBasicVersionControl::NonRecursive));

    dlg->deleteLater();
}

void VcsPluginHelper::cancelCommit(KDevelop::VcsCommitDialog* dlg)
{
    dlg->deleteLater();
}

}

#include "vcspluginhelper.moc"
