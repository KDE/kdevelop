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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef AUTOMAKEPROJECTMODEL_H
#define AUTOMAKEPROJECTMODEL_H

#include <QList>
#include <QIcon>
#include "kdevprojectmodel.h"

class AutoMakeItem : public KDevProjectItem
{
public:
    AutoMakeItem( const QString& name,  KDevItemGroup* parent = 0 );
    virtual ~AutoMakeItem();

    virtual KDevProjectItem* itemAt( int index ) const;
    virtual KDevProjectFolderItem* folder() const { return 0; }
    virtual KDevProjectTargetItem* target() const { return 0; }
    virtual KDevProjectFileItem* file() const { return 0; }

    virtual QList<KDevProjectFolderItem*> folderList() const
    {
        return QList<KDevProjectFolderItem*>();
    }

    virtual QList<KDevProjectTargetItem*> targetList() const
    {
        return QList<KDevProjectTargetItem*>();
    }

    virtual QList<KDevProjectFileItem*> fileList() const
    {
        return QList<KDevProjectFileItem*>();
    }

};

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
    AutoMakeTargetItem( const QString& name, KDevItemGroup* parent = 0 );
    virtual ~AutoMakeTargetItem();

    virtual KDevProjectTargetItem* target() const
    {
        return const_cast<AutoMakeTargetItem*>( this );
    }


private:
    QString m_prefix;
    QString m_target;
    QList<AutoMakeFileItem*> m_fileList;

};

#endif // AUTOMAKEPROJECTMODEL_H
