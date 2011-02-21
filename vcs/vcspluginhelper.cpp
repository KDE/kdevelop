
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

#include <config-kdevplatform.h>
#if HAVE_KOMPARE
#include <interfaces/ipatchdocument.h>
#endif
#include <interfaces/ipatchsource.h>
#include <KTemporaryFile>
#include <qtextedit.h>
#include "vcsstatusinfo.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include <QMenu>
#include "widgets/vcsdiffpatchsources.h"
#include <interfaces/isession.h>
#include "vcsevent.h"
#include <KCompositeJob>

namespace KDevelop
{

struct VcsPluginHelper::VcsPluginHelperPrivate {
    IPlugin * plugin;
    IBasicVersionControl * vcs;

    KUrl::List ctxUrls;
    KAction * commitAction;
    KAction * addAction;
    KAction * updateAction;
    KAction * historyAction;
    KAction * annotationAction;
    KAction * diffToHeadAction;
    KAction * diffToBaseAction;
    KAction * revertAction;
    KAction * diffForRevAction;
    
    void createActions(QObject * parent) {
        commitAction = new KAction(KIcon("svn-commit"), i18n("Commit..."), parent);
        updateAction = new KAction(KIcon("svn-update"), i18n("Update"), parent);
        addAction = new KAction(KIcon("list-add"), i18n("Add"), parent);
        diffToHeadAction = new KAction(i18n("Compare to Head..."), parent);
        diffToBaseAction = new KAction(i18n("Compare to Base..."), parent);
        revertAction = new KAction(KIcon("archive-remove"), i18n("Revert"), parent);
        historyAction = new KAction(KIcon("view-history"), i18n("History..."), parent);
        annotationAction = new KAction(KIcon("user-properties"), i18n("Annotation..."), parent);
        diffForRevAction = new KAction(KIcon("vcs_diff"), i18n("Show Diff..."), parent);
        
        connect(commitAction, SIGNAL(triggered()), parent, SLOT(commit()));
        connect(addAction, SIGNAL(triggered()), parent, SLOT(add()));
        connect(updateAction, SIGNAL(triggered()), parent, SLOT(update()));
        connect(diffToHeadAction, SIGNAL(triggered()), parent, SLOT(diffToHead()));
        connect(diffToBaseAction, SIGNAL(triggered()), parent, SLOT(diffToBase()));
        connect(revertAction, SIGNAL(triggered()), parent, SLOT(revert()));
        connect(historyAction, SIGNAL(triggered()), parent, SLOT(history()));
        connect(annotationAction, SIGNAL(triggered()), parent, SLOT(annotation()));
        connect(diffForRevAction, SIGNAL(triggered()), parent, SLOT(diffForRev()));
    }
    
    bool allLocalFiles(const KUrl::List& urls)
    {
        bool ret=true;
        foreach(const KUrl& url, urls) {
            QFileInfo info(url.toLocalFile());
            ret &= info.isFile();
        }
        return ret;
    }
    
    QMenu* createMenu()
    {
        bool allVersioned=true;
        foreach(const KUrl& url, ctxUrls) {
            allVersioned=allVersioned && vcs->isVersionControlled(url);
            
            if(!allVersioned)
                break;
        }
        
        QMenu* menu=new QMenu(vcs->name());
        menu->setIcon(KIcon(ICore::self()->pluginController()->pluginInfo(plugin).icon()));
        
        menu->addAction(commitAction);
        menu->addAction(updateAction);
        menu->addSeparator();
        menu->addAction(addAction);
        menu->addAction(revertAction);
        menu->addSeparator();
        menu->addAction(historyAction);
        menu->addAction(annotationAction);
        menu->addAction(diffToHeadAction);
        menu->addAction(diffToBaseAction);
        
        addAction->setEnabled(!allVersioned);
        
        const bool singleVersionedFile = ctxUrls.count() == 1 && allVersioned;
        historyAction->setEnabled(singleVersionedFile);
        annotationAction->setEnabled(singleVersionedFile && allLocalFiles(ctxUrls));
        diffToHeadAction->setEnabled(singleVersionedFile);
        diffToBaseAction->setEnabled(singleVersionedFile);
        commitAction->setEnabled(singleVersionedFile);
        
        return menu;
    }
};


VcsPluginHelper::VcsPluginHelper(KDevelop::IPlugin* parent, KDevelop::IBasicVersionControl* vcs)
        : QObject(parent)
        , d(new VcsPluginHelperPrivate())
{
    Q_ASSERT(vcs);
    Q_ASSERT(parent);
    d->plugin = parent;
    d->vcs = vcs;
    d->createActions(this);
}

VcsPluginHelper::~VcsPluginHelper()
{}

void VcsPluginHelper::setupFromContext(Context* context)
{
    d->ctxUrls.clear();
    {
        KDevelop::ProjectItemContext* prjctx = dynamic_cast<KDevelop::ProjectItemContext*>(context);

        if (prjctx) {
            foreach(KDevelop::ProjectBaseItem* item, prjctx->items()) {
                if(!item->target())
                    d->ctxUrls.append(item->url());
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

QMenu* VcsPluginHelper::commonActions()
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
    return d->createMenu();
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
    ICore::self()->documentController()->saveAllDocuments();
    KDevelop::VcsJob* job = iface->diff(url,
                                        KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Head),
                                        KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Working));

    connect(job, SIGNAL(finished(KJob*)), this, SLOT(diffJobFinished(KJob*)));
    d->plugin->core()->runController()->registerJob(job);
}

void VcsPluginHelper::diffJobFinished(KJob* job)
{
    KDevelop::VcsJob* vcsjob = qobject_cast<KDevelop::VcsJob*>(job);
    Q_ASSERT(vcsjob);

    if (vcsjob->status() == KDevelop::VcsJob::JobSucceeded) {
        KDevelop::VcsDiff d = vcsjob->fetchResults().value<KDevelop::VcsDiff>();
        if(d.isEmpty())
            KMessageBox::information(ICore::self()->uiController()->activeMainWindow(),
                                     i18n("There are no differences."),
                                     i18n("VCS support"));
        else {
            VCSDiffPatchSource* patch=new VCSDiffPatchSource(d);
            showVcsDiff(patch);
        }
    } else {
        KMessageBox::error(ICore::self()->uiController()->activeMainWindow(), vcsjob->errorString(), i18n("Unable to get difference."));
    }
}

void VcsPluginHelper::diffToBase()
{
    SINGLEURL_SETUP_VARS
    ICore::self()->documentController()->saveAllDocuments();

    VCSDiffPatchSource* patch =new VCSDiffPatchSource(new VCSStandardDiffUpdater(iface, url));
    showVcsDiff(patch);
}

void VcsPluginHelper::diffForRev()
{
    QAction* action = qobject_cast<QAction*>( sender() );
    Q_ASSERT(action);
    Q_ASSERT(action->data().canConvert<VcsRevision>());
    VcsRevision rev = action->data().value<VcsRevision>();

    SINGLEURL_SETUP_VARS
    ICore::self()->documentController()->saveAllDocuments();
    VcsRevision prev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Previous);
    KDevelop::VcsJob* job = iface->diff(url, prev, rev );

    connect(job, SIGNAL(finished(KJob*)), this, SLOT(diffJobFinished(KJob*)));
    d->plugin->core()->runController()->registerJob(job);
}

void VcsPluginHelper::history()
{
    SINGLEURL_SETUP_VARS
    KDevelop::VcsJob *job = iface->log(url);
    if( !job ) 
    {
        kWarning() << "Couldn't create log job for:" << url << "with iface:" << iface << dynamic_cast<KDevelop::IPlugin*>( iface );
        return;
    }
    KDialog* dlg = new KDialog();
    dlg->setButtons(KDialog::Close);
    dlg->setCaption(i18n("%2 History (%1)", url.pathOrUrl(), iface->name()));
    KDevelop::VcsEventWidget* logWidget = new KDevelop::VcsEventWidget(url, job, dlg);
    dlg->setMainWidget(logWidget);
    dlg->show();
    connect( dlg, SIGNAL(closeClicked()), job, SLOT(kill()) );
    connect( dlg, SIGNAL(closeClicked()), dlg, SLOT(deleteLater()) );
}

void VcsPluginHelper::annotation()
{
    SINGLEURL_SETUP_VARS
    KDevelop::IDocument* doc = ICore::self()->documentController()->documentForUrl(url);

    if (!doc)
        doc = ICore::self()->documentController()->openDocument(url);

    if (doc && doc->textDocument()) {
        KDevelop::VcsJob* job = iface->annotate(url);
        if( !job ) 
        {
            kWarning() << "Couldn't create annotate job for:" << url << "with iface:" << iface << dynamic_cast<KDevelop::IPlugin*>( iface );
            return;
        }
        KTextEditor::AnnotationInterface* annotateiface = qobject_cast<KTextEditor::AnnotationInterface*>(doc->textDocument());
        KTextEditor::AnnotationViewInterface* viewiface = qobject_cast<KTextEditor::AnnotationViewInterface*>(doc->textDocument()->activeView());

        if (annotateiface && viewiface) {
            KDevelop::VcsAnnotationModel* model = new KDevelop::VcsAnnotationModel(job, url, doc->textDocument());
            annotateiface->setAnnotationModel(model);
            viewiface->setAnnotationBorderVisible(true);
            connect(doc->textDocument()->activeView(),
                    SIGNAL(annotationContextMenuAboutToShow( KTextEditor::View*, QMenu*, int )),
                    this, SLOT(annotationContextMenuAboutToShow( KTextEditor::View*, QMenu*, int )));
        } else {
            KMessageBox::error(0, i18n("Cannot display annotations, missing interface KTextEditor::AnnotationInterface for the editor."));
            delete job;
        }
    } else {
        KMessageBox::error(0, i18n("Cannot execute annotate action because the "
                                   "document was not found, or was not a text document:\n%1", url.pathOrUrl()));
    }
}

void VcsPluginHelper::annotationContextMenuAboutToShow( KTextEditor::View* view, QMenu* menu, int line )
{
    KTextEditor::AnnotationInterface* annotateiface =
        qobject_cast<KTextEditor::AnnotationInterface*>(view->document());

    VcsAnnotationModel* model = qobject_cast<VcsAnnotationModel*>( annotateiface->annotationModel() );
    Q_ASSERT(model);

    VcsRevision rev = model->revisionForLine(line);
    d->diffForRevAction->setData(QVariant::fromValue(rev));
    menu->addAction(d->diffForRevAction);
}

void VcsPluginHelper::update()
{
    EXECUTE_VCS_METHOD(update);
}

void VcsPluginHelper::add()
{
    EXECUTE_VCS_METHOD(add);
}

void VcsPluginHelper::commit()
{
    Q_ASSERT(!d->ctxUrls.isEmpty());
    ICore::self()->documentController()->saveAllDocuments();

    KUrl url = d->ctxUrls.first();
    QScopedPointer<VcsJob> statusJob(d->vcs->status(url));
    QMap<KUrl, VcsStatusInfo::State> changes;
    if( statusJob->exec() && statusJob->status() == VcsJob::JobSucceeded )
    {
        QVariant varlist = statusJob->fetchResults();

        foreach( const QVariant &var, varlist.toList() )
        {
            VcsStatusInfo info = qVariantValue<KDevelop::VcsStatusInfo>( var );
            
            if(info.state()!=VcsStatusInfo::ItemUpToDate)
                changes[info.url()] = info.state();
        }
    }
    else
        kDebug() << "Couldn't get status for urls: " << url;
    
    
    // We start the commit UI no matter whether there is real differences, as it can also be used to commit untracked files
    VCSCommitDiffPatchSource* patchSource = new VCSCommitDiffPatchSource(new VCSStandardDiffUpdater(d->vcs, url), changes, d->vcs, retrieveOldCommitMessages());
    
    bool ret = showVcsDiff(patchSource);
    
    Q_ASSERT(ret && "Make sure PatchReview plugin is installed correctly");
    if(ret) {
        connect(patchSource, SIGNAL(reviewFinished(QString,QList<KUrl>)), this, SLOT(executeCommit(QString,QList<KUrl>)));
        connect(patchSource, SIGNAL(reviewCancelled(QString)), this, SLOT(commitReviewCancelled(QString)));
    } else {
        delete patchSource;
    }
}

void VcsPluginHelper::executeCommit(const QString& message, const QList<KUrl>& urls)
{
    VcsJob* job=d->vcs->commit(message, urls, KDevelop::IBasicVersionControl::NonRecursive);
    d->plugin->core()->runController()->registerJob(job);
    addOldCommitMessage(message);
}

void VcsPluginHelper::commitReviewCancelled(QString message)
{
    addOldCommitMessage(message);
}

void VcsPluginHelper::cancelCommit(KDevelop::VcsCommitDialog* dlg)
{
    dlg->deleteLater();
}

QStringList retrieveOldCommitMessages()
{
    KConfigGroup vcsGroup(ICore::self()->activeSession()->config(), "VCS");
    return vcsGroup.readEntry("OldCommitMessages", QStringList());
}

namespace {
    int maxMessages = 10;
}

void addOldCommitMessage(QString message)
{
    if(ICore::self()->shuttingDown())
        return;
    
    QStringList oldMessages = retrieveOldCommitMessages();
    
    if(oldMessages.contains(message))
        oldMessages.removeAll(message);
    
    oldMessages.push_front(message);
    while(oldMessages.size() > maxMessages)
        oldMessages.pop_back();
    
    KConfigGroup vcsGroup(ICore::self()->activeSession()->config(), "VCS");
    vcsGroup.writeEntry("OldCommitMessages", oldMessages);
}

}


#include "vcspluginhelper.moc"
