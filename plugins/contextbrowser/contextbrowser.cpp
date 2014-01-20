/*
  * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include "contextbrowser.h"

#include "contextbrowserview.h"
#include "browsemanager.h"

#include <cstdlib>

#include <QAction>
#include <QWidgetAction>
#include <QTimer>
#include <QToolButton>
#include <QLayout>
#include <QMenu>
#include <qalgorithms.h>

#include <KLocale>
#include <KActionCollection>
#include <KAboutData>
#include <KDebug>

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KTextEditor/TextHintInterface>
#include <KTextEditor/CodeCompletionInterface>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iplugincontroller.h>

#include <language/interfaces/codecontext.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/interfaces/iquickopen.h>

#include <language/duchain/duchain.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/use.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/uses.h>
#include <language/duchain/specializationstore.h>
#include <language/duchain/aliasdeclaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/navigation/abstractnavigationwidget.h>

#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>

#include <language/util/navigationtooltip.h>

#include <sublime/mainwindow.h>

using KDevelop::ILanguage;
using KTextEditor::Attribute;
using KTextEditor::View;

// Helper that follows the QObject::parent() chain, and returns the highest widget that has no parent.
QWidget* masterWidget(QWidget* w)
{
  while(w && w->parent() && qobject_cast<QWidget*>(w->parent()))
    w = qobject_cast<QWidget*>(w->parent());
  return w;
}

namespace {
const unsigned int highlightingTimeout = 150;
const float highlightingZDepth = -5000;
const int maxHistoryLength = 30;

// Helper that determines the context to use for highlighting at a specific position
DUContext* contextForHighlightingAt(const SimpleCursor& position, TopDUContext* topContext)
{
  DUContext* ctx = topContext->findContextAt(topContext->transformToLocalRevision(position));
  while(ctx && ctx->parentContext()
        && (ctx->type() == DUContext::Template || ctx->type() == DUContext::Helper
            || ctx->localScopeIdentifier().isEmpty()))
  {
    ctx = ctx->parentContext();
  }
  return ctx;
}

///Duchain must be locked
DUContext* getContextAt(const KUrl& url, KTextEditor::Cursor cursor)
{
  TopDUContext* topContext = DUChainUtils::standardContextForUrl(url);
  if (!topContext) return 0;
  return contextForHighlightingAt(SimpleCursor(cursor), topContext);
}

DeclarationPointer cursorDeclaration()
{
  IDocument* doc = ICore::self()->documentController()->activeDocument();
  if (!doc) {
    return DeclarationPointer();
  }

  KTextEditor::Document* textDoc = doc->textDocument();
  if (!textDoc) {
    return DeclarationPointer();
  }

  KTextEditor::View* view = textDoc->activeView();
  if (!view) {
    return DeclarationPointer();
  }

  DUChainReadLocker lock;

  Declaration *decl = DUChainUtils::declarationForDefinition(DUChainUtils::itemUnderCursor(doc->url(), SimpleCursor(view->cursorPosition())));
  return DeclarationPointer(decl);
}

}

class ContextBrowserViewFactory: public KDevelop::IToolViewFactory
{
public:
    ContextBrowserViewFactory(ContextBrowserPlugin *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        ContextBrowserView* ret = new ContextBrowserView(m_plugin, parent);
        return ret;
    }

    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::BottomDockWidgetArea;
    }

    virtual QString id() const
    {
        return "org.kdevelop.ContextBrowser";
    }

private:
    ContextBrowserPlugin *m_plugin;
};

KXMLGUIClient* ContextBrowserPlugin::createGUIForMainWindow( Sublime::MainWindow* window )
{
    KXMLGUIClient* ret = KDevelop::IPlugin::createGUIForMainWindow( window );
    
    m_browseManager = new BrowseManager(this);
    
    connect(ICore::self()->documentController(), SIGNAL(documentJumpPerformed(KDevelop::IDocument*,KTextEditor::Cursor,KDevelop::IDocument*,KTextEditor::Cursor)), this, SLOT(documentJumpPerformed(KDevelop::IDocument*,KTextEditor::Cursor,KDevelop::IDocument*,KTextEditor::Cursor)));
    
    m_previousButton = new QToolButton();
    m_previousButton->setToolTip(i18n("Go back in context history"));
    m_previousButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_previousButton->setIcon(QIcon::fromTheme("go-previous"));
    m_previousButton->setEnabled(false);
    m_previousButton->setFocusPolicy(Qt::NoFocus);
    m_previousMenu = new QMenu();
    m_previousButton->setMenu(m_previousMenu);
    connect(m_previousButton, SIGNAL(clicked(bool)), this, SLOT(historyPrevious()));
    connect(m_previousMenu, SIGNAL(aboutToShow()), this, SLOT(previousMenuAboutToShow()));

    m_nextButton = new QToolButton();
    m_nextButton->setToolTip(i18n("Go forward in context history"));
    m_nextButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_nextButton->setIcon(QIcon::fromTheme("go-next"));
    m_nextButton->setEnabled(false);
    m_nextButton->setFocusPolicy(Qt::NoFocus);
    m_nextMenu = new QMenu();
    m_nextButton->setMenu(m_nextMenu);
    connect(m_nextButton, SIGNAL(clicked(bool)), this, SLOT(historyNext()));
    connect(m_nextMenu, SIGNAL(aboutToShow()), this, SLOT(nextMenuAboutToShow()));

    m_browseButton = new QToolButton();
    m_browseButton->setIcon(QIcon::fromTheme("games-hint"));
    m_browseButton->setToolTip(i18n("Enable/disable source browse mode"));
    m_browseButton->setWhatsThis(i18n("When this is enabled, you can browse the source-code by clicking in the editor."));
    m_browseButton->setCheckable(true);
    m_browseButton->setFocusPolicy(Qt::NoFocus);
    
    connect(m_browseButton, SIGNAL(clicked(bool)), m_browseManager, SLOT(setBrowsing(bool)));

    IQuickOpen* quickOpen = KDevelop::ICore::self()->pluginController()->extensionForPlugin<IQuickOpen>("org.kdevelop.IQuickOpen");
    
    if(quickOpen) {
      m_outlineLine = quickOpen->createQuickOpenLine(QStringList(), QStringList() << i18n("Outline"), IQuickOpen::Outline);
      m_outlineLine->setDefaultText(i18n("Outline..."));
      m_outlineLine->setToolTip(i18n("Navigate outline of active document, click to browse."));
    }

    connect(m_browseManager, SIGNAL(startDelayedBrowsing(KTextEditor::View*)),
            this, SLOT(startDelayedBrowsing(KTextEditor::View*)));
    connect(m_browseManager, SIGNAL(stopDelayedBrowsing()),
            this, SLOT(stopDelayedBrowsing()));

    m_toolbarWidget = toolbarWidgetForMainWindow(window);
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

    connect(ICore::self()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*)), m_outlineLine, SLOT(clear()));
    connect(ICore::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), 
      this, SLOT(documentActivated(KDevelop::IDocument*)));

    return ret;
}

void ContextBrowserPlugin::createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile,
                                                      KActionCollection& actions)
{
    xmlFile = "kdevcontextbrowser.rc" ;

    QAction* previousContext = actions.addAction("previous_context");
    previousContext->setText( i18n("&Previous Visited Context") );
    previousContext->setIcon( QIcon::fromTheme("go-previous-context" ) );
    previousContext->setShortcut( Qt::META | Qt::Key_Left );
    QObject::connect(previousContext, SIGNAL(triggered(bool)), this, SLOT(previousContextShortcut()));

    QAction* nextContext = actions.addAction("next_context");
    nextContext->setText( i18n("&Next Visited Context") );
    nextContext->setIcon( QIcon::fromTheme("go-next-context" ) );
    nextContext->setShortcut( Qt::META | Qt::Key_Right );
    QObject::connect(nextContext, SIGNAL(triggered(bool)), this, SLOT(nextContextShortcut()));

    QAction* previousUse = actions.addAction("previous_use");
    previousUse->setText( i18n("&Previous Use") );
    previousUse->setIcon( QIcon::fromTheme("go-previous-use") );
    previousUse->setShortcut( Qt::META | Qt::SHIFT |  Qt::Key_Left );
    QObject::connect(previousUse, SIGNAL(triggered(bool)), this, SLOT(previousUseShortcut()));

    QAction* nextUse = actions.addAction("next_use");
    nextUse->setText( i18n("&Next Use") );
    nextUse->setIcon( QIcon::fromTheme("go-next-use") );
    nextUse->setShortcut( Qt::META | Qt::SHIFT | Qt::Key_Right );
    QObject::connect(nextUse, SIGNAL(triggered(bool)), this, SLOT(nextUseShortcut()));

    QWidgetAction* outline = new QWidgetAction(this);
    outline->setText(i18n("Context Browser"));
    QWidget* w = toolbarWidgetForMainWindow(window);
    w->setHidden(false);
    outline->setDefaultWidget(w);
    actions.addAction("outline_line", outline);
    // Add to the actioncollection so one can set global shortcuts for the action
    actions.addAction("find_uses", m_findUses);
}

void ContextBrowserPlugin::nextContextShortcut()
{
  // TODO: cleanup
  historyNext();
}

void ContextBrowserPlugin::previousContextShortcut()
{
  // TODO: cleanup
  historyPrevious();
}

K_PLUGIN_FACTORY(ContextBrowserFactory, registerPlugin<ContextBrowserPlugin>(); )
K_EXPORT_PLUGIN(ContextBrowserFactory(KAboutData("kdevcontextbrowser","kdevcontextbrowser",ki18n("Context Browser"), "0.1", ki18n("This plugin shows information about the current language context in a side view, and highlights relevant declarations and uses."), KAboutData::License_GPL)))

ContextBrowserPlugin::ContextBrowserPlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(ContextBrowserFactory::componentData(), parent)
    , m_viewFactory(new ContextBrowserViewFactory(this))
    , m_nextHistoryIndex(0)
{
  KDEV_USE_EXTENSION_INTERFACE( IContextBrowser )

  core()->uiController()->addToolView(i18n("Code Browser"), m_viewFactory);

  connect( core()->documentController(), SIGNAL(textDocumentCreated(KDevelop::IDocument*)), this, SLOT(textDocumentCreated(KDevelop::IDocument*)) );
  connect( core()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), this, SLOT(parseJobFinished(KDevelop::ParseJob*)));

  connect( DUChain::self(), SIGNAL(declarationSelected(KDevelop::DeclarationPointer)),
           this, SLOT(declarationSelectedInUI(KDevelop::DeclarationPointer)) );

  m_updateTimer = new QTimer(this);
  m_updateTimer->setSingleShot(true);
  connect( m_updateTimer, SIGNAL(timeout()), this, SLOT(updateViews()) );
  
  //Needed global action for the context-menu extensions
  m_findUses = new QAction(i18n("Find Uses"), this);
  connect(m_findUses, SIGNAL(triggered(bool)), this, SLOT(findUses()));
}

ContextBrowserPlugin::~ContextBrowserPlugin()
{
  ///TODO: QObject inheritance should suffice?
  delete m_nextMenu;
  delete m_previousMenu;
  delete m_toolbarWidgetLayout;

  delete m_previousButton;
  delete m_outlineLine;
  delete m_nextButton;
  delete m_browseButton;
}

void ContextBrowserPlugin::unload()
{
  core()->uiController()->removeToolView(m_viewFactory);
}

KDevelop::ContextMenuExtension ContextBrowserPlugin::contextMenuExtension(KDevelop::Context* context)
{
  KDevelop::ContextMenuExtension menuExt = KDevelop::IPlugin::contextMenuExtension( context );

  KDevelop::DeclarationContext *codeContext = dynamic_cast<KDevelop::DeclarationContext*>(context);

  if (!codeContext)
      return menuExt;

  DUChainReadLocker lock(DUChain::lock());
  
  if(!codeContext->declaration().data())
    return menuExt;
  
  qRegisterMetaType<KDevelop::IndexedDeclaration>("KDevelop::IndexedDeclaration");
  
  menuExt.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, m_findUses);

  return menuExt;
}

void ContextBrowserPlugin::showUses(const DeclarationPointer& declaration)
{
  QMetaObject::invokeMethod(this, "showUsesDelayed", Qt::QueuedConnection,
                            Q_ARG(KDevelop::DeclarationPointer, declaration));
}

void ContextBrowserPlugin::showUsesDelayed(const DeclarationPointer& declaration)
{
  DUChainReadLocker lock;

  Declaration* decl = declaration.data();
  if(!decl) {
    return;
  }
  QWidget* toolView = ICore::self()->uiController()->findToolView(i18n("Code Browser"), m_viewFactory, KDevelop::IUiController::CreateAndRaise);
  if(!toolView) {
    return;
  }
  ContextBrowserView* view = dynamic_cast<ContextBrowserView*>(toolView);
  Q_ASSERT(view);
  view->allowLockedUpdate();
  view->setDeclaration(decl, decl->topContext(), true);
  //We may get deleted while the call to acceptLink, so make sure we don't crash in that case
  QPointer<AbstractNavigationWidget> widget = dynamic_cast<AbstractNavigationWidget*>(view->navigationWidget());
  if(widget && widget->context()) {
    NavigationContextPointer nextContext = widget->context()->execute(
      NavigationAction(declaration, KDevelop::NavigationAction::ShowUses));

    if(widget) {
      widget->setContext( nextContext );
    }
  }
}

void ContextBrowserPlugin::findUses()
{
  showUses(cursorDeclaration());
}

void ContextBrowserPlugin::textHintRequested(const KTextEditor::Cursor& cursor, QString&) {
  m_mouseHoverCursor = SimpleCursor(cursor);
  View* view = dynamic_cast<View*>(sender());
  if(!view) {
    kWarning() << "could not cast to view";
  }else{
    m_mouseHoverDocument = view->document()->url();
    m_updateViews << view;
  }
  m_updateTimer->start(1); // triggers updateViews()
  
  showToolTip(view, cursor);
}

void ContextBrowserPlugin::stopDelayedBrowsing() {
  if(m_currentToolTip) {
    m_currentToolTip->deleteLater();
    m_currentToolTip = 0;
    m_currentNavigationWidget = 0;
  }
}

void ContextBrowserPlugin::startDelayedBrowsing(KTextEditor::View* view) {
  if(!m_currentToolTip) {
    showToolTip(view, view->cursorPosition());
  }
}

void ContextBrowserPlugin::hideToolTip() {
  if(m_currentToolTip) {
    m_currentToolTip->deleteLater();
    m_currentToolTip = 0;
    m_currentNavigationWidget = 0;
  }
}

// TODO: this is a hack, but Kate does not provide interface for this
static int getLineHeight(KTextEditor::View* view, int curLine)
{
  KTextEditor::Cursor c(curLine, 0);
  int currentHeight = view->cursorToCoordinate(c).y();
  c.setLine(curLine + 1);
  if (view->cursorToCoordinate(c).y() < 0) {
    c.setLine(curLine - 1);
  }
  return std::abs(view->cursorToCoordinate(c).y() - currentHeight);
}

static QRect getItemBoundingRect(const KUrl& viewUrl, KTextEditor::View* view, KTextEditor::Cursor itemPosition)
{
  DUChainReadLocker lock;
  KTextEditor::Range itemRange = DUChainUtils::itemRangeUnderCursor(viewUrl, SimpleCursor(itemPosition));
  QPoint startPoint = view->mapToGlobal(view->cursorToCoordinate(itemRange.start()));
  QPoint endPoint = view->mapToGlobal(view->cursorToCoordinate(itemRange.end()));
  endPoint.ry() += getLineHeight(view, itemPosition.line());
  return QRect(startPoint, endPoint);
}

void ContextBrowserPlugin::showToolTip(KTextEditor::View* view, KTextEditor::Cursor position) {
  
  ContextBrowserView* contextView = browserViewForWidget(view);
  if(contextView && contextView->isVisible() && !contextView->isLocked())
    return; // If the context-browser view is visible, it will care about updating by itself
  
  KUrl viewUrl(view->document()->url());
  QList<ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(viewUrl);
  
  QWidget* navigationWidget = 0;
  {
    DUChainReadLocker lock(DUChain::lock());
    foreach( ILanguage* language, languages) {
      navigationWidget = language->languageSupport()->specialLanguageObjectNavigationWidget(viewUrl, SimpleCursor(position));
      if(navigationWidget)
        break;
    }
    
    if(!navigationWidget) {
      Declaration* decl = DUChainUtils::declarationForDefinition( DUChainUtils::itemUnderCursor(viewUrl, SimpleCursor(position)) );
      if (decl && decl->kind() == Declaration::Alias) {
        AliasDeclaration* alias = dynamic_cast<AliasDeclaration*>(decl);
        Q_ASSERT(alias);
        DUChainReadLocker lock;
        decl = alias->aliasedDeclaration().declaration();
      }
      if(decl) {
        if(m_currentToolTipDeclaration == IndexedDeclaration(decl) && m_currentToolTip)
          return;
        m_currentToolTipDeclaration = IndexedDeclaration(decl);
        navigationWidget = decl->context()->createNavigationWidget(decl, DUChainUtils::standardContextForUrl(viewUrl));
      }
    }
  }

  if(navigationWidget) {

    // If we have an invisible context-view, assign the tooltip navigation-widget to it.
    // If the user makes the context-view visible, it will instantly contain the correct widget.
    if(contextView && !contextView->isLocked())
      contextView->setNavigationWidget(navigationWidget);
      
    if(m_currentToolTip) {
      m_currentToolTip->deleteLater();
      m_currentToolTip = 0;
      m_currentNavigationWidget = 0;
    }
    
    KDevelop::NavigationToolTip* tooltip = new KDevelop::NavigationToolTip(view, view->mapToGlobal(view->cursorToCoordinate(position)) + QPoint(20, 40), navigationWidget);
    tooltip->addExtendRect(getItemBoundingRect(viewUrl, view, position));
    tooltip->resize( navigationWidget->sizeHint() + QSize(10, 10) );
    kDebug() << "tooltip size" << tooltip->size();
    m_currentToolTip = tooltip;
    m_currentNavigationWidget = navigationWidget;
    ActiveToolTip::showToolTip(tooltip);

    if ( ! navigationWidget->property("DoNotCloseOnCursorMove").toBool() ) {
      connect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)),
              this, SLOT(hideToolTip()), Qt::UniqueConnection);
    }
    connect(view, SIGNAL(focusOut(KTextEditor::View*)), this, SLOT(hideToolTip()), Qt::UniqueConnection);
    
  }else{
    kDebug() << "not showing tooltip, no navigation-widget";
  }
}

void ContextBrowserPlugin::clearMouseHover() {
  m_mouseHoverCursor = SimpleCursor::invalid();
  m_mouseHoverDocument.clear();
}

Attribute::Ptr highlightedUseAttribute() {
  static Attribute::Ptr standardAttribute = Attribute::Ptr();
  if( !standardAttribute ) {
    standardAttribute= Attribute::Ptr( new Attribute() );
    standardAttribute->setBackgroundFillWhitespace(true);

    // mixing (255, 255, 0, 100) with white yields this:
    standardAttribute->setBackground(QColor(251, 250, 150));

    // force a foreground color to overwrite default Kate highlighting, i.e. of Q_OBJECT or similar
    // foreground color could change, hence apply it everytime
    standardAttribute->setForeground(QColor(0, 0, 0, 255)); //Don't use alpha here, as kate uses the alpha only to blend with the document background color
  }
  return standardAttribute;
}

Attribute::Ptr highlightedSpecialObjectAttribute() {
  static Attribute::Ptr standardAttribute = Attribute::Ptr();
  if( !standardAttribute ) {
    standardAttribute = Attribute::Ptr( new Attribute() );
    standardAttribute->setBackgroundFillWhitespace(true);
    // mixing (90, 255, 0, 100) with white yields this:
    standardAttribute->setBackground(QColor(190, 255, 155));
    // force a foreground color to overwrite default Kate highlighting, i.e. of Q_OBJECT or similar
    // foreground color could change, hence apply it everytime
    standardAttribute->setForeground(QColor(0, 0, 0, 255)); //Don't use alpha here, as kate uses the alpha only to blend with the document background color
  }
  return standardAttribute;
}

void ContextBrowserPlugin::addHighlight( View* view, KDevelop::Declaration* decl ) {
  if( !view || !decl ) {
    kDebug() << "invalid view/declaration";
    return;
  }
  
  ViewHighlights& highlights(m_highlightedRanges[view]);

  KDevelop::DUChainReadLocker lock;

  // Highlight the declaration
  highlights.highlights << decl->createRangeMoving();
  highlights.highlights.back()->setAttribute(highlightedUseAttribute());
  highlights.highlights.back()->setZDepth(highlightingZDepth);
  
  // Highlight uses
  {
    QMap< IndexedString, QList< SimpleRange > > currentRevisionUses = decl->usesCurrentRevision();
    for(QMap< IndexedString, QList< SimpleRange > >::iterator fileIt = currentRevisionUses.begin(); fileIt != currentRevisionUses.end(); ++fileIt)
    {
      for(QList< SimpleRange >::const_iterator useIt = (*fileIt).constBegin(); useIt != (*fileIt).constEnd(); ++useIt)
      {
        highlights.highlights << PersistentMovingRange::Ptr(new PersistentMovingRange(*useIt, fileIt.key()));
        highlights.highlights.back()->setAttribute(highlightedUseAttribute());
        highlights.highlights.back()->setZDepth(highlightingZDepth);
      }
    }
  }
  
  if( FunctionDefinition* def = FunctionDefinition::definition(decl) )
  {
    highlights.highlights << def->createRangeMoving();
    highlights.highlights.back()->setAttribute(highlightedUseAttribute());
    highlights.highlights.back()->setZDepth(highlightingZDepth);
  }
}

Declaration* ContextBrowserPlugin::findDeclaration(View* view, const SimpleCursor& position, bool mouseHighlight)
{
      Q_UNUSED(mouseHighlight);
      Declaration* foundDeclaration = 0;
      if(m_useDeclaration.data()) {
        foundDeclaration = m_useDeclaration.data();
      }else{
        //If we haven't found a special language object, search for a use/declaration and eventually highlight it
        foundDeclaration = DUChainUtils::declarationForDefinition( DUChainUtils::itemUnderCursor(view->document()->url(), position) );
        if (foundDeclaration && foundDeclaration->kind() == Declaration::Alias) {
          AliasDeclaration* alias = dynamic_cast<AliasDeclaration*>(foundDeclaration);
          Q_ASSERT(alias);
          DUChainReadLocker lock;
          foundDeclaration = alias->aliasedDeclaration().declaration();
        }
      }
      return foundDeclaration;
}

ContextBrowserView* ContextBrowserPlugin::browserViewForWidget(QWidget* widget)
{
  foreach(ContextBrowserView* contextView, m_views) {
    if(masterWidget(contextView) == masterWidget(widget)) {
      return contextView;
    }
  }
      
  return 0;
}

void ContextBrowserPlugin::updateForView(View* view)
{
    bool allowHighlight = true;
    if(view->selection())
    {
      // If something is selected, we unhighlight everything, so that we don't conflict with the
      // kate plugin that highlights occurrences of the selected string, and also to reduce the
      // overall amount of concurrent highlighting.
      allowHighlight = false;
    }
    
    if(m_highlightedRanges[view].keep)
    {
      m_highlightedRanges[view].keep = false;
      return;
    }
    
    // Clear all highlighting
    m_highlightedRanges.clear();
    
    // Re-highlight
    ViewHighlights& highlights = m_highlightedRanges[view];
    
    KUrl url = view->document()->url();
    IDocument* activeDoc = core()->documentController()->activeDocument();
    
    bool mouseHighlight = (url == m_mouseHoverDocument) && (m_mouseHoverCursor.isValid());
    bool shouldUpdateBrowser = (mouseHighlight || (view->isActiveView() && activeDoc && activeDoc->textDocument() == view->document()));
    
    SimpleCursor highlightPosition;
    if (mouseHighlight)
     highlightPosition = m_mouseHoverCursor;
    else
     highlightPosition = SimpleCursor(view->cursorPosition());

    ///Pick a language
    ILanguage* language = 0;
    
    if(ICore::self()->languageController()->languagesForUrl(url).isEmpty()) {
      kDebug() << "found no language for document" << url;
      return;
    }else{
      language = ICore::self()->languageController()->languagesForUrl(url).front();
    }
    
    ///Check whether there is a special language object to highlight (for example a macro)
    
    SimpleRange specialRange = language->languageSupport()->specialLanguageObjectRange(url, highlightPosition);
    ContextBrowserView* updateBrowserView = shouldUpdateBrowser ?  browserViewForWidget(view) : 0;
    
    if(specialRange.isValid())
    {
      // Highlight a special language object
      if(allowHighlight)
      {
        highlights.highlights << PersistentMovingRange::Ptr(new PersistentMovingRange(specialRange, IndexedString(url)));
        highlights.highlights.back()->setAttribute(highlightedSpecialObjectAttribute());
        highlights.highlights.back()->setZDepth(highlightingZDepth);
      }
      if(updateBrowserView)
        updateBrowserView->setSpecialNavigationWidget(language->languageSupport()->specialLanguageObjectNavigationWidget(url, highlightPosition));
    }else{
      KDevelop::DUChainReadLocker lock( DUChain::lock(), 100 );
      if(!lock.locked()) {
        kDebug() << "Failed to lock du-chain in time";
        return;
      }
      
      TopDUContext* topContext = DUChainUtils::standardContextForUrl(view->document()->url());
      if (!topContext)
        return;
      DUContext* ctx = contextForHighlightingAt(highlightPosition, topContext);
      if (!ctx) 
        return;
      
      //Only update the history if this context is around the text cursor
      if(core()->documentController()->activeDocument() && highlightPosition == SimpleCursor(view->cursorPosition()) && view->document() == core()->documentController()->activeDocument()->textDocument())
      {
        updateHistory(ctx, highlightPosition);
      }
      
      Declaration* foundDeclaration = findDeclaration(view, highlightPosition, mouseHighlight);
      
      if( foundDeclaration ) {
        m_lastHighlightedDeclaration = highlights.declaration = IndexedDeclaration(foundDeclaration);
        if(allowHighlight)
          addHighlight( view, foundDeclaration );
        
        if(updateBrowserView)
          updateBrowserView->setDeclaration(foundDeclaration, topContext);
      }else{
        if(updateBrowserView)
          updateBrowserView->setContext(ctx);
      }
    }
}

void ContextBrowserPlugin::updateViews()
{
  foreach( View* view, m_updateViews ) {
    updateForView(view);
  }
  m_updateViews.clear();
  m_useDeclaration = IndexedDeclaration();
}

void ContextBrowserPlugin::declarationSelectedInUI(const DeclarationPointer& decl)
{
  m_useDeclaration = IndexedDeclaration(decl.data());
  if(core()->documentController()->activeDocument() && core()->documentController()->activeDocument()->textDocument() && core()->documentController()->activeDocument()->textDocument()->activeView())
    m_updateViews << core()->documentController()->activeDocument()->textDocument()->activeView();

  m_updateTimer->start(highlightingTimeout); // triggers updateViews()
}

void ContextBrowserPlugin::parseJobFinished(KDevelop::ParseJob* job)
{
  for(QMap< View*, ViewHighlights >::iterator it = m_highlightedRanges.begin(); it != m_highlightedRanges.end(); ++it) {
    if(it.key()->document()->url() == job->document().toUrl()) {
      if(m_updateViews.isEmpty())
        m_updateTimer->start(highlightingTimeout);
      
      if(!m_updateViews.contains(it.key())) {
        kDebug() << "adding view for update";
        m_updateViews << it.key();
        
        // Don't change the highlighted declaration after finished parse-jobs
        (*it).keep = true;
      }
    }
  }
}

void ContextBrowserPlugin::textDocumentCreated( KDevelop::IDocument* document )
{
  Q_ASSERT(document->textDocument());

  connect( document->textDocument(), SIGNAL(viewCreated(KTextEditor::Document*,KTextEditor::View*)), this, SLOT(viewCreated(KTextEditor::Document*,KTextEditor::View*)) );

  foreach( View* view, document->textDocument()->views() )
    viewCreated( document->textDocument(), view );
}

void ContextBrowserPlugin::documentActivated( IDocument* doc )
{
  m_outlineLine->clear();
  if (doc->textDocument() && doc->textDocument()->activeView())
  {
    cursorPositionChanged(doc->textDocument()->activeView(), doc->textDocument()->activeView()->cursorPosition());
  }
}

void ContextBrowserPlugin::viewDestroyed( QObject* obj )
{
  m_highlightedRanges.remove(static_cast<KTextEditor::View*>(obj));
  m_updateViews.remove(static_cast<View*>(obj));
}

void ContextBrowserPlugin::selectionChanged( View* view )
{
  clearMouseHover();
  m_updateViews.insert(view);
  m_updateTimer->start(highlightingTimeout/2); // triggers updateViews()
}

void ContextBrowserPlugin::cursorPositionChanged( View* view, const KTextEditor::Cursor& newPosition )
{
  if(view->document() == m_lastInsertionDocument && newPosition == m_lastInsertionPos)
  {
    //Do not update the highlighting while typing
    m_lastInsertionDocument = 0;
    m_lastInsertionPos = KTextEditor::Cursor();
    if(m_highlightedRanges.contains(view))
      m_highlightedRanges[view].keep = true;
  }else{
    if(m_highlightedRanges.contains(view))
      m_highlightedRanges[view].keep = false;
  }
  clearMouseHover();
  m_updateViews.insert(view);
  m_updateTimer->start(highlightingTimeout/2); // triggers updateViews()
}

void ContextBrowserPlugin::textInserted(KTextEditor::Document* doc, KTextEditor::Range range)
{
  m_lastInsertionDocument = doc;
  m_lastInsertionPos = range.end();
}

void ContextBrowserPlugin::viewCreated( KTextEditor::Document* , View* v )
{
  disconnect( v, SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), this, SLOT(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)) ); ///Just to make sure that multiple connections don't happen
  connect( v, SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), this, SLOT(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)) );
  connect( v, SIGNAL(destroyed(QObject*)), this, SLOT(viewDestroyed(QObject*)) );
  disconnect( v->document(), SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), this, SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
  connect( v->document(), SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), this, SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
  disconnect( v, SIGNAL(selectionChanged(KTextEditor::View*)), this, SLOT(selectionChanged(KTextEditor::View*)));

  KTextEditor::TextHintInterface *iface = dynamic_cast<KTextEditor::TextHintInterface*>(v);
  if( !iface )
      return;

  iface->enableTextHints(highlightingTimeout);

  connect(v, SIGNAL(needTextHint(KTextEditor::Cursor,QString&)), this, SLOT(textHintRequested(KTextEditor::Cursor,QString&)));
}

void ContextBrowserPlugin::registerToolView(ContextBrowserView* view)
{
  m_views << view;
}

void ContextBrowserPlugin::previousUseShortcut()
{
  switchUse(false);
}

void ContextBrowserPlugin::nextUseShortcut()
{
  switchUse(true);
}

KTextEditor::Range cursorToRange(SimpleCursor cursor) {
  return KTextEditor::Range(cursor.textCursor(), cursor.textCursor());
}

void ContextBrowserPlugin::switchUse(bool forward)
{
  if(core()->documentController()->activeDocument() && core()->documentController()->activeDocument()->textDocument() && core()->documentController()->activeDocument()->textDocument()->activeView()) {
    KTextEditor::Document* doc = core()->documentController()->activeDocument()->textDocument();


    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    KDevelop::TopDUContext* chosen = DUChainUtils::standardContextForUrl(doc->url());

    if( chosen )
    {
      SimpleCursor cCurrent(doc->activeView()->cursorPosition());
      KDevelop::CursorInRevision c = chosen->transformToLocalRevision(cCurrent);
      
      Declaration* decl = 0;
      //If we have a locked declaration, use that for jumping
      foreach(ContextBrowserView* view, m_views) {
        decl = view->lockedDeclaration().data(); ///@todo Somehow match the correct context-browser view if there is multiple
        if(decl)
          break;
      }

      if(!decl) //Try finding a declaration under the cursor
        decl = DUChainUtils::itemUnderCursor(doc->url(), cCurrent);

      if (decl && decl->kind() == Declaration::Alias) {
        AliasDeclaration* alias = dynamic_cast<AliasDeclaration*>(decl);
        Q_ASSERT(alias);
        DUChainReadLocker lock;
        decl = alias->aliasedDeclaration().declaration();
      }

      if(decl) {

        Declaration* target = 0;
        if(forward)
          //Try jumping from definition to declaration
          target = DUChainUtils::declarationForDefinition(decl, chosen);
        else if(decl->url().toUrl() == doc->url() && decl->range().contains(c))
          //Try jumping from declaration to definition
          target = FunctionDefinition::definition(decl);
        
          if(target && target != decl) {
            SimpleCursor jumpTo = target->rangeInCurrentRevision().start;
            KUrl document = target->url().toUrl();
            lock.unlock();
            core()->documentController()->openDocument( document, cursorToRange(jumpTo)  );
            return;
          }else{
            //Always work with the declaration instead of the definition
            decl = DUChainUtils::declarationForDefinition(decl, chosen);
          }
      }
      
      if(!decl) {
        //Pick the last use we have highlighted
        decl = m_lastHighlightedDeclaration.data();
      }
      
      if(decl) {
        KDevVarLengthArray<IndexedTopDUContext> usingFiles = DUChain::uses()->uses(decl->id());
        
        if(DUChainUtils::contextHasUse(decl->topContext(), decl) && usingFiles.indexOf(decl->topContext()) == -1)
          usingFiles.insert(0, decl->topContext());
        
        if(decl->range().contains(c) && decl->url() == chosen->url()) {
            //The cursor is directly on the declaration. Jump to the first or last use.
            if(!usingFiles.isEmpty()) {
            TopDUContext* top = (forward ? usingFiles[0] : usingFiles.back()).data();
            if(top) {
              QList<RangeInRevision> useRanges = allUses(top, decl, true);
              qSort(useRanges);
              if(!useRanges.isEmpty()) {
                KUrl url = top->url().toUrl();
                SimpleRange selectUse = chosen->transformFromLocalRevision(forward ? useRanges.first() : useRanges.back());
                lock.unlock();
                core()->documentController()->openDocument(url, cursorToRange(selectUse.start));
              }
            }
          }
          return;
        }
        //Check whether we are within a use
        QList<RangeInRevision> localUses = allUses(chosen, decl, true);
        qSort(localUses);
        for(int a = 0; a < localUses.size(); ++a) {
          int nextUse = (forward ? a+1 : a-1);
          bool pick = localUses[a].contains(c);
          
          if(!pick && forward && a+1 < localUses.size() && localUses[a].end <= c && localUses[a+1].start > c) {
            //Special case: We aren't on a use, but we are jumping forward, and are behind this and the next use
            pick = true;
          }
          if(!pick && !forward && a-1 >= 0 && c < localUses[a].start && c >= localUses[a-1].end) {
            //Special case: We aren't on a use, but we are jumping backward, and are in front of this use, but behind the previous one
            pick = true;
          }
          if(!pick && a == 0 && c < localUses[a].start) {
            if(!forward) {
              //Will automatically jump to previous file
            }else{
              nextUse = 0; //We are before the first use, so jump to it.
            }
            pick = true;
          }
          if(!pick && a == localUses.size()-1 && c >= localUses[a].end) {
            if(forward) {
              //Will automatically jump to next file
            }else{ //We are behind the last use, but moving backward. So pick the last use.
              nextUse = a;
            }
            pick = true;
          }
          
          if(pick) {
          
          //Make sure we end up behind the use
          if(nextUse != a)
            while(forward && nextUse < localUses.size() && (localUses[nextUse].start <= localUses[a].end || localUses[nextUse].isEmpty()))
              ++nextUse;
          
          //Make sure we end up before the use
          if(nextUse != a)
            while(!forward && nextUse >= 0 && (localUses[nextUse].start >= localUses[a].start || localUses[nextUse].isEmpty()))
              --nextUse;
          //Jump to the next use
            
          kDebug() << "count of uses:" << localUses.size() << "nextUse" << nextUse;
          
          if(nextUse < 0 || nextUse == localUses.size()) {
            kDebug() << "jumping to next file";
            //Jump to the first use in the next using top-context
            int indexInFiles = usingFiles.indexOf(chosen);
            if(indexInFiles != -1) {
              
              int nextFile = (forward ? indexInFiles+1 : indexInFiles-1);
              kDebug() << "current file" << indexInFiles << "nextFile" << nextFile;
              
              if(nextFile < 0 || nextFile >= usingFiles.size()) {
                //Open the declaration, or the definition
                if(nextFile >= usingFiles.size()) {
                  Declaration* definition = FunctionDefinition::definition(decl);
                  if(definition)
                    decl = definition;
                }
                KUrl u(decl->url().str());
                SimpleRange range = decl->rangeInCurrentRevision();
                range.end = range.start;
                lock.unlock();
                core()->documentController()->openDocument(u, range.textRange());
                return;
              }else{
                TopDUContext* nextTop = usingFiles[nextFile].data();
                
                KUrl u(nextTop->url().str());
                
                QList<RangeInRevision> nextTopUses = allUses(nextTop, decl, true);
                qSort(nextTopUses);
                
                if(!nextTopUses.isEmpty()) {
                  SimpleRange range =  chosen->transformFromLocalRevision(forward ? nextTopUses.front() : nextTopUses.back());
                  range.end = range.start;
                  lock.unlock();
                  core()->documentController()->openDocument(u, range.textRange());
                }
                return;
              }
            }else{
              kDebug() << "not found own file in use list";
            }
          }else{
              KUrl url(chosen->url().str());
              SimpleRange range = chosen->transformFromLocalRevision(localUses[nextUse]);
              range.end = range.start;
              lock.unlock();
              core()->documentController()->openDocument(url, range.textRange());
              return;
          }
        }
        }
      }
    }
  }
}

void ContextBrowserPlugin::unRegisterToolView(ContextBrowserView* view)
{
  m_views.removeAll(view);
}

// history browsing

QWidget* ContextBrowserPlugin::toolbarWidgetForMainWindow( Sublime::MainWindow* window )
{
  //TODO: support multiple windows (if that ever gets revived)
  if (!m_toolbarWidget) {
    m_toolbarWidget = new QWidget(window);
  }
  return m_toolbarWidget;
}

void ContextBrowserPlugin::documentJumpPerformed( KDevelop::IDocument* newDocument,
                                            const KTextEditor::Cursor& newCursor,
                                            KDevelop::IDocument* previousDocument,
                                            const KTextEditor::Cursor& previousCursor) {
        
    DUChainReadLocker lock(DUChain::lock());

    /*TODO: support multiple windows if that ever gets revived
    if(newDocument && newDocument->textDocument() && newDocument->textDocument()->activeView() && masterWidget(newDocument->textDocument()->activeView()) != masterWidget(this))
        return;
    */

    if(previousDocument && previousCursor.isValid()) {
        kDebug() << "updating jump source";
        DUContext* context = getContextAt(previousDocument->url(), previousCursor);
        if(context) {
            updateHistory(context, SimpleCursor(previousCursor), true);
        }else{
            //We just want this place in the history
            m_history.resize(m_nextHistoryIndex); // discard forward history
            m_history.append(HistoryEntry(DocumentCursor(IndexedString(previousDocument->url()), SimpleCursor(previousCursor))));
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
            m_history.append(HistoryEntry(DocumentCursor(IndexedString(newDocument->url()), SimpleCursor(newCursor))));
            ++m_nextHistoryIndex;
            m_outlineLine->clear();
        }
    }
}

void ContextBrowserPlugin::updateButtonState()
{
    m_nextButton->setEnabled( m_nextHistoryIndex < m_history.size() );
    m_previousButton->setEnabled( m_nextHistoryIndex >= 2 );
}

void ContextBrowserPlugin::historyNext() {
    if(m_nextHistoryIndex >= m_history.size()) {
        return;
    }
    openDocument(m_nextHistoryIndex); // opening the document at given position 
                                      // will update the widget for us
    ++m_nextHistoryIndex;
    updateButtonState();
}

void ContextBrowserPlugin::openDocument(int historyIndex) {
    Q_ASSERT_X(historyIndex >= 0, "openDocument", "negative history index");
    Q_ASSERT_X(historyIndex < m_history.size(), "openDocument", "history index out of range");
    DocumentCursor c = m_history[historyIndex].computePosition();
    if (c.isValid() && !c.document.str().isEmpty()) {
        
        disconnect(ICore::self()->documentController(), SIGNAL(documentJumpPerformed(KDevelop::IDocument*,KTextEditor::Cursor,KDevelop::IDocument*,KTextEditor::Cursor)), this,      SLOT(documentJumpPerformed(KDevelop::IDocument*,KTextEditor::Cursor,KDevelop::IDocument*,KTextEditor::Cursor)));
        
        ICore::self()->documentController()->openDocument(c.document.toUrl(), c.textCursor());
        
        connect(ICore::self()->documentController(), SIGNAL(documentJumpPerformed(KDevelop::IDocument*,KTextEditor::Cursor,KDevelop::IDocument*,KTextEditor::Cursor)), this, SLOT(documentJumpPerformed(KDevelop::IDocument*,KTextEditor::Cursor,KDevelop::IDocument*,KTextEditor::Cursor)));

        KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
        updateDeclarationListBox(m_history[historyIndex].context.data());
    }
}

void ContextBrowserPlugin::historyPrevious() {
    if(m_nextHistoryIndex < 2) {
        return;
    }
    --m_nextHistoryIndex;
    openDocument(m_nextHistoryIndex-1); // opening the document at given position 
                                        // will update the widget for us
    updateButtonState();
}

QString ContextBrowserPlugin::actionTextFor(int historyIndex) const
{
    const HistoryEntry& entry = m_history.at(historyIndex);
    QString actionText = entry.context.data() ? entry.context.data()->scopeIdentifier(true).toString() : QString();
    if(actionText.isEmpty())
        actionText = entry.alternativeString;
    if(actionText.isEmpty())
        actionText = "<unnamed>";
    actionText += " @ ";
    QString fileName = entry.absoluteCursorPosition.document.toUrl().fileName();
    actionText += QString("%1:%2").arg(fileName).arg(entry.absoluteCursorPosition.line+1);
    return actionText;
}

/*
inline QDebug operator<<(QDebug debug, const ContextBrowserPlugin::HistoryEntry &he)
{
    DocumentCursor c = he.computePosition();
    debug << "\n\tHistoryEntry " << c.line << " " << c.document.str();
    return debug;
}
*/

void ContextBrowserPlugin::nextMenuAboutToShow() {
    QList<int> indices;
    for(int a = m_nextHistoryIndex; a < m_history.size(); ++a) {
        indices << a;
    }
    fillHistoryPopup(m_nextMenu, indices);
}

void ContextBrowserPlugin::previousMenuAboutToShow() {
    QList<int> indices;
    for(int a = m_nextHistoryIndex-2; a >= 0; --a) {
        indices << a;
    }
    fillHistoryPopup(m_previousMenu, indices);
}

void ContextBrowserPlugin::fillHistoryPopup(QMenu* menu, const QList<int>& historyIndices) {
    menu->clear();
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    foreach(int index, historyIndices) {
        QAction* action = new QAction(actionTextFor(index), menu);
        action->setData(index);
        menu->addAction(action);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(actionTriggered()));
    }
}

bool ContextBrowserPlugin::isPreviousEntry(KDevelop::DUContext* context,
                                           const KDevelop::SimpleCursor& /*position*/) const
{
    if (m_nextHistoryIndex == 0) return false;
    Q_ASSERT(m_nextHistoryIndex <= m_history.count());
    const HistoryEntry& he = m_history.at(m_nextHistoryIndex-1);
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() ); // is this necessary??
    Q_ASSERT(context);
    return IndexedDUContext(context) == he.context;
}

void ContextBrowserPlugin::updateHistory(KDevelop::DUContext* context, const KDevelop::SimpleCursor& position, bool force)
{
    kDebug() << "updating history";
    
    if(m_outlineLine->isVisible())
        updateDeclarationListBox(context);
    
    if(!context || (!context->owner() && !force)) {
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

void ContextBrowserPlugin::setAllowBrowsing(bool allow) {
    m_browseButton->setChecked(allow);
}

void ContextBrowserPlugin::updateDeclarationListBox(DUContext* context) {
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

void ContextBrowserPlugin::actionTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action); Q_ASSERT(action->data().type() == QVariant::Int);
    int historyPosition = action->data().toInt();
    // kDebug() << "history pos" << historyPosition << m_history.size() << m_history;
    if(historyPosition >= 0 && historyPosition < m_history.size()) {
        m_nextHistoryIndex = historyPosition + 1;
        openDocument(historyPosition);
        updateButtonState();
    }
}

void ContextBrowserPlugin::doNavigate(NavigationActionType action)
{
  KTextEditor::View* view = qobject_cast<KTextEditor::View*>(sender());
  if(!view) {
      kWarning() << "sender is not a view";
      return;
  }
  KTextEditor::CodeCompletionInterface* iface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
  if(!iface || iface->isCompletionActive())
      return; // If code completion is active, the actions should be handled by the completion widget

  QWidget* widget = m_currentNavigationWidget.data();
  
  if(!widget || !widget->isVisible())
  {
    ContextBrowserView* contextView = browserViewForWidget(view);
    if(contextView)
      widget = contextView->navigationWidget();
  }
  
  if(widget)
  {
    AbstractNavigationWidget* navWidget = qobject_cast<AbstractNavigationWidget*>(widget);
    if (navWidget)
    {
      switch(action) {
        case Accept:
          navWidget->accept();
          break;
        case Back:
          navWidget->back();
          break;
        case Left:
          navWidget->previous();
          break;
        case Right:
          navWidget->next();
          break;
        case Up:
          navWidget->up();
          break;
        case Down:
          navWidget->down();
          break;
      }
    }
  }
}

void ContextBrowserPlugin::navigateAccept() {
  doNavigate(Accept);
}

void ContextBrowserPlugin::navigateBack() {
  doNavigate(Back);
}

void ContextBrowserPlugin::navigateDown() {
  doNavigate(Down);
}

void ContextBrowserPlugin::navigateLeft() {
  doNavigate(Left);
}

void ContextBrowserPlugin::navigateRight() {
  doNavigate(Right);
}

void ContextBrowserPlugin::navigateUp() {
  doNavigate(Up);
}


//BEGIN HistoryEntry
ContextBrowserPlugin::HistoryEntry::HistoryEntry(KDevelop::DocumentCursor pos) : absoluteCursorPosition(pos) {
}

ContextBrowserPlugin::HistoryEntry::HistoryEntry(IndexedDUContext ctx, const KDevelop::SimpleCursor& cursorPosition) : context(ctx) {
        //Use a position relative to the context
        setCursorPosition(cursorPosition);
        if(ctx.data())
            alternativeString = ctx.data()->scopeIdentifier(true).toString();
        if(!alternativeString.isEmpty())
            alternativeString += i18n("(changed)"); //This is used when the context was deleted in between
}

DocumentCursor ContextBrowserPlugin::HistoryEntry::computePosition() const {
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    DocumentCursor ret;
    if(context.data()) {
        ret = DocumentCursor(context.data()->url(), relativeCursorPosition);
        ret.line += context.data()->range().start.line;
    }else{
        ret = absoluteCursorPosition;
    }
    return ret;
}

void ContextBrowserPlugin::HistoryEntry::setCursorPosition(const KDevelop::SimpleCursor& cursorPosition) {
    KDevelop::DUChainReadLocker lock( KDevelop::DUChain::lock() );
    if(context.data()) {
        absoluteCursorPosition =  DocumentCursor(context.data()->url(), cursorPosition);
        relativeCursorPosition = cursorPosition;
        relativeCursorPosition.line -= context.data()->range().start.line;
    }
}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
