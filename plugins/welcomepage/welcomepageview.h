/*
    SPDX-FileCopyrightText: 2011 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef WELCOMEPAGEVIEW_H
#define WELCOMEPAGEVIEW_H

#include <QQuickWidget>

namespace Sublime {
class Area;
}

namespace KDevelop {
class IProject;
}

class WelcomePageWidget : public QQuickWidget
{
    Q_OBJECT
public:
    explicit WelcomePageWidget(const QList< KDevelop::IProject* >& projects, QWidget* parent = nullptr);

public Q_SLOTS:
    void areaChanged(Sublime::Area* a);
};

#endif // WELCOMEPAGEVIEW_H
