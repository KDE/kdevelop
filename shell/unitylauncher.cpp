/*
 * Copyright 2016 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
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
