/*
    SPDX-FileCopyrightText: 2017 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
