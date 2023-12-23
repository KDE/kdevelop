/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "vcsdiffpatchsources.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QFontDatabase>
#include <QLabel>
#include <QTemporaryFile>

#include <KComboBox>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEdit>

#include <interfaces/ibasicversioncontrol.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/isession.h>
#include "vcsdiff.h"
#include "vcsjob.h"
#include "debug.h"


using namespace KDevelop;

VCSCommitDiffPatchSource::VCSCommitDiffPatchSource(VCSDiffUpdater* updater)
    : VCSDiffPatchSource(updater), m_vcs(updater->vcs())
{
    Q_ASSERT(m_vcs);
    m_commitMessageWidget = new QWidget;
    auto* layout = new QVBoxLayout(m_commitMessageWidget.data());
    layout->setContentsMargins(0, 0, 0, 0);

    m_commitMessageEdit = new KTextEdit;
    m_commitMessageEdit.data()->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_commitMessageEdit.data()->setLineWrapMode(QTextEdit::NoWrap);
    m_vcs->setupCommitMessageEditor(updater->url(), m_commitMessageEdit.data());

    auto* titleLayout = new QHBoxLayout;
    titleLayout->addWidget(new QLabel(i18nc("@label:textbox", "Commit message:")));

    m_oldMessages = new KComboBox(m_commitMessageWidget.data());

    m_oldMessages->addItem(i18n("Old Messages"));
    const auto oldMessages = this->oldMessages();
    for (const QString& message : oldMessages) {
        m_oldMessages->addItem(message, message);
    }
    m_oldMessages->setMaximumWidth(200);

    connect(m_oldMessages, &QComboBox::currentTextChanged,
            this, &VCSCommitDiffPatchSource::oldMessageChanged);

    titleLayout->addWidget(m_oldMessages);

    layout->addLayout(titleLayout);
    layout->addWidget(m_commitMessageEdit.data());
    connect(this, &VCSCommitDiffPatchSource::reviewCancelled, this, &VCSCommitDiffPatchSource::addMessageToHistory);
    connect(this, &VCSCommitDiffPatchSource::reviewFinished, this, &VCSCommitDiffPatchSource::addMessageToHistory);
}

QStringList VCSCommitDiffPatchSource::oldMessages() const
{
    KConfigGroup vcsGroup(ICore::self()->activeSession()->config(), QStringLiteral("VCS"));
    return vcsGroup.readEntry("OldCommitMessages", QStringList());
}

void VCSCommitDiffPatchSource::addMessageToHistory(const QString& message)
{
    if(ICore::self()->shuttingDown())
        return;

    KConfigGroup vcsGroup(ICore::self()->activeSession()->config(), QStringLiteral("VCS"));

    const int maxMessages = 10;
    QStringList oldMessages = vcsGroup.readEntry("OldCommitMessages", QStringList());

    oldMessages.removeAll(message);
    oldMessages.push_front(message);
    oldMessages = oldMessages.mid(0, maxMessages);

    vcsGroup.writeEntry("OldCommitMessages", oldMessages);
}

void VCSCommitDiffPatchSource::oldMessageChanged(const QString& text)
{
    if(m_oldMessages->currentIndex() != 0)
    {
        m_oldMessages->setCurrentIndex(0);
        m_commitMessageEdit.data()->setText(text);
    }
}

void VCSCommitDiffPatchSource::jobFinished(KJob *job)
{
    if (!job || job->error() != 0 )
    {
        QString details = job ? job->errorText() : QString();
        if (details.isEmpty()) {    //errorText may be empty
            details = i18n("For more detailed information please see the Version Control tool view.");
        }
        KMessageBox::detailedError(nullptr, i18n("Unable to commit"), details, i18nc("@title:window", "Commit Unsuccessful"));
    }

    deleteLater();
}

VCSDiffPatchSource::VCSDiffPatchSource(VCSDiffUpdater* updater)
    : m_updater(updater)
{
    update();
    KDevelop::IBasicVersionControl* vcs = m_updater->vcs();
    QUrl url = m_updater->url();

    QScopedPointer<VcsJob> statusJob(vcs->status(QList<QUrl>() << url));
    QVariant varlist;

    if( statusJob->exec() && statusJob->status() == VcsJob::JobSucceeded )
    {
        varlist = statusJob->fetchResults();

        const auto vars = varlist.toList();
        m_infos.reserve(m_infos.size() + vars.size());
        for (const auto& var : vars) {
            VcsStatusInfo info = var.value<KDevelop::VcsStatusInfo>();

            m_infos += info;
            if(info.state()!=VcsStatusInfo::ItemUpToDate)
                m_selectable[info.url()] = info.state();
        }
    }
    else
        qCDebug(VCS) << "Couldn't get status for urls: " << url;
}

VCSDiffPatchSource::VCSDiffPatchSource(const KDevelop::VcsDiff& diff)
    : m_updater(nullptr)
{
    updateFromDiff(diff);
}

VCSDiffPatchSource::~VCSDiffPatchSource()
{
    QFile::remove(m_file.toLocalFile());
    delete m_updater;
}

QUrl VCSDiffPatchSource::baseDir() const {
    return m_base;
}

QUrl VCSDiffPatchSource::file() const {
    return m_file;
}

QString VCSDiffPatchSource::name() const {
    return m_name;
}

uint VCSDiffPatchSource::depth() const {
    return m_depth;
}

void VCSDiffPatchSource::updateFromDiff(const VcsDiff& vcsdiff)
{
    if(!m_file.isValid())
    {
        QTemporaryFile temp2(QDir::tempPath() + QLatin1String("/kdevelop_XXXXXX.patch"));
        temp2.setAutoRemove(false);
        temp2.open();
        QTextStream t2(&temp2);
        t2 << vcsdiff.diff();
        qCDebug(VCS) << "filename:" << temp2.fileName();
        m_file = QUrl::fromLocalFile(temp2.fileName());
        temp2.close();
    }else{
        QFile file(m_file.path());
        file.open(QIODevice::WriteOnly);
        QTextStream t2(&file);
        t2 << vcsdiff.diff();
    }

    qCDebug(VCS) << "using file" << m_file << vcsdiff.diff() << "base" << vcsdiff.baseDiff();

    m_name = QStringLiteral("VCS Diff");
    m_base = vcsdiff.baseDiff();
    m_depth = vcsdiff.depth();

    emit patchChanged();
}

void VCSDiffPatchSource::update() {
    if(!m_updater)
        return;
    updateFromDiff(m_updater->update());
}

VCSCommitDiffPatchSource::~VCSCommitDiffPatchSource() {
    delete m_commitMessageWidget.data();
}

bool VCSCommitDiffPatchSource::canSelectFiles() const {
    return true;
}

QMap< QUrl, KDevelop::VcsStatusInfo::State> VCSDiffPatchSource::additionalSelectableFiles() const {
    return m_selectable;
}

QWidget* VCSCommitDiffPatchSource::customWidget() const {
    return m_commitMessageWidget.data();
}

QString VCSCommitDiffPatchSource::finishReviewCustomText() const {
    return i18nc("@action:button To make a commit", "Commit");
}

bool VCSCommitDiffPatchSource::canCancel() const {
    return true;
}

void VCSCommitDiffPatchSource::cancelReview() {

    QString message;

    if (m_commitMessageEdit)
        message = m_commitMessageEdit.data()->toPlainText();

    emit reviewCancelled(message);

    deleteLater();
}

bool VCSCommitDiffPatchSource::finishReview(const QList<QUrl>& selection)
{
    QString message;

    if (m_commitMessageEdit)
        message = m_commitMessageEdit.data()->toPlainText();

    qCDebug(VCS) << "Finishing with selection" << selection;
    QString files;
    files.reserve(selection.size());
    for (const QUrl& url : selection) {
        files += QLatin1String("<li>") + ICore::self()->projectController()->prettyFileName(url, KDevelop::IProjectController::FormatPlain) + QLatin1String("</li>");
    }

    QString text = i18n("<qt>Files will be committed:\n<ul>%1</ul>\nWith message:\n <pre>%2</pre></qt>", files, message);

    int res = KMessageBox::warningContinueCancel(nullptr, text, i18nc("@title:window", "About to Commit to Repository"),
                                                 KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                 QStringLiteral("ShouldAskConfirmCommit"));
    if (res != KMessageBox::Continue) {
        return false;
    }

    emit reviewFinished(message, selection);

    VcsJob* job = m_vcs->commit(message, selection, KDevelop::IBasicVersionControl::NonRecursive);
    if (!job) {
        return false;
    }

    connect (job, &VcsJob::finished,
             this, &VCSCommitDiffPatchSource::jobFinished);
    ICore::self()->runController()->registerJob(job);
    return true;
}

bool showVcsDiff(IPatchSource* vcsDiff)
{
    auto* patchReview = ICore::self()->pluginController()->extensionForPlugin<IPatchReview>(QStringLiteral("org.kdevelop.IPatchReview"));

    if( patchReview ) {
        patchReview->startReview(vcsDiff);
        return true;
    } else {
        qCWarning(VCS) << "Patch review plugin not found";
        return false;
    }
}

VcsDiff VCSStandardDiffUpdater::update() const
{
    QScopedPointer<VcsJob> diffJob(m_vcs->diff(m_url,
                                   KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Base),
                                   KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Working)));
    const bool success = diffJob ? diffJob->exec() : false;
    if (!success) {
        KMessageBox::error(nullptr, i18n("Could not create a patch for the current version."));
        return {};
    }

    return diffJob->fetchResults().value<VcsDiff>();
}

VCSStandardDiffUpdater::VCSStandardDiffUpdater(IBasicVersionControl* vcs, const QUrl& url)
    : m_vcs(vcs)
    , m_url(url)
{
}

VCSStandardDiffUpdater::~VCSStandardDiffUpdater() {
}

VCSDiffUpdater::~VCSDiffUpdater() {
}

#include "moc_vcsdiffpatchsources.cpp"
