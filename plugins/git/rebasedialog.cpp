/*
    SPDX-FileCopyrightText: 2019 José Millán Soto <jmillan@kde-espana.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rebasedialog.h"
#include "gitplugin.h"
#include "ui_rebasedialog.h"

#include <vcs/models/brancheslistmodel.h>
#include <vcs/dvcs/dvcsjob.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <QDir>
#include <QUrl>
#include <QComboBox>
#include <KLocalizedString>

using namespace KDevelop;

RebaseDialog::RebaseDialog(GitPlugin* plugin, const QUrl &repository, QWidget* parent)
    : QDialog(parent), m_plugin(plugin), m_repository(repository)
{
    setWindowTitle(i18nc("@title:window", "Branch Selection"));

    m_ui = new Ui::RebaseDialog();
    m_ui->setupUi(this);

    m_model = new BranchesListModel(this);
    m_model->initialize(plugin, repository);
    m_ui->branches->setModel(m_model);
    connect(m_ui->rebaseButton, &QPushButton::clicked, this, &RebaseDialog::performRebase);
}

RebaseDialog::~RebaseDialog()
{
    delete m_ui;
}

void RebaseDialog::performRebase()
{
    QString selectedBranch = m_ui->branches->currentText();
    VcsJob *job = m_plugin->rebase(m_repository, selectedBranch);
    ICore::self()->runController()->registerJob(job);
    accept();
}
