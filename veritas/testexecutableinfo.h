/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef VERITAS_TESTEXECUTABLE_INFO
#define VERITAS_TESTEXECUTABLE_INFO

#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <kurl.h>
#include "veritasexport.h"

namespace Veritas
{

class VERITAS_EXPORT TestExecutableInfo
{
public:
    TestExecutableInfo();
    virtual ~TestExecutableInfo();

    void setCommand(const QString& command);
    QString command() const;

    void setArguments(const QStringList& flags);
    QStringList arguments() const;

    void setWorkingDirectory(const KUrl& wd);
    KUrl workingDirectory() const;

    void setName(const QString& name);
    QString name() const;

    void addProperty(const QString& key, const QVariant& value);
    bool hasProperty(const QString& key) const;
    QVariant property(const QString& key) const;

private:
    QString m_command;
    QMap<QString, QVariant> m_properties;
    QStringList m_arguments;
    KUrl m_workingDirectory;
    QString m_name;
};

} // Veritas

#endif // VERITAS_TESTEXECUTABLE_INFO
