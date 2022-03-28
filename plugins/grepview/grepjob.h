/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2010 Silvère Lestang <silvere.lestang@gmail.com>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_GREPJOB_H
#define KDEVPLATFORM_PLUGIN_GREPJOB_H

#include <QPointer>
#include <QUrl>

#include <KJob>

#include <interfaces/istatus.h>

#include "grepfindthread.h"
#include "grepoutputmodel.h"

namespace KDevelop
{
    class IProject;
}

class QRegExp;
class GrepViewPlugin;
class FindReplaceTest; //FIXME: this is useful only for tests

struct GrepJobSettings
{
    bool fromHistory = false;

    bool projectFilesOnly = false;
    bool caseSensitive = true;
    bool regexp = true;

    int depth = -1;

    QString pattern;
    QString searchTemplate;
    QString replacementTemplate;
    QString files;
    QString exclude;
    QString searchPaths;
};

Q_DECLARE_TYPEINFO(GrepJobSettings, Q_MOVABLE_TYPE);


class GrepJob : public KJob, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IStatus )

    friend class GrepViewPlugin;
    friend class FindReplaceTest;

private:
    ///Job can only be instanciated by plugin
    explicit GrepJob( QObject *parent = nullptr );

public:
    void setSettings(const GrepJobSettings& settings);
    GrepJobSettings settings() const;

    void setOutputModel(GrepOutputModel * model);
    void setDirectoryChoice(const QList<QUrl> &choice);

    void start() override;

    QString statusName() const override;
protected:
    bool doKill() override;

//    GrepOutputModel* model() const;

private Q_SLOTS:
    void slotFindFinished();
    void testFinishState(KJob *job);

Q_SIGNALS:
    void clearMessage( KDevelop::IStatus* ) override;
    void showMessage( KDevelop::IStatus*, const QString & message, int timeout = 0) override;
    void showErrorMessage(const QString & message, int timeout = 0) override;
    void hideProgress( KDevelop::IStatus* ) override;
    void showProgress( KDevelop::IStatus*, int minimum, int maximum, int value) override;
    void foundMatches( const QString& filename, const GrepOutputItem::List& matches);

private:
    Q_INVOKABLE void slotWork();

    QList<QUrl> m_directoryChoice;
    QString m_errorMessage;

    QRegExp m_regExp;
    QString m_regExpSimple;
    QPointer<GrepOutputModel> m_outputModel;

    enum {
        WorkCollectFiles,
        WorkGrep,
        WorkIdle,
        WorkCancelled
    } m_workState;

    QList<QUrl> m_fileList;
    int m_fileIndex;
    QPointer<GrepFindFilesThread> m_findThread;

    GrepJobSettings m_settings;

    bool m_findSomething;
};

//FIXME: this function is used externally only for tests, find a way to keep it
//       static for a regular compilation
GrepOutputItem::List grepFile(const QString &filename, const QRegExp &re);

#endif
