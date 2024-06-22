/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "quickopenmodel.h"
#include "debug.h"

#include <QTreeView>
#include <QTimer>

#include <KTextEditor/CodeCompletionModel>
#include <typeinfo>
#include <utility>
#include <vector>

#include <util/algorithm.h>

#include "expandingtree/expandingtree.h"
#include "projectfilequickopen.h"
#include "duchainitemquickopen.h"

#define QUICKOPEN_USE_ITEM_CACHING

using namespace KDevelop;

QuickOpenModel::QuickOpenModel(QWidget* parent) : ExpandingWidgetModel(parent)
    , m_treeView(nullptr)
    , m_expandingWidgetHeightIncrease(0)
    , m_resetBehindRow(0)
{
    m_resetTimer = new QTimer(this);
    m_resetTimer->setSingleShot(true);
    m_resetTimer->setInterval(0);
    connect(m_resetTimer, &QTimer::timeout, this, &QuickOpenModel::resetTimer);
}

void QuickOpenModel::setExpandingWidgetHeightIncrease(int pixels)
{
    m_expandingWidgetHeightIncrease = pixels;
}

QStringList QuickOpenModel::allScopes() const
{
    QStringList scopes;
    for (const ProviderEntry& provider : m_providers) {
        for (const QString& scope : provider.scopes) {
            if (!scopes.contains(scope)) {
                scopes << scope;
            }
        }
    }

    return scopes;
}

QStringList QuickOpenModel::allTypes() const
{
    QSet<QString> types;
    for (const ProviderEntry& provider : m_providers) {
        types += provider.types;
    }

    return types.values();
}

void QuickOpenModel::registerProvider(const QStringList& scopes, const QStringList& types, KDevelop::QuickOpenDataProviderBase* provider)
{
    ProviderEntry e;
    e.scopes = QSet<QString>(scopes.begin(), scopes.end());
    e.types = QSet<QString>(types.begin(), types.end());
    e.provider = provider;

    m_providers << e; //.insert( types, e );

    connect(provider, &QuickOpenDataProviderBase::destroyed, this, &QuickOpenModel::destroyed);

    restart(true);
}

bool QuickOpenModel::removeProvider(KDevelop::QuickOpenDataProviderBase* provider)
{
    bool ret = false;
    for (ProviderList::iterator it = m_providers.begin(); it != m_providers.end(); ++it) {
        if ((*it).provider == provider) {
            m_providers.erase(it);
            disconnect(provider, &QuickOpenDataProviderBase::destroyed, this, &QuickOpenModel::destroyed);
            ret = true;
            break;
        }
    }

    restart(true);

    return ret;
}

void QuickOpenModel::enableProviders(const QStringList& _items, const QStringList& _scopes)
{
    const QSet<QString> items(_items.begin(), _items.end());
    const QSet<QString> scopes(_scopes.begin(), _scopes.end());
    if (m_enabledItems == items && m_enabledScopes == scopes && !items.isEmpty() && !scopes.isEmpty()) {
        return;
    }
    m_enabledItems = items;
    m_enabledScopes  = scopes;
    qCDebug(PLUGIN_QUICKOPEN) << "params " << items << " " << scopes;

    //We use 2 iterations here: In the first iteration, all providers that implement QuickOpenFileSetInterface are initialized, then the other ones.
    //The reason is that the second group can refer to the first one.
    for (auto& provider : m_providers) {
        if (!qobject_cast<QuickOpenFileSetInterface*>(provider.provider)) {
            continue;
        }
        qCDebug(PLUGIN_QUICKOPEN) << "comparing" << provider.scopes << provider.types;
        if ((scopes.isEmpty() || !(scopes & provider.scopes).isEmpty()) && (!(items & provider.types).isEmpty() || items.isEmpty())) {
            qCDebug(PLUGIN_QUICKOPEN) << "enabling " << provider.types << " " << provider.scopes;
            provider.enabled = true;
            provider.provider->enableData(_items, _scopes);
        } else {
            qCDebug(PLUGIN_QUICKOPEN) << "disabling " << provider.types << " " << provider.scopes;
            provider.enabled = false;
            if ((scopes.isEmpty() || !(scopes & provider.scopes).isEmpty())) {
                provider.provider->enableData(_items, _scopes); //The provider may still provide files
            }
        }
    }

    for (auto & provider : m_providers) {
        if (qobject_cast<QuickOpenFileSetInterface*>(provider.provider)) {
            continue;
        }
        qCDebug(PLUGIN_QUICKOPEN) << "comparing" << provider.scopes << provider.types;
        if ((scopes.isEmpty() || !(scopes & provider.scopes).isEmpty()) && (!(items & provider.types).isEmpty() || items.isEmpty())) {
            qCDebug(PLUGIN_QUICKOPEN) << "enabling " << provider.types << " " << provider.scopes;
            provider.enabled = true;
            provider.provider->enableData(_items, _scopes);
        } else {
            qCDebug(PLUGIN_QUICKOPEN) << "disabling " << provider.types << " " << provider.scopes;
            provider.enabled = false;
        }
    }

    restart(true);
}

void QuickOpenModel::textChanged(const QString& str)
{
    if (m_filterText == str) {
        return;
    }

    beginResetModel();

    m_filterText = str;
    for (const ProviderEntry& provider : std::as_const(m_providers)) {
        if (provider.enabled) {
            provider.provider->setFilterText(str);
        }
    }

    m_cachedData.clear();
    clearExpanding();

    //Get the 50 first items, so the data-providers notice changes without ui-glitches due to resetting
    for (int a = 0; a < 50 && a < rowCount(QModelIndex()); ++a) {
        getItem(a, true);
    }

    endResetModel();
}

void QuickOpenModel::restart(bool keepFilterText)
{
    // make sure we do not restart recursively which could lead to
    // recursive loading of provider plugins e.g. (happened for the cpp plugin)
    QMetaObject::invokeMethod(this, "restart_internal", Qt::QueuedConnection,
                              Q_ARG(bool, keepFilterText));
}

void QuickOpenModel::restart_internal(bool keepFilterText)
{
    if (!keepFilterText) {
        m_filterText.clear();
    }

    bool anyEnabled = std::any_of(m_providers.constBegin(), m_providers.constEnd(), [](const ProviderEntry& e) {
        return e.enabled;
    });

    if (!anyEnabled) {
        return;
    }

    for (const ProviderEntry& provider : std::as_const(m_providers)) {
        if (!qobject_cast<QuickOpenFileSetInterface*>(provider.provider)) {
            continue;
        }

        ///Always reset providers that implement QuickOpenFileSetInterface and have some matching scopes, because they may be needed by other providers.
        if (m_enabledScopes.isEmpty() || !(m_enabledScopes & provider.scopes).isEmpty()) {
            provider.provider->reset();
        }
    }

    for (const ProviderEntry& provider : std::as_const(m_providers)) {
        if (qobject_cast<QuickOpenFileSetInterface*>(provider.provider)) {
            continue;
        }

        if (provider.enabled && provider.provider) {
            provider.provider->reset();
        }
    }

    if (keepFilterText) {
        textChanged(m_filterText);
    } else {
        beginResetModel();
        m_cachedData.clear();
        clearExpanding();
        endResetModel();
    }
}

void QuickOpenModel::destroyed(QObject* obj)
{
    removeProvider(static_cast<KDevelop::QuickOpenDataProviderBase*>(obj));
}

QModelIndex QuickOpenModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
    if (column >= columnCount() || row >= rowCount(QModelIndex())) {
        return QModelIndex();
    }
    if (row < 0 || column < 0) {
        return QModelIndex();
    }
    return createIndex(row, column);
}

QModelIndex QuickOpenModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int QuickOpenModel::rowCount(const QModelIndex& i) const
{
    if (i.isValid()) {
        return 0;
    }

    int count = 0;
    for (const ProviderEntry& provider : m_providers) {
        if (provider.enabled) {
            count += provider.provider->itemCount();
        }
    }

    return count;
}

int QuickOpenModel::unfilteredRowCount() const
{
    int count = 0;
    for (const ProviderEntry& provider : m_providers) {
        if (provider.enabled) {
            count += provider.provider->unfilteredItemCount();
        }
    }

    return count;
}

int QuickOpenModel::columnCount() const
{
    return 2;
}

int QuickOpenModel::columnCount(const QModelIndex& index) const
{
    if (index.parent().isValid()) {
        return 0;
    } else {
        return columnCount();
    }
}

QVariant QuickOpenModel::data(const QModelIndex& index, int role) const
{
    QuickOpenDataPointer d = getItem(index.row());

    if (!d) {
        return QVariant();
    }

    switch (role) {
    case KTextEditor::CodeCompletionModel::ItemSelected: {
        QString desc = d->htmlDescription();
        if (desc.isEmpty()) {
            return QVariant();
        } else {
            return desc;
        }
    }

    case KTextEditor::CodeCompletionModel::IsExpandable:
        return d->isExpandable();
    case KTextEditor::CodeCompletionModel::ExpandingWidget: {
        QWidget* w =  d->expandingWidget();
        if (w && m_expandingWidgetHeightIncrease) {
            w->resize(w->width(), w->height() + m_expandingWidgetHeightIncrease);
        }
        return QVariant::fromValue(w);
    }
    case ExpandingTree::ProjectPathRole:
        // TODO: put this into the QuickOpenDataBase API
        //       we cannot do this in 5.0, cannot change ABI
        if (auto projectFile = dynamic_cast<const ProjectFileData*>(d.constData())) {
            return QVariant::fromValue(projectFile->projectPath());
        } else if (auto duchainItem = dynamic_cast<const DUChainItemData*>(d.constData())) {
            return QVariant::fromValue(duchainItem->projectPath());
        }
    }

    if (index.column() == 1) { //This column contains the actual content
        switch (role) {
        case Qt::DecorationRole:
            return d->icon();

        case Qt::DisplayRole:
            return d->text();
        case KTextEditor::CodeCompletionModel::HighlightingMethod:
            return KTextEditor::CodeCompletionModel::CustomHighlighting;
        case KTextEditor::CodeCompletionModel::CustomHighlight:
            return d->highlighting();
        }
    } else if (index.column() == 0) { //This column only contains the expanded/not expanded icon
        switch (role) {
        case Qt::DecorationRole:
        {
            if (isExpandable(index)) {
                //Show the expanded/unexpanded handles
                if (isExpanded(index)) {
                    return QIcon::fromTheme(QStringLiteral("arrow-down"));
                } else {
                    return QIcon::fromTheme(QStringLiteral("arrow-right"));
                }
            }
        }
        }
    }

    return ExpandingWidgetModel::data(index, role);
}

void QuickOpenModel::resetTimer()
{
    int currentRow = treeView() ? mapToSource(treeView()->currentIndex()).row() : -1;

    beginResetModel();
    //Remove all cached data behind row m_resetBehindRow
    for (DataList::iterator it = m_cachedData.begin(); it != m_cachedData.end(); ) {
        if (it.key() > m_resetBehindRow) {
            it = m_cachedData.erase(it);
        } else {
            ++it;
        }
    }

    endResetModel();

    if (currentRow != -1) {
        treeView()->setCurrentIndex(mapFromSource(index(currentRow, 0, QModelIndex()))); //Preserve the current index
    }
    m_resetBehindRow = 0;
}

QuickOpenDataPointer QuickOpenModel::getItem(int row, bool noReset) const
{
    ///@todo mix all the models alphabetically here. For now, they are simply ordered.
    ///@todo Deal with unexpected item-counts, like for example in the case of overloaded function-declarations

#ifdef QUICKOPEN_USE_ITEM_CACHING
    const auto dataIt = m_cachedData.constFind(row);
    if (dataIt != m_cachedData.constEnd()) {
        return *dataIt;
    }
#endif
    int rowOffset = 0;

    Q_ASSERT(row < rowCount(QModelIndex()));

    for (const ProviderEntry& provider : m_providers) {
        if (!provider.enabled) {
            continue;
        }
        uint itemCount = provider.provider->itemCount();
        if (( uint )row < itemCount) {
            QuickOpenDataPointer item = provider.provider->data(row);

            if (!noReset && provider.provider->itemCount() != itemCount) {
                qCDebug(PLUGIN_QUICKOPEN) << "item-count in provider has changed, resetting model";
                m_resetTimer->start();
                m_resetBehindRow = rowOffset + row; //Don't reset everything, only everything behind this position
            }

#ifdef QUICKOPEN_USE_ITEM_CACHING
            m_cachedData[row + rowOffset] = item;
#endif
            return item;
        } else {
            row -= provider.provider->itemCount();
            rowOffset += provider.provider->itemCount();
        }
    }

//   qWarning() << "No item for row " <<  row;

    return QuickOpenDataPointer();
}

QSet<IndexedString> QuickOpenModel::fileSet() const
{
    std::vector<QSet<IndexedString>> sets;
    for (const ProviderEntry& provider : m_providers) {
        if (m_enabledScopes.isEmpty() || !(m_enabledScopes & provider.scopes).isEmpty()) {
            if (auto* iface = qobject_cast<QuickOpenFileSetInterface*>(provider.provider)) {
                sets.push_back(iface->files());
                //qCDebug(PLUGIN_QUICKOPEN) << "got file-list with" << ifiles.count() << "entries from data-provider" << typeid(*iface).name();
            }
        }
    }
    return Algorithm::unite(std::move(sets));
}

QTreeView* QuickOpenModel::treeView() const
{
    return m_treeView;
}

bool QuickOpenModel::indexIsItem(const QModelIndex& index) const
{
    Q_ASSERT(index.model() == this);
    Q_UNUSED(index);
    return true;
}

void QuickOpenModel::setTreeView(QTreeView* view)
{
    m_treeView = view;
}

int QuickOpenModel::contextMatchQuality(const QModelIndex& /*index*/) const
{
    return -1;
}

bool QuickOpenModel::execute(const QModelIndex& index, QString& filterText)
{
    qCDebug(PLUGIN_QUICKOPEN) << "executing model";
    if (!index.isValid()) {
        qCWarning(PLUGIN_QUICKOPEN) << "Invalid index executed";
        return false;
    }

    QuickOpenDataPointer item = getItem(index.row());

    if (item) {
        return item->execute(filterText);
    } else {
        qCWarning(PLUGIN_QUICKOPEN) << "Got no item for row " << index.row() << " ";
    }

    return false;
}

#include "moc_quickopenmodel.cpp"
