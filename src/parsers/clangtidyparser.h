/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
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

#ifndef _CLANGTIDYPARSER_H_
#define _CLANGTIDYPARSER_H_

// KDevPlatform
#include <interfaces/iproblem.h>
// Qt
#include <QObject>

namespace ClangTidy
{
/**
 * \class
 * \brief Implements a parser for clang-tidy's standard output.
 */
class ClangTidyParser : public QObject
{
public:
    explicit ClangTidyParser(QObject* parent = nullptr);
    ~ClangTidyParser() = default;
    /**
     * \return all problems collected from clang-tidy stdout.
     */
    QVector<KDevelop::IProblem::Ptr> problems() const { return m_problems; }

    void parse();

    /**
     * \brief meant to be used by Job class to pass the standard output to be parsed.
     */
    void addData(const QStringList& stdoutList) { m_stdout = stdoutList; }

private:
    QVector<KDevelop::IProblem::Ptr> m_problems; ///< problems collected from clang-tidy stdout.
    QStringList m_stdout;
};
} // namespace ClangTidy

#endif
