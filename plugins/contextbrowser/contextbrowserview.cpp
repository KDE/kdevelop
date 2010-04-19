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
#include <qapplication.h>
#include <language/interfaces/codecontext.h>
#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iplugincontroller.h>
#include <ktexteditor/codecompletioninterface.h>
#include <language/interfaces/iquickopen.h>

const int maxHistoryLength = 30;

using namespace KDevelop;

///Duchain must be locked
DUContext* getContextAt(KUrl url, KTextEditor::Cursor cursor) {
    TopDUContext* topContext = DUChainUtils::standardContextForUrl(url);
    if (!topContext) return 0;
    return contextAt(SimpleCursor(cursor), topContext);
}

static bool useNavigationFromView(QObject* viewObject) {
    KTextEditor::View* view = qobject_cast<KTextEditor::View*>(viewObject);
    if(!view) {
        kWarning() << "sender is not a view";
        return false;
    }
    KTextEditor::CodeCompletionInterface* iface = dynamic_cast<KTextEditor::CodeCompletionInterface*>(view);
    if(!iface || iface->isCompletionActive())
        return false;
    
    return true;
}

void ContextBrowserView::navigateAccept() {
    if(!useNavigationFromView(sender()))
        return;
    
    if(AbstractNavigationWidget* widget = dynamic_cast<AbstractNavigationWidget*>(m_navigationWidget.data()))
        widget->accept();
}

void ContextBrowserView::navigateBack() {
    if(!useNavigationFromView(sender()))
        return;
    
    if(AbstractNavigationWidget* widget = dynamic_cast<AbstractNavigationWidget*>(m_navigationWidget.data()))
        widget->back();
}

void ContextBrowserView::navigateDown() {
    if(!useNavigationFromView(sender()))
        return;
    
    if(AbstractNavigationWidget* widget = dynamic_cast<AbstractNavigationWidget*>(m_navigationWidget.data()))
        widget->down();
}

void ContextBrowserView::navigateLeft() {
    if(!useNavigationFromView(sender()))
        return;
    
    if(AbstractNavigationWidget* widget = dynamic_cast<AbstractNavigationWidget*>(m_navigationWidget.data()))
        widget->previous();
}

void ContextBrowserView::navigateRight() {
    if(!useNavigationFromView(sender()))
        return;
    
    if(AbstractNavigationWidget* widget = dynamic_cast<AbstractNavigationWidget*>(m_navigationWidget.data()))
        widget->next();
}

void ContextBrowserView::navigateUp() {
    if(!useNavigationFromView(sender()))
        return;
    
    if(AbstractNavigationWidget* widget = dynamic_cast<AbstractNavigationWidget*>(m_navigationWidget.data()))
        widget->up();
}

void ContextBrowserView::documentJumpPerformed( KDevelop::IDocument* newDocument, KTextEditor::Cursor newCursor, KDevelop::IDocument* previousDocument, KTextEditor::Cursor previousCursor) {
        
    DUChainReadLocker lock(DUChain::lock());
    
    if(newDocument && newDocument->textDocument() && newDocument->textDocument()->activeView() && masterWidget(newDocument->textDocument()->activeView()) != masterWidget(this))
        return;
    
    if(previousDocument && previousCursor.isValid()) {
        kDebug() << "updating jump source";
        DUContext* context = getContextAt(previousDocument->url(), previousCursor);
        if(context) {
            updateHistory(context, SimpleCursor(previousCursor), true);
        }else{
            //We just want this place in the history
            m_history.resize(m_nextHistoryIndex); // discard forward history
            m_history.append(HistoryEntry(DocumentCursor(previousDocument->url().prettyUrl(), previousCursor)));
            ++m_nextHistoryIndex;
        }
    }
    kDebug() << "new doc: " << newDocument << " new cursor: " << newCursor;
    if(newDocument && newCursor.isValid()) {
        kDebug() << "updating jump target";
        DUContext* context = getContextAt(newDocument->url(), newCursor);
        if(context) {
            updateHistory(context, SimpleCursor(newCursor), true);
        }else{
            //We just want this place in the history
            m_history.resize(m_nextHistoryIndex); // discard forward history
            m_history.append(HistoryEntry(DocumentCursor(newDocument->url().prettyUrl(), newCursor)));
            ++m_nextHistoryIndex;
            m_outlineLine->clear();
        }
    }
}

void ContextBrowserView::updateButtonState()
{
    m_nextButton->setEnabled( m_nextHistoryIndex < m_history.size() );
    m_previousButton->setEnabled( m_nextHistoryIndex >= 2 );
}

void ContextBrowserView::historyNext() {
    if(m_nextHistoryIndex >= m_history.size()) {
        return;
    }
    allowLockedUpdate();
    openDocument(m_nextHistoryIndex); // opening the document at given position 
                                      // will update the widget for us
    ++m_nextHistoryIndex;
    updateButtonState();
}

void ContextBrowserView::openDocument(int historyIndex) {
    Q_ASSERT_X(historyIndex >= 0, "openDocument", "negative history index");
    Q_ASSERT_X(historyIndex < m_history.size(), "openDocument", "history index out of range");
    DocumentCursor c = m_history[historyIndex].computePosition();
    if (c.isValid() && !c.document().str().isEmpty()) {
        
        disconnect(ICore::self()->documentController(), SIGNAL(documentJumpPerformed(KDevelop::IDocument*, KTextEditor::Cursor, KDevelop::IDocument*, KTextEditor::Cursor)), this,      SLOT(documentJumpPerformed(KDevelop::IDocument*, KTextEditor::Cursor, KDevelop::IDocument*, KTextEditor::Cursor)));
        
        ICore::self()->documentController()->openDocument(KUrl(c.document().str()), c);
        
        connect(ICore::self()->documentController(), SIGNAL(documentJumpPerformed(KDevelop::IDocument*, KTextEditor::Cursor, KDevelop::IDocument*, KTextEditor::Cursor)), this, SLOT(documentJumpPerformed(KDevelop::IDocument*, KTextEditor::Cursor, KDevelop::IDocument*, KTextEditor::Cursor)));

        KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
        updateDeclarationListBox(m_history[historyIndex].context.data());
    }
}

void ContextBrowserView::historyPrevious() {
    if(m_nextHistoryIndex < 2) {
        return;
    }
    --m_nextHistoryIndex;
    allowLockedUpdate();
    openDocument(m_nextHistoryIndex-1); // opening the document at given position 
                                        // will update the widget for us
    updateButtonState();
}

QString ContextBrowserView::actionTextFor(int historyIndex)
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

inline QDebug operator<<(QDebug debug, const ContextBrowserView::HistoryEntry &he)
{
    DocumentCursor c = he.computePosition();
    debug << "\n\tHistoryEntry " << c.line() << " " << c.document().str();
    return debug;
}

void ContextBrowserView::nextMenuAboutToShow() {
    QList<int> indices;
    for(int a = m_nextHistoryIndex; a < m_history.size(); ++a) {
        indices << a;
    }
    fillHistoryPopup(m_nextMenu, indices);
}

void ContextBrowserView::previousMenuAboutToShow() {
    QList<int> indices;
    for(int a = m_nextHistoryIndex-2; a >= 0; --a) {
        indices << a;
    }
    fillHistoryPopup(m_previousMenu, indices);
}

void ContextBrowserView::fillHistoryPopup(QMenu* menu, const QList<int>& historyIndices) {
    menu->clear();
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    foreach(int index, historyIndices) {
        QAction* action = new QAction(actionTextFor(index), menu);
        action->setData(index);
        menu->addAction(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    }
}

void ContextBrowserView::switchFocusToContextBrowser() {
    if(isVisible()) {
        kDebug() << "switching focus to context-browser";
        if(QApplication::focusWidget() != this)
            m_focusBackWidget = QApplication::focusWidget();
        setFocus();
    }
}

void ContextBrowserView::actionTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action); Q_ASSERT(action->data().type() == QVariant::Int);
    int historyPosition = action->data().toInt();
    // kDebug() << "history pos" << historyPosition << m_history.size() << m_history;
    if(historyPosition >= 0 && historyPosition < m_history.size()) {
        m_nextHistoryIndex = historyPosition + 1;
        allowLockedUpdate(); // opening the document at given position 
                                     // will update the widget for us
        openDocument(historyPosition);
        updateButtonState();
    }
}

ContextBrowserView::HistoryEntry::HistoryEntry(KDevelop::DocumentCursor pos) : absoluteCursorPosition(pos) {
}

ContextBrowserView::HistoryEntry::HistoryEntry(IndexedDUContext ctx, const KDevelop::SimpleCursor& cursorPosition) : context(ctx) {
        //Use a position relative to the context
        setCursorPosition(cursorPosition);
        if(ctx.data())
            alternativeString = ctx.data()->scopeIdentifier(true).toString();
        if(!alternativeString.isEmpty())
            alternativeString += i18n("(changed)"); //This is used when the context was deleted in between
}

DocumentCursor ContextBrowserView::HistoryEntry::computePosition() const {
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

void ContextBrowserView::HistoryEntry::setCursorPosition(const KDevelop::SimpleCursor& cursorPosition) {
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    if(context.data()) {
        absoluteCursorPosition =  DocumentCursor(context.data()->url().str(), cursorPosition.textCursor());
        relativeCursorPosition = cursorPosition;
        relativeCursorPosition.line -= context.data()->range().start.line;
    }
}

bool ContextBrowserView::isPreviousEntry(KDevelop::DUContext* context, const KDevelop::SimpleCursor& /*position*/) {
    if (m_nextHistoryIndex == 0) return false;
    Q_ASSERT(m_nextHistoryIndex <= m_history.count());
    HistoryEntry& he = m_history[m_nextHistoryIndex-1];
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() ); // is this necessary??
    Q_ASSERT(context);
    return IndexedDUContext(context) == he.context;
}

void ContextBrowserView::updateDeclarationListBox(DUContext* context) {
    if(!context || !context->owner()) {
        kDebug() << "not updating box";
        m_listUrl = IndexedString(); ///@todo Compute the context in the document here
        m_outlineLine->clear();
        return;
    }
    
    Declaration* decl = context->owner();
    
    m_listUrl = context->url();

    Declaration* specialDecl = SpecializationStore::self().applySpecialization(decl, decl->topContext());

    FunctionType::Ptr function = specialDecl->type<FunctionType>();
    QString text = specialDecl->qualifiedIdentifier().toString();
    if(function)
        text += function->partToString(KDevelop::FunctionType::SignatureArguments);
    
    if(!m_outlineLine->hasFocus())
    {
        m_outlineLine->setText(text);
        m_outlineLine->setCursorPosition(0); 
    } 
    
    kDebug() << "updated" << text;
}

void ContextBrowserView::updateHistory(KDevelop::DUContext* context, const KDevelop::SimpleCursor& position, bool force)
{
    kDebug() << "updating history";
    
    if(m_outlineLine->isVisible())
        updateDeclarationListBox(context);
    
    if(!context && !context->owner() && !force) {
        return; //Only add history-entries for contexts that have owners, which in practice should be functions and classes
                //This keeps the history cleaner
    }

    if (isPreviousEntry(context, position)) {
        if(m_nextHistoryIndex) {
            HistoryEntry& he = m_history[m_nextHistoryIndex-1];
            he.setCursorPosition(position);
        }
        return;
    } else { // Append new history entry
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
    m_lockButton->setIcon(KIcon(checked ? "document-encrypt" : "document-decrypt"));
}

ContextBrowserView::ContextBrowserView( ContextBrowserPlugin* plugin, QWidget* parent ) : QWidget(parent), m_plugin(plugin), m_navigationWidget(new KTextBrowser()), m_nextHistoryIndex(0), m_outlineLine(0) {
    setWindowIcon( KIcon("applications-development-web") );

    m_browseManager = new BrowseManager(this);
    
    connect(ICore::self()->documentController(), SIGNAL(documentJumpPerformed(KDevelop::IDocument*, KTextEditor::Cursor, KDevelop::IDocument*, KTextEditor::Cursor)), this, SLOT(documentJumpPerformed(KDevelop::IDocument*, KTextEditor::Cursor, KDevelop::IDocument*, KTextEditor::Cursor)));
    
    connect(m_browseManager, SIGNAL(shiftKeyTriggered()), this, SLOT(switchFocusToContextBrowser()));
    
    m_previousButton = new QToolButton();
    m_previousButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_previousButton->setIcon(KIcon("go-previous"));
    m_previousButton->setEnabled(false);
    m_previousButton->setFocusPolicy(Qt::NoFocus);
    m_previousMenu = new QMenu();
    m_previousButton->setMenu(m_previousMenu);
    connect(m_previousButton, SIGNAL(clicked(bool)), this, SLOT(historyPrevious()));
    connect(m_previousMenu, SIGNAL(aboutToShow()), this, SLOT(previousMenuAboutToShow()));

    m_nextButton = new QToolButton();
    m_nextButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_nextButton->setIcon(KIcon("go-next"));
    m_nextButton->setEnabled(false);
    m_nextButton->setFocusPolicy(Qt::NoFocus);
    m_nextMenu = new QMenu();
    m_nextButton->setMenu(m_nextMenu);
    connect(m_nextButton, SIGNAL(clicked(bool)), this, SLOT(historyNext()));
    connect(m_nextMenu, SIGNAL(aboutToShow()), this, SLOT(nextMenuAboutToShow()));

    m_browseButton = new QToolButton();
    m_browseButton->setIcon(KIcon("games-hint"));
    m_browseButton->setToolTip(i18n("Enable/disable source browse mode"));
    m_browseButton->setWhatsThis(i18n("When this is enabled, you can browse the source-code by clicking in the editor."));
    m_browseButton->setCheckable(true);
    m_browseButton->setFocusPolicy(Qt::NoFocus);
    
    connect(m_browseButton, SIGNAL(clicked(bool)), m_browseManager, SLOT(setBrowsing(bool)));

    IQuickOpen* quickOpen = KDevelop::ICore::self()->pluginController()->extensionForPlugin<IQuickOpen>("org.kdevelop.IQuickOpen");
    
    if(quickOpen) {
      m_outlineLine = quickOpen->createQuickOpenLine(QStringList(), QStringList() << i18n("Outline"), IQuickOpen::Outline);
      m_outlineLine->setDefaultText(i18n("Outline..."));
    }
    
    connect(m_browseManager, SIGNAL(startDelayedBrowsing(KTextEditor::View*)), this, SIGNAL(startDelayedBrowsing(KTextEditor::View*)));
    connect(m_browseManager, SIGNAL(stopDelayedBrowsing()), this, SIGNAL(stopDelayedBrowsing()));
    
    m_allowLockedUpdate = false;
    
    m_buttons = new QHBoxLayout;
    m_lockButton = new QToolButton();
    m_lockButton->setCheckable(true);
    m_lockButton->setChecked(false);
    m_lockButton->setToolTip(i18n("Lock current view"));
    updateLockIcon(m_lockButton->isChecked());
    connect(m_lockButton, SIGNAL(toggled(bool)), SLOT(updateLockIcon(bool)));

    m_declarationMenuButton = new QToolButton();
    m_declarationMenuButton->setIcon(KIcon("code-class"));
    m_declarationMenuButton->setToolTip(i18n("Declaration menu"));
    connect(m_declarationMenuButton, SIGNAL(clicked(bool)), SLOT(declarationMenu()));
    
    m_toolbarWidget = plugin->toolbarWidgetForMainWindow(this);
    m_toolbarWidgetLayout = new QHBoxLayout;
    m_toolbarWidgetLayout->setSizeConstraint(QLayout::SetMaximumSize);
    m_previousButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_nextButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_browseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_toolbarWidgetLayout->setMargin(0);
    
    m_toolbarWidgetLayout->addWidget(m_previousButton);
    m_toolbarWidgetLayout->addWidget(m_outlineLine);
    m_outlineLine->setMaximumWidth(600);
    m_toolbarWidgetLayout->addWidget(m_nextButton);
    m_toolbarWidgetLayout->addWidget(m_browseButton);
    
    if(m_toolbarWidget->children().isEmpty())
        m_toolbarWidget->setLayout(m_toolbarWidgetLayout);
    
    m_buttons->addStretch();
    m_buttons->addWidget(m_declarationMenuButton);
    m_buttons->addWidget(m_lockButton);

    m_layout = new QVBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->addLayout(m_buttons);
    m_layout->addWidget(m_navigationWidget);
    //m_layout->addStretch();
    setLayout(m_layout);

    m_plugin->registerToolView(this);
    
    connect(ICore::self()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*)), m_outlineLine, SLOT(clear()));
    connect(ICore::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), m_outlineLine, SLOT(clear()));
}

ContextBrowserView::~ContextBrowserView() {
    m_plugin->unRegisterToolView(this);
    delete m_nextMenu;
    delete m_previousMenu;
    delete m_toolbarWidgetLayout;
    
    delete m_previousButton;
    delete m_outlineLine;
    delete m_nextButton;
    delete m_browseButton;
    
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
            DUContext* context = 0;
            KDevelop::IDocument* doc = ICore::self()->documentController()->activeDocument();
            if(doc && doc->textDocument() && doc->textDocument()->activeView()) {
                KTextEditor::Cursor c = doc->textDocument()->activeView()->cursorPosition();
                context = getContextAt(top->url().toUrl(), c);
            }
            updateDeclarationListBox(context);
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
    }
}

void ContextBrowserView::setDeclaration(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext, bool force) {
    m_lastUsedTopContext = IndexedTopDUContext(topContext);
    
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
    }
}

void ContextBrowserView::setAllowBrowsing(bool allow) {
    m_browseButton->setChecked(allow);
}


#include "contextbrowserview.moc"
