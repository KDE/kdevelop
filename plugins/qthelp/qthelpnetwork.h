/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPNETWORK_H
#define QTHELPNETWORK_H

#include "debug.h"

#include <QNetworkReply>
#include <QTimer>
#include <QHelpEngine>
#include <QMimeDatabase>
#include <QMimeType>

class HelpNetworkReply : public QNetworkReply
{
		Q_OBJECT
	public:
		HelpNetworkReply(const QNetworkRequest &request, const QByteArray &fileData, const QString &mimeType);
		
		void abort() override {}
		qint64 bytesAvailable() const override { return data.length() + QNetworkReply::bytesAvailable(); }
		
	protected:
		qint64 readData(char *data, qint64 maxlen) override;
		
	private:
		QByteArray data;
		qint64 origLen;
};

class HelpNetworkAccessManager : public QNetworkAccessManager
{
		Q_OBJECT
	public:
		explicit HelpNetworkAccessManager(QHelpEngineCore *engine, QObject *parent = nullptr);
        ~HelpNetworkAccessManager() override;

	protected:
		QNetworkReply *createRequest(Operation op,
			const QNetworkRequest &request, QIODevice *outgoingData = nullptr) override;

	private:
		QHelpEngineCore *m_helpEngine;
};

#endif
