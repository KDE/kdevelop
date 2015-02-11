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

#ifndef KDEPROVIDERWIDGET_H
#define KDEPROVIDERWIDGET_H

#include <interfaces/iprojectprovider.h>

class QModelIndex;
class KConfigDialog;
class KFilterProxySearchLine;
class QListView;
class KDEProviderWidget : public KDevelop::IProjectProviderWidget
{
    Q_OBJECT
    public:
        KDEProviderWidget(QWidget* parent = 0);
        
        virtual KDevelop::VcsJob* createWorkingCopy(const QUrl &destinationDirectory) override;
        virtual bool isCorrect() const override;
        
    private slots:
        void showSettings();
        void projectIndexChanged(const QModelIndex& currentIndex);
        
    private:
        QListView* m_projects;
};

#endif // KDEPROVIDERWIDGET_H
