/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2014 David Stevens <dgedstevens@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef TESTASSISTANTS_H
#define TESTASSISTANTS_H

#include <QObject>

class TestAssistants : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testRenameAssistant_data();
    void testRenameAssistant();
    void testRenameAssistantUndoRename();
    void testSignatureAssistant_data();
    void testSignatureAssistant();
    void testUnknownDeclarationAssistant_data();
    void testUnknownDeclarationAssistant();

    void testMoveIntoSource_data();
    void testMoveIntoSource();

    void testHeaderGuardAssistant();
    void testHeaderGuardAssistant_data();
};

#endif
