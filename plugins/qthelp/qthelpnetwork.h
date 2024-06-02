/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPNETWORK_H
#define QTHELPNETWORK_H

#include <QNetworkReply>
#include <QBuffer>

class QHelpEngineCore;

class HelpNetworkReply : public QNetworkReply
{
    Q_OBJECT
public:
    HelpNetworkReply(const QNetworkRequest& request, const QByteArray& fileData, const QString& mimeType);

    void abort() override
    {
        close();
    }

    qint64 bytesAvailable() const override
    {
        return m_buffer.bytesAvailable() + QNetworkReply::bytesAvailable();
    }

    bool isSequential() const override
    {
        return true;
    }

    qint64 size() const override
    {
        return m_buffer.size();
    }

protected:
    qint64 readData(char* data, qint64 maxlen) override;

private:
    QByteArray m_data;
    QBuffer m_buffer;
};

class HelpNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit HelpNetworkAccessManager(QHelpEngineCore* engine, QObject* parent = nullptr);
    ~HelpNetworkAccessManager() override;

protected:
    QNetworkReply* createRequest(Operation op, const QNetworkRequest& request,
                                 QIODevice* outgoingData = nullptr) override;

private:
    QHelpEngineCore* m_helpEngine;
};

#endif
