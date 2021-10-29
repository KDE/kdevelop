/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "unitylauncher.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QVariantMap>

using namespace KDevelop;

UnityLauncher::UnityLauncher(QObject *parent) : QObject(parent)
{

}

UnityLauncher::~UnityLauncher() = default;

QString UnityLauncher::launcherId() const
{
    return m_launcherId;
}

void UnityLauncher::setLauncherId(const QString &launcherId)
{
    m_launcherId = launcherId;
}

bool UnityLauncher::progressVisible() const
{
    return m_progressVisible;
}

void UnityLauncher::setProgressVisible(bool progressVisible)
{
    if (m_progressVisible != progressVisible) {
        m_progressVisible = progressVisible;

        update({ {QStringLiteral("progress-visible"), progressVisible} });
    }
}

int UnityLauncher::progress() const
{
    return m_progress;
}

void UnityLauncher::setProgress(int progress)
{
    if (m_progress != progress) {
        m_progress = progress;

        update({ {QStringLiteral("progress"), progress / 100.0} });
    }
}

void UnityLauncher::update(const QVariantMap &properties)
{
    if (m_launcherId.isEmpty()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/org/kdevelop/UnityLauncher"),
                                                      QStringLiteral("com.canonical.Unity.LauncherEntry"),
                                                      QStringLiteral("Update"));
    message.setArguments({m_launcherId, properties});
    QDBusConnection::sessionBus().send(message);
}
