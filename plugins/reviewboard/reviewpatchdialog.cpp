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
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "ui_reviewpatch.h"
#include "reviewboardjobs.h"
#include <KDebug>

ReviewPatchDialog::ReviewPatchDialog(QWidget* parent)
    : KDialog(parent)
{
    m_ui=new Ui::ReviewPatch;
    QWidget* w= new QWidget(this);
    m_ui->setupUi(w);
    connect(m_ui->repositoriesFilter, SIGNAL(textChanged(QString)), SLOT(filterChanged(QString)));
    setMainWidget(w);

    connect(m_ui->server, SIGNAL(textChanged(QString)), SLOT(serverChanged()));
    enableButtonOk(false);
}

ReviewPatchDialog::~ReviewPatchDialog()
{
    delete m_ui;
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

void ReviewPatchDialog::setRepository(const QString& repo)
{
    m_preferredRepository = repo;
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
    QStandardItemModel* model = new QStandardItemModel(this);
    ReviewBoard::ProjectsListRequest* pl=dynamic_cast<ReviewBoard::ProjectsListRequest*>(job);
    QVariantList repos = pl->repositories();
    foreach(const QVariant& repo, repos) {
        QVariantMap repoMap=repo.toMap();
        QStandardItem *repoItem = new QStandardItem;

        repoItem->setText(repoMap["name"].toString());
        repoItem->setData(repoMap["path"], Qt::UserRole);
        model->appendRow(repoItem);
    }
    
    model->sort(0, Qt::AscendingOrder);
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    m_ui->repositories->setModel(proxy);
    connect(m_ui->repositories->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(repositoryChanged(QItemSelection)));
    
    if(!m_preferredRepository.isEmpty()) {
        QModelIndexList idxs = model->match(model->index(0,0), Qt::UserRole, m_preferredRepository, 1, Qt::MatchExactly);
        if(!idxs.isEmpty()) {
            QModelIndex idx = proxy->mapFromSource(idxs.first());
            m_ui->repositories->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
            m_ui->repositories->scrollTo(idx, QAbstractItemView::PositionAtCenter);
        } else
            kDebug() << "no repository called" << m_preferredRepository;
    }
    m_ui->repositoriesBox->setEnabled(job->error()==0);
}

QString ReviewPatchDialog::repository() const
{
    QModelIndexList selected = m_ui->repositories->selectionModel()->selectedIndexes();
    if(!selected.isEmpty()) {
        QModelIndex idx = selected.first();
        Q_ASSERT(idx.isValid());
        return idx.data(Qt::UserRole).toString();
    }
    return QString();
}

void ReviewPatchDialog::repositoryChanged(const QItemSelection& idx)
{
    enableButtonOk(!idx.isEmpty());
}

void ReviewPatchDialog::filterChanged(const QString& text)
{
    QSortFilterProxyModel* proxy = qobject_cast<QSortFilterProxyModel*>(m_ui->repositories->model());
    proxy->setFilterFixedString(text);
    if(proxy->rowCount()==1) {
        m_ui->repositories->selectionModel()->select(proxy->index(0,0), QItemSelectionModel::ClearAndSelect);
    }
}
