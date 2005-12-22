/*
 *  KDevelop Automake Support
 *  Copyright (C) 2005 Matt Rogers <mattr@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include "automakeprojectmodel.h"

AutoMakeDirItem::AutoMakeDirItem( const QString& name, KDevItemGroup* parent )
    : KDevProjectFolderItem( name, parent )
{
}

AutoMakeDirItem::~AutoMakeDirItem()
{
}


AutoMakeFileItem::AutoMakeFileItem( const QFileInfo& name, KDevItemGroup* parent )
    : KDevProjectFileItem( name, parent )
{
}

AutoMakeFileItem::~AutoMakeFileItem()
{
}

AutoMakeTargetItem::AutoMakeTargetItem( const QString& name, KDevItemGroup* parent )
    : KDevProjectTargetItem( name, parent )
{
}

AutoMakeTargetItem::~AutoMakeTargetItem()
{
}

#include "automakeprojectmodel.h"
// kate: indent-mode csands; space-indent off; tab-width 4; auto-insert-doxygen on;
