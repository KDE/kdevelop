/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "classtree.h"

#include <QMenu>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QScrollBar>

#include "interfaces/contextmenuextension.h"
#include "interfaces/icore.h"
#include "interfaces/idocument.h"
#include "interfaces/iplugincontroller.h"

#include "language/interfaces/codecontext.h"

#include "language/duchain/duchainbase.h"
#include "language/duchain/duchain.h"
#include "language/duchain/duchainlock.h"
#include "language/duchain/declaration.h"

#include "language/classmodel/classmodel.h"
#include "classbrowserplugin.h"

using namespace KDevelop;

ClassTree::ClassTree(QWidget* parent, ClassBrowserPlugin* plugin)
    : QTreeView(parent)
    , m_plugin(plugin)
    , m_tooltip(nullptr)
{
    header()->hide();

    setIndentation(10);
    setUniformRowHeights(true);

    connect(this, &ClassTree::activated, this, &ClassTree::itemActivated);
}

ClassTree::~ClassTree()
{
}

static bool _populatingClassBrowserContextMenu = false;

bool ClassTree::populatingClassBrowserContextMenu()
{
    return _populatingClassBrowserContextMenu;
}

void ClassTree::contextMenuEvent(QContextMenuEvent* e)
{
    auto* menu = new QMenu(this);
    QModelIndex index = indexAt(e->pos());
    if (index.isValid()) {
        Context* c;
        {
            DUChainReadLocker readLock(DUChain::lock());
            if (auto* decl = dynamic_cast<Declaration*>(model()->duObjectForIndex(index)))
                c = new DeclarationContext(decl);
            else
            {
                delete menu;
                return;
            }
        }
        _populatingClassBrowserContextMenu = true;

        QList<ContextMenuExtension> extensions =
            ICore::self()->pluginController()->queryPluginsForContextMenuExtensions(c, menu);
        ContextMenuExtension::populateMenu(menu, extensions);

        _populatingClassBrowserContextMenu = false;
    }

    if (!menu->actions().isEmpty())
        menu->exec(e->globalPos());
    delete menu;
}

bool ClassTree::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip) {
        // if we request a tooltip over a duobject item, show a tooltip for it
        auto* helpEvent = static_cast<QHelpEvent*>(event);
        const QModelIndex idxView = indexAt(helpEvent->pos());

        DUChainReadLocker readLock(DUChain::lock());
        if (auto* decl = dynamic_cast<Declaration*>(model()->duObjectForIndex(idxView))) {
            if (m_tooltip) {
                m_tooltip->close();
            }
            if (auto* navigationWidget = decl->topContext()->createNavigationWidget(decl)) {
                m_tooltip = new KDevelop::NavigationToolTip(this, helpEvent->globalPos() + QPoint(40,
                                                                                                  0), navigationWidget);
                m_tooltip->resize(navigationWidget->sizeHint() + QSize(10, 10));
                ActiveToolTip::showToolTip(m_tooltip);
                return true;
            }
        }
    }

    return QAbstractItemView::event(event);
}

ClassModel* ClassTree::model()
{
    return static_cast<ClassModel*>(QTreeView::model());
}

void ClassTree::itemActivated(const QModelIndex& index)
{
    DUChainReadLocker readLock(DUChain::lock());

    DeclarationPointer decl = DeclarationPointer(dynamic_cast<Declaration*>(model()->duObjectForIndex(index)));
    readLock.unlock();

// Delegate to plugin function
    m_plugin->showDefinition(decl);

    if (isExpanded(index))
        collapse(index);
    else
        expand(index);
}

void ClassTree::highlightIdentifier(const KDevelop::IndexedQualifiedIdentifier& a_id)
{
    QModelIndex index = model()->indexForIdentifier(a_id);
    if (!index.isValid())
        return;

    // expand and select the item.
    selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    scrollTo(index, PositionAtCenter);
    horizontalScrollBar()->setValue(horizontalScrollBar()->minimum());
    expand(index);
}
