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
#include <QFont>
#include <QApplication>

#include <KDebug>
#include <KGlobalSettings>


#include <set>

Q_DECLARE_METATYPE(QVector<KDevelop::FilteredItem>)
Q_DECLARE_METATYPE(KDevelop::IFilterStrategy*)

namespace KDevelop
{

/**
 * Number of lines that are processed in one go before we notify the GUI thread
 * about the result. It is generally faster to add multiple items to a model
 * in one go compared to adding each item independently.
 */
static const int BATCH_SIZE = 50;

/**
 * Time in ms that we wait in the parse worker for new incoming lines before
 * actually processing them. If we already have enough for one batch though
 * we process immediately.
 */
static const int BATCH_AGGREGATE_TIME_DELAY = 50;

class ParseWorker : public QObject
{
    Q_OBJECT
public:
    ParseWorker()
        : QObject(0)
        , m_filter( new NoFilterStrategy )
        , m_timer(new QTimer(this))
    {
        m_timer->setInterval(BATCH_AGGREGATE_TIME_DELAY);
        m_timer->setSingleShot(true);
        connect(m_timer, SIGNAL(timeout()), SLOT(process()));
    }

public slots:
    void changeFilterStrategy( KDevelop::IFilterStrategy* newFilterStrategy )
    {
        m_filter = QSharedPointer<IFilterStrategy>( newFilterStrategy );
    }

    void addLines( const QStringList& lines )
    {
        m_cachedLines << lines;

        if (m_cachedLines.size() >= BATCH_SIZE) {
            // if enough lines were added, process immediately
            m_timer->stop();
            process();
        } else if (!m_timer->isActive()) {
            m_timer->start();
        }
    }

signals:
    void parsedBatch(const QVector<KDevelop::FilteredItem>& filteredItems);

private slots:
    /**
     * Process *all* cached lines, emit parsedBatch for each batch
     */
    void process()
    {
        QVector<KDevelop::FilteredItem> filteredItems;
        filteredItems.reserve(qMin(BATCH_SIZE, m_cachedLines.size()));

        foreach(const QString& line, m_cachedLines) {
            FilteredItem item = m_filter->errorInLine(line);
            if( item.type == FilteredItem::InvalidItem ) {
                item = m_filter->actionInLine(line);
            }

            filteredItems << item;

            if( filteredItems.size() == BATCH_SIZE ) {
                emit parsedBatch(filteredItems);
                filteredItems.clear();
                filteredItems.reserve(qMin(BATCH_SIZE, m_cachedLines.size()));
            }
        }

        // Make sure to emit the rest as well
        if( !filteredItems.isEmpty() ) {
            emit parsedBatch(filteredItems);
        }
        m_cachedLines.clear();
    }

private:
    QSharedPointer<IFilterStrategy> m_filter;
    QStringList m_cachedLines;

    QTimer* m_timer;
};

class ParsingThread
{
public:
    ParsingThread()
    {
        m_thread.setObjectName("OutputFilterThread");
    }
    virtual ~ParsingThread()
    {
        if (m_thread.isRunning()) {
            m_thread.quit();
            m_thread.wait();
        }
    }
    void addWorker(ParseWorker* worker)
    {
        if (!m_thread.isRunning()) {
            m_thread.start();
        }
        worker->moveToThread(&m_thread);
    }
private:
    QThread m_thread;
};

Q_GLOBAL_STATIC(ParsingThread, s_parsingThread);

struct OutputModelPrivate
{
    OutputModelPrivate( OutputModel* model, const KUrl& builddir = KUrl() );
    ~OutputModelPrivate();
    bool isValidIndex( const QModelIndex&, int currentRowCount ) const;

    OutputModel* model;
    ParseWorker* worker;

    QVector<FilteredItem> m_filteredItems;
    // We use std::set because that is ordered
    std::set<int> m_errorItems; // Indices of all items that we want to move to using previous and next
    KUrl m_buildDir;

    void linesParsed(const QVector<KDevelop::FilteredItem>& items)
    {
        model->beginInsertRows( QModelIndex(), model->rowCount(), model->rowCount() + items.size() -  1);

        foreach( const FilteredItem& item, items ) {
            if( item.type == FilteredItem::ErrorItem ) {
                m_errorItems.insert(m_filteredItems.size());
            }
            m_filteredItems << item;
        }

        model->endInsertRows();
    }
};

OutputModelPrivate::OutputModelPrivate( OutputModel* model_, const KUrl& builddir)
: model(model_)
, worker(new ParseWorker )
, m_buildDir( builddir )
{
    qRegisterMetaType<QVector<KDevelop::FilteredItem> >();
    qRegisterMetaType<KDevelop::IFilterStrategy*>();
    s_parsingThread->addWorker(worker);
    model->connect(worker, SIGNAL(parsedBatch(QVector<KDevelop::FilteredItem>)),
                   model, SLOT(linesParsed(QVector<KDevelop::FilteredItem>)));
}

bool OutputModelPrivate::isValidIndex( const QModelIndex& idx, int currentRowCount ) const
{
    return ( idx.isValid() && idx.row() >= 0 && idx.row() < currentRowCount && idx.column() == 0 );
}

OutputModelPrivate::~OutputModelPrivate()
{
    worker->deleteLater();
}

OutputModel::OutputModel( const KUrl& builddir, QObject* parent )
: QAbstractListModel(parent)
, d( new OutputModelPrivate( this, builddir ) )
{
}

OutputModel::OutputModel( QObject* parent )
    : QAbstractListModel(parent)
, d( new OutputModelPrivate( this ) )
{
}

OutputModel::~OutputModel()
{
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
    // TODO: Turn into factory, decouple from OutputModel
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
        case NativeAppErrorFilter:
            filter = new NativeAppErrorFilterStrategy;
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

    QMetaObject::invokeMethod(d->worker, "changeFilterStrategy",
                              Q_ARG(KDevelop::IFilterStrategy*, filter));
}

void OutputModel::appendLines( const QStringList& lines )
{
    if( lines.isEmpty() )
        return;

    QMetaObject::invokeMethod(d->worker, "addLines",
                              Q_ARG(QStringList, lines));
}

void OutputModel::appendLine( const QString& l )
{
    appendLines( QStringList() << l );
}

}

#include "outputmodel.moc"
#include "moc_outputmodel.cpp"
