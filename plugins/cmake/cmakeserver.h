/* KDevelop CMake Support
 *
 * Copyright 2017 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKESERVER_H
#define CMAKESERVER_H

#include <util/path.h>
#include <QProcess>
#include <QLocalSocket>
#include "cmakecommonexport.h"

namespace KDevelop { class IProject; }

class KDEVCMAKECOMMON_EXPORT CMakeServer : public QObject
{
Q_OBJECT
public:
    explicit CMakeServer(KDevelop::IProject* project);
    ~CMakeServer() override;

    bool isServerAvailable();
    void sendCommand(const QJsonObject& object);

    void handshake(const KDevelop::Path& source, const KDevelop::Path& build);
    void configure(const QStringList &args);
    void compute();
    void codemodel();

Q_SIGNALS:
    void connected();
    void disconnected();
    void finished(int code);
    void response(const QJsonObject &value);

private:
    void processOutput();
    void emitResponse(const QByteArray &data);
    void setConnected(bool connected);

    QLocalSocket* m_localSocket;
    QByteArray m_buffer;
    QProcess m_process;
    bool m_connected = false;
};

#endif // CMAKESERVER_H
