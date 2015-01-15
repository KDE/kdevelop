/*
    This file is part of KDevelop

    Copyright 2015 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "unsavedfile.h"

#include <QDebug>

#include <clang-c/Index.h>

#include <algorithm>

UnsavedFile::UnsavedFile(const QString& fileName, const QStringList& contents)
    : m_fileName(fileName)
    , m_contents(contents)
{
}

CXUnsavedFile UnsavedFile::toClangApi() const
{
    if (m_fileNameUtf8.isEmpty()) {
        const_cast<UnsavedFile*>(this)->convertToUtf8();
    }

    CXUnsavedFile file;
    file.Contents = m_contentsUtf8.data();
    file.Length = m_contentsUtf8.size();
    file.Filename = m_fileNameUtf8.data();

    return file;
}

void UnsavedFile::convertToUtf8()
{
    m_fileNameUtf8 = m_fileName.toUtf8();
    m_contentsUtf8.clear();
    foreach(const QString& line, m_contents) {
        m_contentsUtf8 += line.toUtf8();
        m_contentsUtf8 += '\n';
    }
}
