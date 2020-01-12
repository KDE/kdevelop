/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVCLAZY_JOB_PARAMETERS_H
#define KDEVCLAZY_JOB_PARAMETERS_H

#include <QObject>
#include <QString>
#include <QUrl>

namespace KDevelop { class IProject; }

namespace Clazy
{

class JobGlobalParameters : public QObject
{
    Q_OBJECT

public:
    JobGlobalParameters(const QUrl& executablePath, const QUrl& docsPath);
    ~JobGlobalParameters() override = default;

    static QUrl defaultExecutablePath();
    static QUrl defaultDocsPath();

    bool isValid() const;
    QString error() const;

Q_SIGNALS:
    void changed();

protected:
    JobGlobalParameters();

    QString m_executablePath;
    QString m_docsPath;

    QString m_error;
};

}

#endif
