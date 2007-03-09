/*
 * This file is part of KDevelop
 *
 * Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
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

#ifndef KDEVCODEMODEL_H
#define KDEVCODEMODEL_H

#include <kdevitemmodel.h>
#include <ktexteditor/cursor.h>
#include <ktexteditor/rangefeedback.h>

#include <QHash>
#include <QTime>
#include <QMutex>
#include "kdevexport.h"

namespace KTextEditor { class SmartRange; }

namespace KDevelop
{

class CodeItem;
class CodeModel;

class KDEVPLATFORM_EXPORT CodeItem: public ItemCollection, public KTextEditor::SmartRangeWatcher
{
public:
    CodeItem( const QString &name, ItemGroup *parent = 0 );
    virtual ~CodeItem();

    virtual CodeItem *itemAt( int index ) const;

    virtual int kind() const
    {
        return -1;
    }
    virtual QString display() const
    {
        return name();
    }
    virtual QIcon decoration() const
    {
        return QIcon();
    }

    virtual bool isSimilar( CodeItem *other, bool strict = true ) const
    {
        if ( strict && kind() != other->kind() )
            return false;
        if ( name() != other->name() )
            return false;
        if ( scope() != other->scope() )
            return false;
        return true;
    }

    virtual QStringList scope() const
    {
        return QStringList();
    }

    virtual uint hashKey() const
    {
        return qHash( QString::number( kind() )
                      + "." + scope().join( "." )
                      + "." + display() );
    }

    virtual KTextEditor::Cursor startPosition() const
    {
        return KTextEditor::Cursor::invalid();
    }
    virtual KTextEditor::Cursor endPosition() const
    {
        return KTextEditor::Cursor::invalid();
    }

    virtual QString fileName() const
    {
        return QString();
    }
    virtual QTime timestamp() const
    {
        return QTime();
    }

    virtual CodeModel *model() const
    {
        return 0L;
    }

    const QList<KTextEditor::SmartRange*>& references() const;

    void addReference(KTextEditor::SmartRange* range);

    KTextEditor::SmartRange* declaration() const;
    void setDeclaration(KTextEditor::SmartRange* range);

    KTextEditor::SmartRange* definition() const;
    void setDefinition(KTextEditor::SmartRange* range);

    /// Override to detect deleted ranges
    virtual void deleted(KTextEditor::SmartRange* range);

private:
    QList<KTextEditor::SmartRange*> m_references;
    KTextEditor::SmartRange* m_declaration;
    KTextEditor::SmartRange* m_definition;
};

class KDEVPLATFORM_EXPORT CodeModel: public ItemModel
{
    Q_OBJECT
public:
    CodeModel( QObject *parent = 0 );
    virtual ~CodeModel();

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    void beginAppendItem( CodeItem *item,
                          ItemCollection *collection = 0 );
    void endAppendItem();
    void beginRemoveItem( CodeItem *item );
    void endRemoveItem();

    virtual CodeItem *item( const QModelIndex &index ) const;

private:
    mutable QMutex m_mutex;
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
