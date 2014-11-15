/*
 * This file is part of KDevelop
 * Copyright 2010-2012 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#ifndef KDEVPLATFORM_PLUGIN_REVIEWPATCHDIALOG_H
#define KDEVPLATFORM_PLUGIN_REVIEWPATCHDIALOG_H

#include <QDialog>
#include <QUrl>

class QItemSelection;
class QModelIndex;

class KJob;

namespace Ui {
    class ReviewPatch;
}

class ReviewPatchDialog : public QDialog
{
    Q_OBJECT
    public:
        ReviewPatchDialog(const QUrl& dirUrl, QWidget* parent = 0);
        virtual ~ReviewPatchDialog();

        void setBaseDir(const QString& dir);
        void setServer(const QUrl& server);
        void setUsername(const QString& user);

        /** @returns the server url with the username and password */
        QUrl server() const;

        /** @returns the selected base directory for the patch */
        QString baseDir() const;
        QString repository() const;
        QString username() const;

        void setRepository(const QString& repo);

        QString review() const;

        bool isUpdateReview();

    private slots:
        void serverChanged();
        void receivedProjects(KJob* job);

    private:
        void initializeFromRC(const QString& filePath);

        Ui::ReviewPatch* m_ui;
        QString m_preferredRepository;
        QMultiHash<QString, QPair<QString, QVariant> > m_reviews;

    private slots:
        void repositoryChanged(int index);
        void receivedReviews(KJob* job);
        void reviewCheckboxChanged(int status);
        void updateReviews();
        void updateReviewsList();
};

#endif
