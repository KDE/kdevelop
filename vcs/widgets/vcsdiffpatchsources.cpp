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
#include <interfaces/iplugincontroller.h>
#include <KComboBox>

using namespace KDevelop;

VCSCommitDiffPatchSource::VCSCommitDiffPatchSource(const KDevelop::VcsDiff& vcsdiff, QMap< KUrl, KDevelop::VcsStatusInfo::State> selectable, IBasicVersionControl* vcs, QStringList oldMessages)
    : VCSDiffPatchSource(vcsdiff), m_selectable(selectable), m_vcs(vcs)
{
    Q_ASSERT(m_vcs);

    m_commitMessageWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_commitMessageWidget);

    m_commitMessageEdit = new QTextEdit;
    m_commitMessageEdit->setFont( KGlobalSettings::fixedFont() );
    
    QHBoxLayout* titleLayout = new QHBoxLayout;
    titleLayout->addWidget(new QLabel(i18n("Commit Message:")));
    
    m_oldMessages = new KComboBox;
    
    m_oldMessages->addItem(i18n("Old Messages"));
    foreach(QString message, oldMessages)
        m_oldMessages->addItem(message, message);
    m_oldMessages->setMaximumWidth(200);
    
    connect(m_oldMessages, SIGNAL(currentIndexChanged(QString)), this, SLOT(oldMessageChanged(QString)));
    
    titleLayout->addWidget(m_oldMessages);
    
    layout->addLayout(titleLayout);
    layout->addWidget(m_commitMessageEdit);
}

void VCSCommitDiffPatchSource::oldMessageChanged(QString text)
{
    if(m_oldMessages->currentIndex() != 0)
    {
        m_oldMessages->setCurrentIndex(0);
        m_commitMessageEdit->setText(text);
    }
}

VCSDiffPatchSource::VCSDiffPatchSource(const KDevelop::VcsDiff& vcsdiff)
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

    kDebug() << "using file" << m_file << vcsdiff.diff();

    m_name = "VCS Diff";
    m_base = vcsdiff.baseDiff();
}

VCSDiffPatchSource::~VCSDiffPatchSource()
{
    QFile::remove(m_file.toLocalFile());
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

void VCSDiffPatchSource::update() {
}

VCSCommitDiffPatchSource::~VCSCommitDiffPatchSource() {
    delete m_commitMessageWidget;
}

bool VCSCommitDiffPatchSource::canSelectFiles() const {
    return true;
}

QMap< KUrl, KDevelop::VcsStatusInfo::State> VCSCommitDiffPatchSource::additionalSelectableFiles() const {
    return m_selectable;
}

QWidget* VCSCommitDiffPatchSource::customWidget() const {
    return m_commitMessageWidget;
}

QString VCSCommitDiffPatchSource::finishReviewCustomText() const {
    return i18n("Commit");
}

bool VCSCommitDiffPatchSource::canCancel() const {
    return true;
}

void VCSCommitDiffPatchSource::cancelReview() {
    
    QString message;

    if (m_commitMessageEdit)
        message = m_commitMessageEdit->toPlainText();

    emit reviewCancelled(message);
    
    deleteLater();
}

bool VCSCommitDiffPatchSource::finishReview(QList< KUrl > selection) {

    QString message;

    if (m_commitMessageEdit)
        message = m_commitMessageEdit->toPlainText();

    kDebug() << "Finishing with selection" << selection;
    QString text = i18n("Files will be committed:\n");
    foreach(const KUrl& url, selection)
        text += ICore::self()->projectController()->prettyFileName(url, KDevelop::IProjectController::FormatPlain) + '\n';

    text += i18n("\nWith message:\n %1", message);

    int res = KMessageBox::warningContinueCancel(0, text, i18n("About to commit to repository"));
    if (res != KMessageBox::Continue) {
        return false;
    }

    emit reviewFinished(message, selection);

    deleteLater();
    
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

#include "vcsdiffpatchsources.moc"
