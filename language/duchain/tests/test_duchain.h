/*
 * This file is part of KDevelop
 *
 * Copyright 2011-2013 Milian Wolff <mail@milianw.de>
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_TEST_DUCHAIN_H
#define KDEVPLATFORM_TEST_DUCHAIN_H

#include <QObject>

class TestDUChain : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testStringSets();
    void testSymbolTableValid();
    void testIndexedStrings();
    void testImportStructure();
    void testLockForWrite();
    void testLockForRead();
    void testLockForReadWrite();
    void testProblemSerialization();
    void testIdentifiers();
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
