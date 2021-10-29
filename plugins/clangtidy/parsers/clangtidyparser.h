/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _CLANGTIDYPARSER_H_
#define _CLANGTIDYPARSER_H_

// KDevPlatform
#include <interfaces/iproblem.h>
// Qt
#include <QRegularExpression>
#include <QObject>

namespace ClangTidy
{

/**
 * \class
 * \brief Implements a parser for clang-tidy's standard output.
 */
class ClangTidyParser : public QObject
{
    Q_OBJECT

public:
    explicit ClangTidyParser(QObject* parent = nullptr);
    ~ClangTidyParser() override = default;

public:
    /**
     * \brief meant to be used by Job class to pass the standard output to be parsed.
     */
    void addData(const QStringList& stdoutList);

Q_SIGNALS:
    void problemsDetected(const QVector<KDevelop::IProblem::Ptr>& problems);

private:
    const QRegularExpression m_hitRegExp;
};

} // namespace ClangTidy

#endif
