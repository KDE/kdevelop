/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCOPETEST_H
#define SCOPETEST_H

#include <QObject>
#include <QTest>


namespace QMake
{
    class StatementAST;
    class ProjectAST;
    class FunctionArgAST;
}

class ScopeTest : public QObject
{
        Q_OBJECT
    public:
        explicit ScopeTest( QObject* parent = nullptr );
        ~ScopeTest() override;
    private Q_SLOTS:
        void init();
        void cleanup();
        void basicScope();
        void basicScope_data();
        void basicScopeBrace();
        void basicScopeBrace_data();
        void nestedScope();
        void nestedScope_data();
        void missingStatement();
        void missingStatement_data();
        void missingColon();
        void missingColon_data();
        void strangeScopeNames();
    private:
        QMake::ProjectAST* ast = nullptr;

};

#endif

