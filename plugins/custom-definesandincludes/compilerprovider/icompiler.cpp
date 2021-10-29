/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "icompiler.h"

using namespace KDevelop;

ICompiler::ICompiler(const QString& name, const QString& path, const QString& factoryName, bool editable):
    m_editable(editable),
    m_name(name),
    m_path(path),
    m_factoryName(factoryName)
{}

void ICompiler::setPath(const QString& path)
{
    if (editable()) {
        m_path = path;
    }
}

QString ICompiler::path() const
{
    return m_path;
}

void ICompiler::setName(const QString& name)
{
    if (editable()) {
        m_name = name;
    }
}

QString ICompiler::name() const
{
    return m_name;
}

bool ICompiler::editable() const
{
    return m_editable;
}

QString ICompiler::factoryName() const
{
    return m_factoryName;
}
