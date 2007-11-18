/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnoutputmodel.h"
#include "kdevsvnplugin.h"
#include "icore.h"
#include "idocumentcontroller.h"

#include <QModelIndex>
#include <QStringList>

SvnOutputItem::SvnOutputItem( const QString &path, const QString &msg )
    : QStandardItem(msg)
    , m_path(path), m_msg(msg), m_stop(false)
{
    //conflict
    if( m_msg.contains("Conflicted") ){
        setData( QVariant( SvnOutputModel::Conflict ) );
        m_stop = true;
    }
}

SvnOutputItem::~SvnOutputItem()
{}

bool SvnOutputItem::stopHere()
{
    return m_stop;
}

SvnOutputModel::SvnOutputModel( KDevSvnPlugin *part, QObject *parent )
    : QStandardItemModel( parent )
    , m_part(part)
{}
SvnOutputModel::~SvnOutputModel()
{}

void SvnOutputModel::activate( const QModelIndex& index )
{
    QStandardItem *item = itemFromIndex( index );
    SvnOutputItem *svnItem = dynamic_cast<SvnOutputItem*>(item);
    if(!item) return;

    if( svnItem->m_path.isEmpty() )
        return;

    m_part->core()->documentController()->openDocument( KUrl(svnItem->m_path) );
}

QModelIndex SvnOutputModel::nextHighlightIndex( const QModelIndex& currentIdx )
{
    int rowCount = this->rowCount();

    bool reachedEnd = false;

    // determine from which index we should start
    int i=0;
    if( currentIdx.isValid() )
        i = currentIdx.row();

    if( i >= rowCount - 1 )
        i = 0;
    else
        i++;

    for( ; i < rowCount; i++ )
    {
        QStandardItem *stditem = item( i );
        SvnOutputItem *outItem = dynamic_cast<SvnOutputItem*>( stditem );
        if( outItem && outItem->stopHere() )
        {
            // yes. found.
            QModelIndex modelIndex = outItem->index();
            return modelIndex;
        }

        if( i >= rowCount - 1 ) // at the last index and couldn't find error yet.
        {
            if( reachedEnd )
            {
                break; // no matching item
            }
            else
            {
                reachedEnd = true;
                i = -1; // search from index 0
            }
        }
    }
    return QModelIndex();
}

QModelIndex SvnOutputModel::previousHighlightIndex( const QModelIndex& currentIdx )
{
    int rowCount = this->rowCount();

    bool reachedFirst = false;

    // determine from which index we should start
    int i = rowCount -1;
    if( currentIdx.isValid() )
        i = currentIdx.row();

    if( ( i > rowCount - 1 ) || ( i == 0 ) )
        i = rowCount-1; // set to last index
    else
        i--;

    for( ; i >= 0; i-- )
    {
        QStandardItem *stditem = item( i );
        SvnOutputItem *outItem = dynamic_cast<SvnOutputItem*>( stditem );
        if( outItem && outItem->stopHere() )
        {
            // yes. found.
            QModelIndex modelIndex = outItem->index();
            return modelIndex;
        }
        if( i <= 0 ) // at the last index and couldn't find error yet.
        {
            if( reachedFirst )
            {
                break; // no matching item
            }
            else
            {
                reachedFirst = true;
                i = rowCount; // search from last index
            }
        }
    }
    return QModelIndex();
}

#include "svnoutputmodel.moc"


