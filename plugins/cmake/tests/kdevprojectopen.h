/*
    SPDX-FileCopyrightText: 2013 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPROJECTOPEN_H
#define KDEVPROJECTOPEN_H

#include <QObject>
#include <QUrl>

namespace KDevelop { class IProject; }

class KDevProjectOpen : public QObject
{
    Q_OBJECT

public:
    explicit KDevProjectOpen(QObject* parent = nullptr);
    void openProject(const QUrl& path);
    void cleanup();
private Q_SLOTS:
    void projectDone(KDevelop::IProject*);

private:
    int m_toOpen = 0;
};

#endif
