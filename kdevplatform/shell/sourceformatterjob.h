/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SOURCEFORMATTERJOB_H
#define KDEVPLATFORM_SOURCEFORMATTERJOB_H

#include <QList>
#include <QUrl>

#include <KJob>

#include <interfaces/istatus.h>


namespace KDevelop
{
class SourceFormatterController;


class SourceFormatterJob : public KJob, public IStatus
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IStatus )

public:
    explicit SourceFormatterJob(SourceFormatterController* sourceFormatterController);

public: // KJob API
    void start() override;

public: // KDevelop::IStatus API
    QString statusName() const override;

public:
    void setFiles(const QList<QUrl>& fileList);

protected: // KJob API
    bool doKill() override;

Q_SIGNALS: // KDevelop::IStatus API
    void clearMessage(KDevelop::IStatus* status) override;
    void showMessage(KDevelop::IStatus* status, const QString& message, int timeout = 0) override;
    void showErrorMessage(const QString& message, int timeout = 0) override;
    void hideProgress(KDevelop::IStatus* status) override;
    void showProgress(KDevelop::IStatus* status, int minimum, int maximum, int value) override;

private:
    Q_INVOKABLE void doWork();

    void formatFile(const QUrl& url);

private:
    SourceFormatterController* const m_sourceFormatterController;

    enum {
        WorkIdle,
        WorkFormat,
        WorkCancelled
    } m_workState;

    QList<QUrl> m_fileList;
    int m_fileIndex;
};

}

#endif
