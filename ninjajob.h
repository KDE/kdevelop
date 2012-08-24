/* This file is part of KDevelop
    Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef NINJAJOB_H
#define NINJAJOB_H

#include <outputview/outputjob.h>
#include <QProcess>

namespace KDevelop {
    class CommandExecutor;
    class OutputModel;
}

class KProcess;
class KUrl;

class NinjaJob : public KDevelop::OutputJob
{
    Q_OBJECT
    enum ErrorTypes {
        Correct = 0,
        Failed
    };
    public:
        NinjaJob(const KUrl& dir, const QStringList& arguments, QObject* parent = 0);
        virtual void start();
        virtual bool doKill();

    private slots:
        void slotFailed(QProcess::ProcessError error);
        void slotCompleted();
        void appendLines(const QStringList& lines);

    private:
        KDevelop::CommandExecutor* m_process;
        KDevelop::OutputModel* m_model;
        bool m_lastLine;
};

#endif // NINJAJOB_H
