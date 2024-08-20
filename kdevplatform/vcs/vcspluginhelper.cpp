/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "vcspluginhelper.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QMenu>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>

#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/MainWindow>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugin.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/isession.h>
#include <interfaces/iuicontroller.h>
#include <util/path.h>
#include <util/scopeddialog.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/models/vcsannotationmodel.h>
#include <vcs/widgets/vcsannotationitemdelegate.h>
#include <vcs/widgets/vcseventwidget.h>
#include <vcs/widgets/vcscommitdialog.h>
#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcslocation.h>
#include <vcs/vcsdiff.h>

#include "interfaces/idistributedversioncontrol.h"
#include "vcsevent.h"
#include "debug.h"
#include "widgets/vcsdiffpatchsources.h"

namespace KDevelop
{

class VcsPluginHelperPrivate
{
public:
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
        auto iconWithFallback = [] (const QString &icon, const QString &fallback) {
            return QIcon::fromTheme(icon, QIcon::fromTheme(fallback));
        };
        commitAction = new QAction(iconWithFallback(QStringLiteral("vcs-commit"), QStringLiteral("svn-commit")), i18nc("@action:inmenu", "Commit..."), parent);
        updateAction = new QAction(iconWithFallback(QStringLiteral("vcs-pull"), QStringLiteral("svn-update")), i18nc("@action:inmenu", "Update"), parent);
        addAction = new QAction(QIcon::fromTheme(QStringLiteral("list-add")), i18nc("@action:inmenu", "Add"), parent);
        diffToBaseAction = new QAction(iconWithFallback(QStringLiteral("vcs-diff"), QStringLiteral("text-x-patch")), i18nc("@action:inmenu", "Show Differences..."), parent);
        revertAction = new QAction(QIcon::fromTheme(QStringLiteral("archive-remove")), i18nc("@action:inmenu", "Revert"), parent);
        historyAction = new QAction(QIcon::fromTheme(QStringLiteral("view-history")), i18nc("@action:inmenu revision history", "History..."), parent);
        annotationAction = new QAction(QIcon::fromTheme(QStringLiteral("user-properties")), i18nc("@action:inmenu", "Annotation..."), parent);
        diffForRevAction = new QAction(iconWithFallback(QStringLiteral("vcs-diff"), QStringLiteral("text-x-patch")), i18nc("@action:inmenu", "Show Diff..."), parent);
        diffForRevGlobalAction = new QAction(iconWithFallback(QStringLiteral("vcs-diff"), QStringLiteral("text-x-patch")), i18nc("@action:inmenu", "Show Diff (All Files)..."), parent);
        pushAction = new QAction(iconWithFallback(QStringLiteral("vcs-push"), QStringLiteral("arrow-up-double")), i18nc("@action:inmenu", "Push"), parent);
        pullAction = new QAction(iconWithFallback(QStringLiteral("vcs-pull"), QStringLiteral("arrow-down-double")), i18nc("@action:inmenu", "Pull"), parent);

        QObject::connect(commitAction, &QAction::triggered, parent, &VcsPluginHelper::commit);
        QObject::connect(addAction, &QAction::triggered, parent, &VcsPluginHelper::add);
        QObject::connect(updateAction, &QAction::triggered, parent, &VcsPluginHelper::update);
        QObject::connect(diffToBaseAction, &QAction::triggered, parent, &VcsPluginHelper::diffToBase);
        QObject::connect(revertAction, &QAction::triggered, parent, &VcsPluginHelper::revert);
        QObject::connect(historyAction, &QAction::triggered, parent, [=] { parent->history(); });
        QObject::connect(annotationAction, &QAction::triggered, parent, &VcsPluginHelper::annotation);
        QObject::connect(diffForRevAction, &QAction::triggered, parent, QOverload<>::of(&VcsPluginHelper::diffForRev));
        QObject::connect(diffForRevGlobalAction, &QAction::triggered, parent, &VcsPluginHelper::diffForRevGlobal);
        QObject::connect(pullAction, &QAction::triggered, parent, &VcsPluginHelper::pull);
        QObject::connect(pushAction, &QAction::triggered, parent, &VcsPluginHelper::push);
    }

    bool allLocalFiles(const QList<QUrl>& urls)
    {
        bool ret=true;
        for (const QUrl& url : urls) {
            QFileInfo info(url.toLocalFile());
            ret &= info.isFile();
        }
        return ret;
    }

    QMenu* createMenu(QWidget* parent)
    {
        auto* menu = new QMenu(vcs->name(), parent);
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

        const bool singleVersionedFile = ctxUrls.size() == 1 && vcs->isVersionControlled(ctxUrls.constFirst());
        historyAction->setEnabled(singleVersionedFile);
        annotationAction->setEnabled(singleVersionedFile && allLocalFiles(ctxUrls));
        diffToBaseAction->setEnabled(singleVersionedFile);
        commitAction->setEnabled(singleVersionedFile);

        return menu;
    }
};


VcsPluginHelper::VcsPluginHelper(KDevelop::IPlugin* parent, KDevelop::IBasicVersionControl* vcs)
        : QObject(parent)
        , d_ptr(new VcsPluginHelperPrivate())
{
    Q_D(VcsPluginHelper);

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
    Q_D(VcsPluginHelper);

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
    Q_D(VcsPluginHelper);

    d->ctxUrls = context->urls();
}

QList<QUrl> VcsPluginHelper::contextUrlList() const
{
    Q_D(const VcsPluginHelper);

    return d->ctxUrls;
}

QMenu* VcsPluginHelper::commonActions(QWidget* parent)
{
    Q_D(VcsPluginHelper);

    /* TODO: the following logic to determine which actions need to be enabled
     * or disabled does not work properly. What needs to be implemented is that
     * project items that are vc-controlled enable all except add, project
     * items that are not vc-controlled enable add action. For urls that cannot
     * be made into a project item, or if the project has no associated VC
     * plugin we need to check whether a VC controls the parent dir, if we have
     * one we assume the urls can be added but are not currently controlled. If
     * the url is already version controlled then just enable all except add
     */
    return d->createMenu(parent);
}

#define EXECUTE_VCS_METHOD( method ) \
    d->plugin->core()->runController()->registerJob( d->vcs-> method ( d->ctxUrls ) )

#define SINGLEURL_SETUP_VARS \
    KDevelop::IBasicVersionControl* iface = d->vcs;\
    const QUrl &url = d->ctxUrls.front();


void VcsPluginHelper::revert()
{
    Q_D(VcsPluginHelper);

    VcsJob* job=d->vcs->revert(d->ctxUrls);
    connect(job, &VcsJob::finished, this, &VcsPluginHelper::revertDone);

    for (const QUrl& url : std::as_const(d->ctxUrls)) {
        IDocument* doc=ICore::self()->documentController()->documentForUrl(url);

        if(doc && doc->textDocument()) {
            doc->textDocument()->setModifiedOnDiskWarning(false);
            doc->textDocument()->setModified(false);
        }
    }
    job->setProperty("urls", QVariant::fromValue(d->ctxUrls));

    d->plugin->core()->runController()->registerJob(job);
}

void VcsPluginHelper::revertDone(KJob* job)
{
    auto* modificationTimer = new QTimer;
    modificationTimer->setInterval(100);
    connect(modificationTimer, &QTimer::timeout, this, &VcsPluginHelper::delayedModificationWarningOn);
    connect(modificationTimer, &QTimer::timeout, modificationTimer, &QTimer::deleteLater);


    modificationTimer->setProperty("urls", job->property("urls"));
    modificationTimer->start();
}

void VcsPluginHelper::delayedModificationWarningOn()
{
    QObject* timer = sender();
    const QList<QUrl> urls = timer->property("urls").value<QList<QUrl>>();

    for (const QUrl& url : urls) {
        IDocument* doc=ICore::self()->documentController()->documentForUrl(url);

        if(doc) {
            doc->reload();
            doc->textDocument()->setModifiedOnDiskWarning(true);
        }
    }
}


void VcsPluginHelper::diffJobFinished(KJob* job)
{
    auto* vcsjob = qobject_cast<KDevelop::VcsJob*>(job);
    Q_ASSERT(vcsjob);

    if (vcsjob->status() == KDevelop::VcsJob::JobSucceeded) {
        KDevelop::VcsDiff d = vcsjob->fetchResults().value<KDevelop::VcsDiff>();
        if(d.isEmpty())
            KMessageBox::information(ICore::self()->uiController()->activeMainWindow(),
                                     i18n("There are no differences."),
                                     i18nc("@title:window", "VCS Support"));
        else {
            auto* patch=new VCSDiffPatchSource(d);
            showVcsDiff(patch);
        }
    } else {
        KMessageBox::error(ICore::self()->uiController()->activeMainWindow(), vcsjob->errorString(), i18nc("@title:window", "Unable to Get Differences"));
    }
}

void VcsPluginHelper::diffToBase()
{
    Q_D(VcsPluginHelper);

    SINGLEURL_SETUP_VARS
    if (!ICore::self()->documentController()->saveAllDocuments()) {
        return;
    }

    auto* patch =new VCSDiffPatchSource(new VCSStandardDiffUpdater(iface, url));
    showVcsDiff(patch);
}

void VcsPluginHelper::diffForRev()
{
    Q_D(VcsPluginHelper);

    if (d->ctxUrls.isEmpty()) {
        return;
    }
    diffForRev(d->ctxUrls.first());
}

void VcsPluginHelper::diffForRevGlobal()
{
    Q_D(VcsPluginHelper);

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
    Q_D(VcsPluginHelper);

    auto* action = qobject_cast<QAction*>( sender() );
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
    Q_D(VcsPluginHelper);

    SINGLEURL_SETUP_VARS
    auto* dlg = new QDialog(ICore::self()->uiController()->activeMainWindow());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(i18nc("@title:window %1: path or URL, %2: name of a version control system",
                          "%2 History (%1)", url.toDisplayString(QUrl::PreferLocalFile), iface->name()));

    auto *mainLayout = new QVBoxLayout(dlg);

    auto* logWidget = new KDevelop::VcsEventWidget(url, rev, iface, dlg);
    mainLayout->addWidget(logWidget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    dlg->connect(buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    dlg->connect(buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    dlg->show();
}

static VcsAnnotationModel* vcsAnnotationModel(const QUrl& url, KTextEditor::Document& document, KTextEditor::View& view)
{
    if (auto* const abstractModel = document.annotationModel()) {
        if (auto* const model = qobject_cast<VcsAnnotationModel*>(abstractModel)) {
            Q_ASSERT(model->referenceCount > 0);
            ++model->referenceCount;
            return model;
        }
        qCWarning(VCS) << "replacing unsupported non-VCS annotation model" << abstractModel << "at"
                       << url.toString(QUrl::PreferLocalFile);
    }

    const auto style = view.defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal);
    const auto foreground = style->foreground().color();
    const auto background = style->hasProperty(QTextFormat::BackgroundBrush) ? style->background().color() : Qt::white;
    auto* const model = new VcsAnnotationModel(url, &document, foreground, background);
    Q_ASSERT(model->referenceCount == 1);

    document.setAnnotationModel(model);
    return model;
}

static VcsAnnotationItemDelegate* vcsAnnotationItemDelegate(KTextEditor::View& view)
{
    if (auto* const delegate = qobject_cast<VcsAnnotationItemDelegate*>(view.annotationItemDelegate())) {
        return delegate;
    }

    auto* const delegate = new VcsAnnotationItemDelegate(&view);
    view.setAnnotationItemDelegate(delegate);
    view.setAnnotationUniformItemSizes(true);
    return delegate;
}

void VcsPluginHelper::annotation()
{
    Q_D(VcsPluginHelper);

    SINGLEURL_SETUP_VARS
    KDevelop::IDocument* doc = ICore::self()->documentController()->documentForUrl(url);

    if (!doc)
        doc = ICore::self()->documentController()->openDocument(url);

    KTextEditor::View* view = doc ? doc->activeTextView() : nullptr;
    if (view && view->isAnnotationBorderVisible()) {
        view->setAnnotationBorderVisible(false);
        return;
    }

    if (doc && doc->textDocument() && iface) {
        KDevelop::VcsJob* job = iface->annotate(url);
        if( !job )
        {
            qCWarning(VCS) << "Couldn't create annotate job for:" << url << "with iface:" << iface << dynamic_cast<KDevelop::IPlugin*>( iface );
            return;
        }

        if (view) {
            auto* const model = vcsAnnotationModel(url, *doc->textDocument(), *view);
            model->setAnnotationJob(job);

            auto* const delegate = vcsAnnotationItemDelegate(*view);
            delegate->enable(model);

            view->setAnnotationBorderVisible(true);
            connect(view, &KTextEditor::View::annotationContextMenuAboutToShow, this,
                    &VcsPluginHelper::annotationContextMenuAboutToShow);
            connect(view, &KTextEditor::View::annotationBorderVisibilityChanged, this,
                    &VcsPluginHelper::handleAnnotationBorderVisibilityChanged);
        } else {
            KMessageBox::error(nullptr, i18n("Cannot display annotations, missing interface KTextEditor::AnnotationInterface for the editor."));
            delete job;
        }
    } else {
        KMessageBox::error(nullptr, i18n("Cannot execute annotate action because the "
                                   "document was not found, or was not a text document:\n%1", url.toDisplayString(QUrl::PreferLocalFile)));
    }
}

void VcsPluginHelper::annotationContextMenuAboutToShow( KTextEditor::View* view, QMenu* menu, int line )
{
    Q_D(VcsPluginHelper);

    view->annotationItemDelegate()->hideTooltip(view);

    const auto* const model = qobject_cast<VcsAnnotationModel*>(view->document()->annotationModel());
    Q_ASSERT(model);

    VcsRevision rev = model->revisionForLine(line);
    // check if the user clicked on a row without revision information
    if (rev.revisionType() == VcsRevision::Invalid) {
        // in this case, do not action depending on revision information
        return;
    }

    d->diffForRevAction->setData(QVariant::fromValue(rev));
    d->diffForRevGlobalAction->setData(QVariant::fromValue(rev));
    menu->addSeparator();
    menu->addAction(d->diffForRevAction);
    menu->addAction(d->diffForRevGlobalAction);

    QAction* copyAction = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18nc("@action:inmenu", "Copy Revision Id"));
    connect(copyAction, &QAction::triggered, this, [rev]() {
        QApplication::clipboard()->setText(rev.revisionValue().toString());
    });

    QAction* historyAction = menu->addAction(QIcon::fromTheme(QStringLiteral("view-history")), i18nc("@action:inmenu revision history", "History..."));
    connect(historyAction, &QAction::triggered, this, [this, rev]() {
        history(rev);
    });
}

void VcsPluginHelper::handleAnnotationBorderVisibilityChanged(KTextEditor::View* view, bool visible)
{
    if (visible) {
        return;
    }

    disconnect(view, &KTextEditor::View::annotationContextMenuAboutToShow, this,
               &VcsPluginHelper::annotationContextMenuAboutToShow);
    disconnect(view, &KTextEditor::View::annotationBorderVisibilityChanged, this,
               &VcsPluginHelper::handleAnnotationBorderVisibilityChanged);

    // Disable rather than destroy our delegate, because calling view->setAnnotationItemDelegate(nullptr)
    // creates a new default delegate KateAnnotationItemDelegate and thus is less efficient.
    if (auto* const delegate = qobject_cast<VcsAnnotationItemDelegate*>(view->annotationItemDelegate())) {
        delegate->disable();
    }

    auto* const document = view->document();
    auto* const model = qobject_cast<VcsAnnotationModel*>(document->annotationModel());
    Q_ASSERT(model);

    --model->referenceCount;
    Q_ASSERT(model->referenceCount >= 0);
    if (model->referenceCount == 0) {
        // The model is not used by another view of this document => destroy it.
        document->setAnnotationModel(nullptr);
        delete model;
    }
}

void VcsPluginHelper::update()
{
    Q_D(VcsPluginHelper);

    EXECUTE_VCS_METHOD(update);
}

void VcsPluginHelper::add()
{
    Q_D(VcsPluginHelper);

    EXECUTE_VCS_METHOD(add);
}

void VcsPluginHelper::commit()
{
    Q_D(VcsPluginHelper);

    Q_ASSERT(!d->ctxUrls.isEmpty());
    ICore::self()->documentController()->saveAllDocuments();

    QUrl url = d->ctxUrls.first();

    // We start the commit UI no matter whether there is real differences, as it can also be used to commit untracked files
    auto* patchSource = new VCSCommitDiffPatchSource(new VCSStandardDiffUpdater(d->vcs, url));

    bool ret = showVcsDiff(patchSource);

    if(!ret) {
        ScopedDialog<VcsCommitDialog> commitDialog(patchSource);
        commitDialog->setCommitCandidates(patchSource->infos());
        commitDialog->exec();
    }
}

void VcsPluginHelper::push()
{
    Q_D(VcsPluginHelper);

    for (const QUrl& url : std::as_const(d->ctxUrls)) {
        VcsJob* job = d->plugin->extension<IDistributedVersionControl>()->push(url, VcsLocation());
        ICore::self()->runController()->registerJob(job);
    }
}

void VcsPluginHelper::pull()
{
    Q_D(VcsPluginHelper);

    for (const QUrl& url : std::as_const(d->ctxUrls)) {
        VcsJob* job = d->plugin->extension<IDistributedVersionControl>()->pull(VcsLocation(), url);
        ICore::self()->runController()->registerJob(job);
    }
}

}

#include "moc_vcspluginhelper.cpp"
