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
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <KIcon>
#include <KTextBrowser>
#include <KLocale>

#include <language/duchain/declaration.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/indexedstring.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include "contextbrowser.h"

const int maxHistoryLength = 30;

using namespace KDevelop;

QToolButton* ContextController::previousButton() const {
    return m_previousButton;
}

QToolButton* ContextController::nextButton() const {
    return m_nextButton;
}

ContextController::ContextController(ContextBrowserView* view) : m_nextHistoryIndex(0), m_view(view) {
    m_previousButton = new QToolButton();
    m_previousButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_previousButton->setIcon(KIcon("go-previous"));
    m_previousButton->setEnabled(false);
    m_previousMenu = new QMenu();
    m_previousButton->setMenu(m_previousMenu);
    connect(m_previousButton, SIGNAL(clicked(bool)), this, SLOT(historyPrevious()));
    connect(m_previousMenu, SIGNAL(aboutToShow()), this, SLOT(previousMenuAboutToShow()));

    m_nextButton = new QToolButton();
    m_nextButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_nextButton->setIcon(KIcon("go-next"));
    m_nextButton->setEnabled(false);
    m_nextMenu = new QMenu();
    m_nextButton->setMenu(m_nextMenu);
    connect(m_nextButton, SIGNAL(clicked(bool)), this, SLOT(historyNext()));
    connect(m_nextMenu, SIGNAL(aboutToShow()), this, SLOT(nextMenuAboutToShow()));

    m_resetButton = new QToolButton();
    m_resetButton->setIcon(KIcon("view-refresh"));
    connect(m_resetButton, SIGNAL(clicked(bool)),this, SLOT(resetHistory()));
}

QToolButton* ContextController::resetButton() const {
    return m_resetButton;
}

void ContextController::resetHistory() {
    m_nextHistoryIndex = 0;
    m_history.clear();
    updateButtonState();
}

ContextController::~ContextController() {
    delete m_nextMenu;
    delete m_previousMenu;
}

void ContextController::updateButtonState()
{
    m_nextButton->setEnabled( m_nextHistoryIndex < m_history.size() );
    m_previousButton->setEnabled( m_nextHistoryIndex >= 2 );
}

void ContextController::historyNext() {
    if(m_nextHistoryIndex >= m_history.size()) {
        return;
    }
    m_view->allowLockedUpdate();
    openDocument(m_nextHistoryIndex); // opening the document at given position 
                                      // will update the widget for us
    ++m_nextHistoryIndex;
    updateButtonState();
}

void ContextController::openDocument(int historyIndex) {
    Q_ASSERT_X(historyIndex >= 0, "openDocument", "negative history index");
    Q_ASSERT_X(historyIndex < m_history.size(), "openDocument", "history index out of range");
    DocumentCursor c = m_history[historyIndex].computePosition();
    if (c.isValid() && !c.document().str().isEmpty()) {
        ICore::self()->documentController()->openDocument(KUrl(c.document().str()), c);
    }
}

void ContextController::historyPrevious() {
    if(m_nextHistoryIndex < 2) {
        return;
    }
    --m_nextHistoryIndex;
    m_view->allowLockedUpdate();
    openDocument(m_nextHistoryIndex-1); // opening the document at given position 
                                        // will update the widget for us
    updateButtonState();
}

QString ContextController::actionTextFor(int historyIndex)
{
    HistoryEntry& entry = m_history[historyIndex];
    QString actionText = entry.context ? entry.context->scopeIdentifier(true).toString() : QString();
    if(actionText.isEmpty())
        actionText = entry.alternativeString;
    if(actionText.isEmpty())
        actionText = "<unnamed>";
    actionText += " @ ";
    QString fileName = KUrl(entry.absoluteCursorPosition.document().str()).fileName();
    actionText += QString("%1:%2").arg(fileName).arg(entry.absoluteCursorPosition.line()+1);
    return actionText;
}

inline QDebug operator<<(QDebug debug, const ContextController::HistoryEntry &he)
{
    DocumentCursor c = he.computePosition();
    debug << "\n\tHistoryEntry " << c.line() << " " << c.document().str();
    return debug;
}

void ContextController::nextMenuAboutToShow() {
    QList<int> indices;
    for(int a = m_nextHistoryIndex; a < m_history.size(); ++a) {
        indices << a;
    }
    fillHistoryPopup(m_nextMenu, indices);
}

void ContextController::previousMenuAboutToShow() {
    QList<int> indices;
    for(int a = m_nextHistoryIndex-2; a >= 0; --a) {
        indices << a;
    }
    fillHistoryPopup(m_previousMenu, indices);
}

void ContextController::fillHistoryPopup(QMenu* menu, const QList<int>& historyIndices) {
    menu->clear();
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    foreach(int index, historyIndices) {
        QAction* action = new QAction(actionTextFor(index), menu);
        action->setData(index);
        menu->addAction(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    }
}

void ContextController::actionTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action); Q_ASSERT(action->data().type() == QVariant::Int);
    int historyPosition = action->data().toInt();
    // kDebug() << "history pos" << historyPosition << m_history.size() << m_history;
    if(historyPosition >= 0 && historyPosition < m_history.size()) {
        m_nextHistoryIndex = historyPosition + 1;
        m_view->allowLockedUpdate(); // opening the document at given position 
                                     // will update the widget for us
        openDocument(historyPosition);
        updateButtonState();
    }
}

ContextController::HistoryEntry::HistoryEntry(DUContextPointer ctx, const KDevelop::SimpleCursor& cursorPosition) : context(ctx) {
        //Use a position relative to the context
        setCursorPosition(cursorPosition);
        if(ctx)
            alternativeString = ctx->scopeIdentifier(true).toString();;
        if(!alternativeString.isEmpty())
            alternativeString += i18n("(changed)"); //This is used when the context was deleted in between
}

DocumentCursor ContextController::HistoryEntry::computePosition() const {
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    DocumentCursor ret;
    if(context) {
        ret = DocumentCursor(context->url().str(), relativeCursorPosition.textCursor());
        ret.setLine(ret.line() + context->range().start.line);
    }else{
        ret = absoluteCursorPosition;
    }
    return ret;
}

void ContextController::HistoryEntry::setCursorPosition(const KDevelop::SimpleCursor& cursorPosition) {
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    if(context) {
        absoluteCursorPosition =  DocumentCursor(context->url().str(), cursorPosition.textCursor());
        relativeCursorPosition = cursorPosition;
        relativeCursorPosition.line -= context->range().start.line;
    }
}

bool ContextController::isPreviousEntry(KDevelop::DUContext* context, const KDevelop::SimpleCursor& position) {
    if (m_nextHistoryIndex == 0) return false;
    Q_ASSERT(m_nextHistoryIndex <= m_history.count());
    HistoryEntry& he = m_history[m_nextHistoryIndex-1];
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() ); // is this necessary??
    Q_ASSERT(he.context); Q_ASSERT(context);
    return (position.line == he.absoluteCursorPosition.line()) && (context->url() == he.context->url());
}

void ContextController::updateHistory(KDevelop::DUContext* context, const KDevelop::SimpleCursor& position)
{
    if (context == 0) return;

    if (isPreviousEntry(context, position)) {
        kDebug() << "Previous history entry, ignoring";
        return;
    } else { // Append new history entry
        m_history.resize(m_nextHistoryIndex); // discard forward history
        m_history.append(HistoryEntry(DUContextPointer(context), position));
        ++m_nextHistoryIndex;

        updateButtonState();
        if(m_history.size() > (maxHistoryLength + 5)) {
            m_history = m_history.mid(m_history.size() - maxHistoryLength);
            m_nextHistoryIndex = m_history.size();
        }
    }
}

QWidget* ContextController::createWidget(KDevelop::DUContext* context) {
        m_context = DUContextPointer(context);
        if(m_context) {
            return m_context->createNavigationWidget();
        }
        return 0;
}

DeclarationController::DeclarationController() {
}

QWidget* DeclarationController::createWidget(Declaration* decl, TopDUContext* topContext) {
    m_declaration = DeclarationPointer(decl);
    return decl->context()->createNavigationWidget(decl, topContext);
}


void ContextBrowserView::resetWidget()
{
    if (m_navigationWidget) {
        delete m_navigationWidget;
        m_navigationWidget = 0;
    }
}

void ContextBrowserView::updateLockIcon(bool checked) {
    m_lockButton->setIcon(KIcon(checked ? "document-encrypt" : "document-decrypt"));
}

ContextBrowserView::ContextBrowserView( ContextBrowserPlugin* plugin ) : m_plugin(plugin), m_navigationWidget(new KTextBrowser()) {
    setWindowIcon( KIcon("applications-development-web") );

    m_declarationCtrl = new DeclarationController();
    m_contextCtrl = new ContextController(this);
    
    QHBoxLayout* buttons = new QHBoxLayout;
    m_lockButton = new QToolButton();
    m_lockButton->setCheckable(true);
    m_lockButton->setChecked(false);
    updateLockIcon(m_lockButton->isChecked());
    connect(m_lockButton, SIGNAL(toggled(bool)), SLOT(updateLockIcon(bool)));

    buttons->addWidget(m_contextCtrl->previousButton());
    buttons->addWidget(m_contextCtrl->nextButton());
    buttons->addWidget(m_contextCtrl->resetButton());
    buttons->addStretch();
    buttons->addWidget(m_lockButton);

    m_layout = new QVBoxLayout;
    m_layout->addLayout(buttons);
    m_layout->addWidget(m_navigationWidget);
    //m_layout->addStretch();
    setLayout(m_layout);

    m_plugin->registerToolView(this);
    connect(plugin, SIGNAL(previousContextShortcut()), m_contextCtrl, SLOT(historyPrevious()));
    connect(plugin, SIGNAL(nextContextShortcut()), m_contextCtrl, SLOT(historyNext()));
}

ContextBrowserView::~ContextBrowserView() {
    m_plugin->unRegisterToolView(this);
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

void ContextBrowserView::updateHistory(KDevelop::DUContext* context, const KDevelop::SimpleCursor& position)
{
    if (!isLocked()) {
        m_contextCtrl->updateHistory(context, position);
    }
}

void ContextBrowserView::updateMainWidget(QWidget* widget)
{
    if (widget) {
        kDebug() << "";
        resetWidget();
        m_navigationWidget = widget;
        m_layout->insertWidget(1, widget, 1);
        m_allowLockedUpdate = false;
    }
}

void ContextBrowserView::setDeclaration(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext) {
    if (!isLocked() && isVisible()) {  // NO-OP if toolview is hidden, for performance reasons
        QWidget* w = m_declarationCtrl->createWidget(decl, topContext);
        updateMainWidget(w);
    }
}

void ContextBrowserView::allowLockedUpdate() {
    m_allowLockedUpdate = true;
}

void ContextBrowserView::setContext(KDevelop::DUContext* context) {
    if (!isLocked() && isVisible()) { // NO-OP if toolview is hidden, for performance reasons
        QWidget* w = m_contextCtrl->createWidget(context);
        updateMainWidget(w);
    }
}

void ContextBrowserView::setSpecialNavigationWidget(QWidget* widget) {
    if (!isLocked() && isVisible()) {
        Q_ASSERT(widget);
        updateMainWidget(widget);
    }
}

#include "contextbrowserview.moc"
