/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

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

#ifndef TestCppAssistants_H
#define TestCppAssistants_H

#include <QObject>

class TestCppAssistants : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();
  void cleanupTestCase();
  void testRenameAssistant_data();
  void testRenameAssistant();
  void testRenameAssistantUndoRename();
  void testSignatureAssistant_data();
  void testSignatureAssistant();
  void testMacroExpansion();
  void testMacroExpansion_data();
};

#endif
