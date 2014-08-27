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

///TODO: cleanup includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QShowEvent>
#include <QAction>
#include <QMenu>
#include <QIcon>

#include <KTextBrowser>
#include <KLocale>
#include <KComboBox>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <language/duchain/declaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <serialization/indexedstring.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include "contextbrowser.h"
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/specializationstore.h>
#include "browsemanager.h"
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <kparts/part.h>
#include <qapplication.h>
#include <language/interfaces/codecontext.h>
#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/navigation/useswidget.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iplugincontroller.h>

using namespace KDevelop;

QWidget* ContextBrowserView::createWidget(KDevelop::DUContext* context) {
        m_context = IndexedDUContext(context);
        if(m_context.data()) {
            return m_context.data()->createNavigationWidget();
        }
        return 0;
}

KDevelop::IndexedDeclaration ContextBrowserView::declaration() const {
    return m_declaration;
}

QWidget* ContextBrowserView::createWidget(Declaration* decl, TopDUContext* topContext) {
    m_declaration = IndexedDeclaration(decl);
    return decl->context()->createNavigationWidget(decl, topContext);
}


void ContextBrowserView::resetWidget()
{
    if (m_navigationWidget) {
        delete m_navigationWidget;
        m_navigationWidget = 0;
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
            QMenu menu;
            QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( c );

            ContextMenuExtension::populateMenu(&menu, extensions);
            menu.exec(QCursor::pos());
        }
    }
}

void ContextBrowserView::updateLockIcon(bool checked) {
    m_lockButton->setIcon(QIcon::fromTheme(checked ? "document-encrypt" : "document-decrypt"));
}

ContextBrowserView::ContextBrowserView( ContextBrowserPlugin* plugin, QWidget* parent ) : QWidget(parent), m_plugin(plugin), m_navigationWidget(new KTextBrowser()), m_autoLocked(false) {
    setWindowIcon( QIcon::fromTheme("applications-development-web") );

    m_allowLockedUpdate = false;

    m_buttons = new QHBoxLayout;
    m_buttons->addStretch();
    m_declarationMenuButton = new QToolButton();
    m_declarationMenuButton->setIcon(QIcon::fromTheme("code-class"));
    m_declarationMenuButton->setToolTip(i18n("Declaration menu"));
    connect(m_declarationMenuButton, SIGNAL(clicked(bool)), SLOT(declarationMenu()));
    m_buttons->addWidget(m_declarationMenuButton);
    m_lockButton = new QToolButton();
    m_lockButton->setCheckable(true);
    m_lockButton->setChecked(false);
    m_lockButton->setToolTip(i18n("Lock current view"));
    updateLockIcon(m_lockButton->isChecked());
    connect(m_lockButton, SIGNAL(toggled(bool)), SLOT(updateLockIcon(bool)));
    m_buttons->addWidget(m_lockButton);

    m_layout = new QVBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->addLayout(m_buttons);
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
    kDebug() << "got focus";
//     parentWidget()->setBackgroundRole(QPalette::ToolTipBase);
/*    m_layout->removeItem(m_buttons);*/
    
    return QWidget::focusInEvent(event);
}

void ContextBrowserView::focusOutEvent(QFocusEvent* event) {
    kDebug() << "lost focus";
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
                m_lockButton->toggle();
        }
    }
    return QWidget::event(event);
}

void ContextBrowserView::showEvent(QShowEvent* event) {
    DUChainReadLocker lock(DUChain::lock());
    TopDUContext* top = m_lastUsedTopContext.data();
    if(top && m_navigationWidgetDeclaration.isValid() && m_navigationWidgetDeclaration.getDeclaration(top)) {
        if(top) {
            //Update the navigation-widget
            Declaration* decl = m_navigationWidgetDeclaration.getDeclaration(top);
            setDeclaration(decl, top, true);
            //Update the declaration combo-box
            /*
            TODO: bring this back if required
            DUContext* context = 0;
            KDevelop::IDocument* doc = ICore::self()->documentController()->activeDocument();
            if(doc && doc->textDocument() && doc->textDocument()->activeView()) {
                KTextEditor::Cursor c = doc->textDocument()->activeView()->cursorPosition();
                context = getContextAt(top->url().toUrl(), c);
            }
            m_plugin->updateDeclarationListBox(context);
            */
        }
    }
    QWidget::showEvent(event);
}

bool ContextBrowserView::isLocked() const {
    bool isLocked;
    if (m_allowLockedUpdate) {
        isLocked = false;
    } else {
        isLocked = m_lockButton->isChecked();
    }
    return isLocked;
}

void ContextBrowserView::updateMainWidget(QWidget* widget)
{
    if (widget) {
        setUpdatesEnabled(false);
        kDebug() << "";
        resetWidget();
        m_navigationWidget = widget;
        m_layout->insertWidget(1, widget, 1);
        m_allowLockedUpdate = false;
        setUpdatesEnabled(true);
        connect(widget, SIGNAL(contextChanged(bool,bool)), this, SLOT(navigationContextChanged(bool,bool)));
    }
}

void ContextBrowserView::navigationContextChanged(bool wasInitial, bool isInitial)
{
    if(wasInitial && !isInitial && !m_lockButton->isChecked())
    {
        m_autoLocked = true;
        m_lockButton->setChecked(true);
    }else if(!wasInitial && isInitial && m_autoLocked)
    {
        m_autoLocked = false;
        m_lockButton->setChecked(false);
    }else if(isInitial) {
        m_autoLocked = false;
    }
}

void ContextBrowserView::setDeclaration(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext, bool force) {
    m_lastUsedTopContext = IndexedTopDUContext(topContext);

    if(isLocked() && (!m_navigationWidget.data() || !isVisible()))
    {
        // Automatically remove the locked state if the view is not visible or the widget was deleted,
        // because the locked state has side-effects on other navigation functionality.
        m_autoLocked = false;
        m_lockButton->setChecked(false);
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
    if(m_lockButton->isChecked())
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

