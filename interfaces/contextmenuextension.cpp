/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
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

#include "contextmenuextension.h"

#include <QtCore/QMap>

#include <QMenu>
#include <QAction>

#include <KLocalizedString>

namespace KDevelop
{

const QString ContextMenuExtension::FileGroup     = QStringLiteral("FileGroup");
const QString ContextMenuExtension::RefactorGroup = QStringLiteral("RefactorGroup");
const QString ContextMenuExtension::BuildGroup    = QStringLiteral("BuildGroup");
const QString ContextMenuExtension::RunGroup      = QStringLiteral("RunGroup");
const QString ContextMenuExtension::DebugGroup    = QStringLiteral("DebugGroup");
const QString ContextMenuExtension::EditGroup     = QStringLiteral("EditGroup");
const QString ContextMenuExtension::VcsGroup      = QStringLiteral("VcsGroup");
const QString ContextMenuExtension::ProjectGroup  = QStringLiteral("ProjectGroup");
const QString ContextMenuExtension::OpenEmbeddedGroup  = QStringLiteral("OpenEmbeddedGroup");
const QString ContextMenuExtension::OpenExternalGroup  = QStringLiteral("OpenExternalGroup");
const QString ContextMenuExtension::AnalyzeGroup = QStringLiteral("AnalyzeGroup");
const QString ContextMenuExtension::ExtensionGroup  = QStringLiteral("ExtensionGroup");


class ContextMenuExtensionPrivate
{
public:
    QMap<QString,QList<QAction*> > extensions;
};

ContextMenuExtension::ContextMenuExtension()
    : d(new ContextMenuExtensionPrivate)
{
}

ContextMenuExtension::~ContextMenuExtension()
{
    delete d;
}


ContextMenuExtension::ContextMenuExtension( const ContextMenuExtension& rhs )
    : d( new ContextMenuExtensionPrivate )
{
    d->extensions = rhs.d->extensions;
}

ContextMenuExtension& ContextMenuExtension::operator=( const ContextMenuExtension& rhs )
{
    if( this == &rhs )
        return *this;

    d->extensions = rhs.d->extensions;
    return *this;
}

QList<QAction*> ContextMenuExtension::actions( const QString& group ) const
{
    return d->extensions.value( group, QList<QAction*>() );
}

void ContextMenuExtension::addAction( const QString& group, QAction* action )
{
    if( !d->extensions.contains( group ) )
    {
        d->extensions.insert( group, QList<QAction*>() << action );
    } else
    {
        d->extensions[group].append( action );
    }
}

void ContextMenuExtension::populateMenu(QMenu* menu, const QList<ContextMenuExtension>& extensions)
{
    QList<QAction*> buildActions;
    QList<QAction*> vcsActions;
    QList<QAction*> extActions;
    QList<QAction*> refactorActions;
    QList<QAction*> debugActions;
    QList<QAction*> analyzeActions;
    foreach( const ContextMenuExtension &ext, extensions )
    {
        foreach( QAction* act, ext.actions( ContextMenuExtension::BuildGroup ) )
        {
            buildActions << act;
        }

        foreach( QAction* act, ext.actions( ContextMenuExtension::VcsGroup ) )
        {
            vcsActions << act;
        }

        foreach( QAction* act, ext.actions( ContextMenuExtension::AnalyzeGroup ) )
        {
            analyzeActions << act;
        }

        foreach( QAction* act, ext.actions( ContextMenuExtension::ExtensionGroup ) )
        {
            extActions << act;
        }

        foreach( QAction* act, ext.actions( ContextMenuExtension::RefactorGroup ) )
        {
            refactorActions << act;
        }

        foreach( QAction* act, ext.actions( ContextMenuExtension::DebugGroup ) )
        {
            debugActions << act;
        }
    }
    
    if(!buildActions.isEmpty())
    {
        foreach(QAction* action, buildActions)
            menu->addAction(action);
        
        menu->addSeparator();
    }

    foreach( const ContextMenuExtension &ext, extensions )
    {
        foreach( QAction* act, ext.actions( ContextMenuExtension::FileGroup ) )
        {
            menu->addAction( act );
        }
        menu->addSeparator();
        foreach( QAction* act, ext.actions( ContextMenuExtension::EditGroup ) )
        {
            menu->addAction( act );
        }
    }
    
    QMenu* debugmenu = menu;
    if( debugActions.count() > 1 )
    {
        debugmenu = menu->addMenu( i18n("Debug") );
    }
    foreach( QAction* act, debugActions )
    {
        debugmenu->addAction( act );
    }
    menu->addSeparator();

    QMenu* refactormenu = menu;
    if( refactorActions.count() > 1 )
    {
        refactormenu = menu->addMenu( i18n("Refactor") );
    }
    foreach( QAction* act, refactorActions )
    {
        refactormenu->addAction( act );
    }
    menu->addSeparator();

    QMenu* vcsmenu = menu;
    if( vcsActions.count() > 1 )
    {
        vcsmenu = menu->addMenu( i18n("Version Control") );
    }
    foreach( QAction* act, vcsActions )
    {
        vcsmenu->addAction( act );
    }
    menu->addSeparator();

    QMenu* analyzeMenu = menu;
    if( analyzeActions.count() > 1 )
    {
        analyzeMenu = menu->addMenu( i18n("Analyze With") );
        analyzeMenu->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok")));
    }
    foreach( QAction* act, analyzeActions )
    {
        analyzeMenu->addAction( act );
    }
    menu->addSeparator();

    foreach( QAction* act, extActions )
    {
        menu->addAction( act );
    }
}

}
