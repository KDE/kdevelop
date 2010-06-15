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

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>

#include <outputview/outputjob.h>
#include <outputview/outputmodel.h>

#include <KPluginFactory>
#include <KAboutData>

#include <QStandardItemModel>
#include <KAction>

K_PLUGIN_FACTORY( ExternalScriptFactory, registerPlugin<ExternalScriptPlugin>(); )
K_EXPORT_PLUGIN( ExternalScriptFactory( KAboutData( "kdevexternalscript", "kdevexternalscript", ki18n( "External Scripts" ),
                                        "0.1", ki18n( "Support for running external scripts." ),
                                        KAboutData::License_GPL ) ) )

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
    return "org.kdevelop.ExternalScript";
  }

private:
  ExternalScriptPlugin *m_plugin;
};

ExternalScriptPlugin* ExternalScriptPlugin::m_self = 0;

ExternalScriptPlugin::ExternalScriptPlugin( QObject* parent, const QVariantList& /*args*/ )
    : IPlugin( ExternalScriptFactory::componentData(), parent ),
    m_model( new QStandardItemModel( this ) ), m_factory( new ExternalScriptViewFactory( this ) )
{
  Q_ASSERT( !m_self );
  m_self = this;

  setXMLFile( "kdevexternalscript.rc" );

  core()->uiController()->addToolView( i18n( "External Scripts" ), m_factory );

  ExternalScriptItem* item = new ExternalScriptItem;
  item->setText("quick compile");
  item->setCommand("g++ -o %b %f && ./%b");
  item->setSaveMode(ExternalScriptItem::SaveCurrentDocument);
  m_model->appendRow(item);

  item = new ExternalScriptItem;
  item->setText( "uniq" );
  item->setCommand( "uniq" );
  item->setInputMode( ExternalScriptItem::InputSelection );
  item->setReplaceMode( ExternalScriptItem::ReplaceSelection );
  m_model->appendRow( item );
}

ExternalScriptPlugin* ExternalScriptPlugin::self()
{
  return m_self;
}

ExternalScriptPlugin::~ExternalScriptPlugin()
{
  m_self = 0;
}

void ExternalScriptPlugin::unload()
{
  core()->uiController()->removeToolView( m_factory );
  KDevelop::IPlugin::unload();
}

QStandardItemModel* ExternalScriptPlugin::model() const
{
  return m_model;
}

void ExternalScriptPlugin::execute( ExternalScriptItem* item ) const
{
  ExternalScriptJob* job = new ExternalScriptJob( item, const_cast<ExternalScriptPlugin*>(this) );

  KDevelop::ICore::self()->runController()->registerJob( job );
}

void ExternalScriptPlugin::executeScriptFromActionData() const
{
  KAction* action = dynamic_cast<KAction*>( sender() );
  Q_ASSERT( action );

  ExternalScriptItem* item = action->data().value<ExternalScriptItem*>();
  Q_ASSERT( item );

  execute( item );
}

#include "externalscriptplugin.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on; 
