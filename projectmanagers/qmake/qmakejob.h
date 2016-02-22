/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2010 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/


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

private slots:
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
