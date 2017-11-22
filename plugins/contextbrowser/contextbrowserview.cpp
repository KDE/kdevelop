/*
 * This file is part of KDevelop
 *
 * Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include "contextbrowserview.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QMenu>
#include <QShowEvent>
#include <QTextBrowser>

#include <KToggleAction>
#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <language/duchain/declaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

#include <interfaces/icore.h>

#include "contextbrowser.h"
#include "debug.h"
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/specializationstore.h>
#include "browsemanager.h"
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <language/interfaces/codecontext.h>
#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/navigation/useswidget.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iplugincontroller.h>

using namespace KDevelop;

namespace {

enum Direction
{
    NextUse,
    PreviousUse
};

void selectUse(ContextBrowserView* view, Direction direction)
{
    auto abstractNaviWidget = dynamic_cast<AbstractNavigationWidget*>(view->navigationWidget());

    if (!abstractNaviWidget) {
        return;
    }

    auto usesWidget = dynamic_cast<UsesWidget*>(abstractNaviWidget->context()->widget());
    if (!usesWidget) {
        return;
    }

    OneUseWidget* first = nullptr, *previous = nullptr, *current = nullptr;
    for (auto item : usesWidget->items()) {
        auto topContext = dynamic_cast<TopContextUsesWidget*>(item);
        if (!topContext) {
            continue;
        }
        for (auto item : topContext->items()) {
            auto navigationList = dynamic_cast<NavigatableWidgetList*>(item);
            if (!navigationList) {
                continue;
            }
            for (auto item : navigationList->items()) {
                auto use = dynamic_cast<OneUseWidget*>(item);
                if (!use) {
                    continue;
                }
                if (!first) {
                    first = use;
                }
                current = use;
                if (direction == PreviousUse && current->isHighlighted() && previous) {
                    previous->setHighlighted(true);
                    previous->activateLink();
                    current->setHighlighted(false);
                    return;
                }
                if (direction == NextUse && previous && previous->isHighlighted()) {
                    current->setHighlighted(true);
                    current->activateLink();
                    previous->setHighlighted(false);
                    return;
                }
                previous = current;
            }
        }
    }
    if (direction == NextUse && first) {
        first->setHighlighted(true);
        first->activateLink();
        if (current && current->isHighlighted())
            current->setHighlighted(false);
        return;
    }
    if (direction == PreviousUse && current) {
        current->setHighlighted(true);
        current->activateLink();
        if (first && first->isHighlighted()) {
            first->setHighlighted(false);
        }
    }
}

}

QWidget* ContextBrowserView::createWidget(KDevelop::DUContext* context) {
        m_context = IndexedDUContext(context);
        if(m_context.data()) {
            return m_context.data()->createNavigationWidget(nullptr, nullptr, {}, {}, AbstractNavigationWidget::EmbeddableWidget);
        }
        return nullptr;
}

KDevelop::IndexedDeclaration ContextBrowserView::declaration() const {
    return m_declaration;
}

QWidget* ContextBrowserView::createWidget(Declaration* decl, TopDUContext* topContext) {
    m_declaration = IndexedDeclaration(decl);
    return decl->context()->createNavigationWidget(decl, topContext, {}, {}, AbstractNavigationWidget::EmbeddableWidget);
}


void ContextBrowserView::resetWidget()
{
    if (m_navigationWidget) {
        delete m_navigationWidget;
        m_navigationWidget = nullptr;
    }
}

void ContextBrowserView::declarationMenu() {
    DUChainReadLocker lock(DUChain::lock());

    AbstractNavigationWidget* navigationWidget = dynamic_cast<AbstractNavigationWidget*>(m_navigationWidget.data());
    if(navigationWidget) {
        AbstractDeclarationNavigationContext* navigationContext = dynamic_cast<AbstractDeclarationNavigationContext*>(navigationWidget->context().data());
        if(navigationContext && navigationContext->declaration().data()) {
            KDevelop::DeclarationContext* c = new KDevelop::DeclarationContext(navigationContext->declaration().data());
            lock.unlock();
            QMenu menu(this);
            QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions(c, &menu);

            ContextMenuExtension::populateMenu(&menu, extensions);
            menu.exec(QCursor::pos());
        }
    }
}

ContextBrowserView::ContextBrowserView( ContextBrowserPlugin* plugin, QWidget* parent ) : QWidget(parent), m_plugin(plugin), m_navigationWidget(new QTextBrowser()), m_autoLocked(false) {
    setWindowTitle(i18n("Code Browser"));
    setWindowIcon( QIcon::fromTheme(QStringLiteral("code-context"), windowIcon()) );

    m_allowLockedUpdate = false;

    m_declarationMenuAction = new QAction(QIcon::fromTheme(QStringLiteral("code-class")), QString(), this);
    m_declarationMenuAction->setToolTip(i18n("Declaration menu"));
    // expose the declaration menu via the context menu; allows hiding the toolbar to save some space
    // (this will not make it behave like a submenu though)
    m_declarationMenuAction->setText(m_declarationMenuAction->toolTip());
    connect(m_declarationMenuAction, &QAction::triggered, this, &ContextBrowserView::declarationMenu);
    addAction(m_declarationMenuAction);
    m_lockAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("object-unlocked")), i18n("Lock Current View"), this);
    m_lockAction->setToolTip(i18n("Lock current view"));
    m_lockAction->setCheckedState(KGuiItem(i18n("Unlock Current View"), QIcon::fromTheme(QStringLiteral("object-locked")), i18n("Unlock current view")));
    m_lockAction->setChecked(false);
    addAction(m_lockAction);

    m_layout = new QVBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->addWidget(m_navigationWidget);
    //m_layout->addStretch();
    setLayout(m_layout);

    m_plugin->registerToolView(this);
}

ContextBrowserView::~ContextBrowserView() {
    m_plugin->unRegisterToolView(this);
}

void ContextBrowserView::focusInEvent(QFocusEvent* event) {
    //Indicate that we have focus
    qCDebug(PLUGIN_CONTEXTBROWSER) << "got focus";
//     parentWidget()->setBackgroundRole(QPalette::ToolTipBase);
/*    m_layout->removeItem(m_buttons);*/

    return QWidget::focusInEvent(event);
}

void ContextBrowserView::focusOutEvent(QFocusEvent* event) {
    qCDebug(PLUGIN_CONTEXTBROWSER) << "lost focus";
//     parentWidget()->setBackgroundRole(QPalette::Background);
/*    m_layout->insertLayout(0, m_buttons);
    for(int a = 0; a < m_buttons->count(); ++a) {
        QWidgetItem* item = dynamic_cast<QWidgetItem*>(m_buttons->itemAt(a));
    }*/
    QWidget::focusOutEvent(event);
}

bool ContextBrowserView::event(QEvent* event) {
    QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);

    if(hasFocus() && keyEvent) {
        AbstractNavigationWidget* navigationWidget = dynamic_cast<AbstractNavigationWidget*>(m_navigationWidget.data());
        if(navigationWidget && event->type() == QEvent::KeyPress) {
            int key = keyEvent->key();
            if(key == Qt::Key_Left)
                navigationWidget->previous();
            if(key == Qt::Key_Right)
                navigationWidget->next();
            if(key == Qt::Key_Up)
                navigationWidget->up();
            if(key == Qt::Key_Down)
                navigationWidget->down();
            if(key == Qt::Key_Return || key == Qt::Key_Enter)
                navigationWidget->accept();


            if(key == Qt::Key_L)
                m_lockAction->toggle();
        }
    }
    return QWidget::event(event);
}

void ContextBrowserView::showEvent(QShowEvent* event)
{
    DUChainReadLocker lock(DUChain::lock(), 200);
    if (!lock.locked()) {
        QWidget::showEvent(event);
        return;
    }

    TopDUContext* top = m_lastUsedTopContext.data();
    if(top && m_navigationWidgetDeclaration.isValid()) {
        //Update the navigation-widget
        Declaration* decl = m_navigationWidgetDeclaration.getDeclaration(top);
        if(decl)
            setDeclaration(decl, top, true);
    }
    QWidget::showEvent(event);
}

bool ContextBrowserView::isLocked() const {
    bool isLocked;
    if (m_allowLockedUpdate) {
        isLocked = false;
    } else {
        isLocked = m_lockAction->isChecked();
    }
    return isLocked;
}

void ContextBrowserView::updateMainWidget(QWidget* widget)
{
    if (widget) {
        setUpdatesEnabled(false);
        qCDebug(PLUGIN_CONTEXTBROWSER) << "";
        resetWidget();
        m_navigationWidget = widget;
        m_layout->insertWidget(1, widget, 1);
        m_allowLockedUpdate = false;
        setUpdatesEnabled(true);
        if (widget->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("contextChanged(bool,bool)")) != -1) {
            connect(widget, SIGNAL(contextChanged(bool,bool)), this, SLOT(navigationContextChanged(bool,bool)));
        }
    }
}

void ContextBrowserView::navigationContextChanged(bool wasInitial, bool isInitial)
{
    if(wasInitial && !isInitial && !m_lockAction->isChecked())
    {
        m_autoLocked = true;
        m_lockAction->setChecked(true);
    }else if(!wasInitial && isInitial && m_autoLocked)
    {
        m_autoLocked = false;
        m_lockAction->setChecked(false);
    }else if(isInitial) {
        m_autoLocked = false;
    }
}

void ContextBrowserView::selectNextItem()
{
    selectUse(this, NextUse);
}

void ContextBrowserView::selectPreviousItem()
{
    selectUse(this, PreviousUse);
}

void ContextBrowserView::setDeclaration(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext, bool force) {
    m_lastUsedTopContext = IndexedTopDUContext(topContext);

    if(isLocked() && (!m_navigationWidget.data() || !isVisible()))
    {
        // Automatically remove the locked state if the view is not visible or the widget was deleted,
        // because the locked state has side-effects on other navigation functionality.
        m_autoLocked = false;
        m_lockAction->setChecked(false);
    }

    if(m_navigationWidgetDeclaration == decl->id() && !force)
        return;

    m_navigationWidgetDeclaration = decl->id();

    if (!isLocked() && (isVisible() || force)) {  // NO-OP if toolview is hidden, for performance reasons

        QWidget* w = createWidget(decl, topContext);
        updateMainWidget(w);
    }
}

KDevelop::IndexedDeclaration ContextBrowserView::lockedDeclaration() const {
    if(m_lockAction->isChecked())
        return declaration();
    else
        return KDevelop::IndexedDeclaration();
}

void ContextBrowserView::allowLockedUpdate() {
    m_allowLockedUpdate = true;
}

void ContextBrowserView::setNavigationWidget(QWidget* widget) {
    updateMainWidget(widget);
}

void ContextBrowserView::setContext(KDevelop::DUContext* context) {
    if(!context)
        return;

    m_lastUsedTopContext = IndexedTopDUContext(context->topContext());

    if(context->owner()) {
        if(context->owner()->id() == m_navigationWidgetDeclaration)
            return;
        m_navigationWidgetDeclaration = context->owner()->id();
    }else{
        m_navigationWidgetDeclaration = DeclarationId();
    }

    if (!isLocked() && isVisible()) { // NO-OP if toolview is hidden, for performance reasons

        QWidget* w = createWidget(context);
        updateMainWidget(w);
    }
}

void ContextBrowserView::setSpecialNavigationWidget(QWidget* widget) {
    if (!isLocked() && isVisible()) {
        Q_ASSERT(widget);
        updateMainWidget(widget);
    } else if(widget) {
        widget->deleteLater();
    }
}

