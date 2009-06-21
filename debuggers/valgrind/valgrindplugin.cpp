/* This file is part of KDevelop
 *  Copyright 2002 Harald Fernengel <harry@kdevelop.org>
 *  Copyright 2007 Hamish Rodda <rodda@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#include "valgrindplugin.h"

#include <unistd.h>

#include <QRegExp>
#include <QFile>
#include <QTreeView>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDomElement>
#include <QApplication>

#include <kiconloader.h>
#include <klocale.h>
#include <kaction.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <kcmultidialog.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/launchconfigurationtype.h>

#include "valgrindmodel.h"
#include "valgrindjob.h"
#include "valgrindconfig.h"
#include "valgrindwidget.h"
#include <execute/iexecuteplugin.h>
#include <interfaces/iplugincontroller.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(ValgrindFactory, registerPlugin<ValgrindPlugin>(); )
K_EXPORT_PLUGIN(ValgrindFactory(KAboutData("kdevvalgrind","kdevvalgrind", ki18n("Valgrind"), "0.1", ki18n("Support for running valgrind"), KAboutData::License_GPL)))

class ValgrindWidgetFactory : public KDevelop::IToolViewFactory
{
public:
    ValgrindWidgetFactory(ValgrindPlugin* plugin)
        : m_plugin( plugin )
    {
    }

    virtual QWidget* create( QWidget *parent = 0 )
    {
        return new ValgrindWidget(m_plugin, parent);
    }

    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::BottomDockWidgetArea;
    }

    virtual QString id() const
    {
        return "org.kdevelop.ValgrindView";
    }

private:
    ValgrindPlugin* m_plugin;
};

ValgrindPlugin::ValgrindPlugin( QObject *parent, const QVariantList& )
    : IPlugin( ValgrindFactory::componentData(), parent)
{
    kDebug() << "setting valgrind rc file";
    setXMLFile( "kdevvalgrind.rc" );

    core()->uiController()->addToolView(i18n("Valgrind"), new ValgrindWidgetFactory(this));
    
    // Initialize actions for the laucnh modes
    KAction* act = actionCollection()->addAction("valgrind_memcheck", this, SLOT(runMemCheck()) );
    act->setText( i18n("Memory Check" ) );
    act->setStatusTip( i18n("Launches the currently selected launch configuration with the Valgrind Memory Checker (memcheck).") );
    act = actionCollection()->addAction("valgrind_callgrind", this, SLOT(runCallGrind()) );
    act->setText( i18n("Call Tracing" ) );
    act->setStatusTip( i18n("Launches the currently selected launch configuration with the Valgrind Call Tracing (callgrind).") );
    act = actionCollection()->addAction("valgrind_helgrind", this, SLOT(runHelGrind()) );
    act->setText( i18n("Race Conditions" ) );
    act->setStatusTip( i18n("Launches the currently selected launch configuration with the Valgrind Race Conditions Checker (helgrind).") );
    act = actionCollection()->addAction("valgrind_cachegrind", this, SLOT(runCacheGrind()) );
    act->setText( i18n("Cache Simulator" ) );
    act->setStatusTip( i18n("Launches the currently selected launch configuration with the Valgrind Cache Simulator (cachegrind).") );
    
    IExecutePlugin* iface = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IExecutePlugin")->extension<IExecutePlugin>();
    Q_ASSERT(iface);
    
    // Initialize launch modes and register them with platform, also put them into our launcher
    ValgrindLauncher* launcher = new ValgrindLauncher();
    ValgrindLaunchMode* mode = new MemCheckLaunchMode();
    KDevelop::ICore::self()->runController()->addLaunchMode( mode );
    launcher->addMode( mode );
    mode = new HelGrindLaunchMode();
    KDevelop::ICore::self()->runController()->addLaunchMode( mode );
    launcher->addMode( mode );
    mode = new CallGrindLaunchMode();
    KDevelop::ICore::self()->runController()->addLaunchMode( mode );
    launcher->addMode( mode );
    mode = new CacheGrindLaunchMode();
    KDevelop::ICore::self()->runController()->addLaunchMode( mode );
    launcher->addMode( mode );
    
    // Add launcher for native apps
    KDevelop::LaunchConfigurationType* type = core()->runController()->launchConfigurationTypeForId( iface->nativeAppConfigTypeId() );
    Q_ASSERT(type);
    type->addLauncher( launcher );
}

ValgrindPlugin::~ValgrindPlugin()
{
}

void ValgrindPlugin::runCacheGrind()
{
    core()->runController()->executeDefaultLaunch( "valgrind_cachegrind" );
}


void ValgrindPlugin::runCallGrind()
{
    core()->runController()->executeDefaultLaunch( "valgrind_callgrind" );
}


void ValgrindPlugin::runHelGrind()
{
    core()->runController()->executeDefaultLaunch( "valgrind_helgrind" );
}


void ValgrindPlugin::runMemCheck()
{
    core()->runController()->executeDefaultLaunch( "valgrind_memcheck" );
}


void ValgrindPlugin::loadOutput()
{
#if 0
    QString fName = KFileDialog::getOpenFileName(QString(), "*", 0, i18n("Open Valgrind Output"));
    if ( fName.isEmpty() )
        return;

    QFile f( fName );
    if ( !f.open( QIODevice::ReadOnly ) ) {
        KMessageBox::sorry( 0, i18n("Could not open valgrind output: %1", fName) );
        return;
    }

    clear();

    /*FIXME
    QTextStream stream( &f );
    while ( !stream.atEnd() ) {
        receivedString( stream.readLine() + "\n" );
    }*/
    f.close();
#endif
}

#if 0
void ValgrindPlugin::slotExecCalltree()
{
    /* FIXME add a mainProgram function or equivalent so this can be ported
    ValgrindDialog* dlg = new ValgrindDialog(ValgrindDialog::Calltree);
    if ( KDevApi::self()->project() && m_lastExec.isEmpty() ) {
        dlg->setExecutable( project()->mainProgram() );
    } else {
        dlg->setExecutable( m_lastExec );
    }
    dlg->setParameters( m_lastParams );
    dlg->setCtExecutable( m_lastCtExec );
    dlg->setKcExecutable( m_lastKcExec );
    dlg->setCtParams( m_lastCtParams );
    kcInfo.runKc = true;
    kcInfo.kcPath = dlg->kcExecutable();
//  kcInfo.kcWorkDir = KUrl(dlg->executableName()).directory();
    if ( dlg->exec() == QDialog::Accepted ) {
        runValgrind( dlg->executableName(), dlg->parameters(), dlg->ctExecutable(), dlg->ctParams() );
    }
    m_lastKcExec = dlg->kcExecutable();
    m_lastCtExec = dlg->ctExecutable();
    m_lastCtParams = dlg->ctParams();
    */
}
#endif

#if 0
void ValgrindPlugin::restorePartialProjectSession( const QDomElement* el )
{
    QDomElement execElem = el->namedItem( "executable" ).toElement();
    m_lastExec = execElem.attribute( "path", "" );
    m_lastParams = execElem.attribute( "params", "" );

    QDomElement valElem = el->namedItem( "valgrind" ).toElement();
    m_lastValExec = valElem.attribute( "path", "" );
    m_lastValParams = valElem.attribute( "params", "" );

    QDomElement ctElem = el->namedItem( "calltree" ).toElement();
    m_lastCtExec = ctElem.attribute( "path", "" );
    m_lastCtParams = ctElem.attribute( "params", "" );

    QDomElement kcElem = el->namedItem( "kcachegrind" ).toElement();
    m_lastKcExec = kcElem.attribute( "path", "" );
}

void ValgrindPlugin::savePartialProjectSession( QDomElement* el )
{
    QDomDocument domDoc = el->ownerDocument();
    if ( domDoc.isNull() )
        return;

    QDomElement execElem = domDoc.createElement( "executable" );
    execElem.setAttribute( "path", m_lastExec );
    execElem.setAttribute( "params", m_lastParams );

    QDomElement valElem = domDoc.createElement( "valgrind" );
    valElem.setAttribute( "path", m_lastValExec );
    valElem.setAttribute( "params", m_lastValParams );

    QDomElement ctElem = domDoc.createElement( "calltree" );
    ctElem.setAttribute( "path", m_lastCtExec );
    ctElem.setAttribute( "params", m_lastCtParams );

    QDomElement kcElem = domDoc.createElement( "kcachegrind" );
    kcElem.setAttribute( "path", m_lastKcExec );

    el->appendChild( execElem );
    el->appendChild( valElem );
    el->appendChild( ctElem );
    el->appendChild( kcElem );
}
#endif

#include "valgrindplugin.moc"
