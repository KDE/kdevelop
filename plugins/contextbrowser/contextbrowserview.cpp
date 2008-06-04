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
#include <QTextEdit>
#include <QSplitter>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QAction>
#include <QMenu>

#include <klocale.h>

#include <duchain/declaration.h>
#include <duchain/ducontext.h>
#include <duchain/duchain.h>
#include <duchain/duchainlock.h>
#include <icore.h>
#include <idocumentcontroller.h>

#include "contextbrowser.h"

const int maxHistoryLength = 30;

using namespace KDevelop;

ContextWidget::ContextWidget() : m_navigationWidget(0), m_nextHistoryIndex(0) {
    m_layout = new QGridLayout;
    QHBoxLayout* buttons = new QHBoxLayout;
    QLabel* label = new QLabel(i18n("Context:"));
    //m_layout->addWidget(label);
    m_layout->setAlignment(Qt::AlignTop);
    
    m_previousButton = new QPushButton("Previous");
    m_nextButton = new QPushButton("Next");
    
    m_previousButton->setEnabled(false);
    m_nextButton->setEnabled(false);
    buttons->addWidget(label);
    buttons->addWidget(m_previousButton);
    buttons->addWidget(m_nextButton);
    
    m_layout->addLayout(buttons, 0, 0, 1, 1);
    
    m_previousMenu = new QMenu(this);
    m_previousButton->setMenu(m_previousMenu);

    m_nextMenu = new QMenu(this);
    m_nextButton->setMenu(m_nextMenu);
    
    setLayout(m_layout);
    
    connect(m_previousButton, SIGNAL(clicked(bool)), this, SLOT(historyPrevious()));
    connect(m_nextButton, SIGNAL(clicked(bool)), this, SLOT(historyNext()));
    connect(m_nextMenu, SIGNAL(aboutToShow()), this, SLOT(nextMenuAboutToShow()));
    connect(m_previousMenu, SIGNAL(aboutToShow()), this, SLOT(previousMenuAboutToShow()));
}

void ContextWidget::historyNext() {
    if(m_nextHistoryIndex >= m_history.size()) {
        kDebug() << "no forward history entry";
        return;
    }
    
    DocumentCursor c = m_history[m_nextHistoryIndex].computePosition();
    
    {
        KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
        setContext(m_history[m_nextHistoryIndex].context.data(), SimpleCursor(c), true);
    }

    ++m_nextHistoryIndex;
    
    if(c.isValid() && !c.document().str().isEmpty()) {
        ICore::self()->documentController()->openDocument(KUrl(c.document().str()), c);
    }else{
        kDebug() << "invalid cursor";
    }

    m_nextButton->setEnabled( m_nextHistoryIndex < m_history.size() );
    m_previousButton->setEnabled( m_nextHistoryIndex > 2 );
}

void ContextWidget::historyPrevious() {
    if(m_nextHistoryIndex < 2) {
        kDebug() << "no back history";
        return;
    }
    --m_nextHistoryIndex;
    
    DocumentCursor c = m_history[m_nextHistoryIndex-1].computePosition();
    
    {
        KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
        setContext(m_history[m_nextHistoryIndex-1].context.data(), SimpleCursor(c), true);
    }
    
    if(c.isValid() && !c.document().str().isEmpty()) {
        ICore::self()->documentController()->openDocument(KUrl(c.document().str()), c);
    }else{
        kDebug() << "invalid cursor";
    }

    m_nextButton->setEnabled( m_nextHistoryIndex < m_history.size() );
    m_previousButton->setEnabled( m_nextHistoryIndex > 2 );
}

void ContextWidget::nextMenuAboutToShow() {
    m_nextMenu->clear();
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    for(int a = m_nextHistoryIndex; a < m_history.size(); ++a) {
        QString actionText = m_history[a].context ? m_history[a].context->scopeIdentifier(true).toString() : m_history[a].absoluteCursorPosition.document().str();
        if(actionText.isEmpty())
            actionText = m_history[a].alternativeString;
        if(actionText.isEmpty()) //ly show file+line-number
            actionText = KUrl(m_history[a].absoluteCursorPosition.document().str()).fileName() + QString(":%1").arg(m_history[a].absoluteCursorPosition.line());
        
        QAction* action = new QAction(actionText, m_nextMenu);
        action->setData(a);
        m_nextMenu->addAction(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    }
}

void ContextWidget::previousMenuAboutToShow() {
    m_previousMenu->clear();
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    for(int a = m_nextHistoryIndex-2; a >= 0; --a) {
        QString actionText = m_history[a].context ? m_history[a].context->scopeIdentifier(true).toString() : m_history[a].absoluteCursorPosition.document().str();
        if(actionText.isEmpty())
            actionText = m_history[a].alternativeString;
        if(actionText.isEmpty())
            continue;
        QAction* action = new QAction(actionText, m_previousMenu);
        action->setData(a);
        m_previousMenu->addAction(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    }
}


void ContextWidget::actionTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if(action && action->data().type() == QVariant::Int) {
        int historyPosition = action->data().toInt();
        if(historyPosition >= 0 && historyPosition < m_history.size()) {
            m_nextHistoryIndex = historyPosition + 1;
            
            DocumentCursor c = m_history[historyPosition].computePosition();
            
            {
                KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
                setContext(m_history[historyPosition].context.data(), SimpleCursor(c), true);
            }
            if(c.isValid() && !c.document().str().isEmpty()) {
                ICore::self()->documentController()->openDocument(KUrl(c.document().str()), c);
            }else{
                kDebug() << "invalid cursor";
            }
            
            m_nextButton->setEnabled( m_nextHistoryIndex < m_history.size() );
            m_previousButton->setEnabled( m_nextHistoryIndex > 2 );
        }
    }else{
        kDebug() << "error";
    }
}

ContextWidget::HistoryEntry::HistoryEntry(DUContextPointer ctx, const KDevelop::SimpleCursor& cursorPosition) : context(ctx) {
        //Use a position relative to the context
        setCursorPosition(cursorPosition);
        if(ctx)
            alternativeString = ctx->scopeIdentifier(true).toString();;
        if(!alternativeString.isEmpty())
            alternativeString += i18n("(changed)"); //This is used when the context was deleted in between
}

DocumentCursor ContextWidget::HistoryEntry::computePosition() {
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    DocumentCursor ret;
    if(context) {
        ret = DocumentCursor(context->url(), relativeCursorPosition.textCursor());
        ret.setLine(ret.line() + context->range().start.line);
    }else{
        ret = absoluteCursorPosition;
    }
    return ret;
}

void ContextWidget::HistoryEntry::setCursorPosition(const KDevelop::SimpleCursor& cursorPosition) {
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    if(context) {
        absoluteCursorPosition =  DocumentCursor(context->url(), cursorPosition.textCursor());
        relativeCursorPosition = cursorPosition;
        relativeCursorPosition.line -= context->range().start.line;
    }
}

void ContextWidget::setContext(KDevelop::DUContext* context, const KDevelop::SimpleCursor& cursorPosition, bool noHistory) {
    
    //Manage history
    if(!noHistory) {
        if(context != m_context.data() && context != 0) {
            m_history.resize(m_nextHistoryIndex); //Dump forward history

            
            m_history.append(HistoryEntry(DUContextPointer(context), cursorPosition));
            ++m_nextHistoryIndex;
            
            m_nextButton->setEnabled(false);
            m_previousButton->setEnabled(true);

            if(m_history.size() > (maxHistoryLength + 5)) {
                m_history = m_history.mid(m_history.size() - maxHistoryLength);
                m_nextHistoryIndex = m_history.size();
            }
        }else if(context == m_context.data()) {
            //Just update the cursor position in the existing history entry
            m_history[m_nextHistoryIndex-1].setCursorPosition(cursorPosition);
        }
    }
    
    //Manage widget
    if(m_context.data() != context) {
        m_context = DUContextPointer(context);
        delete m_navigationWidget;
        m_navigationWidget = 0;
        if(m_context)
            m_navigationWidget = m_context->createNavigationWidget();
        if(m_navigationWidget)
            m_layout->addWidget(m_navigationWidget);
    }
}

DeclarationWidget::DeclarationWidget() : m_navigationWidget(0) {
    m_layout = new QGridLayout;
    m_layout->setAlignment(Qt::AlignTop);
    QHBoxLayout* labelLayout = new QHBoxLayout;
    labelLayout->addWidget(new QLabel(i18n("Declaration:")));
    m_lockButton = new QCheckBox(i18n("lock"));
    m_lockButton->setChecked(false);
    labelLayout->addWidget(m_lockButton);
    labelLayout->setAlignment(m_lockButton, Qt::AlignRight);
    m_layout->addLayout(labelLayout, 0, 0, 1, 1);
    setLayout(m_layout);
}

void DeclarationWidget::setDeclaration(Declaration* decl, TopDUContext* topContext) {
    m_declaration = DeclarationPointer(decl);

    if(m_lockButton->isChecked())
        return;
    
    delete m_navigationWidget;
    m_navigationWidget = decl->context()->createNavigationWidget(decl, topContext);
    if(m_navigationWidget)
        m_layout->addWidget(m_navigationWidget);
    
}

void DeclarationWidget::setSpecialNavigationWidget(QWidget* widget) {
    if(m_lockButton->isChecked())
        return;

    m_declaration = DeclarationPointer();
    delete m_navigationWidget;
    
    m_navigationWidget = widget;
    if(m_navigationWidget)
        m_layout->addWidget(m_navigationWidget);
}

ContextBrowserView::ContextBrowserView( ContextBrowserPlugin* plugin ) : m_plugin(plugin) {
    
    m_contextWidget = new ContextWidget;
    m_declarationWidget = new DeclarationWidget;
    
    m_splitter = new QSplitter;
    QGridLayout *layout = new QGridLayout;
    
    m_splitter->addWidget(m_contextWidget);
    m_splitter->addWidget(m_declarationWidget);

    resizeEvent(0);
    
    layout->addWidget(m_splitter, 0, 0);
    setLayout(layout);
    
    m_plugin->registerToolView(this);
    
    connect(plugin, SIGNAL(previousContextShortcut()), m_contextWidget, SLOT(historyPrevious()));
    connect(plugin, SIGNAL(nextContextShortcut()), m_contextWidget, SLOT(historyNext()));
}

ContextBrowserView::~ContextBrowserView() {
    m_plugin->unRegisterToolView(this);
}

void ContextBrowserView::resizeEvent ( QResizeEvent * /*event*/ ) {
    int total = 0;
    if(width() > height()) {
        total = width();
        m_splitter->setOrientation(Qt::Horizontal);
    } else {
        total = height();
        m_splitter->setOrientation(Qt::Vertical);
    }
    QList<int> sizes;
    sizes << total/2 << total/2;
    m_splitter->setSizes( sizes );
}

DeclarationWidget* ContextBrowserView::declarationWidget() {
    return m_declarationWidget;
}

ContextWidget* ContextBrowserView::contextWidget() {
    return m_contextWidget;
}


#include "contextbrowserview.moc"
