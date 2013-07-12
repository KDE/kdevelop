/*
 * This file is part of KDevelop
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Lior Mualem <lior.m.kde@gmail.com>
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

#ifndef KDEVPLATFORM_PLUGIN_CLASSBROWSERPLUGIN_H
#define KDEVPLATFORM_PLUGIN_CLASSBROWSERPLUGIN_H

#include <interfaces/iplugin.h>
#include <QtCore/QVariant>
#include <language/duchain/duchainpointer.h>

class ClassTree;

namespace KDevelop
{
  class Declaration;
}

class ClassBrowserPlugin : public KDevelop::IPlugin
{
  Q_OBJECT

public:
  ClassBrowserPlugin(QObject *parent, const QVariantList & = QVariantList() );
  virtual ~ClassBrowserPlugin();

  void setActiveClassTree(ClassTree* a_classTree) { m_activeClassTree = a_classTree; }

public: // KDevelop::Plugin overrides
  virtual void unload();
  virtual KDevelop::ContextMenuExtension contextMenuExtension( KDevelop::Context* );

  // The duchain must not be locked when this is called!
  void showDefinition(KDevelop::DeclarationPointer declaration);
private Q_SLOTS:
  void findInClassBrowser();

private:
  class ClassBrowserFactory* m_factory;
  ClassTree* m_activeClassTree;
  QAction* m_findInBrowser;
};

#endif // KDEVPLATFORM_PLUGIN_CLASSBROWSERPLUGIN_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
