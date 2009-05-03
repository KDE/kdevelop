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

#include "valgrindmodel.h"
#include "valgrindcontrol.h"
#include "valgrindwidget.h"

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
    setXMLFile( "kdevvalgrind.rc" );

    core()->uiController()->addToolView(i18n("Valgrind"), new ValgrindWidgetFactory(this));

    KAction* action = new KAction( i18n("&Valgrind Memory Leak Check"), this);
    actionCollection()->addAction("tools_valgrind", action);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_V);
    action->setToolTip(i18n("Valgrind memory leak check and other tools"));
    action->setWhatsThis(i18n("<b>Valgrind memory leak check</b><p>Runs Valgrind &mdash; a tool to help you find memory-management problems in your programs, and which provides other tools.</p>"));
    action->setIcon(KIcon("fork"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotExecValgrind()));
}

ValgrindPlugin::~ValgrindPlugin()
{
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

void ValgrindPlugin::slotExecValgrind()
{
    //TODO: Port to launch framework
    //KDevelop::IRun run = KDevelop::ICore::self()->runController()->defaultRun();
    //run.setInstrumentor("memcheck");
    //KDevelop::ICore::self()->runController()->execute(run);
}

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
