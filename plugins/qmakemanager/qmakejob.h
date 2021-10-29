/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


#ifndef QMAKEJOB_H
#define QMAKEJOB_H

#include <outputview/outputjob.h>

#include <QProcess>

namespace KDevelop {
class OutputModel;
}

class QProcess;
class QMakeJob : public KDevelop::OutputJob
{
    Q_OBJECT

public:
    QMakeJob( QString  srcDir, QString buildDir, QObject* parent = nullptr );
    void start() override;
    ~QMakeJob() override;
    void setQMakePath(const QString& path);
    void setInstallPrefix(const QString& prefix);
    void setBuildType(int comboboxSelectedIndex); // --> qmakebuilddirchooser.ui
    void setExtraArguments(const QString& args);
    

    QString errorString() const override;

protected:
    bool doKill() override;

private Q_SLOTS:
  void processError( QProcess::ProcessError error );
  void processFinished( int exitCode, QProcess::ExitStatus status );

private:
    QString m_srcDir;
    QString m_buildDir;
    QString m_qmakePath;
    QString m_installPrefix;
    int m_buildType;
    QString m_extraArguments;
    QProcess* m_process;
    KDevelop::OutputModel* m_model;
};

#endif // QMAKEJOB_H
