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

#include "qthelpnetwork.h"

HelpNetworkAccessManager::HelpNetworkAccessManager(QHelpEngineCore* engine, QObject* parent)
    : QNetworkAccessManager(parent), m_helpEngine(engine)
{}

HelpNetworkAccessManager::~HelpNetworkAccessManager()
{
}

HelpNetworkReply::HelpNetworkReply(const QNetworkRequest &request, const QByteArray &fileData, const QString &mimeType)
    : data(fileData), origLen(fileData.length())
{
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);

    // Instantly finish processing if data is empty. Without this code the loadFinished()
    // signal will never be emitted by the corresponding QWebView.
    if (!origLen) {
        qCDebug(QTHELP) << "Empty data for" << request.url().toDisplayString();
        QTimer::singleShot(0, this, &QNetworkReply::finished);
    }

    // Fix broken CSS images (tested on Qt 5.5.1 and 5.7.0)
    if (request.url().fileName() == QLatin1String("offline.css")) {
        data.replace("../images", "images");
    }
    // Fix flickering when loading, the page has the offline-simple.css stylesheet which is replaced
    // later by offline.css  by javascript which causes flickering so we force the full stylesheet
    // from the beginning
    if (request.url().fileName().endsWith(QLatin1String(".html"))) {
        data.replace("offline-simple.css", "offline.css");
    }

    setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(origLen));
    QTimer::singleShot(0, this, &QNetworkReply::metaDataChanged);
    QTimer::singleShot(0, this, &QIODevice::readyRead);
}

qint64 HelpNetworkReply::readData(char *buffer, qint64 maxlen)
{
	qint64 len = qMin(qint64(data.length()), maxlen);
	if (len) {
		memcpy(buffer, data.constData(), len);
		data.remove(0, len);
	}
	if (!data.length())
		QTimer::singleShot(0, this, &QNetworkReply::finished);
	return len;
}

QNetworkReply *HelpNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
	QString scheme = request.url().scheme();
	if (scheme == QLatin1String("qthelp") || scheme == QLatin1String("about")) {
		QString mimeType = QMimeDatabase().mimeTypeForUrl(request.url()).name();
		if (mimeType == QLatin1String("application/x-extension-html")) {
			// see also: https://bugs.kde.org/show_bug.cgi?id=288277
			// firefox seems to add this bullshit mimetype above
			// which breaks displaying of qthelp documentation :(
			mimeType = QStringLiteral("text/html");
		}
		return new HelpNetworkReply(request, m_helpEngine->fileData(request.url()), mimeType);
	}
	return QNetworkAccessManager::createRequest(op, request, outgoingData);
}
