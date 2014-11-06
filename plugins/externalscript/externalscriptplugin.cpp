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

#include "externalscriptplugin.h"

#include "externalscriptview.h"
#include "externalscriptitem.h"
#include "externalscriptjob.h"
#include "externalscriptdebug.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/isession.h>

#include <outputview/outputjob.h>
#include <outputview/outputmodel.h>

#include <project/projectmodel.h>
#include <util/path.h>

#include <language/interfaces/editorcontext.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KProcess>
#include <KLocalizedString>

#include <QAction>
#include <QStandardItemModel>
#include <QDBusConnection>
#include <QMenu>


K_PLUGIN_FACTORY_WITH_JSON(ExternalScriptFactory, "kdevexternalscript.json", registerPlugin<ExternalScriptPlugin>();)

class ExternalScriptViewFactory: public KDevelop::IToolViewFactory
{
public:
  ExternalScriptViewFactory( ExternalScriptPlugin *plugin ): m_plugin( plugin ) {}

  virtual QWidget* create( QWidget *parent = 0 ) {
    return new ExternalScriptView( m_plugin, parent );
  }

  virtual Qt::DockWidgetArea defaultPosition() {
    return Qt::RightDockWidgetArea;
  }

  virtual QString id() const {
    return "org.kdevelop.ExternalScriptView";
  }

private:
  ExternalScriptPlugin *m_plugin;
};

ExternalScriptPlugin* ExternalScriptPlugin::m_self = 0;

ExternalScriptPlugin::ExternalScriptPlugin( QObject* parent, const QVariantList& /*args*/ )
    : IPlugin( "kdevexternalscript", parent ),
    m_model( new QStandardItemModel( this ) ), m_factory( new ExternalScriptViewFactory( this ) )
{
  Q_ASSERT( !m_self );
  m_self = this;

  QDBusConnection::sessionBus().registerObject( "/org/kdevelop/ExternalScriptPlugin", this, QDBusConnection::ExportScriptableSlots );

  setXMLFile( "kdevexternalscript.rc" );

  //BEGIN load config
  KConfigGroup config = getConfig();
  foreach( const QString group, config.groupList() ) {
    KConfigGroup script = config.group( group );
    if ( script.hasKey( "name" ) && script.hasKey( "command" ) ) {
      ExternalScriptItem* item = new ExternalScriptItem;
      item->setText( script.readEntry( "name" ) );
      item->setCommand( script.readEntry( "command" ));
      item->setInputMode( static_cast<ExternalScriptItem::InputMode>( script.readEntry( "inputMode", 0u ) ) );
      item->setOutputMode( static_cast<ExternalScriptItem::OutputMode>( script.readEntry( "outputMode", 0u ) ) );
      item->setErrorMode( static_cast<ExternalScriptItem::ErrorMode>( script.readEntry( "errorMode", 0u ) ) );
      item->setSaveMode( static_cast<ExternalScriptItem::SaveMode>( script.readEntry( "saveMode", 0u ) ) );
      item->setFilterMode( script.readEntry( "filterMode", 0u ));
      item->action()->setShortcut( QKeySequence( script.readEntry( "shortcuts" ) ) );
      item->setShowOutput( script.readEntry( "showOutput", true ) );
      m_model->appendRow( item );
    }
  }
  //END load config

  core()->uiController()->addToolView( i18n( "External Scripts" ), m_factory );

  connect( m_model, &QStandardItemModel::rowsRemoved,
           this, &ExternalScriptPlugin::rowsRemoved );
  connect( m_model, &QStandardItemModel::rowsInserted,
           this, &ExternalScriptPlugin::rowsInserted );

  const bool firstUse = config.readEntry( "firstUse", true );
  if ( firstUse ) {
    // some example scripts
    ExternalScriptItem* item = new ExternalScriptItem;
    item->setText( i18n("Quick Compile") );
    item->setCommand( "g++ -o %b %f && ./%b" );
    m_model->appendRow( item );

    item = new ExternalScriptItem;
    item->setText( i18n("Google Selection") );
    item->setCommand( "xdg-open \"http://www.google.de/search?q=%s\"" );
    item->setShowOutput( false );
    m_model->appendRow( item );

    item = new ExternalScriptItem;
    item->setText( i18n("Sort Selection") );
    item->setCommand( "sort" );
    item->setInputMode( ExternalScriptItem::InputSelectionOrDocument );
    item->setOutputMode( ExternalScriptItem::OutputReplaceSelectionOrDocument );
    item->setShowOutput( false );
    m_model->appendRow( item );

    config.writeEntry( "firstUse", false );
    config.sync();
  }
}

ExternalScriptPlugin* ExternalScriptPlugin::self()
{
  return m_self;
}

ExternalScriptPlugin::~ExternalScriptPlugin()
{
  m_self = 0;
}

KDevelop::ContextMenuExtension ExternalScriptPlugin::contextMenuExtension( KDevelop::Context* context )
{
  m_urls.clear();

  int folderCount = 0;

  if ( context->type() == KDevelop::Context::FileContext ) {
    KDevelop::FileContext* filectx = dynamic_cast<KDevelop::FileContext*>( context );
    m_urls = filectx->urls();
  } else if ( context->type() == KDevelop::Context::ProjectItemContext ) {
    KDevelop::ProjectItemContext* projctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
    foreach( KDevelop::ProjectBaseItem* item, projctx->items() ) {
      if ( item->file() ) {
        m_urls << item->file()->path().toUrl();
      } else if ( item->folder() ) {
        m_urls << item->folder()->path().toUrl();
        folderCount++;
      }
    }
  } else if ( context->type() == KDevelop::Context::EditorContext ) {
      KDevelop::EditorContext *econtext = dynamic_cast<KDevelop::EditorContext*>(context);
      m_urls << econtext->url();
  }

  if ( !m_urls.isEmpty() ) {
    KDevelop::ContextMenuExtension ext;
    QMenu* menu = new QMenu();
    menu->setTitle( i18n("External Scripts") );

    for ( int row = 0; row < m_model->rowCount(); ++row ) {
      ExternalScriptItem* item = dynamic_cast<ExternalScriptItem*>( m_model->item( row ) );
      Q_ASSERT( item );

      if (context->type() != KDevelop::Context::EditorContext) {
        // filter scripts that depend on an opened document
        // if the context menu was not requested inside the editor
        if (item->performParameterReplacement() && item->command().contains("%s")) {
          continue;
        } else if (item->inputMode() == ExternalScriptItem::InputSelectionOrNone) {
          continue;
        }
      }

      if ( folderCount == m_urls.count() ) {
        // when only folders filter items that don't have %d parameter (or another parameter)
        if (item->performParameterReplacement() &&
          (!item->command().contains("%d") ||
            item->command().contains("%s") ||
            item->command().contains("%u") ||
            item->command().contains("%f") ||
            item->command().contains("%b") ||
            item->command().contains("%n")
          )
        ) {
          continue;
        }
      }

      QAction* scriptAction = new QAction( item->text(), this );
      scriptAction->setData( QVariant::fromValue<ExternalScriptItem*>( item ));
      connect( scriptAction, &QAction::triggered, this, &ExternalScriptPlugin::executeScriptFromContextMenu );
      menu->addAction( scriptAction );
    }

    ext.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, menu->menuAction() );

    return ext;
  }

  return KDevelop::IPlugin::contextMenuExtension( context );
}

void ExternalScriptPlugin::unload()
{
  core()->uiController()->removeToolView( m_factory );
  KDevelop::IPlugin::unload();
}

KConfigGroup ExternalScriptPlugin::getConfig() const
{
  return KSharedConfig::openConfig()->group("External Scripts");
}

QStandardItemModel* ExternalScriptPlugin::model() const
{
  return m_model;
}

void ExternalScriptPlugin::execute( ExternalScriptItem* item, const QUrl& url ) const
{
  ExternalScriptJob* job = new ExternalScriptJob( item, url, const_cast<ExternalScriptPlugin*>(this) );

  KDevelop::ICore::self()->runController()->registerJob( job );
}

void ExternalScriptPlugin::execute(ExternalScriptItem* item) const
{
  auto document = KDevelop::ICore::self()->documentController()->activeDocument();
  execute( item, document ? document->url() : QUrl() );
}

bool ExternalScriptPlugin::executeCommand ( QString command, QString workingDirectory ) const
{
  // We extend ExternalScriptJob so that it deletes the temporarily created item on destruction
  class ExternalScriptJobOwningItem : public ExternalScriptJob {
  public:
    ExternalScriptJobOwningItem( ExternalScriptItem* item, const QUrl &url, ExternalScriptPlugin* parent ) : ExternalScriptJob(item, url, parent), m_item(item) {
    }
    ~ExternalScriptJobOwningItem() {
      delete m_item;
    }
  private:
    ExternalScriptItem* m_item;
  };
  ExternalScriptItem* item = new ExternalScriptItem;
  item->setCommand(command);
  item->setWorkingDirectory(workingDirectory);
  item->setPerformParameterReplacement(false);
  qCDebug(PLUGIN_EXTERNALSCRIPT) << "executing command " << command << " in dir " << workingDirectory << " as external script";
  ExternalScriptJobOwningItem* job = new ExternalScriptJobOwningItem( item, QUrl(), const_cast<ExternalScriptPlugin*>(this) );
  // When a command is executed, for example through the terminal, we don't want the command output to be risen
  job->setVerbosity(KDevelop::OutputJob::Silent);

  KDevelop::ICore::self()->runController()->registerJob( job );
  return true;
}

QString ExternalScriptPlugin::executeCommandSync ( QString command, QString workingDirectory ) const
{
  qCDebug(PLUGIN_EXTERNALSCRIPT) << "executing command " << command << " in working-dir " << workingDirectory;
  KProcess process;
  process.setWorkingDirectory( workingDirectory );
  process.setShellCommand( command );
  process.setOutputChannelMode( KProcess::OnlyStdoutChannel );
  process.execute();
  return QString::fromLocal8Bit(process.readAll());
}

void ExternalScriptPlugin::executeScriptFromActionData() const
{
  QAction* action = dynamic_cast<QAction*>( sender() );
  Q_ASSERT( action );

  ExternalScriptItem* item = action->data().value<ExternalScriptItem*>();
  Q_ASSERT( item );

  execute( item );
}

void ExternalScriptPlugin::executeScriptFromContextMenu() const
{
  QAction* action = dynamic_cast<QAction*>( sender() );
  Q_ASSERT( action );

  ExternalScriptItem* item = action->data().value<ExternalScriptItem*>();
  Q_ASSERT( item );

  foreach( const QUrl& url, m_urls) {
    KDevelop::ICore::self()->documentController()->openDocument( url );
    execute( item, url );
  }
}

void ExternalScriptPlugin::rowsInserted( const QModelIndex& /*parent*/, int start, int end )
{
  for ( int i = start; i <= end; ++i ) {
    saveItemForRow( i );
  }
}

void ExternalScriptPlugin::rowsRemoved( const QModelIndex& /*parent*/, int start, int end )
{
  KConfigGroup config = getConfig();
  for ( int i = start; i <= end; ++i ) {
    KConfigGroup child = config.group( QString("script %1").arg(i) );
    qCDebug(PLUGIN_EXTERNALSCRIPT) << "removing config group:" << child.name();
    child.deleteGroup();
  }
  config.sync();
}

void ExternalScriptPlugin::saveItem( const ExternalScriptItem* item )
{
  const QModelIndex index = m_model->indexFromItem( item );
  Q_ASSERT( index.isValid() );
  saveItemForRow( index.row() );
}

void ExternalScriptPlugin::saveItemForRow( int row )
{
  const QModelIndex idx = m_model->index( row, 0 );
  Q_ASSERT( idx.isValid() );

  ExternalScriptItem* item = dynamic_cast<ExternalScriptItem*>( m_model->item( row ) );
  Q_ASSERT( item );

  qCDebug(PLUGIN_EXTERNALSCRIPT) << "save extern script:" << item << idx;
  KConfigGroup config = getConfig().group( QString("script %1").arg( row ) );
  config.writeEntry( "name", item->text() );
  config.writeEntry( "command", item->command() );
  config.writeEntry( "inputMode", (uint) item->inputMode() );
  config.writeEntry( "outputMode", (uint) item->outputMode() );
  config.writeEntry( "errorMode", (uint) item->errorMode() );
  config.writeEntry( "saveMode", (uint) item->saveMode() );
  config.writeEntry( "shortcuts", item->action()->shortcut().toString() );
  config.writeEntry( "showOutput", item->showOutput() );
  config.writeEntry( "filterMode", item->filterMode());
  config.sync();
}

#include "externalscriptplugin.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on;
