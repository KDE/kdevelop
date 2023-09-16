/*
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Oswald Buddenhagen <ossi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _PROCESSLINEMAKER_H_
#define _PROCESSLINEMAKER_H_

#include <QObject>
#include "utilexport.h"

/**
@file processlinemaker.h
Utility objects for process output views.
*/

class QProcess;

#include <QStringList>

/**
Convenience class to catch output of QProcess.
*/

namespace KDevelop {
class ProcessLineMakerPrivate;

class KDEVPLATFORMUTIL_EXPORT ProcessLineMaker : public QObject
{
    Q_OBJECT

public:
    explicit ProcessLineMaker(QObject* parent = nullptr);
    explicit ProcessLineMaker(QProcess* process, QObject* parent = nullptr);

    ~ProcessLineMaker() override;

    /**
     * clears out the internal buffers, this drops any data without
     * emitting the related signal
     */
    void discardBuffers();

    /**
     * Flush the data from the buffers and then clear them.
     * This should be called once when the process has
     * exited to make sure all data that was received from the
     * process is properly converted and emitted.
     *
     * Note: Connecting this class to the process finished signal
     * is not going to work, as the user of this class will do
     * that itself too and possibly delete the process, making
     * it impossible to fetch the last output.
     */
    void flushBuffers();

public Q_SLOTS:
    /**
     * This should be used (instead of hand-crafted code) when
     * you need to do custom things with the process output
     * before feeding it to the linemaker and have it convert
     * it to QString lines.
     * @param buffer the output from the process
     */
    void slotReceivedStdout(const QByteArray& buffer);

    /**
     * This should be used (instead of hand-crafted code) when
     * you need to do custom things with the process error output
     * before feeding it to the linemaker and have it convert
     * it to QString lines.
     * @param buffer the output from the process
     */
    void slotReceivedStderr(const QByteArray& buffer);

Q_SIGNALS:
    /**
     * Emitted whenever the process prints something
     * to its standard output. The output is converted
     * to a QString using fromLocal8Bit() and will
     * be split on '\n'.
     * @param lines the lines that the process printed
     */
    void receivedStdoutLines(const QStringList& lines);

    /**
     * Emitted whenever the process prints something
     * to its error output. The output is converted
     * to a QString using fromLocal8Bit() and will
     * be split on '\n'.
     * @param lines the lines that the process printed
     */
    void receivedStderrLines(const QStringList& lines);

private:
    const QScopedPointer<class ProcessLineMakerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProcessLineMaker)
    friend class ProcessLineMakerPrivate;
};

}

#endif
