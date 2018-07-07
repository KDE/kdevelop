/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
