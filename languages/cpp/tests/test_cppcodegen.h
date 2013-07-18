/*
   This file is part of KDevelop
   Copyright 2009 Ramon Zarazua <killerfox512+kde@gmail.com>
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TEST_CPPCODEGEN_H
#define TEST_CPPCODEGEN_H

#include <QtCore/QObject>

class TestCppCodegen : public QObject
{
    Q_OBJECT
    
  private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Tests
    void testAssistants();
    void testAssistants_data();
    
    void testSimplifiedUpdating();
    void testUpdateIndices();
    
    void testAstDuChainMapping();
    void testClassGeneration();
    void testPrivateImplementation();

    void testMacroDeclarationOrder();

    void testMoveIntoSource();
    void testMoveIntoSource_data();
};

#endif // TEST_CPPCODEGEN_H
