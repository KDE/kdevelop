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
#include "ui_reviewpatch.h"
#include <KIO/Job>

ReviewPatchDialog::ReviewPatchDialog(QWidget* parent)
    : KDialog(parent)
{
    m_ui=new Ui::ReviewPatch;
    QWidget* w= new QWidget(this);
    m_ui->setupUi(w);
    setMainWidget(w);
}

void ReviewPatchDialog::setRepository(const QString& repo)
{
    m_ui->repository->setText(repo);
}

void ReviewPatchDialog::setServer(const KUrl& server)
{
    m_ui->server->setUrl(server);
}

void ReviewPatchDialog::setUsername(const QString& user)
{
    m_ui->username->setText(user);
}

void ReviewPatchDialog::setPatch(const KUrl& file)
{
    m_patch = file;
}

struct Service
{
    Service(const QString& username, const QString& password, const KUrl& server)
        : username(username), password(password), server(server)
    {}
    
    QString username;
    QString password;
    KUrl server;
};

void postReview(const Service& s, const QString& repository_path, const KUrl& patch)
{
    KUrl url=s.server;
    url.addPath("api/json/reviewrequests/new/");
    KJob* job = KIO::http_post(url, "repository_path="+repository_path.toLatin1()+"&submit_as="+s.username.toLatin1());
}