/*
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "unsavedfile.h"

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
    for (const QString& line : std::as_const(m_contents)) {
        m_contentsUtf8 += line.toUtf8() + '\n';
    }
}
