/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "outputmodel.h"
#include "filtereditem.h"
#include "outputfilteringstrategies.h"
#include "debug.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <util/kdevstringhandler.h>

#include <QStringList>
#include <QTimer>
#include <QThread>
#include <QFont>
#include <QFontDatabase>

#include <functional>
#include <set>

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
        : QObject(nullptr)
        , m_filter(new NoFilterStrategy)
        , m_timer(new QTimer(this))
    {
        m_timer->setInterval(BATCH_AGGREGATE_TIME_DELAY);
        m_timer->setSingleShot(true);
        connect(m_timer, &QTimer::timeout, this, &ParseWorker::process);
    }

public Q_SLOTS:
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

    void flushBuffers()
    {
        m_timer->stop();
        process();
        emit allDone();
    }

Q_SIGNALS:
    void parsedBatch(const QVector<KDevelop::FilteredItem>& filteredItems);
    void progress(const KDevelop::IFilterStrategy::Progress& progress);
    void allDone();

private Q_SLOTS:
    /**
     * Process *all* cached lines, emit parsedBatch for each batch
     */
    void process()
    {
        QVector<KDevelop::FilteredItem> filteredItems;
        filteredItems.reserve(qMin(BATCH_SIZE, m_cachedLines.size()));

        // apply pre-filtering functions
        std::transform(m_cachedLines.constBegin(), m_cachedLines.constEnd(),
                       m_cachedLines.begin(), &KDevelop::stripAnsiSequences);

        // apply filtering strategy
        for (const QString& line : std::as_const(m_cachedLines)) {
            FilteredItem item = m_filter->errorInLine(line);
            if( item.type == FilteredItem::InvalidItem ) {
                item = m_filter->actionInLine(line);
            }

            filteredItems << item;

            auto progress = m_filter->progressInLine(line);
            if (progress.percent >= 0 && m_progress.percent != progress.percent) {
                m_progress = progress;
                emit this->progress(m_progress);
            }

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
    IFilterStrategy::Progress m_progress;
};

class ParsingThread
{
public:
    ParsingThread()
    {
        m_thread.setObjectName(QStringLiteral("OutputFilterThread"));
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

Q_GLOBAL_STATIC(ParsingThread, s_parsingThread)

class OutputModelPrivate
{
public:
    explicit OutputModelPrivate( OutputModel* model, const QUrl& builddir = QUrl() );
    ~OutputModelPrivate();
    bool isValidIndex( const QModelIndex&, int currentRowCount ) const;

    OutputModel* model;
    ParseWorker* worker;

    QVector<FilteredItem> m_filteredItems;
    // We use std::set because that is ordered
    std::set<int> m_errorItems; // Indices of all items that we want to move to using previous and next
    QUrl m_buildDir;

    void linesParsed(const QVector<KDevelop::FilteredItem>& items)
    {
        model->beginInsertRows( QModelIndex(), model->rowCount(), model->rowCount() + items.size() -  1);

        m_filteredItems.reserve(m_filteredItems.size() + items.size());
        for (const FilteredItem& item : items) {
            if( item.type == FilteredItem::ErrorItem ) {
                m_errorItems.insert(m_filteredItems.size());
            }
            m_filteredItems << item;
        }

        model->endInsertRows();
    }
};

OutputModelPrivate::OutputModelPrivate( OutputModel* model_, const QUrl& builddir)
: model(model_)
, worker(new ParseWorker )
, m_buildDir( builddir )
{
    qRegisterMetaType<QVector<KDevelop::FilteredItem> >();
    qRegisterMetaType<KDevelop::IFilterStrategy*>();
    qRegisterMetaType<KDevelop::IFilterStrategy::Progress>();

    s_parsingThread->addWorker(worker);
    model->connect(worker, &ParseWorker::parsedBatch,
                   model, [=] (const QVector<KDevelop::FilteredItem>& items) { linesParsed(items); });
    model->connect(worker, &ParseWorker::allDone,
                   model, &OutputModel::allDone);
    model->connect(worker, &ParseWorker::progress,
                   model, &OutputModel::progress);
}

bool OutputModelPrivate::isValidIndex( const QModelIndex& idx, int currentRowCount ) const
{
    return ( idx.isValid() && idx.row() >= 0 && idx.row() < currentRowCount && idx.column() == 0 );
}

OutputModelPrivate::~OutputModelPrivate()
{
    worker->deleteLater();
}

OutputModel::OutputModel( const QUrl& builddir, QObject* parent )
: QAbstractListModel(parent)
, d_ptr(new OutputModelPrivate(this, builddir))
{
}

OutputModel::OutputModel( QObject* parent )
    : QAbstractListModel(parent)
    , d_ptr(new OutputModelPrivate(this))
{
}

OutputModel::~OutputModel() = default;

QVariant OutputModel::data(const QModelIndex& idx , int role ) const
{
    Q_D(const OutputModel);

    if( d->isValidIndex(idx, rowCount()) )
    {
        switch( role )
        {
            case Qt::DisplayRole:
                return d->m_filteredItems.at( idx.row() ).originalLine;
            case OutputModel::OutputItemTypeRole:
                return static_cast<int>(d->m_filteredItems.at( idx.row() ).type);
            case Qt::FontRole:
                return QFontDatabase::systemFont(QFontDatabase::FixedFont);
        }
    }
    return QVariant();
}

int OutputModel::rowCount( const QModelIndex& parent ) const
{
    Q_D(const OutputModel);

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
    Q_D(OutputModel);

    if( index.model() != this || !d->isValidIndex(index, rowCount()) )
    {
        return;
    }
    qCDebug(OUTPUTVIEW) << "Model activated" << index.row();


    FilteredItem item = d->m_filteredItems.at( index.row() );
    if( item.isActivatable )
    {
        qCDebug(OUTPUTVIEW) << "activating:" << item.lineNo << item.url;
        KTextEditor::Cursor range( item.lineNo, item.columnNo );
        KDevelop::IDocumentController *docCtrl = KDevelop::ICore::self()->documentController();
        QUrl url = item.url;
        if (item.url.isEmpty()) {
            qCWarning(OUTPUTVIEW) << "trying to open empty url";
            return;
        }
        if(url.isRelative()) {
            url = d->m_buildDir.resolved(url);
        }
        Q_ASSERT(!url.isRelative());
        docCtrl->openDocument( url, range );
    } else {
        qCDebug(OUTPUTVIEW) << "not an activateable item";
    }
}

QModelIndex OutputModel::firstHighlightIndex()
{
    Q_D(OutputModel);

    if( !d->m_errorItems.empty() ) {
        return index( *d->m_errorItems.begin(), 0, QModelIndex() );
    }

    for( int row = 0; row < rowCount(); ++row ) {
        if( d->m_filteredItems.at( row ).isActivatable ) {
            return index( row, 0, QModelIndex() );
        }
    }

    return QModelIndex();
}

QModelIndex OutputModel::nextHighlightIndex( const QModelIndex &currentIdx )
{
    Q_D(OutputModel);

    int startrow = d->isValidIndex(currentIdx, rowCount()) ? currentIdx.row() + 1 : 0;

    if( !d->m_errorItems.empty() )
    {
        qCDebug(OUTPUTVIEW) << "searching next error";
        // Jump to the next error item
        auto next = d->m_errorItems.lower_bound( startrow );
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
    Q_D(OutputModel);

    //We have to ensure that startrow is >= rowCount - 1 to get a positive value from the % operation.
    int startrow = rowCount() + (d->isValidIndex(currentIdx, rowCount()) ? currentIdx.row() : rowCount()) - 1;

    if(!d->m_errorItems.empty())
    {
        qCDebug(OUTPUTVIEW) << "searching previous error";

        // Jump to the previous error item
        auto previous = d->m_errorItems.lower_bound( currentIdx.row() );

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

QModelIndex OutputModel::lastHighlightIndex()
{
    Q_D(OutputModel);

    if( !d->m_errorItems.empty() ) {
        return index( *d->m_errorItems.rbegin(), 0, QModelIndex() );
    }

    for( int row = rowCount()-1; row >=0; --row ) {
        if( d->m_filteredItems.at( row ).isActivatable ) {
            return index( row, 0, QModelIndex() );
        }
    }

    return QModelIndex();
}

void OutputModel::setFilteringStrategy(const OutputFilterStrategy& currentStrategy)
{
    Q_D(OutputModel);

    // TODO: Turn into factory, decouple from OutputModel
    IFilterStrategy* filter = nullptr;
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
    }
    if (!filter) {
        filter = new NoFilterStrategy;
    }

    QMetaObject::invokeMethod(d->worker, "changeFilterStrategy",
                              Q_ARG(KDevelop::IFilterStrategy*, filter));
}

void OutputModel::setFilteringStrategy(IFilterStrategy* filterStrategy)
{
    Q_D(OutputModel);

    QMetaObject::invokeMethod(d->worker, "changeFilterStrategy",
                              Q_ARG(KDevelop::IFilterStrategy*, filterStrategy));
}

void OutputModel::appendLines( const QStringList& lines )
{
    Q_D(OutputModel);

    if( lines.isEmpty() )
        return;

    QMetaObject::invokeMethod(d->worker, "addLines",
                              Q_ARG(QStringList, lines));
}

void OutputModel::appendLine( const QString& line )
{
    appendLines( QStringList() << line );
}

void OutputModel::ensureAllDone()
{
    Q_D(OutputModel);

    QMetaObject::invokeMethod(d->worker, "flushBuffers");
}

void OutputModel::clear()
{
    Q_D(OutputModel);

    ensureAllDone();
    beginResetModel();
    d->m_filteredItems.clear();
    endResetModel();
}

}

#include "outputmodel.moc"
#include "moc_outputmodel.cpp"
