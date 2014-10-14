/***************************************************************************
 *   Copyright 2006 Alexander Dymo <adymo@kdevelop.org>             *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "kdevfilemanagerplugin.h"

#include <QTimer>

#include <KLocalizedString>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kactioncollection.h>
#include <kaboutdata.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include "filemanager.h"

K_PLUGIN_FACTORY_WITH_JSON(KDevFileManagerFactory, "kdevfilemanager.json", registerPlugin<KDevFileManagerPlugin>();)

class KDevFileManagerViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevFileManagerViewFactory(KDevFileManagerPlugin *plugin): m_plugin(plugin) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        Q_UNUSED(parent)
        return new FileManager(m_plugin,parent);
    }

    QList<QAction*> toolBarActions( QWidget* w ) const
    {
        FileManager* m = qobject_cast<FileManager*>(w);
        if( m )
            return m->toolBarActions();
        return KDevelop::IToolViewFactory::toolBarActions( w );
    }

    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::LeftDockWidgetArea;
    }

    virtual QString id() const
    {
        return "org.kdevelop.FileManagerView";
    }

    virtual bool allowMultiple() const
    {
        return true;
    }

private:
    KDevFileManagerPlugin *m_plugin;
};

KDevFileManagerPlugin::KDevFileManagerPlugin(QObject *parent, const QVariantList &/*args*/)
    :KDevelop::IPlugin("kdevfilemanager", parent)
{
    setXMLFile("kdevfilemanager.rc");

    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

void KDevFileManagerPlugin::init()
{
    m_factory = new KDevFileManagerViewFactory(this);
    core()->uiController()->addToolView(i18n("Filesystem"), m_factory);
}

KDevFileManagerPlugin::~KDevFileManagerPlugin()
{
}

void KDevFileManagerPlugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

#include "kdevfilemanagerplugin.moc"

