#ifndef QTHELPNETWORK_H
#define QTHELPNETWORK_H

#include <QNetworkReply>
#include <QTimer>
#include <QHelpEngine>
#include <kmimetype.h>

class HelpNetworkReply : public QNetworkReply
{
	public:
		HelpNetworkReply(const QNetworkRequest &request, const QByteArray &fileData, const QString &mimeType);
		
		virtual void abort() {}
		virtual qint64 bytesAvailable() const { return data.length() + QNetworkReply::bytesAvailable(); }
		
	protected:
		virtual qint64 readData(char *data, qint64 maxlen);
		
	private:
		QByteArray data;
		qint64 origLen;
};

HelpNetworkReply::HelpNetworkReply(const QNetworkRequest &request,
        const QByteArray &fileData, const QString &mimeType)
    : data(fileData), origLen(fileData.length())
{
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);

    setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(origLen));
    QTimer::singleShot(0, this, SIGNAL(metaDataChanged()));
    QTimer::singleShot(0, this, SIGNAL(readyRead()));
}

qint64 HelpNetworkReply::readData(char *buffer, qint64 maxlen)
{
	qint64 len = qMin(qint64(data.length()), maxlen);
	if (len) {
		qMemCopy(buffer, data.constData(), len);
		data.remove(0, len);
	}
	if (!data.length())
		QTimer::singleShot(0, this, SIGNAL(finished()));
	return len;
}

class HelpNetworkAccessManager : public QNetworkAccessManager
{
	public:
		HelpNetworkAccessManager(QHelpEngineCore *engine, QObject *parent)
			: QNetworkAccessManager(parent), m_helpEngine(engine) {}

	protected:
		virtual QNetworkReply *createRequest(Operation op,
			const QNetworkRequest &request, QIODevice *outgoingData = 0);

	private:
		QHelpEngineCore *m_helpEngine;
};

QNetworkReply *HelpNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
	QString scheme = request.url().scheme();
	if (scheme == QLatin1String("qthelp") || scheme == QLatin1String("about")) {
		return new HelpNetworkReply(request, m_helpEngine->fileData(request.url()), KMimeType::findByUrl(request.url())->name());
	}
	return QNetworkAccessManager::createRequest(op, request, outgoingData);
}

#endif
