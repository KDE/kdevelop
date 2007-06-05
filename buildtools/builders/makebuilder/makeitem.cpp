/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "makeitem.h"
#include <QList>
#include <QAction>
#include <QBrush>
#include <QColor>
#include <QFont>

#include <kdebug.h>
#include <klocale.h>
#include <idocumentcontroller.h>
#include <icore.h>
#include "makebuilder.h"
#include <ktexteditor/cursor.h>

// MakeItem::MakeItem( const QString &text )
//     : IOutputViewItem( text )
// {}
//
// MakeItem::~MakeItem()
// {}
//
// void MakeItem::activated()
// {
// }
//
// QList<QAction*> MakeItem::contextMenuActions()
// {
//     QList<QAction*> actions;
//     return actions;
// }

//////////////////////////////////

MakeWarningItem::MakeWarningItem( const QString &text, const MakeBuilder *builder )
    : IOutputViewItem( text )
    , file( "" ), lineNo(-1), errorText( "" )
    , m_builder(builder)
{}

MakeWarningItem::~MakeWarningItem()
{}

void MakeWarningItem::activate()
{
    KDevelop::IDocumentController *docCtrl = m_builder->core()->documentController();
    if( !docCtrl )
        return;

    KTextEditor::Cursor range(lineNo, 0);
    docCtrl->openDocument( file, range );
    kDebug(9038) << "filename " << file << " lineNo " << lineNo << " errorText " << errorText << endl;
}

QList<QAction*> MakeWarningItem::contextMenuActions()
{
    QList<QAction*> actions;

    // Such code would be one HOWTO regarding how to create actions.
    // But I wonder this is the right approach
//     QAction* errAction = new QAction("jump to error", m_builder);
//     m_builder->setContextItem( this );
//     connect( errAction, SIGNAL(triggered( bool )), m_builder, SLOT( slotItemContextMenu() ) );
//     at the slotItemContextMenu(), retrieve ErrorItem by contextItem() and do appropriate things..

    return actions;
}

//////////////////////////////////

MakeErrorItem::MakeErrorItem( const QString &text, const MakeBuilder *builder )
    : MakeWarningItem( text, builder )
{}

MakeErrorItem::~MakeErrorItem()
{}

IOutputViewItem::StopAtItemMode MakeErrorItem::stopHere()
{
    return IOutputViewItem::Stop;
}

//////////////////////////////////

MakeActionItem::MakeActionItem( const QString& action, const QString& file, const QString& tool, const QString& line )
    : IOutputViewItem( QString() )
{
    if( tool.isEmpty() || file.isEmpty() )
    {
        setText( line );
    }
    else
    {
        QString itemString = QString(action).append(" ").append(file);
        itemString.append(" (").append(tool).append(")");

        setText( itemString );
    }

    QFont newfont( font() );
    newfont.setBold( true );
    setFont( newfont );
}

MakeActionItem::~MakeActionItem()
{}

