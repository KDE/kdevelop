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

K_PLUGIN_FACTORY(KDevQuickopenFactory, registerPlugin<QuickopenPart>(); )
K_EXPORT_PLUGIN(KDevQuickopenFactory("kdevquickopen"))

QuickopenPart::QuickopenPart(QObject *parent,
                                 const QVariantList&)
    : KDevelop::IPlugin(KDevQuickopenFactory::componentData(), parent)
{
    setXMLFile( "kdevquickopen.rc" );
    ///@todo register shortcuts in all main-windows+-
    KActionCollection* actions = core()->uiController()->activeMainWindow()->actionCollection();
    KAction* quickOpen = new KAction("Quickopen", this);
    KAction* quickOpenFile = new KAction("Quickopen File",this);
    KAction* quickOpenClass = new KAction("Quickopen Class",this);
    KAction* quickOpenFunction = new KAction("Quickopen Function",this);

    ///@todo are these configurable now?
    quickOpen->setShortcut( KShortcut( QKeySequence( Qt::CTRL | Qt::ALT | Qt::Key_Q ) ) );
    quickOpenClass->setShortcut( KShortcut( QKeySequence( Qt::CTRL | Qt::ALT | Qt::Key_C ) ) );
    quickOpenFunction->setShortcut( KShortcut( QKeySequence( Qt::CTRL | Qt::ALT | Qt::Key_M ) ) );
    quickOpenFile->setShortcut( KShortcut( QKeySequence( Qt::CTRL | Qt::ALT | Qt::Key_O ) ) );

    actions->addAction("Quickopen", quickOpen);
    actions->addAction("Quickopen Class", quickOpenClass);
    actions->addAction("Quickopen File", quickOpenFile);
    actions->addAction("Quickopen Function", quickOpenFunction);

    connect(quickOpen, SIGNAL("triggered(Qt::MouseButtons, Qt::KeyboardModifiers)"), this, SLOT("quickOpen()"));
    connect(quickOpenClass, SIGNAL("triggered(Qt::MouseButtons, Qt::KeyboardModifiers)"), this, SLOT("quickOpenClass()"));
    connect(quickOpenFunction, SIGNAL("triggered(Qt::MouseButtons, Qt::KeyboardModifiers)"), this, SLOT("quickOpenFunction()"));
    connect(quickOpenFile, SIGNAL("triggered(Qt::MouseButtons, Qt::KeyboardModifiers)"), this, SLOT("quickOpenFile()"));
}

QuickopenPart::~QuickopenPart()
{
}

void QuickopenPart::unload()
{
}

void QuickopenPart::quickOpen()
{
  Q_ASSERT(0);
  QDialog* d = new QDialog( core()->uiController()->activeMainWindow() );
  d->setAttribute( Qt::WA_DeleteOnClose, true );
  Ui::Quickopen o;
  o.setupUi( d );
  d->show();
}

void QuickopenPart::quickOpenFile()
{
  quickOpen();
}

void QuickopenPart::quickOpenFunction()
{
  quickOpen();
}

void QuickopenPart::quickOpenClass()
{
  quickOpen();
}

#include "quickopen_part.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
