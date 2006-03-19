/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef AUTOMAKEPROJECTMODEL_H
#define AUTOMAKEPROJECTMODEL_H

#include <QList>
#include <QIcon>
#include "kdevprojectmodel.h"
#include "makefileinterface.h"


class AutoMakeDirItem : public KDevProjectFolderItem
{
public:
    AutoMakeDirItem( const QString& name, KDevItemGroup* parent = 0 );
    virtual ~AutoMakeDirItem();

    virtual KDevProjectFolderItem* folder() const
    {
        return const_cast<AutoMakeDirItem*>( this );
    }
};

class AutoMakeFileItem : public KDevProjectFileItem
{
public:
    AutoMakeFileItem( const QFileInfo& fileInfo, KDevItemGroup* parent = 0 );
    virtual ~AutoMakeFileItem();

    virtual KDevProjectFileItem* fileItem() const
    {
        return const_cast<AutoMakeFileItem*>( this );
    }

    virtual QIcon icon() const { return QIcon(); }

};

class AutoMakeTargetItem : public KDevProjectTargetItem
{
public:
    AutoMakeTargetItem( const TargetInfo& target, KDevItemGroup* parent = 0 );
    virtual ~AutoMakeTargetItem();

    virtual KDevProjectTargetItem* target() const
    {
        return const_cast<AutoMakeTargetItem*>( this );
    }


private:
    TargetInfo m_target;
};

#endif // AUTOMAKEPROJECTMODEL_H
// kate: indent-mode csands; space-indent on; indent-width 4; replace-tabs on;

