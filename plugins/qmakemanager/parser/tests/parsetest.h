/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARSETEST_H
#define PARSETEST_H

#include <QObject>
#include <QTest>

namespace QMake
{
    class ProjectAST;
}

class ParseTest : public QObject
{
        Q_OBJECT
    public:
        explicit ParseTest( QObject* parent = nullptr );
        ~ParseTest() override;
    private Q_SLOTS:
        void init();
        void cleanup();
        void successSimpleProject();
        void successSimpleProject_data();
        void failSimpleProject();
        void failSimpleProject_data();
        void successFullProject();
        void successFullProject_data();
        void failFullProject();
        void failFullProject_data();
    private:
        QMake::ProjectAST* ast = nullptr;
};

#endif

