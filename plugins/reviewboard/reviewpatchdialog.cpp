/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "reviewpatchdialog.h"
#include <QDebug>
#include "ui_reviewpatch.h"
#include "reviewboardjobs.h"

ReviewPatchDialog::ReviewPatchDialog(QWidget* parent)
    : KDialog(parent)
{
    m_ui=new Ui::ReviewPatch;
    QWidget* w= new QWidget(this);
    m_ui->setupUi(w);
    m_ui->repositories->setInsertPolicy(QComboBox::InsertAlphabetically);
    setMainWidget(w);
    
    connect(m_ui->server, SIGNAL(textChanged(QString)), SLOT(serverChanged()));
}

void ReviewPatchDialog::setBaseDir(const QString& repo)
{
    m_ui->basedir->setText(repo);
}

void ReviewPatchDialog::setServer(const KUrl& server)
{
    m_ui->server->setUrl(server);
}

void ReviewPatchDialog::setUsername(const QString& user)
{
    m_ui->username->setText(user);
}

QString ReviewPatchDialog::baseDir() const
{
    return m_ui->basedir->text();
}

KUrl ReviewPatchDialog::server() const
{
    KUrl server=m_ui->server->url();
    server.setUser(m_ui->username->text());
    server.setPassword(m_ui->password->text());
    return server;
}

void ReviewPatchDialog::serverChanged()
{
    ReviewBoard::ProjectsListRequest* repo = new ReviewBoard::ProjectsListRequest(m_ui->server->url(), this);
    connect(repo, SIGNAL(finished(KJob*)), SLOT(receivedProjects(KJob*)));
    repo->start();
}

void ReviewPatchDialog::receivedProjects(KJob* job)
{
    ReviewBoard::ProjectsListRequest* pl=dynamic_cast<ReviewBoard::ProjectsListRequest*>(job);
    QVariantList repos = pl->repositories();
    foreach(const QVariant& repo, repos) {
        QVariantMap repoMap=repo.toMap();
        m_ui->repositories->addItem(repoMap["name"].toString(), repoMap["path"].toString());
    }
    
    m_ui->repositoriesBox->setEnabled(job->error()==0);
}

QString ReviewPatchDialog::repository() const
{
    Q_ASSERT(m_ui->repositories->currentIndex()>=0);
    return m_ui->repositories->itemData(m_ui->repositories->currentIndex(),Qt::UserRole).toString();
}
