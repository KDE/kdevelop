/*
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "branchmanager.h"

#include <QInputDialog>

#include <KMessageBox>
#include <KLocalizedString>

#include "../dvcsplugin.h"
#include <util/shellutils.h>
#include <vcs/vcsjob.h>
#include <vcs/models/brancheslistmodel.h>
#include "ui_branchmanager.h"
#include "debug.h"
#include "widgets/vcsdiffpatchsources.h"

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <util/wildcardhelpers.h>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <KParts/MainWindow>

using namespace KDevelop;

BranchManager::BranchManager(const QString& repository, KDevelop::DistributedVersionControlPlugin* executor, QWidget *parent)
    : QDialog(parent)
    , m_repository(repository)
    , m_dvcPlugin(executor)
{
    setWindowTitle(i18nc("@title:window", "Branch Manager"));

    KDevelop::restoreAndAutoSaveGeometry(*this, QStringLiteral("VCS"), QStringLiteral("BranchManager"));

    auto* mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);

    m_ui = new Ui::BranchDialogBase;
    auto* w = new QWidget(this);
    m_ui->setupUi(w);
    mainLayout->addWidget(w);

    auto iconWithFallback = [] (const QString &icon, const QString &fallback) {
        return QIcon::fromTheme(icon, QIcon::fromTheme(fallback));
    };
    m_ui->newButton->setIcon(iconWithFallback(QStringLiteral("vcs-branch"), QStringLiteral("list-add")));
    m_ui->deleteButton->setIcon(iconWithFallback(QStringLiteral("vcs-branch-delete"), QStringLiteral("edit-delete")));
    m_ui->diffButton->setIcon(iconWithFallback(QStringLiteral("vcs-diff"), QStringLiteral("text-x-patch")));
    m_ui->mergeButton->setIcon(iconWithFallback(QStringLiteral("vcs-merge"), QStringLiteral("merge")));

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &BranchManager::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &BranchManager::reject);
    mainLayout->addWidget(buttonBox);

    m_model = new BranchesListModel(this);
    m_model->initialize(m_dvcPlugin, QUrl::fromLocalFile(repository));

    // Filter Model
    m_filterModel = new QSortFilterProxyModel();
    m_filterModel->setSourceModel(m_model);
    m_filterModel->sort(0, Qt::AscendingOrder);

    //Changes in filter edit trigger filtering
    connect(m_ui->branchFilterEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        WildcardHelpers::setFilterNonPathWildcard(*m_filterModel, text);
    });

    m_ui->branchView->setModel(m_filterModel);

    QString branchName = m_model->currentBranch();
    // apply initial selection
    const auto currentBranchIndices =
        m_model->match(m_model->index(0, 0), Qt::DisplayRole, branchName, -1, Qt::MatchExactly);
    if (!currentBranchIndices.isEmpty()) {
        if (currentBranchIndices.size() != 1) {
            qCWarning(VCS) << "more than one branch matches the current branch, selecting the first one of"
                           << currentBranchIndices.size();
        }
        m_ui->branchView->setCurrentIndex(m_filterModel->mapFromSource(currentBranchIndices.constFirst()));
    }

    connect(m_ui->newButton, &QPushButton::clicked, this, &BranchManager::createBranch);
    connect(m_ui->deleteButton, &QPushButton::clicked, this, &BranchManager::deleteBranch);
    connect(m_ui->renameButton, &QPushButton::clicked, this, &BranchManager::renameBranch);
    connect(m_ui->checkoutButton, &QPushButton::clicked, this, &BranchManager::checkoutBranch);

    // checkout branch on double-click
    connect(m_ui->branchView, &QListView::doubleClicked, this, &BranchManager::checkoutBranch);

    connect(m_ui->mergeButton, &QPushButton::clicked, this, &BranchManager::mergeBranch);
    connect(m_ui->diffButton, &QPushButton::clicked, this, &BranchManager::diffFromBranch);
}

BranchManager::~BranchManager()
{
    delete m_ui;
}

void BranchManager::createBranch()
{
    const QModelIndex currentBranchIdx = m_ui->branchView->currentIndex();
    if (!currentBranchIdx.isValid()) {
        KMessageBox::error(this, i18n("You must select a base branch from the list before creating a new branch."));
        return;
    }
    QString baseBranch = currentBranchIdx.data().toString();
    bool branchNameEntered = false;
    QString newBranch = QInputDialog::getText(this, i18nc("@title:window", "New Branch"), i18nc("@label:textbox", "Name of the new branch:"),
            QLineEdit::Normal, QString(), &branchNameEntered);
    if (!branchNameEntered)
        return;

    if (!m_model->findItems(newBranch).isEmpty())
    {
        KMessageBox::error(this, i18n("Branch \"%1\" already exists.\n"
                                      "Please, choose another name.",
                                      newBranch));
    }
    else
        m_model->createBranch(baseBranch, newBranch);
}

void BranchManager::deleteBranch()
{
    QString baseBranch = m_ui->branchView->selectionModel()->selection().indexes().first().data().toString();

    if (baseBranch == m_model->currentBranch())
    {
        KMessageBox::error(this, i18n("Currently at the branch \"%1\".\n"
                                      "To delete it, please change to another branch.",
                                      baseBranch));
        return;
    }

    int ret = KMessageBox::warningTwoActions(
        this, i18n("Are you sure you want to irreversibly delete the branch '%1'?", baseBranch), {},
        KStandardGuiItem::del(), KStandardGuiItem::cancel());
    if (ret == KMessageBox::PrimaryAction)
        m_model->removeBranch(baseBranch);
}

void BranchManager::renameBranch()
{
    QModelIndex currentIndex = m_ui->branchView->currentIndex();
    if (!currentIndex.isValid())
        return;

    m_ui->branchView->edit(currentIndex);
}

void BranchManager::checkoutBranch()
{
    QString branch = m_ui->branchView->currentIndex().data().toString();
    if (branch == m_model->currentBranch())
    {
        KMessageBox::information(this, i18n("Already on branch \"%1\"\n", branch));
        return;
    }

    qCDebug(VCS) << "Switching to" << branch << "in" << m_repository;
    KDevelop::VcsJob *branchJob = m_dvcPlugin->switchBranch(QUrl::fromLocalFile(m_repository), branch);
//     connect(branchJob, SIGNAL(finished(KJob*)), m_model, SIGNAL(resetCurrent()));
    if (branchJob) {
        ICore::self()->runController()->registerJob(branchJob);
        close();
    }
}

void BranchManager::mergeBranch()
{
    const QModelIndex branchToMergeIdx = m_ui->branchView->currentIndex();

    if (branchToMergeIdx.isValid()) {
        QString branchToMerge = branchToMergeIdx.data().toString();

        if (m_model->findItems(branchToMerge).isEmpty()) {
            KMessageBox::error(this, i18n("Branch \"%1\" doesn't exists.\n"
                                          "Please, choose another name.", branchToMerge));
        } else {
            KDevelop::VcsJob* branchJob = m_dvcPlugin->mergeBranch(QUrl::fromLocalFile(m_repository), branchToMerge);
            ICore::self()->runController()->registerJob(branchJob);
            close();
        }
    } else {
        KMessageBox::error(this, i18n("You must select a branch to merge into current one from the list."));
    }
}

// adapted from VCSStandardDiffUpdater
class VCSBranchDiffUpdater : public VCSDiffUpdater {
public:
    VCSBranchDiffUpdater(const QString& repo, const QString& src, KDevelop::DistributedVersionControlPlugin* vcs);
    ~VCSBranchDiffUpdater() override;
    KDevelop::VcsDiff update() const override;
    KDevelop::IBasicVersionControl* vcs() const override { return m_vcs; }
    QUrl url() const override { return QUrl::fromLocalFile(m_repository); }
private:
    const QString m_repository;
    const QString m_src;
    KDevelop::DistributedVersionControlPlugin* m_vcs;
};

VCSBranchDiffUpdater::VCSBranchDiffUpdater(const QString& repo, const QString& src,
        KDevelop::DistributedVersionControlPlugin* vcs)
    : m_repository(repo)
    , m_src(src)
    , m_vcs(vcs)
{
}

VCSBranchDiffUpdater::~VCSBranchDiffUpdater() {
}

VcsDiff VCSBranchDiffUpdater::update() const
{
    VcsRevision srcRev;
    srcRev.setRevisionValue(m_src, KDevelop::VcsRevision::GlobalNumber);
    // see comment in BranchManager::diffFromBranch()
    const auto destRev = VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Working);
    QScopedPointer<VcsJob> diffJob(m_vcs->diff(QUrl::fromLocalFile(m_repository), srcRev, destRev));
    const bool success = diffJob ? diffJob->exec() : false;
    if (!success) {
        KMessageBox::error(nullptr, i18n("Could not create a patch for the current version."));
        return {};
    }

    return diffJob->fetchResults().value<VcsDiff>();
}

void BranchManager::diffFromBranch()
{
    const auto dest = m_model->currentBranch();
    const auto src = m_ui->branchView->currentIndex().data().toString();
    if (src == dest) {
        KMessageBox::information(this, i18n("Already on branch \"%1\"\n", src));
        return;
    }

    VcsRevision srcRev;
    srcRev.setRevisionValue(src, KDevelop::VcsRevision::GlobalNumber);
    // We have two options here:
    // * create a regular VcsRevision to represent the last commit on the current branch or
    // * create a special branch to reflect the staging area. I chose this one.
    // If the staging area is clean it automatically defaults to the first option.
    const auto destRev = VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Working);
    const auto job = m_dvcPlugin->diff(QUrl::fromLocalFile(m_repository), srcRev, destRev);
    connect(job, &VcsJob::finished, this, &BranchManager::diffJobFinished);
    m_dvcPlugin->core()->runController()->registerJob(job);
}

void BranchManager::diffJobFinished(KJob* job)
{
    auto vcsjob = qobject_cast<KDevelop::VcsJob*>(job);
    Q_ASSERT(vcsjob);

    if (vcsjob->status() != KDevelop::VcsJob::JobSucceeded) {
        KMessageBox::error(ICore::self()->uiController()->activeMainWindow(), vcsjob->errorString(),
                           i18nc("@titlew:indow", "Unable to Retrieve Diff"));
        return;
    }

    auto diff = vcsjob->fetchResults().value<KDevelop::VcsDiff>();
    if(diff.isEmpty()){
        KMessageBox::information(ICore::self()->uiController()->activeMainWindow(),
                                    i18n("There are no committed differences."),
                                    i18nc("@title:window", "VCS Support"));
        return;
    }

    auto patch = new VCSDiffPatchSource(new VCSBranchDiffUpdater(m_repository,
        m_ui->branchView->currentIndex().data().toString(), m_dvcPlugin));
    showVcsDiff(patch);
    close();
}

#include "moc_branchmanager.cpp"
