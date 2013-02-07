/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *   Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com        *
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
#include "filtereditem.h"
#include "outputfilteringstrategies.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QThread>

#include <KDebug>
#include <kglobalsettings.h>

#include <QFont>

Q_DECLARE_METATYPE(QList<KDevelop::FilteredItem>)

namespace KDevelop
{

class ParseWorker : public QObject
{
    Q_OBJECT
public:
    ParseWorker()
        : QObject(0)
        , m_filter( new NoFilterStrategy )
    {
    }

public slots:
    void changeFilterStrategy( IFilterStrategy* newFilterStrategy ) {
        m_filter = QSharedPointer<IFilterStrategy>( newFilterStrategy );
    }
    void addLines( const QStringList& lines ) {
        QList<KDevelop::FilteredItem> filteredItems;
        for(int i = 0; i < lines.size(); ++i) {
            const QString line = lines.at(0);
            FilteredItem item = m_filter->errorInLine(line);
            if( item.type == FilteredItem::InvalidItem ) {
                item = m_filter->actionInLine(line);
            }

            filteredItems << item;
            if( filteredItems.size() == 50 ) {
                emit parsedBatch(filteredItems);
                filteredItems.clear();
            }
        }
        // Make sure to emit the rest as well
        if( filteredItems.size() > 0 ) {
            emit parsedBatch(filteredItems);
        }
    }

signals:
    void parsedBatch(const QList<KDevelop::FilteredItem>& filteredItems);
private:
    QSharedPointer<IFilterStrategy> m_filter;
};

struct OutputModelPrivate
{
    OutputModelPrivate();
    OutputModelPrivate( const KUrl& builddir );
    ~OutputModelPrivate();
    bool isValidIndex( const QModelIndex&, int currentRowCount ) const;
    QList<FilteredItem> m_filteredItems;
    // We use std::set because that is ordered
    std::set<int> m_errorItems; // Indices of all items that we want to move to using previous and next
    KUrl m_buildDir;

    QSharedPointer<QThread> parsingThread;
    QSharedPointer<ParseWorker> worker;
    void setupParser( OutputModel* model ) {
        parsingThread = QSharedPointer<QThread>( new QThread( model ) );
        worker = QSharedPointer<ParseWorker>( new ParseWorker() );
        worker->moveToThread(parsingThread.data());
        model->connect(worker.data(), SIGNAL(parsedBatch(QList<KDevelop::FilteredItem>)), model, SLOT(linesParsed(QList<KDevelop::FilteredItem>)));
        model->connect(model, SIGNAL(linesAdded(QStringList)), worker.data(), SLOT(addLines(QStringList)));
        model->connect(model, SIGNAL(filterStrategyChanged(IFilterStrategy*)), worker.data(), SLOT(changeFilterStrategy(IFilterStrategy*)));
        parsingThread->start();
    }
};

OutputModelPrivate::OutputModelPrivate()
{
}

OutputModelPrivate::OutputModelPrivate(const KUrl& builddir)
: m_buildDir( builddir )
{
}

bool OutputModelPrivate::isValidIndex( const QModelIndex& idx, int currentRowCount ) const
{
    return ( idx.isValid() && idx.row() >= 0 && idx.row() < currentRowCount && idx.column() == 0 );
}


OutputModelPrivate::~OutputModelPrivate()
{
}

OutputModel::OutputModel( const KUrl& builddir, QObject* parent )
: QAbstractListModel(parent)
, d( new OutputModelPrivate( builddir ) )
{
    d->setupParser( this );
}

OutputModel::OutputModel( QObject* parent )
    : QAbstractListModel(parent)
, d( new OutputModelPrivate )
{
    qRegisterMetaType<QList<KDevelop::FilteredItem> >();
    d->setupParser( this );
}

OutputModel::~OutputModel()
{
    d->parsingThread->quit();
    delete d;
}

QVariant OutputModel::data(const QModelIndex& idx , int role ) const
{
    if( d->isValidIndex(idx, rowCount()) )
    {
        switch( role )
        {
            case Qt::DisplayRole:
                return d->m_filteredItems.at( idx.row() ).shortenedText;
                break;
            case OutputModel::OutputItemTypeRole:
                return static_cast<int>(d->m_filteredItems.at( idx.row() ).type);
                break;
            case Qt::FontRole:
                return KGlobalSettings::fixedFont();
                break;
            default:
                break;
        }
    }
    return QVariant();
}

int OutputModel::rowCount( const QModelIndex& parent ) const
{
    if( !parent.isValid() )
        return d->m_filteredItems.count();
    return 0;
}

QVariant OutputModel::headerData( int, Qt::Orientation, int ) const
{
    return QVariant();
}

void OutputModel::activate( const QModelIndex& index )
{
    if( index.model() != this || !d->isValidIndex(index, rowCount()) )
    {
        return;
    }
    kDebug() << "Model activated" << index.row();


    FilteredItem item = d->m_filteredItems.at( index.row() );
    if( item.isActivatable )
    {
        kDebug() << "activating:" << item.lineNo << item.url;
        KTextEditor::Cursor range( item.lineNo, item.columnNo );
        KDevelop::IDocumentController *docCtrl = KDevelop::ICore::self()->documentController();
        KUrl url = item.url;
        if(url.isRelative()) {
            url = KUrl(d->m_buildDir, url.path());
        }
        docCtrl->openDocument( url, range );
    } else {
        kDebug() << "not an activateable item";
    }
}

QModelIndex OutputModel::nextHighlightIndex( const QModelIndex &currentIdx )
{
    int startrow = d->isValidIndex(currentIdx, rowCount()) ? currentIdx.row() + 1 : 0;

    if( !d->m_errorItems.empty() )
    {
        kDebug() << "searching next error";
        // Jump to the next error item
        std::set< int >::const_iterator next = d->m_errorItems.lower_bound( startrow );
        if( next == d->m_errorItems.end() )
            next = d->m_errorItems.begin();

        return index( *next, 0, QModelIndex() );
    }

    for( int row = 0; row < rowCount(); ++row ) 
    {
        int currow = (startrow + row) % rowCount();
        if( d->m_filteredItems.at( currow ).isActivatable )
        {
            return index( currow, 0, QModelIndex() );
        }
    }
    return QModelIndex();
}

QModelIndex OutputModel::previousHighlightIndex( const QModelIndex &currentIdx )
{
    //We have to ensure that startrow is >= rowCount - 1 to get a positive value from the % operation.
    int startrow = rowCount() + (d->isValidIndex(currentIdx, rowCount()) ? currentIdx.row() : rowCount()) - 1;

    if(!d->m_errorItems.empty())
    {
        kDebug() << "searching previous error";

        // Jump to the previous error item
        std::set< int >::const_iterator previous = d->m_errorItems.lower_bound( currentIdx.row() );

        if( previous == d->m_errorItems.begin() )
            previous = d->m_errorItems.end();

        --previous;

        return index( *previous, 0, QModelIndex() );
    }

    for ( int row = 0; row < rowCount(); ++row )
    {
        int currow = (startrow - row) % rowCount();
        if( d->m_filteredItems.at( currow ).isActivatable )
        {
            return index( currow, 0, QModelIndex() );
        }
    }
    return QModelIndex();
}

void OutputModel::setFilteringStrategy(const OutputFilterStrategy& currentStrategy)
{
    IFilterStrategy* filter = 0;
    switch( currentStrategy )
    {
        case NoFilter:
            filter = new NoFilterStrategy;
            break;
        case CompilerFilter:
            filter = new CompilerFilterStrategy( d->m_buildDir );
            break;
        case ScriptErrorFilter:
            filter = new ScriptErrorFilterStrategy;
            break;
        case StaticAnalysisFilter:
            filter = new StaticAnalysisFilterStrategy;
            break;
        default:
            // assert(false);
            filter = new NoFilterStrategy;
            break;
    }
    Q_ASSERT(filter);
    emit filterStrategyChanged( filter );
}

void OutputModel::appendLines( const QStringList& lines )
{
    if( lines.isEmpty() )
        return;
    emit linesAdded( lines );
}

void OutputModel::appendLine( const QString& l )
{
    appendLines( QStringList() << l );
}

void OutputModel::linesParsed(const QList<KDevelop::FilteredItem>& items)
{
    beginInsertRows( QModelIndex(), rowCount(), rowCount() + items.size() -  1);

    Q_FOREACH( FilteredItem item, items ) {
        if( item.type == FilteredItem::ErrorItem ) {
            d->m_errorItems.insert(d->m_filteredItems.size());
        }
        d->m_filteredItems << item;
    }

    endInsertRows();
}

}

#include "outputmodel.moc"
#include "moc_outputmodel.cpp"