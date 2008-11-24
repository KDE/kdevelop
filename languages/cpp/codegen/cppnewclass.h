/*
 * KDevelop C++ Language Support
 *
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
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

#ifndef CPP_NEWCLASS_H
#define CPP_NEWCLASS_H

#include <language/codegen/createclass.h>
#include <language/codegen/overridespage.h>

class CppClassIdentifierPage : public KDevelop::ClassIdentifierPage
{
  Q_OBJECT

public:
  CppClassIdentifierPage(QWizard* parent);
};

class CppOverridesPage : public KDevelop::OverridesPage
{
  Q_OBJECT

public:
    CppOverridesPage(QWizard* parent);

    virtual void addPotentialOverride(QTreeWidgetItem* classItem, KDevelop::Declaration* childDeclaration);
};

class CppNewClass : public KDevelop::CreateClass
{
  Q_OBJECT

public:
  CppNewClass(QWidget* parent, KUrl baseUrl = KUrl());

  virtual void generate();
  void generateHeader();
  void generateImplementation();

  virtual KUrl headerUrlFromBase(QString className, KUrl baseUrl);
  virtual KUrl implementationUrlFromBase(QString className, KUrl baseUrl);

  virtual CppClassIdentifierPage* newIdentifierPage();
  virtual CppOverridesPage* newOverridesPage();

private:
  QStringList m_baseClasses;
  KUrl m_url;
};

#endif // CPP_NEWCLASS_H
