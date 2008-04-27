/*
 * This file is part of KDevelop
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "duchainviewplugin.h"
#include "duchainmodel.h"
#include "duchaintree.h"

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <icore.h>
#include <iuicontroller.h>
#include <idocumentcontroller.h>

K_PLUGIN_FACTORY(KDevDUChainViewFactory, registerPlugin<DUChainViewPlugin>(); )
K_EXPORT_PLUGIN(KDevDUChainViewFactory("kdevduchainview"))

class DUChainViewFactory: public KDevelop::IToolViewFactory
{
public:
    DUChainViewFactory(DUChainViewPlugin *plugin): mplugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        QTreeView* view = new DUChainTree(parent, mplugin);
        QObject::connect(view, SIGNAL(doubleClicked(const QModelIndex &)), mplugin->model(), SLOT(doubleClicked(const QModelIndex &)));
        return view;
    }

    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QString id() const
    {
        return "org.kdevelop.DUChainView";
    }

private:
    DUChainViewPlugin *mplugin;
};

DUChainViewPlugin::DUChainViewPlugin(QObject *parent,
                                 const QVariantList&)
    : KDevelop::IPlugin(KDevDUChainViewFactory::componentData(), parent)
    , m_model(new DUChainModel(this))
    , m_factory(new DUChainViewFactory(this))
{
    core()->uiController()->addToolView(i18n("DUChain Viewer"), m_factory);
    setXMLFile( "kdevduchainview.rc" );

    connect(core()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), m_model, SLOT(documentActivated(KDevelop::IDocument*)));
}

DUChainViewPlugin::~DUChainViewPlugin()
{
}

void DUChainViewPlugin::unload()
{
    core()->uiController()->removeToolView(m_factory);
}

DUChainModel* DUChainViewPlugin::model() const
{
    return m_model;
}

#include "duchainviewplugin.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
