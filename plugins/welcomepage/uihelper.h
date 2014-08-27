/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#ifndef UIHELPER_H
#define UIHELPER_H

#include <QObject>
#include <QStringList>

class QUrl;
class QAction;
class UiHelper : public QObject
{
    Q_OBJECT
    public:
        UiHelper(QObject* parent);
        
    public slots:
        QAction* retrieveMenuAction(const QString& name);
        void showMenu(const QString& name);
        void setArea(const QString& name);
        void raiseToolView(const QString& id);
};

#endif // UIHELPER_H

