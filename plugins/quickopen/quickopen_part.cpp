/*
 * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include "quickopen_part.h"

#include <QtGui/QTreeView>
#include <QtGui/QHeaderView>
#include <QDialog>

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kparts/mainwindow.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kshortcut.h>

#include <icore.h>
#include <iuicontroller.h>

#include "ui_quickopen.h"
#include "quickopenmodel.h"

K_PLUGIN_FACTORY(KDevQuickOpenFactory, registerPlugin<QuickOpenPart>(); )
K_EXPORT_PLUGIN(KDevQuickOpenFactory("kdevquickopen"))

QuickOpenPart::QuickOpenPart(QObject *parent,
                                 const QVariantList&)
    : KDevelop::IPlugin(KDevQuickOpenFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IQuickOpen )
    setXMLFile( "kdevquickopen.rc" );
    ///@todo Make the whole thing work using the own action-collection, it simply doesn't
    m_model = new QuickOpenModel();

    KActionCollection* actions = actionCollection();

    ///@todo All the locally created actions do not work,, also not if they are put into actionCollection()
    QAction* quickOpen = actions->addAction("quick_open");
    QAction* quickOpenFile = actions->addAction("quick_open_file");
    QAction* quickOpenClass = actions->addAction("quick_open_class");
    QAction* quickOpenFunction = actions->addAction("quick_open_function");

    quickOpen->setText( i18n("&Quick Open") );
    quickOpenClass->setText( i18n("Quick Open &Class") );
    quickOpenFunction->setText( i18n("Quick Open &Function") );
    quickOpenFile->setText( i18n("Quick Open &File") );
    
    quickOpen->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_Q );
    quickOpenClass->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_C );
    quickOpenFunction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_M );
    quickOpenFile->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_O );

    connect(quickOpen, SIGNAL(triggered(bool)), this, SLOT(quickOpen()));
    connect(quickOpenClass, SIGNAL(triggered(bool)), this, SLOT(quickOpenClass()));
    connect(quickOpenFunction, SIGNAL(triggered(bool)), this, SLOT(quickOpenFunction()));
    connect(quickOpenFile, SIGNAL(triggered(bool)), this, SLOT(quickOpenFile()));
}

QuickOpenPart::~QuickOpenPart()
{
  delete m_model;
}

void QuickOpenPart::unload()
{
}

void QuickOpenPart::showQuickOpen( ModelTypes modes )
{
  QDialog* d = new QDialog( core()->uiController()->activeMainWindow() );
  d->setAttribute( Qt::WA_DeleteOnClose, true );
  Ui::QuickOpen o;
  o.setupUi( d );
  m_model->restart();
  o.list->setModel( m_model );
  d->show();
}

void QuickOpenPart::quickOpen()
{
  showQuickOpen( All );
}

void QuickOpenPart::quickOpenFile()
{
  showQuickOpen( Files );
}

void QuickOpenPart::quickOpenFunction()
{
  showQuickOpen( Functions );
}

void QuickOpenPart::quickOpenClass()
{
  showQuickOpen( Classes );
}

void QuickOpenPart::registerProvider( const QString& name, KDevelop::QuickOpenDataProviderBase* provider )
{
  m_model->registerProvider( name, provider );
}

bool QuickOpenPart::removeProvider( KDevelop::QuickOpenDataProviderBase* provider )
{
  m_model->removeProvider( provider );
}


#include "quickopen_part.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
