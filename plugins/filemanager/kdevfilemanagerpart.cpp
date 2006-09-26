/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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

#include <kdevcore.h>
#include <kdevmainwindow.h>

#include "filemanager.h"

typedef KGenericFactory<KDevFileManagerPart> KDevFileManagerFactory;
K_EXPORT_COMPONENT_FACTORY(kdevfilemanager, KDevFileManagerFactory("kdevfilemanager"))

KDevFileManagerPart::KDevFileManagerPart(QObject *parent, const QStringList &/*args*/)
    :KDevPlugin(KDevFileManagerFactory::instance(), parent)
{
    setXMLFile("kdevfilemanager.rc");

//     QTimer::singleShot(0, this, SLOT(init()));
    init();
}

void KDevFileManagerPart::init()
{
    m_view = new FileManager(this);
}

KDevFileManagerPart::~KDevFileManagerPart()
{
    delete m_view;
}

Qt::DockWidgetArea KDevFileManagerPart::dockWidgetAreaHint() const
{
    return Qt::LeftDockWidgetArea;
}

QWidget *KDevFileManagerPart::pluginView() const
{
    return m_view;
}

#include "kdevfilemanagerpart.moc"
