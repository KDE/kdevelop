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

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KProcess>
#include <KLocalizedString>
#include <KMessageBox>

#include <QStandardItemModel>
#include <QApplication>
#include <QFileInfo>

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

ExternalScriptPlugin::ExternalScriptPlugin( QObject* parent, const QVariantList& /*args*/ )
    : IPlugin( ExternalScriptFactory::componentData(), parent ),
    m_model( new QStandardItemModel( this ) ), m_factory( new ExternalScriptViewFactory( this ) )
{
  setXMLFile( "kdevexternalscript.rc" );

  core()->uiController()->addToolView( i18n( "External Scripts" ), m_factory );

  ExternalScriptItem* item = new ExternalScriptItem;
  item->setText("quick compile");
  item->setCommand("g++ -o %b %f && ./%b");
  item->setSaveMode(ExternalScriptItem::SaveCurrentDocument);

  m_model->appendRow(item);
}

ExternalScriptPlugin::~ExternalScriptPlugin()
{

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
  ///TODO: user feedback
  if ( item->command().isEmpty() ) {
    return;
  }

  KProcess* cmd = new KProcess( const_cast<ExternalScriptPlugin*>( this ) );

  KDevelop::IDocument* active = KDevelop::ICore::self()->documentController()->activeDocument();

  QString command = item->command();

  if ( active ) {
    const KUrl url = active->url();
    if ( url.isLocalFile() ) {
      cmd->setWorkingDirectory( active->url().directory() );
    }
    ///TODO: make those placeholders escapeable
    command.replace( "%u", url.pathOrUrl() );
    ///TODO: does that work with remote files?
    ///TODO: document the available placeholders
    QFileInfo info( url.pathOrUrl() );
    command.replace( "%f", info.filePath() );
    command.replace( "%b", info.baseName() );
    command.replace( "%n", info.fileName() );
    command.replace( "%d", info.path() );
  }

  cmd->setShellCommand( command );
  cmd->setOutputChannelMode( KProcess::SeparateChannels );
  connect( cmd, SIGNAL(finished(int, QProcess::ExitStatus)),
           this, SLOT(scriptFinished(int, QProcess::ExitStatus)));
  cmd->start();
}

void ExternalScriptPlugin::scriptFinished( int exitCode , QProcess::ExitStatus exitStatus )
{
  KProcess* cmd = dynamic_cast<KProcess*>( sender() );
  Q_ASSERT( cmd );
  qDebug() << "stderr:" << cmd->readAllStandardError();
  qDebug() << "stdout:" << cmd->readAllStandardOutput();
  qDebug() << "exitcode:" << exitCode << "status:" << exitStatus;

  delete cmd;
}

#include "externalscriptplugin.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on; 
