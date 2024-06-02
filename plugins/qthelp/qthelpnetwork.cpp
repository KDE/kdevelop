/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpnetwork.h"

#include <QHelpEngineCore>
#include <QMimeDatabase>

HelpNetworkAccessManager::HelpNetworkAccessManager(QHelpEngineCore* engine, QObject* parent)
    : QNetworkAccessManager(parent), m_helpEngine(engine)
{}

HelpNetworkAccessManager::~HelpNetworkAccessManager()
{
}

HelpNetworkReply::HelpNetworkReply(const QNetworkRequest& request, const QByteArray& fileData, const QString& mimeType)
    : m_data(fileData)
{
    setRequest(request);
    setUrl(request.url());
    setOperation(QNetworkAccessManager::GetOperation);
    setFinished(true);
    QNetworkReply::open(QIODevice::ReadOnly);

#ifdef USE_QTWEBKIT
    // Fix broken CSS images (tested on Qt 5.5.1, 5.7.0, 5.9.7 and 5.14.1)
    if (request.url().fileName() == QLatin1String("offline.css")) {
        data.replace("../images", "images");
    }
#endif

    // Fix flickering when loading, the page has the offline-simple.css stylesheet which is replaced
    // later by offline.css  by javascript which causes flickering so we force the full stylesheet
    // from the beginning
    if (request.url().fileName().endsWith(QLatin1String(".html"))) {
        m_data.replace("offline-simple.css", "offline.css");
    }

    setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    const auto contentLength = m_data.size();
    setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(contentLength));

    m_buffer.setData(m_data);
    m_buffer.open(QIODevice::ReadOnly);

    QMetaObject::invokeMethod(
        this,
        [this, contentLength]() {
            emit metaDataChanged();
            emit downloadProgress(contentLength, contentLength);
            emit readyRead();
            emit finished();
        },
        Qt::QueuedConnection);
}

qint64 HelpNetworkReply::readData(char *buffer, qint64 maxlen)
{
    return m_buffer.read(buffer, maxlen);
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

#include "moc_qthelpnetwork.cpp"
