/***************************************************************************
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "vcspluginhelper.h"

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
#include "interfaces/idistributedversioncontrol.h"
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

#include <interfaces/ipatchsource.h>
#include <KTemporaryFile>
#include <qtextedit.h>
#include "vcsstatusinfo.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include "widgets/vcsdiffpatchsources.h"
#include "widgets/flexibleaction.h"
#include <interfaces/isession.h>
#include "vcsevent.h"
#include <KCompositeJob>
#include <QClipboard>
#include <QApplication>
#include <ktexteditor/modificationinterface.h>
#include <QTimer>

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
    KAction * diffToBaseAction;
    KAction * revertAction;
    KAction * diffForRevAction;
    KAction * diffForRevGlobalAction;
    KAction * pushAction;
    KAction * pullAction;
    
    void createActions(VcsPluginHelper* parent) {
        commitAction = new KAction(KIcon("svn-commit"), i18n("Commit..."), parent);
        updateAction = new KAction(KIcon("svn-update"), i18n("Update"), parent);
        addAction = new KAction(KIcon("list-add"), i18n("Add"), parent);
        diffToBaseAction = new KAction(KIcon("text-x-patch"), i18n("Show Differences..."), parent);
        revertAction = new KAction(KIcon("archive-remove"), i18n("Revert"), parent);
        historyAction = new KAction(KIcon("view-history"), i18n("History..."), parent);
        annotationAction = new KAction(KIcon("user-properties"), i18n("Annotation..."), parent);
        diffForRevAction = new KAction(KIcon("text-x-patch"), i18n("Show Diff..."), parent);
        diffForRevGlobalAction = new KAction(KIcon("text-x-patch"), i18n("Show Diff (all files)..."), parent);
        pushAction = new KAction(KIcon("arrow-up-double"), i18n("Push"), parent);
        pullAction = new KAction(KIcon("arrow-down-double"), i18n("Pull"), parent);
        
        connect(commitAction, SIGNAL(triggered()), parent, SLOT(commit()));
        connect(addAction, SIGNAL(triggered()), parent, SLOT(add()));
        connect(updateAction, SIGNAL(triggered()), parent, SLOT(update()));
        connect(diffToBaseAction, SIGNAL(triggered()), parent, SLOT(diffToBase()));
        connect(revertAction, SIGNAL(triggered()), parent, SLOT(revert()));
        connect(historyAction, SIGNAL(triggered()), parent, SLOT(history()));
        connect(annotationAction, SIGNAL(triggered()), parent, SLOT(annotation()));
        connect(diffForRevAction, SIGNAL(triggered()), parent, SLOT(diffForRev()));
        connect(diffForRevGlobalAction, SIGNAL(triggered()), parent, SLOT(diffForRevGlobal()));
        connect(pullAction, SIGNAL(triggered()), parent, SLOT(pull()));
        connect(pushAction, SIGNAL(triggered()), parent, SLOT(push()));
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
        if(plugin->extension<IDistributedVersionControl>()) {
            menu->addAction(pushAction);
            menu->addAction(pullAction);
        } else {
            menu->addAction(updateAction);
        }
        menu->addSeparator();
        menu->addAction(addAction);
        menu->addAction(revertAction);
        menu->addSeparator();
        menu->addAction(historyAction);
        menu->addAction(annotationAction);
        menu->addAction(diffToBaseAction);
        
        const bool singleVersionedFile = ctxUrls.count() == 1 && allVersioned;
        historyAction->setEnabled(singleVersionedFile);
        annotationAction->setEnabled(singleVersionedFile && allLocalFiles(ctxUrls));
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
    VcsJob* job=d->vcs->revert(d->ctxUrls);
    connect(job, SIGNAL(finished(KJob*)), SLOT(revertDone(KJob*)));
    
    foreach(const KUrl& url, d->ctxUrls) {
        IDocument* doc=ICore::self()->documentController()->documentForUrl(url);
        
        if(doc && doc->textDocument()) {
            KTextEditor::ModificationInterface* modif = dynamic_cast<KTextEditor::ModificationInterface*>(doc->textDocument());
            if (modif) {
                modif->setModifiedOnDiskWarning(false);
            }
            doc->textDocument()->setModified(false);
        }
    }
    job->setProperty("urls", d->ctxUrls);
    
    d->plugin->core()->runController()->registerJob(job);
}

void VcsPluginHelper::revertDone(KJob* job)
{
    QTimer* modificationTimer = new QTimer;
    modificationTimer->setInterval(100);
    connect(modificationTimer, SIGNAL(timeout()), SLOT(delayedModificationWarningOn()));
    connect(modificationTimer, SIGNAL(timeout()), modificationTimer, SLOT(deleteLater()));

    
    modificationTimer->setProperty("urls", job->property("urls"));
    modificationTimer->start();
}

void VcsPluginHelper::delayedModificationWarningOn()
{
    QObject* timer = sender();
    KUrl::List urls = timer->property("urls").value<KUrl::List>();
    
    foreach(const KUrl& url, urls) {
        IDocument* doc=ICore::self()->documentController()->documentForUrl(url);
        
        if(doc) {
            doc->reload();
            
            KTextEditor::ModificationInterface* modif=dynamic_cast<KTextEditor::ModificationInterface*>(doc->textDocument());
            modif->setModifiedOnDiskWarning(true);
        }
    }
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

void VcsPluginHelper::diffForRevGlobal()
{
    for(int a = 0; a < d->ctxUrls.size(); ++a)
    {
        KUrl& url(d->ctxUrls[a]);
        IProject* project = ICore::self()->projectController()->findProjectForUrl( url );
        if( project )
            url = project->folder();
    }
    diffForRev();
}

void VcsPluginHelper::history(const VcsRevision& rev)
{
    SINGLEURL_SETUP_VARS
    KDialog* dlg = new KDialog(ICore::self()->uiController()->activeMainWindow());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setButtons(KDialog::Close);
    dlg->setCaption(i18nc("%1: path or URL, %2: name of a version control system",
                          "%2 History (%1)", url.pathOrUrl(), iface->name()));
    KDevelop::VcsEventWidget* logWidget = new KDevelop::VcsEventWidget(url, rev, iface, dlg);
    dlg->setMainWidget(logWidget);
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
                    SIGNAL(annotationContextMenuAboutToShow(KTextEditor::View*,QMenu*,int)),
                    this, SLOT(annotationContextMenuAboutToShow(KTextEditor::View*,QMenu*,int)));
        } else {
            KMessageBox::error(0, i18n("Cannot display annotations, missing interface KTextEditor::AnnotationInterface for the editor."));
            delete job;
        }
    } else {
        KMessageBox::error(0, i18n("Cannot execute annotate action because the "
                                   "document was not found, or was not a text document:\n%1", url.pathOrUrl()));
    }
}

class CopyFunction : public AbstractFunction
{
    public:
        CopyFunction(const QString& tocopy)
            : m_tocopy(tocopy) {}
        
        void operator()() { QApplication::clipboard()->setText(m_tocopy); }
    private:
        QString m_tocopy;
};

class HistoryFunction : public AbstractFunction
{
    public:
        HistoryFunction(VcsPluginHelper* helper, const VcsRevision& rev)
            : m_helper(helper), m_rev(rev) {}
            
            void operator()() { m_helper->history(m_rev); }
        
    private:
        VcsPluginHelper* m_helper;
        VcsRevision m_rev;
};

void VcsPluginHelper::annotationContextMenuAboutToShow( KTextEditor::View* view, QMenu* menu, int line )
{
    KTextEditor::AnnotationInterface* annotateiface =
        qobject_cast<KTextEditor::AnnotationInterface*>(view->document());

    VcsAnnotationModel* model = qobject_cast<VcsAnnotationModel*>( annotateiface->annotationModel() );
    Q_ASSERT(model);

    VcsRevision rev = model->revisionForLine(line);
    // check if the user clicked on a row without revision information
    if (rev.revisionType() == VcsRevision::Invalid) {
        // in this case, do not action depending on revision informations
        return;
    }

    d->diffForRevAction->setData(QVariant::fromValue(rev));
    d->diffForRevGlobalAction->setData(QVariant::fromValue(rev));
    menu->addSeparator();
    menu->addAction(d->diffForRevAction);
    menu->addAction(d->diffForRevGlobalAction);
    menu->addAction(new FlexibleAction(KIcon("edit-copy"), i18n("Copy Revision"), new CopyFunction(rev.revisionValue().toString()), menu));
    menu->addAction(new FlexibleAction(KIcon("view-history"), i18n("History..."), new HistoryFunction(this, rev), menu));
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
    
    // We start the commit UI no matter whether there is real differences, as it can also be used to commit untracked files
    VCSCommitDiffPatchSource* patchSource = new VCSCommitDiffPatchSource(new VCSStandardDiffUpdater(d->vcs, url));
    
    bool ret = showVcsDiff(patchSource);

    if(!ret) {
        VcsCommitDialog *commitDialog = new VcsCommitDialog(patchSource);
        commitDialog->setCommitCandidates(patchSource->infos());
        commitDialog->exec();
    }
}

void VcsPluginHelper::push()
{
    foreach(const KUrl& url, d->ctxUrls) {
        VcsJob* job = d->plugin->extension<IDistributedVersionControl>()->push(url, VcsLocation());
        ICore::self()->runController()->registerJob(job);
    }
}

void VcsPluginHelper::pull()
{
    foreach(const KUrl& url, d->ctxUrls) {
        VcsJob* job = d->plugin->extension<IDistributedVersionControl>()->pull(VcsLocation(), url);
        ICore::self()->runController()->registerJob(job);
    }
}

}


#include "vcspluginhelper.moc"
