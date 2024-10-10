/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "stashmanagerdialog.h"
#include "ui_stashmanagerdialog.h"
#include "gitplugin.h"
#include "stashpatchsource.h"
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <util/shellutils.h>
#include <vcs/dvcs/dvcsjob.h>

#include <QDialogButtonBox>
#include <QInputDialog>
#include <QPushButton>

#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEditor/Document>

using namespace KDevelop;

StashManagerDialog::StashManagerDialog(const QDir& stashed, GitPlugin* plugin, QWidget* parent)
    : QDialog(parent), m_plugin(plugin), m_dir(stashed)
{
    setWindowTitle(i18nc("@title:window", "Stash Manager"));

    m_ui = new Ui::StashManager;
    m_ui->setupUi(this);

    KDevelop::restoreAndAutoSaveGeometry(*this, QStringLiteral("VCS"), QStringLiteral("StashManagerDialog"));

    auto* m = new StashModel(stashed, plugin, this);
    m_ui->stashView->setModel(m);

    connect(m_ui->show,   &QPushButton::clicked, this, &StashManagerDialog::showStash);
    connect(m_ui->apply,  &QPushButton::clicked, this, &StashManagerDialog::applyClicked);
    connect(m_ui->branch, &QPushButton::clicked, this, &StashManagerDialog::branchClicked);
    connect(m_ui->pop,    &QPushButton::clicked, this, &StashManagerDialog::popClicked);
    connect(m_ui->drop,   &QPushButton::clicked, this, &StashManagerDialog::dropClicked);
    connect(m, &StashModel::rowsInserted, this, &StashManagerDialog::stashesFound);

    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &StashManagerDialog::reject);

    setEnabled(false); //we won't enable it until we have the model with data and selection
}

StashManagerDialog::~StashManagerDialog()
{
    delete m_ui;
}

void StashManagerDialog::stashesFound()
{
    QModelIndex firstIdx=m_ui->stashView->model()->index(0, 0);
    m_ui->stashView->setCurrentIndex(firstIdx);
    setEnabled(true);
}

QString StashManagerDialog::selection() const
{
    QModelIndex idx = m_ui->stashView->currentIndex();
    Q_ASSERT(idx.isValid());
    return idx.data(StashModel::RefRole).toString();
}

void StashManagerDialog::runStash(const QStringList& arguments)
{
    VcsJob* job = m_plugin->gitStash(m_dir, arguments, OutputJob::Verbose);
    connect(job, &VcsJob::result, this, &StashManagerDialog::accept);

    setEnabled(false);

    ICore::self()->runController()->registerJob(job);
}

void StashManagerDialog::showStash()
{
    IPatchSource::Ptr stashPatch(new StashPatchSource(selection(), m_plugin, m_dir));

    if (auto * review = ICore::self()->pluginController()->extensionForPlugin<IPatchReview>()) {
        review->startReview(stashPatch);
    } else {
        auto* docCtrl = ICore::self()->documentController();
        connect(stashPatch, &StashPatchSource::patchChanged, docCtrl, [=] {
            auto* doc = docCtrl->openDocument(
                stashPatch->file(),
                KTextEditor::Range::invalid(),
                IDocumentController::DoNotAddToRecentOpen
            );
            doc->setPrettyName(stashPatch->name());
            doc->textDocument()->setReadWrite(false);
            doc->textDocument()->setMode(QStringLiteral("diff"));
            doc->textDocument()->setHighlightingMode(QStringLiteral("diff"));
            docCtrl->activateDocument(doc);
            connect(ICore::self(), &ICore::aboutToShutdown, docCtrl, [=] {doc->close();});
        });
    }
    accept();
}

void StashManagerDialog::applyClicked()
{
    runStash(QStringList{QStringLiteral("apply"), selection()});
}

void StashManagerDialog::popClicked()
{
    runStash(QStringList{QStringLiteral("pop"), selection()});
}

void StashManagerDialog::dropClicked()
{
    QString sel = selection();
    int ret = KMessageBox::questionTwoActions(this, i18n("Are you sure you want to drop the stash '%1'?", sel), {},
                                              KGuiItem(i18nc("@action:button", "Drop"), QStringLiteral("edit-delete")),
                                              KStandardGuiItem::cancel());

    if (ret == KMessageBox::PrimaryAction)
        runStash(QStringList{QStringLiteral("drop"), sel});
}

void StashManagerDialog::branchClicked()
{
    QString branchName = QInputDialog::getText(this, i18nc("@title:window", "Git Stash"), i18nc("@label:textbox", "Name for the new branch:"));

    if(!branchName.isEmpty())
        runStash(QStringList{QStringLiteral("branch"), branchName, selection()});
}

//////////////////StashModel

StashModel::StashModel(const QDir& dir, GitPlugin* git, QObject* parent)
    : QStandardItemModel(parent)
{
    auto job = git->stashList(dir, OutputJob::Silent);
    connect(job, &VcsJob::finished, this, &StashModel::stashListReady);
    ICore::self()->runController()->registerJob(job);
}

void StashModel::stashListReady(KJob* _job)
{
    auto* job = qobject_cast<VcsJob*>(_job);

    for(const auto& s_item: job->fetchResults().value<QList<GitPlugin::StashItem>>()) {
        const QString itemValue = i18nc("%1: stack depth, %2: branch, %3: parent description", "%1. %2: %3",
                                        QString::number(s_item.stackDepth), s_item.branch, s_item.parentDescription);
        auto* item = new QStandardItem(itemValue);
        item->setData(s_item.shortRef, StashModel::RefRole);
        item->setData(s_item.parentSHA, StashModel::CommitHashRole);
        item->setData(s_item.parentDescription, StashModel::CommitDescRole);
        item->setData(s_item.branch, StashModel::BranchRole);
        item->setData(s_item.message, StashModel::MessageRole);
        item->setData(s_item.creationTime, StashModel::DateRole);
        item->setData(
            i18n("%1 created on %2", s_item.branch, s_item.creationTime.toString()),
            Qt::ToolTipRole
        );
        appendRow(item);
    };
}

#include "moc_stashmanagerdialog.cpp"
