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
#include <QShowEvent>
#include <QAction>
#include <QMenu>
#include <KIcon>
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
#include <language/duchain/indexedstring.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include "contextbrowser.h"
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/specializationstore.h>
#include "browsemanager.h"
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <kparts/part.h>

const int maxHistoryLength = 30;

using namespace KDevelop;

QToolButton* ContextController::previousButton() const {
    return m_previousButton;
}

QToolButton* ContextController::nextButton() const {
    return m_nextButton;
}

QToolButton* ContextController::browseButton() const {
    return m_browseButton;
}

ContextController::ContextController(ContextBrowserView* view) : m_nextHistoryIndex(0), m_view(view) {
    m_browseManager = new BrowseManager(this);
    
    connect(ICore::self()->documentController(), SIGNAL(documentJumpPerformed(KDevelop::IDocument*, KTextEditor::Cursor, KDevelop::IDocument*, KTextEditor::Cursor)), this, SLOT(documentJumpPerformed(KDevelop::IDocument*, KTextEditor::Cursor, KDevelop::IDocument*, KTextEditor::Cursor)));
    
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

    m_browseButton = new QToolButton();
    m_browseButton->setIcon(KIcon("games-hint"));
    m_browseButton->setToolTip(i18n("Enable/disable source browse mode"));
    m_browseButton->setWhatsThis(i18n("When this is enabled, you can browse the source-code by clicking in the editor."));
    m_browseButton->setCheckable(true);
    connect(m_browseButton, SIGNAL(clicked(bool)), m_browseManager, SLOT(setBrowsing(bool)));

    m_currentContextBox = new KComboBox();
    m_currentContextBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(m_currentContextBox, SIGNAL(activated(int)), this, SLOT(comboItemActivated(int)));
}

///Duchain must be locked
DUContext* getContextAt(KUrl url, KTextEditor::Cursor cursor) {
    TopDUContext* topContext = DUChainUtils::standardContextForUrl(url);
    if (!topContext) return 0;
    return contextAt(SimpleCursor(cursor), topContext);
}

void ContextController::documentJumpPerformed( KDevelop::IDocument* newDocument, KTextEditor::Cursor newCursor, KDevelop::IDocument* previousDocument, KTextEditor::Cursor previousCursor) {
    if(newCursor.isValid() && previousCursor.isValid()) {
        
        KUrl oldIgnore = m_ignoreJump;
        m_ignoreJump = KUrl();
        if(newDocument->url() == oldIgnore)
            return;
        
        DUChainReadLocker lock(DUChain::lock());
        
        if(previousDocument && previousCursor.isValid())
            updateHistory(getContextAt(previousDocument->url(), previousCursor), SimpleCursor(previousCursor), true);
        
        if(newDocument && newCursor.isValid())
            updateHistory(getContextAt(newDocument->url(), newCursor), SimpleCursor(newCursor), true);
    }
}

ContextBrowserView* ContextController::view() const {
    return m_view;
}

KComboBox* ContextController::currentContextBox() const {
    return m_currentContextBox;
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

void ContextController::comboItemActivated(int index)
{
    if(index >= 0 && index < m_listDeclarations.size()) {
        IndexedString u;
        SimpleCursor c;
        {
            KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
            Declaration* activated = m_listDeclarations[index].data();
            if(activated) {
                u = activated->url();
                c = activated->range().start;
                if(activated->internalContext() && activated->internalContext()->url() == u) {
                    c = activated->internalContext()->range().start;
                    if(c.line+1 <= activated->internalContext()->range().end.line)
                        c = SimpleCursor(c.line+1, 0); //Move more into the body
                }
            }
        }
        if(c.isValid())
            ICore::self()->documentController()->openDocument(KUrl(u.str()), c.textCursor());
    }
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
        m_ignoreJump = KUrl(c.document().str());
        ICore::self()->documentController()->openDocument(m_ignoreJump, c);

        KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
        updateDeclarationListBox(m_history[historyIndex].context.data());
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
    QString actionText = entry.context.data() ? entry.context.data()->scopeIdentifier(true).toString() : QString();
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

ContextController::HistoryEntry::HistoryEntry(IndexedDUContext ctx, const KDevelop::SimpleCursor& cursorPosition) : context(ctx) {
        //Use a position relative to the context
        setCursorPosition(cursorPosition);
        if(ctx.data())
            alternativeString = ctx.data()->scopeIdentifier(true).toString();;
        if(!alternativeString.isEmpty())
            alternativeString += i18n("(changed)"); //This is used when the context was deleted in between
}

DocumentCursor ContextController::HistoryEntry::computePosition() const {
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    DocumentCursor ret;
    if(context.data()) {
        ret = DocumentCursor(context.data()->url().str(), relativeCursorPosition.textCursor());
        ret.setLine(ret.line() + context.data()->range().start.line);
    }else{
        ret = absoluteCursorPosition;
    }
    return ret;
}

void ContextController::HistoryEntry::setCursorPosition(const KDevelop::SimpleCursor& cursorPosition) {
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    if(context.data()) {
        absoluteCursorPosition =  DocumentCursor(context.data()->url().str(), cursorPosition.textCursor());
        relativeCursorPosition = cursorPosition;
        relativeCursorPosition.line -= context.data()->range().start.line;
    }
}

bool ContextController::isPreviousEntry(KDevelop::DUContext* context, const KDevelop::SimpleCursor& /*position*/) {
    if (m_nextHistoryIndex == 0) return false;
    Q_ASSERT(m_nextHistoryIndex <= m_history.count());
    HistoryEntry& he = m_history[m_nextHistoryIndex-1];
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() ); // is this necessary??
    Q_ASSERT(context);
    return IndexedDUContext(context) == he.context;
}

void ContextController::updateDeclarationListBox(DUContext* context) {
    if(!context) {
        m_listUrl = IndexedString(); ///@todo Compute the context in the document here
        m_currentContextBox->clear();
        return;
    }
    m_listUrl = context->url();

    class FunctionListFilter : public DUChainUtils::DUChainItemFilter {
        public:
        FunctionListFilter(KComboBox* box, QList<IndexedDeclaration>& _declarations) : m_box(box), declarations(_declarations) {
            box->clear();
            declarations.clear();
        }
        
        virtual bool accept(Declaration* decl) {
            if(decl->range().isEmpty())
                return false;
                
            if(decl->isFunctionDeclaration() || (decl->internalContext() && decl->internalContext()->type() == DUContext::Class)) {
                Declaration* specialDecl = SpecializationStore::self().applySpecialization(decl, decl->topContext());

                FunctionType::Ptr function = specialDecl->type<FunctionType>();
                QString text = specialDecl->qualifiedIdentifier().toString();
                if(function)
                    text += function->partToString(KDevelop::FunctionType::SignatureArguments);

                m_box->addItem(text);
                declarations.append(decl);
            }

            return true;
        }
        virtual bool accept(DUContext* ctx) {
        return ctx->type() == DUContext::Global || ctx->type() == DUContext::Namespace || ctx->type() == DUContext::Class;
        }
        KComboBox* m_box;
        QList<IndexedDeclaration>& declarations;
    };

    m_currentContextBox->clear();
    FunctionListFilter f(currentContextBox(), m_listDeclarations);
    DUChainUtils::collectItems(context->topContext(), f);
    currentContextBox()->setCurrentIndex(m_listDeclarations.indexOf(context->owner()));
}

void ContextController::updateHistory(KDevelop::DUContext* context, const KDevelop::SimpleCursor& position, bool force)
{
    if (context == 0) return;
    if(!context->owner() && !force)
        return; //Only add history-entries for contexts that have owners, which in practice should be functions and classes
                //This keeps the history cleaner

    if (!isPreviousEntry(context, position) || context->url() != m_listUrl)
        if(m_currentContextBox->isVisible())
            updateDeclarationListBox(context);

    if (isPreviousEntry(context, position)) {
        if(m_nextHistoryIndex) {
            HistoryEntry& he = m_history[m_nextHistoryIndex-1];
            he.setCursorPosition(position);
        }
        return;
    } else { // Append new history entry
        //To have a more useful history, do not duplicate items in it, so remove old occurrences of the same context
        for(int a = 0; a < m_nextHistoryIndex; ++a) {
            if(m_history[a].context == IndexedDUContext(context)) {
                m_history.remove(a);
                --m_nextHistoryIndex;
            }
        }

        m_history.resize(m_nextHistoryIndex); // discard forward history
        m_history.append(HistoryEntry(IndexedDUContext(context), position));
        ++m_nextHistoryIndex;

        updateButtonState();
        if(m_history.size() > (maxHistoryLength + 5)) {
            m_history = m_history.mid(m_history.size() - maxHistoryLength);
            m_nextHistoryIndex = m_history.size();
        }
    }
}

QWidget* ContextController::createWidget(KDevelop::DUContext* context) {
        m_context = IndexedDUContext(context);
        if(m_context.data()) {
            return m_context.data()->createNavigationWidget();
        }
        return 0;
}

DeclarationController::DeclarationController() {
}

KDevelop::IndexedDeclaration DeclarationController::declaration() {
    return m_declaration;
}

QWidget* DeclarationController::createWidget(Declaration* decl, TopDUContext* topContext) {
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

void ContextBrowserView::updateLockIcon(bool checked) {
    m_lockButton->setIcon(KIcon(checked ? "document-encrypt" : "document-decrypt"));
}

ContextBrowserView::ContextBrowserView( ContextBrowserPlugin* plugin ) : m_plugin(plugin), m_navigationWidget(new KTextBrowser()) {
    setWindowIcon( KIcon("applications-development-web") );

    m_declarationCtrl = new DeclarationController();
    m_contextCtrl = new ContextController(this);
    
    m_allowLockedUpdate = false;
    
    QHBoxLayout* buttons = new QHBoxLayout;
    m_lockButton = new QToolButton();
    m_lockButton->setCheckable(true);
    m_lockButton->setChecked(false);
    updateLockIcon(m_lockButton->isChecked());
    connect(m_lockButton, SIGNAL(toggled(bool)), SLOT(updateLockIcon(bool)));

    buttons->addWidget(m_contextCtrl->previousButton());
    buttons->addWidget(m_contextCtrl->currentContextBox());
    buttons->addWidget(m_contextCtrl->nextButton());
    buttons->addWidget(m_contextCtrl->browseButton());
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

void ContextBrowserView::showEvent(QShowEvent* event) {
    DUChainReadLocker lock(DUChain::lock());
    if(m_navigationWidgetDeclaration.isValid()) {
        TopDUContext* top = m_lastUsedTopContext.data();
        if(top) {
            //Update the navigation-widget
            Declaration* decl = m_navigationWidgetDeclaration.getDeclaration(top);
            setDeclaration(decl, top, true);
            
            //Update the declaration combo-box
            DUContext* context = 0;
            KDevelop::IDocument* doc = ICore::self()->documentController()->activeDocument();
            if(doc && doc->textDocument() && doc->textDocument()->activeView()) {
                KTextEditor::Cursor c = doc->textDocument()->activeView()->cursorPosition();
                context = getContextAt(top->url().toUrl(), c);
            }
            m_contextCtrl->updateDeclarationListBox(context);
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

void ContextBrowserView::updateHistory(KDevelop::DUContext* context, const KDevelop::SimpleCursor& position)
{
    if (!isLocked()) {
        m_contextCtrl->updateHistory(context, position);
    }
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
    }
}

void ContextBrowserView::setDeclaration(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext, bool force) {
    m_lastUsedTopContext = IndexedTopDUContext(topContext);
    
    if(m_navigationWidgetDeclaration == decl->id() && !force)
        return;
    m_navigationWidgetDeclaration = decl->id();
    
    if (!isLocked() && (isVisible() || force)) {  // NO-OP if toolview is hidden, for performance reasons
        
        QWidget* w = m_declarationCtrl->createWidget(decl, topContext);
        updateMainWidget(w);
    }
}

KDevelop::IndexedDeclaration ContextBrowserView::lockedDeclaration() const {
    if(m_lockButton->isChecked())
        return m_declarationCtrl->declaration();
    else
        return KDevelop::IndexedDeclaration();
}

void ContextBrowserView::allowLockedUpdate() {
    m_allowLockedUpdate = true;
}

void ContextBrowserView::setContext(KDevelop::DUContext* context) {
    m_lastUsedTopContext = IndexedTopDUContext(context->topContext());
    
    if(context->owner()) {
        if(context->owner()->id() == m_navigationWidgetDeclaration)
            return;
        m_navigationWidgetDeclaration = context->owner()->id();
    }else{
        m_navigationWidgetDeclaration = DeclarationId();
    }
    
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
