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

#include "testexecutableinfo.h"

using Veritas::TestExecutableInfo;

TestExecutableInfo::TestExecutableInfo()
{
}

TestExecutableInfo::~TestExecutableInfo()
{
}

QString TestExecutableInfo::name() const
{
    return m_name;
}

void TestExecutableInfo::setName(const QString& name)
{
    m_name = name;
}

QVariant TestExecutableInfo::property(const QString& key) const
{
    if (m_properties.contains(key)) {
        return m_properties[key];
    } else {
        return QVariant();
    }
}

bool TestExecutableInfo::hasProperty(const QString& key) const
{
    return m_properties.contains(key);
}

void TestExecutableInfo::addProperty(const QString& key, const QVariant& value)
{
    m_properties[key] = value;
}

void TestExecutableInfo::setCommand(const QString& command)
{
    m_command = command;
}

QString TestExecutableInfo::command() const
{
    return m_command;
}

void TestExecutableInfo::setArguments(const QStringList& args)
{
    m_arguments = args;
}

QStringList TestExecutableInfo::arguments() const
{
    return m_arguments;
}

KUrl TestExecutableInfo::workingDirectory() const
{
    return m_workingDirectory;
}

void TestExecutableInfo::setWorkingDirectory(const KUrl& dir)
{
    m_workingDirectory = dir;
}

void TestExecutableInfo::setSource(const KUrl& source)
{
    m_source = source;
}

KUrl TestExecutableInfo::source() const
{
    return m_source;
}

