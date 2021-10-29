/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_CHECKS_DB_H
#define KDEVCLAZY_CHECKS_DB_H

#include <QMap>
#include <QUrl>

namespace Clazy
{

struct Level;

struct Check
{
    const Level* level = nullptr;
    QString name;
    QString description;
};

struct Level
{
    QString name;
    QString displayName;
    QString description;

    QMap<QString, Check*> checks;
};

class ChecksDB
{
public:
    explicit ChecksDB(const QUrl& docsPath);
    ~ChecksDB();

public:
    bool isValid() const;
    QString error() const;

    const QMap<QString, Level*>& levels() const;

    const QMap<QString, Check*>& checks() const;

    static QString defaultChecks();

private:
    Q_DISABLE_COPY(ChecksDB)

    QString m_error;

    QMap<QString, Check*> m_checks;
    QMap<QString, Level*> m_levels;
};

}

#endif
