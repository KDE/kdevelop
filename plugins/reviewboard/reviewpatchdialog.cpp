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
#include <Qt>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <KDebug>
#include <KLocalizedString>
#include "ui_reviewpatch.h"
#include "reviewboardjobs.h"

ReviewPatchDialog::ReviewPatchDialog(const QUrl& dirUrl, QWidget* parent)
    : KDialog(parent)
{
    m_ui = new Ui::ReviewPatch;
    QWidget* w = new QWidget(this);
    m_ui->setupUi(w);
    setMainWidget(w);

    connect(m_ui->server, SIGNAL(textChanged(QString)), SLOT(serverChanged()));
    connect(m_ui->reviewCheckbox, SIGNAL(stateChanged(int)), SLOT(reviewCheckboxChanged(int)));
    enableButtonOk(false);

    if (dirUrl.isLocalFile()) {
        QDir d(dirUrl.toLocalFile());
        while(!QFile::exists(d.filePath(".reviewboardrc"))) {
            if(!d.cdUp())
                break;
        }
        if(!d.isRoot())
            initializeFromRC(d.filePath(".reviewboardrc"));
    }
}

ReviewPatchDialog::~ReviewPatchDialog()
{
    delete m_ui;
}

void ReviewPatchDialog::setBaseDir(const QString& repo)
{
    m_ui->basedir->setText(repo);
}

void ReviewPatchDialog::setServer(const QUrl& server)
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

QUrl ReviewPatchDialog::server() const
{
    QUrl server=m_ui->server->url();
    server.setUserName(m_ui->username->text());
    server.setPassword(m_ui->password->text());
    return server;
}

QString ReviewPatchDialog::username() const
{
    return m_ui->username->text();
}

void ReviewPatchDialog::serverChanged()
{
    m_ui->repositories->clear();
    //TODO reviewboards with private repositories don't work. Use user/pass if set.
    ReviewBoard::ProjectsListRequest* repo = new ReviewBoard::ProjectsListRequest(m_ui->server->url(), this);
    connect(repo, SIGNAL(finished(KJob*)), SLOT(receivedProjects(KJob*)));
    repo->start();
}

void ReviewPatchDialog::receivedProjects(KJob* job)
{
    // TODO:  check error
    ReviewBoard::ProjectsListRequest* pl=dynamic_cast<ReviewBoard::ProjectsListRequest*>(job);
    QVariantList repos = pl->repositories();
    // Add default value with no repo selected.
    m_ui->repositories->addItem(i18n("Repository not selected"), 0);

    foreach(const QVariant& repo, repos) {
        QVariantMap repoMap=repo.toMap();
        m_ui->repositories->addItem(repoMap["name"].toString(), repoMap["path"]);
    }

    connect(m_ui->repositories, SIGNAL(currentIndexChanged(int)), SLOT(repositoryChanged(int)));

    QAbstractItemModel* model = m_ui->repositories->model();
    if(!m_preferredRepository.isEmpty()) {
        QModelIndexList idxs = model->match(model->index(0,0), Qt::UserRole, m_preferredRepository, 1, Qt::MatchExactly);
        if(idxs.isEmpty()) {
            idxs = model->match(model->index(0,0), Qt::DisplayRole, QUrl::fromUserInput(m_preferredRepository).fileName(), 1, Qt::MatchExactly);
        }
        if(!idxs.isEmpty()) {
            m_ui->repositories->setCurrentIndex(idxs.first().row());
        } else
            qDebug() << "couldn't fucking find it" << m_preferredRepository;
    }
    m_ui->repositoriesBox->setEnabled(job->error()==0);
}

QString ReviewPatchDialog::repository() const
{
    QComboBox* repositories = m_ui->repositories;
    if(repositories->currentIndex() != -1) {
        return repositories->itemData(repositories->currentIndex(), Qt::UserRole).toString();
    }
    return QString();
}

void ReviewPatchDialog::repositoryChanged(int index)
{
    enableButtonOk((!isUpdateReview() && index > 0) || m_ui->reviews->currentIndex() != -1);
}

void ReviewPatchDialog::reviewCheckboxChanged(int status)
{
    if (status == Qt::Checked) {
        m_ui->reviews->setEnabled(true);
        connect(m_ui->username, SIGNAL(editingFinished()), SLOT(updateReviews()));
        connect(m_ui->password, SIGNAL(editingFinished()), SLOT(updateReviews()));
        connect(m_ui->server, SIGNAL(returnPressed()), SLOT(updateReviews()));
        connect(m_ui->repositories, SIGNAL(currentIndexChanged(int)), SLOT(updateReviewsList()));
    } else {
        m_ui->reviews->setEnabled(false);
        disconnect(m_ui->username, SIGNAL(editingFinished()), this, SLOT(updateReviews()));
        disconnect(m_ui->password, SIGNAL(editingFinished()), this, SLOT(updateReviews()));
        disconnect(m_ui->server, SIGNAL(returnPressed()), this, SLOT(updateReviews()));
        disconnect(m_ui->repositories, SIGNAL(currentIndexChanged(int)), this, SLOT(updateReviewsList()));
    }
    updateReviews();
}

void ReviewPatchDialog::receivedReviews(KJob* job)
{
    m_reviews.clear();
    // TODO: check errors
    QVariantList reviews = dynamic_cast<ReviewBoard::ReviewListRequest*>(job)->reviews();
    foreach(const QVariant& review, reviews) {
        QVariantMap reviewMap = review.toMap();
        QVariantMap repoMap = reviewMap["links"].toMap()["repository"].toMap();
        m_reviews.insert(repoMap["title"].toString(), qMakePair<QString, QVariant>(reviewMap["summary"].toString(), reviewMap["id"]));
    }

    updateReviewsList();
}

QString ReviewPatchDialog::review() const
{
    return m_ui->reviews->itemData(m_ui->reviews->currentIndex(), Qt::UserRole).toString();
}

void ReviewPatchDialog::updateReviews()
{
    if (isUpdateReview()) {
        //TODO: reviewboards with private reviews don't work. Use user/pass if set.
        if (!m_ui->server->text().isEmpty() && !m_ui->username->text().isEmpty()) {
            ReviewBoard::ReviewListRequest* repo = new ReviewBoard::ReviewListRequest(m_ui->server->url(), username(), "pending", this);
            connect(repo, SIGNAL(finished(KJob*)), SLOT(receivedReviews(KJob*)));
            repo->start();
        }
    } else {
        // Clear reviews combobox and enable OK Button if a repository is selected.
        enableButtonOk(m_ui->repositories->currentIndex() != -1);
    }
}

bool ReviewPatchDialog::isUpdateReview()
{
    return m_ui->reviewCheckbox->checkState() == Qt::Checked;
}

void ReviewPatchDialog::updateReviewsList()
{
    QString repo = m_ui->repositories->currentText();
    QPair<QString, QVariant> kv;
    m_ui->reviews->clear();

    if (m_ui->repositories->currentIndex() < 1) {
        // Show all Review
        foreach (const QString& key, m_reviews.uniqueKeys()) {
            foreach (kv, m_reviews.values(key)) {
                 m_ui->reviews->addItem(kv.first, kv.second);
            }
        }
    } else {
        // Filter using actual repository.
        foreach (kv, m_reviews.values(repo)) {
            m_ui->reviews->addItem(kv.first, kv.second);
        }
    }

    enableButtonOk(m_ui->reviews->currentIndex() != -1);
}

void ReviewPatchDialog::initializeFromRC(const QString& filePath)
{
    //The .reviewboardrc files are python files, we'll read and if it doesn't work
    //Well bad luck. See: http://www.reviewboard.org/docs/rbtools/dev/rbt/configuration/

    QRegExp rx("([\\w_]+) *= *[\"'](.*)[\"']");
    QFile f(filePath);
    if(!f.open(QFile::ReadOnly | QFile::Text))
        return;

    QHash<QString, QString> values;
    QTextStream stream(&f);
    for(; !stream.atEnd(); ) {
        if(rx.exactMatch(stream.readLine())) {
            values.insert(rx.cap(1), rx.cap(2));
        }
    }

    if(values.contains("REVIEWBOARD_URL"))
        setServer(QUrl(values["REVIEWBOARD_URL"]));
    if(values.contains("REPOSITORY"))
        setRepository(values["REPOSITORY"]);
    kDebug() << "found:" << values;
}
