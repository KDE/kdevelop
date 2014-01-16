/*
    This plugin is part of KDevelop.

    Copyright (C) 2010 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTPLUGIN_H
#define KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariantList>
#include <KConfigGroup>
#include <QModelIndex>

class ExternalScriptItem;

class QStandardItem;
class QStandardItemModel;

class ExternalScriptPlugin : public KDevelop::IPlugin
{
  Q_OBJECT
  Q_CLASSINFO( "D-Bus Interface", "org.kdevelop.ExternalScriptPlugin" )

public:
  ExternalScriptPlugin( QObject *parent, const QVariantList &args = QVariantList() );

  virtual ~ExternalScriptPlugin();
  virtual void unload();
  virtual KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);

  static ExternalScriptPlugin* self();

  /**
   * @return The model storing all external scripts managed by this plugin.
   * @NOTE: always append() items, never insert in the middle!
   */
  QStandardItemModel* model() const;

  /**
   * Executes @p script.
   */
  void execute(ExternalScriptItem* item, const KUrl &url) const;

  /**
   * Executes @p script.
   */
  void execute(ExternalScriptItem* item) const;

  /**
   * Returns config group to store all settings for this plugin in.
   */
  KConfigGroup getConfig() const;

  void saveItem(const ExternalScriptItem* item);

public slots:
  void executeScriptFromActionData() const;

  /**
   * Executes the command (Used by the shell-integration)
   * */
  Q_SCRIPTABLE bool executeCommand(QString command, QString workingDirectory) const;

  /**
   * Executes the command synchronously and returns the output text (Used by the shell-integration)
   * */
  Q_SCRIPTABLE QString executeCommandSync(QString command, QString workingDirectory) const;
  
private slots:
  void rowsRemoved( const QModelIndex& parent, int start, int end );
  void rowsInserted( const QModelIndex& parent, int start, int end );
  void executeScriptFromContextMenu() const;

private:
  /// @param item row in the model for the item to save
  void saveItemForRow( int row );

  QStandardItemModel* m_model;
  KUrl::List m_urls;
  static ExternalScriptPlugin* m_self;

  class ExternalScriptViewFactory *m_factory;
};

#endif // KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTPLUGIN_H

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on; 
