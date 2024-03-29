/*
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_OUTPUTFILTERINGSTRATEGIES_H
#define KDEVPLATFORM_OUTPUTFILTERINGSTRATEGIES_H

/**
 * @file outputfilteringstrategies.h
 *
 * This file contains Concrete strategies for filtering output
 * in KDevelop output model. I.e. classes that inherit from ifilterstrategy.
 * New filtering strategies should be added here
 */


#include "ifilterstrategy.h"
#include <outputview/outputviewexport.h>

#include <QVector>
#include <QUrl>
#include <QScopedPointer>

namespace KDevelop
{
class CompilerFilterStrategyPrivate;

/**
 * This filter strategy is for not applying any filtering at all. Implementation of the
 * interface methods are basically noops
 **/
class KDEVPLATFORMOUTPUTVIEW_EXPORT NoFilterStrategy : public IFilterStrategy
{

public:
    NoFilterStrategy();

    FilteredItem errorInLine(const QString& line) override;

    FilteredItem actionInLine(const QString& line) override;

};

/**
 * This filter strategy checks if a given line contains output
 * that is defined as an error (or an action) from a compiler.
 **/
class KDEVPLATFORMOUTPUTVIEW_EXPORT CompilerFilterStrategy : public IFilterStrategy
{

public:
    explicit CompilerFilterStrategy(const QUrl& buildDir);
    ~CompilerFilterStrategy() override;

    FilteredItem errorInLine(const QString& line) override;

    FilteredItem actionInLine(const QString& line) override;

    QVector<QString> currentDirs() const;

private:
    const QScopedPointer<class CompilerFilterStrategyPrivate> d_ptr;
    Q_DECLARE_PRIVATE(CompilerFilterStrategy)
};

/**
 * This filter strategy filters out errors (no actions) from Python and PHP scripts.
 **/
class KDEVPLATFORMOUTPUTVIEW_EXPORT ScriptErrorFilterStrategy : public IFilterStrategy
{

public:
    ScriptErrorFilterStrategy();

    FilteredItem errorInLine(const QString& line) override;

    FilteredItem actionInLine(const QString& line) override;

};

/**
 * This filter strategy filters out errors (no actions) from runtime debug output of native applications
 *
 * This is especially useful for runtime output of Qt applications, for example lines such as:
 * "ASSERT: "errors().isEmpty()" in file /tmp/foo/bar.cpp", line 49"
 */
class KDEVPLATFORMOUTPUTVIEW_EXPORT NativeAppErrorFilterStrategy : public IFilterStrategy
{
public:
    NativeAppErrorFilterStrategy();

    FilteredItem errorInLine(const QString& line) override;
    FilteredItem actionInLine(const QString& line) override;
};

/**
 * This filter strategy filters out errors (no actions) from Static code analysis tools (Cppcheck,)
 **/
class KDEVPLATFORMOUTPUTVIEW_EXPORT StaticAnalysisFilterStrategy : public IFilterStrategy
{

public:
    StaticAnalysisFilterStrategy();

    FilteredItem errorInLine(const QString& line) override;

    FilteredItem actionInLine(const QString& line) override;

};

} // namespace KDevelop
#endif // KDEVPLATFORM_OUTPUTFILTERINGSTRATEGIES_H

