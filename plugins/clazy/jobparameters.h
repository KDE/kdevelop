/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_JOB_PARAMETERS_H
#define KDEVCLAZY_JOB_PARAMETERS_H

#include <QObject>
#include <QString>
#include <QUrl>

namespace KDevelop { class IProject; }

namespace Clazy
{

class JobGlobalParameters : public QObject
{
    Q_OBJECT

public:
    JobGlobalParameters(const QUrl& executablePath, const QUrl& docsPath);
    ~JobGlobalParameters() override = default;

    static QUrl defaultExecutablePath();
    static QUrl defaultDocsPath();

    bool isValid() const;
    QString error() const;

Q_SIGNALS:
    void changed();

protected:
    JobGlobalParameters();

    QString m_executablePath;
    QString m_docsPath;

    QString m_error;
};

}

#endif
