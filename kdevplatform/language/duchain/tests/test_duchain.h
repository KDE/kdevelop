/*
    SPDX-FileCopyrightText: 2011-2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_DUCHAIN_H
#define KDEVPLATFORM_TEST_DUCHAIN_H

#include <QObject>

class TestDUChain
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

#ifndef Q_OS_WIN
    // Causes stack overflow on Windows (MSVC2015)
    void testStringSets();
#endif
    void testDefinitions();
    void testSymbolTableValid();
    void testIndexedStrings();
    void testImportStructure();
    void testLockForWrite();
    void testLockForRead();
    void testLockForReadWrite();
    void testProblemSerialization();
    void testIdentifiers();
    void testTypePtr();
    ///NOTE: these are not "automated"!
//     void testImportCache();

    void benchCodeModel();
    void benchTypeRegistry();
    void benchTypeRegistry_data();
    void benchDuchainWriteLocker();
    void benchDuchainReadLocker();
    void benchDUChainItemFactory_copy();
    void benchDUChainItemFactory_copy_data();
    void benchDeclarationQualifiedIdentifier();
};

#endif // KDEVPLATFORM_TEST_DUCHAIN_H
