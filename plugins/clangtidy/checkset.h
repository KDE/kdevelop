/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLANGTIDY_CHECKSET_H
#define CLANGTIDY_CHECKSET_H

// Qt
#include <QStringList>

namespace ClangTidy
{

/**
 * \brief Provides all available checks by running clang-tidy with the following parameters: "--checks=*
 * --list-checks".
 */
class CheckSet
{
public:
    CheckSet() = default;

public:
    /**
     * @param path the system path for the clang-tidy program.
     */
    void setClangTidyPath(const QString& path);

    const QStringList &all() const { return m_allChecks; }
    QStringList defaults() const;

private:
    QString m_clangTidyPath;
    QStringList m_allChecks;
};

}

#endif
