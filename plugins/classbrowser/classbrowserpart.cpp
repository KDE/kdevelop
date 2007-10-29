/*
 * This file is part of KDevelop
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#include "classbrowserpart.h"

#include <QTreeView>

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <icore.h>
#include <iuicontroller.h>
#include <idocumentcontroller.h>

#include "classmodel.h"

K_PLUGIN_FACTORY(KDevClassBrowserFactory, registerPlugin<ClassBrowserPart>(); )
K_EXPORT_PLUGIN(KDevClassBrowserFactory("kdevclassbrowser"))

class ClassBrowserFactory: public KDevelop::IToolViewFactory
{
public:
  ClassBrowserFactory(ClassBrowserPart *part): m_part(part) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    QTreeView* view = new QTreeView(parent /*, m_part*/);
    view->setModel(m_part->model());
    //QObject::connect(view, SIGNAL(doubleClicked(const QModelIndex &)), m_part->model(), SLOT(doubleClicked(const QModelIndex &)));
    return view;
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::LeftDockWidgetArea;
  }

private:
  ClassBrowserPart *m_part;
};

ClassBrowserPart::ClassBrowserPart(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(KDevClassBrowserFactory::componentData(), parent)
    , m_factory(new ClassBrowserFactory(this))
    , m_model(new ClassModel(this))
{
  core()->uiController()->addToolView(i18n("Class Browser"), m_factory);
  setXMLFile( "kdevclassbrowser.rc" );

  //connect(core()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), m_model, SLOT(documentActivated(KDevelop::IDocument*)));
}

ClassBrowserPart::~ClassBrowserPart()
{
}

void ClassBrowserPart::unload()
{
  core()->uiController()->removeToolView(m_factory);
}

ClassModel* ClassBrowserPart::model() const
{
  return m_model;
}

#include "classbrowserpart.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
