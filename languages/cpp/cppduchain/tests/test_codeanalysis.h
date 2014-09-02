/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#ifndef TEST_CODEANALYSIS_H
#define TEST_CODEANALYSIS_H

#include <QObject>
#include "test_helper.h"

class TestCodeAnalysis : public QObject, public Cpp::TestHelper
{
  Q_OBJECT
  public:
    explicit TestCodeAnalysis(QObject* parent = 0);
    
  private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testUseReadWrite();
    void testUseReadWrite_data();
    
    void testControlFlowCreation();
    void testControlFlowCreation_data();
};

#endif // TEST_CODEANALYSIS_H
