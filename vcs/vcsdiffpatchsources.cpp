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
#include <klocalizedstring.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <kmessagebox.h>
#include <interfaces/iruncontroller.h>
#include "vcsjob.h"
#include <interfaces/iplugincontroller.h>

using namespace KDevelop;

///@todo The subversion library returns borked diffs, where the headers are at the end. This function
///           takes those headers, and moves them into the correct place to create a valid working diff.
///           Find the source of this problem.
QString repairDiff(QString diff) {
    kDebug() << "diff before repair:" << diff;
    QStringList lines = diff.split('\n');
    QMap<QString, QString> headers;
    for(int a = 0; a < lines.size()-1; ++a) {
        if(lines[a].startsWith("Index: ") && lines[a+1].startsWith("=====")) {
            QString fileName = lines[a].mid(strlen("Index: ")).trimmed();
            headers[fileName] = lines[a];
            kDebug() << "found header for" << fileName;
            lines[a] = QString();
            if(lines[a+1].startsWith("======")) {
                headers[fileName] += "\n" + lines[a+1];
            lines[a+1] = QString();
            }
        }
    }
    
    QRegExp spaceRegExp("\\s");
    
    for(int a = 0; a < lines.size()-1; ++a) {
        if(lines[a].startsWith("--- ")) {
            QString tail = lines[a].mid(strlen("--- "));
            if(tail.indexOf(spaceRegExp) != -1) {
                QString file = tail.left(tail.indexOf(spaceRegExp));
                kDebug() << "checking for" << file;
                if(headers.contains(file)) {
                    kDebug() << "adding header for" << file << ":" << headers[file];
                    lines[a] = headers[file] + "\n" + lines[a];
                }
            }
        }
    }
    QString ret = lines.join("\n");
    kDebug() << "repaired diff:" << ret;
    return ret;
}

VCSCommitDiffPatchSource::VCSCommitDiffPatchSource(const QString& d, QMap< KUrl, QString > selectable, IBasicVersionControl* vcs) : VCSDiffPatchSource(d), m_selectable(selectable), m_vcs(vcs) {
    
    Q_ASSERT(m_vcs);

    m_commitMessageWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_commitMessageWidget);

    m_commitMessageEdit = new QTextEdit;

    layout->addWidget(new QLabel(i18n("Commit Message:")));
    layout->addWidget(m_commitMessageEdit);
}

VCSDiffPatchSource::VCSDiffPatchSource(const QString& diff) {
    KTemporaryFile temp2;
    temp2.setSuffix("2.patch");
    temp2.setAutoRemove(false);
    temp2.open();
    QTextStream t2(&temp2);
    t2 << diff;
    kDebug() << "filename:" << temp2.fileName();
    m_file = KUrl(temp2.fileName());
    temp2.close();

    kDebug() << "using file" << m_file;

    m_name = "VCS Diff";
    m_base = KUrl("/");
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

QMap< KUrl, QString > VCSCommitDiffPatchSource::additionalSelectableFiles() const {
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
    deleteLater();
}

bool VCSCommitDiffPatchSource::finishReview(QList< KUrl > selection) {

    QString message;

    if (m_commitMessageEdit)
        message = m_commitMessageEdit->toPlainText();

    kDebug() << "Finishing with selection" << selection;
    QString text = i18n("Files will be committed:") + "\n";
    foreach(KUrl url, selection)
        text += ICore::self()->projectController()->prettyFileName(url, KDevelop::IProjectController::FormatPlain) + "\n";

    text += "\n" + i18n("With message:") + "\n" + message;

    int res = KMessageBox::warningContinueCancel(0, text, i18n("About to commit to repository"));
    if (res != KMessageBox::Continue) {
        return false;
    }

    emit reviewFinished(message, selection);

    return true;
}

static KDevelop::IPatchSource::Ptr currentShownDiff;

bool showVcsDiff(IPatchSource* vcsDiff)
{
    KDevelop::IPatchReview* patchReview = ICore::self()->pluginController()->extensionForPlugin<IPatchReview>("org.kdevelop.IPatchReview");

    //Only give one VCS diff at a time to the patch review plugin
    if(currentShownDiff)
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
