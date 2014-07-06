/*
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "vcsdiffpatchsources.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include <kglobalsettings.h>
#include <klocalizedstring.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <kmessagebox.h>
#include <interfaces/iruncontroller.h>
#include "vcsjob.h"
#include "vcsdiff.h"
#include <interfaces/iplugincontroller.h>
#include <KComboBox>
#include <KTextEdit>
#include <interfaces/isession.h>
#include <interfaces/ibasicversioncontrol.h>

using namespace KDevelop;

VCSCommitDiffPatchSource::VCSCommitDiffPatchSource(VCSDiffUpdater* updater)
    : VCSDiffPatchSource(updater), m_vcs(updater->vcs())
{
    Q_ASSERT(m_vcs);
    m_commitMessageWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_commitMessageWidget.data());

    m_commitMessageEdit = new KTextEdit;
    m_commitMessageEdit.data()->setFont( KGlobalSettings::fixedFont() );
    m_commitMessageEdit.data()->setLineWrapMode(QTextEdit::NoWrap);
    m_vcs->setupCommitMessageEditor(updater->url(), m_commitMessageEdit.data());
    
    QHBoxLayout* titleLayout = new QHBoxLayout;
    titleLayout->addWidget(new QLabel(i18n("Commit Message:")));
    
    m_oldMessages = new KComboBox(m_commitMessageWidget.data());
    
    m_oldMessages->addItem(i18n("Old Messages"));
    foreach(QString message, oldMessages())
        m_oldMessages->addItem(message, message);
    m_oldMessages->setMaximumWidth(200);
    
    connect(m_oldMessages, SIGNAL(currentIndexChanged(QString)), this, SLOT(oldMessageChanged(QString)));
    
    titleLayout->addWidget(m_oldMessages);
    
    layout->addLayout(titleLayout);
    layout->addWidget(m_commitMessageEdit.data());
    connect(this, SIGNAL(reviewCancelled(QString)), SLOT(addMessageToHistory(QString)));
    connect(this, SIGNAL(reviewFinished(QString,QList<KUrl>)), SLOT(addMessageToHistory(QString)));
}

QStringList VCSCommitDiffPatchSource::oldMessages() const
{
    KConfigGroup vcsGroup(ICore::self()->activeSession()->config(), "VCS");
    return vcsGroup.readEntry("OldCommitMessages", QStringList());
}

void VCSCommitDiffPatchSource::addMessageToHistory(const QString& message)
{
    if(ICore::self()->shuttingDown())
        return;
    
    KConfigGroup vcsGroup(ICore::self()->activeSession()->config(), "VCS");
    
    const int maxMessages = 10;
    QStringList oldMessages = vcsGroup.readEntry("OldCommitMessages", QStringList());
    
    oldMessages.removeAll(message);
    oldMessages.push_front(message);
    oldMessages = oldMessages.mid(0, maxMessages);
    
    vcsGroup.writeEntry("OldCommitMessages", oldMessages);
}

void VCSCommitDiffPatchSource::oldMessageChanged(QString text)
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
            details = i18n("For more detailed information please see the Version Control toolview");
        }
        KMessageBox::detailedError(0, i18n("Unable to commit"), details, i18n("Commit unsuccessful"));
    }

    deleteLater();
}

VCSDiffPatchSource::VCSDiffPatchSource(VCSDiffUpdater* updater)
    : m_updater(updater)
{
    update();
    KDevelop::IBasicVersionControl* vcs = m_updater->vcs();
    KUrl url = m_updater->url();

    QScopedPointer<VcsJob> statusJob(vcs->status(url));
    QVariant varlist;

    if( statusJob->exec() && statusJob->status() == VcsJob::JobSucceeded )
    {
        varlist = statusJob->fetchResults();

        foreach( const QVariant &var, varlist.toList() )
        {
            VcsStatusInfo info = var.value<KDevelop::VcsStatusInfo>();
            
            m_infos += info;
            if(info.state()!=VcsStatusInfo::ItemUpToDate)
                m_selectable[info.url()] = info.state();
        }
    }
    else
        kDebug() << "Couldn't get status for urls: " << url;
}

VCSDiffPatchSource::VCSDiffPatchSource(const KDevelop::VcsDiff& diff)
    : m_updater(0)
{
    updateFromDiff(diff);
}

VCSDiffPatchSource::~VCSDiffPatchSource()
{
    QFile::remove(m_file.toLocalFile());
    delete m_updater;
}

KUrl VCSDiffPatchSource::baseDir() const {
    return m_base;
}

KUrl VCSDiffPatchSource::file() const {
    return m_file;
}

QString VCSDiffPatchSource::name() const {
    return m_name;
}

void VCSDiffPatchSource::updateFromDiff(VcsDiff vcsdiff)
{
    if(!m_file.isValid())
    {
        KTemporaryFile temp2;
        temp2.setSuffix("2.patch");
        temp2.setAutoRemove(false);
        temp2.open();
        QTextStream t2(&temp2);
        t2 << vcsdiff.diff();
        kDebug() << "filename:" << temp2.fileName();
        m_file = KUrl(temp2.fileName());
        temp2.close();
    }else{
        QFile file(m_file.path());
        file.open(QIODevice::WriteOnly);
        QTextStream t2(&file);
        t2 << vcsdiff.diff();
    }

    kDebug() << "using file" << m_file << vcsdiff.diff() << "base" << vcsdiff.baseDiff();

    m_name = "VCS Diff";
    m_base = vcsdiff.baseDiff();
    m_base.addPath("/");
    
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

QMap< KUrl, KDevelop::VcsStatusInfo::State> VCSDiffPatchSource::additionalSelectableFiles() const {
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

bool VCSCommitDiffPatchSource::finishReview(QList< KUrl > selection) {

    QString message;

    if (m_commitMessageEdit)
        message = m_commitMessageEdit.data()->toPlainText();

    kDebug() << "Finishing with selection" << selection;
    QString files;
    foreach(const KUrl& url, selection)
        files += "<li>"+ICore::self()->projectController()->prettyFileName(url, KDevelop::IProjectController::FormatPlain) + "</li>";

    QString text = i18n("<qt>Files will be committed:\n<ul>%1</ul>\nWith message:\n <pre>%2</pre></qt>", files, message);

    int res = KMessageBox::warningContinueCancel(0, text, i18n("About to commit to repository"),
                                                 KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                 "ShouldAskConfirmCommit");
    if (res != KMessageBox::Continue) {
        return false;
    }

    emit reviewFinished(message, selection);

    VcsJob* job=m_vcs->commit(message, selection, KDevelop::IBasicVersionControl::NonRecursive);

    connect (job, SIGNAL(finished(KJob*)),
             this, SLOT(jobFinished(KJob*)));
    ICore::self()->runController()->registerJob(job);


    return true;
}

static KDevelop::IPatchSource::Ptr currentShownDiff;

bool showVcsDiff(IPatchSource* vcsDiff)
{
    KDevelop::IPatchReview* patchReview = ICore::self()->pluginController()->extensionForPlugin<IPatchReview>("org.kdevelop.IPatchReview");

    //Only give one VCS diff at a time to the patch review plugin
    delete currentShownDiff;
    
    currentShownDiff = vcsDiff;
    
    if( patchReview ) {
        patchReview->startReview(currentShownDiff);
        return true;
    } else {
        kWarning() << "Patch review plugin not found";
        return false;
    }
}

VcsDiff VCSStandardDiffUpdater::update() const {
    QScopedPointer<VcsJob> diffJob(m_vcs->diff(m_url,
                                   KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Base),
                                   KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Working)));

    VcsDiff diff;
    bool correctDiff = diffJob->exec();
    if (correctDiff)
        diff = diffJob->fetchResults().value<VcsDiff>();

    if (!correctDiff)
        KMessageBox::error(0, i18n("Could not create a patch for the current version."));

    return diff;
}

VCSStandardDiffUpdater::VCSStandardDiffUpdater(IBasicVersionControl* vcs, KUrl url) : m_vcs(vcs), m_url(url) {
}

VCSStandardDiffUpdater::~VCSStandardDiffUpdater() {
}

VCSDiffUpdater::~VCSDiffUpdater() {
}

#include "vcsdiffpatchsources.moc"
