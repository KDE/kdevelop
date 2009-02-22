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
#include "documentswitcherplugin.h"

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include <kaboutdata.h>

#include <interfaces/icore.h>

K_PLUGIN_FACTORY(DocumentSwitcherFactory, registerPlugin<DocumentSwitcherPlugin>(); )
K_EXPORT_PLUGIN(DocumentSwitcherFactory(KAboutData("kdevdocumentswitcher","kdevdocumentswitcher",ki18n("Document Switcher"), "0.1", ki18n("Switch between open documents using most-recently-used list"), KAboutData::License_GPL)))


DocumentSwitcherPlugin::DocumentSwitcherPlugin(QObject *parent, const QVariantList &/*args*/)
    :KDevelop::IPlugin(DocumentSwitcherFactory::componentData(), parent)
{
    setXMLFile("kdevdocumentswitcher.rc");

}

DocumentSwitcherPlugin::~DocumentSwitcherPlugin()
{
}

#include "documentswitcherplugin.moc"

