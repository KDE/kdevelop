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

#include <outputview/outputexecutejob.h>

namespace KDevelop {
    class OutputModel;
    class ProjectBaseItem;
}

class KProcess;
class KUrl;

class NinjaJob : public KDevelop::OutputExecuteJob
{
    Q_OBJECT
    enum ErrorTypes {
        Correct = 0,
        Failed
    };
    public:
        NinjaJob( KDevelop::ProjectBaseItem* item, const QStringList& arguments, const QByteArray& signal, QObject* parent );
        void setIsInstalling( bool isInstalling );

        virtual KUrl workingDirectory() const;
        virtual QStringList privilegedExecutionCommand() const;

    protected slots:
        virtual void postProcessStdout( const QStringList& lines );
        virtual void postProcessStderr( const QStringList& lines );

    private slots:
        void emitProjectBuilderSignal(KJob* job);

    private:
        KDevelop::ProjectBaseItem* item() const;
        bool m_isInstalling;
        QPersistentModelIndex m_idx;
        QByteArray m_signal;

        void appendLines( const QStringList& lines );
};

#endif // NINJAJOB_H
