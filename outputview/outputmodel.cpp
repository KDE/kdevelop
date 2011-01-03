/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
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

#include "outputmodel.h"

#include <QtCore/QStringList>
#include <QtCore/QTimer>

#include <kglobalsettings.h>

namespace KDevelop
{

struct OutputModelPrivate
{
    QTimer* timer;
    QVector<QStandardItem*> pending;
    
    static const int MAX_SIZE;
    static const int INTERVAL_MS;
};

const int OutputModelPrivate::MAX_SIZE=10000;
const int OutputModelPrivate::INTERVAL_MS=50;

OutputModel::OutputModel( QObject* parent )
    : QStandardItemModel( parent ), d(new OutputModelPrivate)
{
    d->timer = new QTimer(this);
    d->timer->setInterval(OutputModelPrivate::INTERVAL_MS);
    d->timer->setSingleShot(true);
    
    d->pending.reserve(OutputModelPrivate::MAX_SIZE);
    
    connect(d->timer, SIGNAL(timeout()), SLOT(addPending()));
}

OutputModel::~OutputModel()
{
    addPending();
    delete d;
}

void OutputModel::appendLine( const QString& line )
{
    QStandardItem* item = new QStandardItem( line );
    item->setFont( KGlobalSettings::fixedFont() );
    
    d->pending.append(item);
    if(d->pending.size()<OutputModelPrivate::MAX_SIZE)
        d->timer->start();
    else
        addPending();
}

void OutputModel::appendLines( const QStringList& lines)
{
    Q_FOREACH( const QString& s, lines )
    {
        appendLine( s );
    }
}

void OutputModel::addPending()
{
    if(!d->pending.isEmpty())
        invisibleRootItem()->appendRows(d->pending.toList());
    
    d->pending.clear();
}

}

#include "outputmodel.moc"
