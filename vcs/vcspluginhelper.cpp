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

#include <kaboutdata.h>
#include <KLocalizedString>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <QAction>
#include <QFileInfo>

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
#include <util/path.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include "interfaces/idistributedversioncontrol.h"
#include <vcs/widgets/vcscommitdialog.h>
#include <vcs/models/vcsannotationmodel.h>
#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcsdiff.h>
#include <vcs/widgets/vcseventwidget.h>

#include <interfaces/ipatchsource.h>
#include <QTemporaryFile>
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
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace KDevelop
{

struct VcsPluginHelper::VcsPluginHelperPrivate {
    IPlugin * plugin;
    IBasicVersionControl * vcs;

    QList<QUrl> ctxUrls;
    QAction* commitAction;
    QAction* addAction;
    QAction* updateAction;
    QAction* historyAction;
    QAction* annotationAction;
    QAction* diffToBaseAction;
    QAction* revertAction;
    QAction* diffForRevAction;
    QAction* diffForRevGlobalAction;
    QAction* pushAction;
    QAction* pullAction;

    void createActions(VcsPluginHelper* parent) {
        commitAction = new QAction(QIcon::fromTheme("svn-commit"), i18n("Commit..."), parent);
        updateAction = new QAction(QIcon::fromTheme("svn-update"), i18n("Update"), parent);
        addAction = new QAction(QIcon::fromTheme("list-add"), i18n("Add"), parent);
        diffToBaseAction = new QAction(QIcon::fromTheme("text-x-patch"), i18n("Show Differences..."), parent);
        revertAction = new QAction(QIcon::fromTheme("archive-remove"), i18n("Revert"), parent);
        historyAction = new QAction(QIcon::fromTheme("view-history"), i18n("History..."), parent);
        annotationAction = new QAction(QIcon::fromTheme("user-properties"), i18n("Annotation..."), parent);
        diffForRevAction = new QAction(QIcon::fromTheme("text-x-patch"), i18n("Show Diff..."), parent);
        diffForRevGlobalAction = new QAction(QIcon::fromTheme("text-x-patch"), i18n("Show Diff (all files)..."), parent);
        pushAction = new QAction(QIcon::fromTheme("arrow-up-double"), i18n("Push"), parent);
        pullAction = new QAction(QIcon::fromTheme("arrow-down-double"), i18n("Pull"), parent);

        connect(commitAction, &QAction::triggered, parent, &VcsPluginHelper::commit);
        connect(addAction, &QAction::triggered, parent, &VcsPluginHelper::add);
        connect(updateAction, &QAction::triggered, parent, &VcsPluginHelper::update);
        connect(diffToBaseAction, &QAction::triggered, parent, &VcsPluginHelper::diffToBase);
        connect(revertAction, &QAction::triggered, parent, &VcsPluginHelper::revert);
        connect(historyAction, &QAction::triggered, parent, [=] { parent->history(); });
        connect(annotationAction, &QAction::triggered, parent, &VcsPluginHelper::annotation);
        connect(diffForRevAction, &QAction::triggered, parent, static_cast<void(VcsPluginHelper::*)()>(&VcsPluginHelper::diffForRev));
        connect(diffForRevGlobalAction, &QAction::triggered, parent, &VcsPluginHelper::diffForRevGlobal);
        connect(pullAction, &QAction::triggered, parent, &VcsPluginHelper::pull);
        connect(pushAction, &QAction::triggered, parent, &VcsPluginHelper::push);
    }

    bool allLocalFiles(const QList<QUrl>& urls)
    {
        bool ret=true;
        foreach(const QUrl &url, urls) {
            QFileInfo info(url.toLocalFile());
            ret &= info.isFile();
        }
        return ret;
    }

    QMenu* createMenu()
    {
        bool allVersioned=true;
        foreach(const QUrl &url, ctxUrls) {
            allVersioned=allVersioned && vcs->isVersionControlled(url);

            if(!allVersioned)
                break;
        }

        QMenu* menu = new QMenu(vcs->name());
        menu->setIcon(QIcon::fromTheme(ICore::self()->pluginController()->pluginInfo(plugin).iconName()));
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

void VcsPluginHelper::addContextDocument(const QUrl &url)
{
    d->ctxUrls.append(url);
}

void VcsPluginHelper::disposeEventually(KTextEditor::View *, bool dont)
{
    if ( ! dont ) {
        deleteLater();
    }
}

void VcsPluginHelper::disposeEventually(KTextEditor::Document *)
{
    deleteLater();
}

void VcsPluginHelper::setupFromContext(Context* context)
{
    static const QVector<int> contextTypes =
        {Context::ProjectItemContext, Context::FileContext, Context::EditorContext};

    if (contextTypes.contains(context->type())) {
        d->ctxUrls = context->urls();
    } else {
        d->ctxUrls.clear();
    }
}

QList<QUrl> VcsPluginHelper::contextUrlList() const
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
    const QUrl &url = d->ctxUrls.front();


void VcsPluginHelper::revert()
{
    VcsJob* job=d->vcs->revert(d->ctxUrls);
    connect(job, &VcsJob::finished, this, &VcsPluginHelper::revertDone);

    foreach(const QUrl &url, d->ctxUrls) {
        IDocument* doc=ICore::self()->documentController()->documentForUrl(url);

        if(doc && doc->textDocument()) {
            KTextEditor::ModificationInterface* modif = dynamic_cast<KTextEditor::ModificationInterface*>(doc->textDocument());
            if (modif) {
                modif->setModifiedOnDiskWarning(false);
            }
            doc->textDocument()->setModified(false);
        }
    }
    job->setProperty("urls", QVariant::fromValue(d->ctxUrls));

    d->plugin->core()->runController()->registerJob(job);
}

void VcsPluginHelper::revertDone(KJob* job)
{
    QTimer* modificationTimer = new QTimer;
    modificationTimer->setInterval(100);
    connect(modificationTimer, &QTimer::timeout, this, &VcsPluginHelper::delayedModificationWarningOn);
    connect(modificationTimer, &QTimer::timeout, modificationTimer, &QTimer::deleteLater);


    modificationTimer->setProperty("urls", job->property("urls"));
    modificationTimer->start();
}

void VcsPluginHelper::delayedModificationWarningOn()
{
    QObject* timer = sender();
    QList<QUrl> urls = timer->property("urls").value<QList<QUrl>>();

    foreach(const QUrl &url, urls) {
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
    if (d->ctxUrls.isEmpty()) {
        return;
    }
    diffForRev(d->ctxUrls.first());
}

void VcsPluginHelper::diffForRevGlobal()
{
    if (d->ctxUrls.isEmpty()) {
        return;
    }
    QUrl url = d->ctxUrls.first();
    IProject* project = ICore::self()->projectController()->findProjectForUrl( url );
    if( project ) {
        url = project->path().toUrl();
    }

    diffForRev(url);
}

void VcsPluginHelper::diffForRev(const QUrl& url)
{
    QAction* action = qobject_cast<QAction*>( sender() );
    Q_ASSERT(action);
    Q_ASSERT(action->data().canConvert<VcsRevision>());
    VcsRevision rev = action->data().value<VcsRevision>();

    ICore::self()->documentController()->saveAllDocuments();
    VcsRevision prev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Previous);
    KDevelop::VcsJob* job = d->vcs->diff(url, prev, rev );

    connect(job, &VcsJob::finished, this, &VcsPluginHelper::diffJobFinished);
    d->plugin->core()->runController()->registerJob(job);
}

void VcsPluginHelper::history(const VcsRevision& rev)
{
    SINGLEURL_SETUP_VARS
    QDialog* dlg = new QDialog(ICore::self()->uiController()->activeMainWindow());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(i18nc("%1: path or URL, %2: name of a version control system",
                          "%2 History (%1)", url.toDisplayString(QUrl::PreferLocalFile), iface->name()));

    QVBoxLayout *mainLayout = new QVBoxLayout(dlg);

    KDevelop::VcsEventWidget* logWidget = new KDevelop::VcsEventWidget(url, rev, iface, dlg);
    mainLayout->addWidget(logWidget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    dlg->connect(buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    dlg->connect(buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    dlg->show();
}

void VcsPluginHelper::annotation()
{
    SINGLEURL_SETUP_VARS
    KDevelop::IDocument* doc = ICore::self()->documentController()->documentForUrl(url);

    if (!doc)
        doc = ICore::self()->documentController()->openDocument(url);

    KTextEditor::AnnotationInterface* annotateiface = qobject_cast<KTextEditor::AnnotationInterface*>(doc->textDocument());
    KTextEditor::AnnotationViewInterface* viewiface = qobject_cast<KTextEditor::AnnotationViewInterface*>(doc->activeTextView());
    if (viewiface && viewiface->isAnnotationBorderVisible()) {
        viewiface->setAnnotationBorderVisible(false);
        return;
    }

    if (doc && doc->textDocument() && iface) {
        KDevelop::VcsJob* job = iface->annotate(url);
        if( !job )
        {
            qWarning() << "Couldn't create annotate job for:" << url << "with iface:" << iface << dynamic_cast<KDevelop::IPlugin*>( iface );
            return;
        }

        QColor foreground(Qt::black);
        QColor background(Qt::white);
        if (KTextEditor::View* view = doc->activeTextView()) {
            KTextEditor::Attribute::Ptr style = view->defaultStyleAttribute(KTextEditor::dsNormal);
            foreground = style->foreground().color();
            if (style->hasProperty(QTextFormat::BackgroundBrush)) {
                background = style->background().color();
            }
        }

        if (annotateiface && viewiface) {
            KDevelop::VcsAnnotationModel* model = new KDevelop::VcsAnnotationModel(job, url, doc->textDocument(),
                                                                                   foreground, background);
            annotateiface->setAnnotationModel(model);
            viewiface->setAnnotationBorderVisible(true);
            // can't use new signal slot syntax here, AnnotationInterface is not a QObject
            connect(doc->activeTextView(),
                    SIGNAL(annotationContextMenuAboutToShow(KTextEditor::View*,QMenu*,int)),
                    this, SLOT(annotationContextMenuAboutToShow(KTextEditor::View*,QMenu*,int)));
        } else {
            KMessageBox::error(0, i18n("Cannot display annotations, missing interface KTextEditor::AnnotationInterface for the editor."));
            delete job;
        }
    } else {
        KMessageBox::error(0, i18n("Cannot execute annotate action because the "
                                   "document was not found, or was not a text document:\n%1", url.toDisplayString(QUrl::PreferLocalFile)));
    }
}

class CopyFunction : public AbstractFunction
{
    public:
        CopyFunction(const QString& tocopy)
            : m_tocopy(tocopy) {}

        void operator()() override { QApplication::clipboard()->setText(m_tocopy); }
    private:
        QString m_tocopy;
};

class HistoryFunction : public AbstractFunction
{
    public:
        HistoryFunction(VcsPluginHelper* helper, const VcsRevision& rev)
            : m_helper(helper), m_rev(rev) {}

            void operator()() override { m_helper->history(m_rev); }

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
    menu->addAction(new FlexibleAction(QIcon::fromTheme("edit-copy"), i18n("Copy Revision"), new CopyFunction(rev.revisionValue().toString()), menu));
    menu->addAction(new FlexibleAction(QIcon::fromTheme("view-history"), i18n("History..."), new HistoryFunction(this, rev), menu));
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

    QUrl url = d->ctxUrls.first();

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
    foreach(const QUrl &url, d->ctxUrls) {
        VcsJob* job = d->plugin->extension<IDistributedVersionControl>()->push(url, VcsLocation());
        ICore::self()->runController()->registerJob(job);
    }
}

void VcsPluginHelper::pull()
{
    foreach(const QUrl &url, d->ctxUrls) {
        VcsJob* job = d->plugin->extension<IDistributedVersionControl>()->pull(VcsLocation(), url);
        ICore::self()->runController()->registerJob(job);
    }
}

}


