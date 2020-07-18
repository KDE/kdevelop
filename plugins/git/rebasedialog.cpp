/*
 * This file is part of KDevelop
 * Copyright 2019 José Millán Soto <jmillan@kde-espana.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
