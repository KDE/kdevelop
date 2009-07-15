/*
   This file is part of KDevelop
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>
   
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

#include <language/duchain/indexedstring.h>

#include <QtCore/QObject>
#include <QMap>
#include <language/duchain/topducontext.h>


class TestCppCodegen : public QObject
{
    Q_OBJECT
    
    typedef QMap<KDevelop::IndexedString, KDevelop::ReferencedTopDUContext> ContextContainer;
    
  private slots:
    // Tests
    void init();
    void initTestCase();
    void cleanupTestCase();
    
    
    void testAstDuChainMapping();
    void testClassGeneration();
    void testPrivateImplementation();
    
  private:
    void parseArtificialCode();
    void resetArtificialCode();
    
    
    void addArtificialCode(KDevelop::IndexedString fileName, const QString & code);
    
    QList<KDevelop::IndexedString> m_artificialCodeNames;
    ContextContainer m_contexts;
    
    static const QString m_testUrl;
};

#endif // TEST_CPPCODEGEN_H
