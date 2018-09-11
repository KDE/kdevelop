/*
 * This file is part of KDevelop
 *
 * Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
