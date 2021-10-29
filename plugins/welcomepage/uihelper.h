/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef UIHELPER_H
#define UIHELPER_H

#include <QObject>

class QString;
class QAction;
class UiHelper : public QObject
{
    Q_OBJECT
    public:
        explicit UiHelper(QObject* parent);
        
    public Q_SLOTS:
        QAction* retrieveMenuAction(const QString& name);
        void showMenu(const QString& name);
        void setArea(const QString& name);
        void raiseToolView(const QString& id);
};

#endif // UIHELPER_H

