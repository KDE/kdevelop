/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo <adymo@kdevelop.org>             *
 *   Copyright (C) 2007 by Andreas Pakulat <apaku@gmx.de>                  *
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
#include "kdevfilemanagerpart.h"

#include <QTimer>

#include <klocale.h>
#include <kgenericfactory.h>

#include <icore.h>
#include <iuicontroller.h>
// #include <mainwindow.h>

#include "filemanager.h"

typedef KGenericFactory<KDevFileManagerPart> KDevFileManagerFactory;
K_EXPORT_COMPONENT_FACTORY(kdevfilemanager, KDevFileManagerFactory("kdevfilemanager"))

class KDevFileManagerViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevFileManagerViewFactory(KDevFileManagerPart *part): m_part(part) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        Q_UNUSED(parent)
        return new FileManager(m_part);
    }
private:
    KDevFileManagerPart *m_part;
};

KDevFileManagerPart::KDevFileManagerPart(QObject *parent, const QStringList &/*args*/)
    :KDevelop::IPlugin(KDevFileManagerFactory::componentData(), parent)
{
    setXMLFile("kdevfilemanager.rc");

//     QTimer::singleShot(0, this, SLOT(init()));
    init();
}

void KDevFileManagerPart::init()
{
    m_factory = new KDevFileManagerViewFactory(this);
    core()->uiController()->addToolView("File Manager", m_factory);
}

KDevFileManagerPart::~KDevFileManagerPart()
{
}

Qt::DockWidgetArea KDevFileManagerPart::dockWidgetAreaHint() const
{
    return Qt::LeftDockWidgetArea;
}

void KDevFileManagerPart::registerExtensions()
{
}

void KDevFileManagerPart::unregisterExtensions()
{
}

QStringList KDevFileManagerPart::extensions() const
{
    return QStringList();
}

void KDevFileManagerPart::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

#include "kdevfilemanagerpart.moc"
