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

#include <QTimer>
#include <QApplication>
#include <qalgorithms.h>
#include <klocale.h>
#include <kaction.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>
#include <KTextEditor/TextHintInterface>
#include <kactioncollection.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/contextmenuextension.h>
#include <language/interfaces/codecontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/use.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/dumpchain.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/parsingenvironment.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>
#include "contextbrowserview.h"
#include <language/duchain/uses.h>
#include <language/duchain/specializationstore.h>
#include <language/util/navigationtooltip.h>
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <language/interfaces/iquickopen.h>
#include <interfaces/iplugincontroller.h>
#include <sublime/mainwindow.h>

const unsigned int highlightingTimeout = 150;

using KDevelop::ILanguage;
using KTextEditor::Attribute;
using KTextEditor::SmartInterface;
using KTextEditor::View;

bool toolTipEnabled = true;

QWidget* masterWidget(QWidget* w) {
  while(w && w->parent() && qobject_cast<QWidget*>(w->parent()))
    w = qobject_cast<QWidget*>(w->parent());
  return w;
}

class ContextBrowserViewFactory: public KDevelop::IToolViewFactory
{
public:
    ContextBrowserViewFactory(ContextBrowserPlugin *plugin): m_plugin(plugin) {}

    virtual QWidget* create(QWidget *parent = 0)
    {
        ContextBrowserView* ret = new ContextBrowserView(m_plugin, parent);
        QObject::connect(ret, SIGNAL(startDelayedBrowsing(KTextEditor::View*)), m_plugin, SLOT(startDelayedBrowsing(KTextEditor::View*)));
        QObject::connect(ret, SIGNAL(stopDelayedBrowsing()), m_plugin, SLOT(stopDelayedBrowsing()));
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

void ContextBrowserPlugin::createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions) {
    xmlFile = "kdevcontextbrowser.rc" ;

    KAction* previousContext = actions.addAction("previous_context");
    previousContext->setText( i18n("&Previous Visited Context") );
    previousContext->setIcon( KIcon("go-previous-context" ) );
    previousContext->setShortcut( Qt::META | Qt::Key_Left );
    QObject::connect(previousContext, SIGNAL(triggered(bool)), this, SLOT(previousContextShortcut()));

    KAction* nextContext = actions.addAction("next_context");
    nextContext->setText( i18n("&Next Visited Context") );
    nextContext->setIcon( KIcon("go-next-context" ) );
    nextContext->setShortcut( Qt::META | Qt::Key_Right );
    QObject::connect(nextContext, SIGNAL(triggered(bool)), this, SLOT(nextContextShortcut()));

    KAction* previousUse = actions.addAction("previous_use");
    previousUse->setText( i18n("&Previous Use") );
    previousUse->setIcon( KIcon("go-previous-use") );
    previousUse->setShortcut( Qt::META | Qt::SHIFT |  Qt::Key_Left );
    QObject::connect(previousUse, SIGNAL(triggered(bool)), this, SLOT(previousUseShortcut()));

    KAction* nextUse = actions.addAction("next_use");
    nextUse->setText( i18n("&Next Use") );
    nextUse->setIcon( KIcon("go-next-use") );
    nextUse->setShortcut( Qt::META | Qt::SHIFT | Qt::Key_Right );
    QObject::connect(nextUse, SIGNAL(triggered(bool)), this, SLOT(nextUseShortcut()));

    KAction* outline = actions.addAction("outline_line");
    outline->setText(i18n("Context Browser"));
    QWidget* w = toolbarWidgetForMainWindow(window);
    w->setHidden(false);
    outline->setDefaultWidget(w);
}


void ContextBrowserPlugin::nextContextShortcut()
{
  foreach(ContextBrowserView* view, m_views)
  {
    kDebug() << masterWidget(ICore::self()->uiController()->activeMainWindow()) << masterWidget(view);
    if(masterWidget(ICore::self()->uiController()->activeMainWindow()) == masterWidget(view)) {
      view->historyNext();
      return;
    }
  }
}

void ContextBrowserPlugin::previousContextShortcut()
{
  foreach(ContextBrowserView* view, m_views)
  {
    if(masterWidget(ICore::self()->uiController()->activeMainWindow()) == masterWidget(view)) {
      view->historyPrevious();
      return;
    }
  }
}

K_PLUGIN_FACTORY(ContextBrowserFactory, registerPlugin<ContextBrowserPlugin>(); )
K_EXPORT_PLUGIN(ContextBrowserFactory(KAboutData("kdevcontextbrowser","kdevcontextbrowser",ki18n("Context Browser"), "0.1", ki18n("Shows information for the current context"), KAboutData::License_GPL)))

ContextBrowserPlugin::ContextBrowserPlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(ContextBrowserFactory::componentData(), parent)
    , m_backupsMutex(QMutex::Recursive)
    , m_viewFactory(new ContextBrowserViewFactory(this)), m_lastInsertionDocument(0)
{
  core()->uiController()->addToolView(i18n("Code Browser"), m_viewFactory);

  connect( core()->documentController(), SIGNAL( textDocumentCreated( KDevelop::IDocument* ) ), this, SLOT( textDocumentCreated( KDevelop::IDocument* ) ) );
  connect( core()->documentController(), SIGNAL( documentClosed( KDevelop::IDocument* ) ), this, SLOT( documentClosed( KDevelop::IDocument* ) ) );
  connect( core()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), this, SLOT(parseJobFinished(KDevelop::ParseJob*)));

  connect( DUChain::self(), SIGNAL( declarationSelected(DeclarationPointer) ), this, SLOT( declarationSelectedInUI(DeclarationPointer) ) );


  m_updateTimer = new QTimer(this);
  m_updateTimer->setSingleShot(true);
  connect( m_updateTimer, SIGNAL( timeout() ), this, SLOT( updateViews() ) );
  
  //Needed global action for the context-menu extensions
  m_findUses = new QAction(i18n("Find Uses"), this);
  connect(m_findUses, SIGNAL(triggered(bool)), this, SLOT(findUses()));
}

ContextBrowserPlugin::~ContextBrowserPlugin()
{
  foreach (KTextEditor::SmartRange* range, m_watchedRanges)
    range->removeWatcher(this);
}

void ContextBrowserPlugin::watchRange(KTextEditor::SmartRange* range)
{
  KTextEditor::SmartInterface* smart = dynamic_cast<KTextEditor::SmartInterface*>(range->document());
  Q_ASSERT(smart);
  QMutexLocker lock(smart->smartMutex());

  if (range->watchers().contains( this ))
    return;

  range->addWatcher(this);
  m_watchedRanges.insert(range);
}

void ContextBrowserPlugin::ignoreRange(KTextEditor::SmartRange* range)
{
  if (!range->watchers().contains( this ))
    return;

  range->removeWatcher(this);
  m_watchedRanges.remove(range);
}

void ContextBrowserPlugin::unload()
{
  core()->uiController()->removeToolView(m_viewFactory);
}

void ContextBrowserPlugin::rangeDeleted( KTextEditor::SmartRange *range ) {
  QMutexLocker lock(&m_backupsMutex);
  m_backups.remove( range );
  m_watchedRanges.remove(range);

  for(QMap<View*, KTextEditor::SmartRange*>::iterator it = m_highlightedRange.begin(); it != m_highlightedRange.end(); ++it)
    if(*it == range) {
      m_highlightedRange.erase(it);
      return;
    }
}

///@todo this doesn't work, we use TextHintInterface instead atm.
void ContextBrowserPlugin::mouseEnteredRange( KTextEditor::SmartRange* range, View* view ) {
  m_mouseHoverCursor = SimpleCursor(range->start());
  m_mouseHoverDocument = view->document()->url();
  m_updateViews << view;
  m_updateTimer->start(1);
}

void ContextBrowserPlugin::mouseExitedRange( KTextEditor::SmartRange* /*range*/, View* view ) {
  clearMouseHover();
  m_updateViews << view;
  m_updateTimer->start(1); // triggers updateViews()
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
  
  m_findUses->setData(QVariant::fromValue(codeContext->declaration()));
  menuExt.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, m_findUses);

  return menuExt;
}

void ContextBrowserPlugin::findUses()
{
  QAction* action = qobject_cast<QAction*>(sender());
  Q_ASSERT(action);
  DUChainReadLocker lock(DUChain::lock());
  
  KDevelop::IndexedDeclaration decl = action->data().value<KDevelop::IndexedDeclaration>();
  if(!decl.data())
    return;
  QWidget* widget = ICore::self()->uiController()->findToolView(i18n("Code Browser"), m_viewFactory, KDevelop::IUiController::CreateAndRaise);
  if(!widget)
    return;
  ContextBrowserView* view = dynamic_cast<ContextBrowserView*>(widget);
  Q_ASSERT(view);
  view->allowLockedUpdate();
  view->setDeclaration(decl.data(), decl.data()->topContext(), true);
  KDevelop::AbstractNavigationWidget* navigationWidget = dynamic_cast<KDevelop::AbstractNavigationWidget*>(view->navigationWidget());
  if(navigationWidget)
    navigationWidget->executeContextAction("show_uses");
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
  
  if(toolTipEnabled)
    showToolTip(view, cursor);
}

void ContextBrowserPlugin::stopDelayedBrowsing() {
  if(m_currentToolTip) {
    m_currentToolTip->deleteLater();
    m_currentToolTip = 0;
  }
}

void ContextBrowserPlugin::startDelayedBrowsing(KTextEditor::View* view) {
  if(!m_currentToolTip) {
    showToolTip(view, view->cursorPosition());
  }
}

void ContextBrowserPlugin::hideTooTip() {
  if(m_currentToolTip) {
    m_currentToolTip->deleteLater();
    m_currentToolTip = 0;
  }
}


void ContextBrowserPlugin::showToolTip(KTextEditor::View* view, KTextEditor::Cursor position) {
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
      
      if(decl) {
        if(m_currentToolTipDeclaration == IndexedDeclaration(decl) && m_currentToolTip)
          return;
        m_currentToolTipDeclaration = IndexedDeclaration(decl);
        navigationWidget = decl->context()->createNavigationWidget(decl, DUChainUtils::standardContextForUrl(viewUrl));
      }
    }
  }

  if(navigationWidget) {

    foreach(ContextBrowserView* contextView, m_views)
      if(masterWidget(contextView) == masterWidget(view)) {
        if(contextView->isVisible()) {
          //There is a context-browser view visible, we don't need a tooltip
          delete navigationWidget;
          return;
        }
        contextView->setNavigationWidget(navigationWidget);
      }
      
    if(m_currentToolTip) {
      m_currentToolTip->deleteLater();
      m_currentToolTip = 0;
    }
    
    KDevelop::NavigationToolTip* tooltip = new KDevelop::NavigationToolTip(view, view->mapToGlobal(view->cursorToCoordinate(position)) + QPoint(20, 40), navigationWidget);
    tooltip->resize( navigationWidget->sizeHint() + QSize(10, 10) );
    kDebug() << "tooltip size" << tooltip->size();
    m_currentToolTip = tooltip;
    ActiveToolTip::showToolTip(tooltip);

    //First disconnect to prevent multiple connections
    disconnect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), this, SLOT(hideTooTip()));
    disconnect(view, SIGNAL(focusOut(KTextEditor::View*)), this, SLOT(hideTooTip()));
    connect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), this, SLOT(hideTooTip()));
    connect(view, SIGNAL(focusOut(KTextEditor::View*)), this, SLOT(hideTooTip()));
    
  }else{
    kDebug() << "not showing tooltip, no navigation-widget";
  }
  
}


void ContextBrowserPlugin::clearMouseHover() {
  m_mouseHoverCursor = SimpleCursor::invalid();
  m_mouseHoverDocument.clear();
}

Attribute::Ptr highlightedUseAttribute(bool /*mouseHighlight*/, bool bold) {
  if(bold) {
    static Attribute::Ptr standardBoldAttribute = Attribute::Ptr();
    if( !standardBoldAttribute ) {
      standardBoldAttribute= Attribute::Ptr( new Attribute() );
      standardBoldAttribute->setBackgroundFillWhitespace(true);

      // mixing (255, 255, 0, 100) with white yields this:
      standardBoldAttribute->setBackground(QColor(251, 250, 150));
      standardBoldAttribute->setFontBold(true);

      // force a foreground color to overwrite default Kate highlighting, i.e. of Q_OBJECT or similar
      // foreground color could change, hence apply it everytime
      standardBoldAttribute->setForeground(QColor(0, 0, 0, 255)); //Don't use alpha here, as kate uses the alpha only to blend with the document background color
    }
    return standardBoldAttribute;
  }else{
    static Attribute::Ptr standardAttribute = Attribute::Ptr();
    if( !standardAttribute ) {
      standardAttribute = Attribute::Ptr( new Attribute() );
      standardAttribute->setBackgroundFillWhitespace(true);
      // mixing (255, 255, 0, 100) with white yields this:
      standardAttribute->setBackground(QColor(251, 250, 150));
      // force a foreground color to overwrite default Kate highlighting, i.e. of Q_OBJECT or similar
      // foreground color could change, hence apply it everytime
      standardAttribute->setForeground(QColor(0, 0, 0, 255)); //Don't use alpha here, as kate uses the alpha only to blend with the document background color
    }
    return standardAttribute;
  }
}

///NOTE: when this gets used again we have to set an alpha value as well
/*
Attribute::Ptr highlightedDeclarationAttribute() {
  static Attribute::Ptr standardAttribute = Attribute::Ptr();
  if( !standardAttribute ) {
    standardAttribute = Attribute::Ptr( new Attribute() );
    standardAttribute->setBackgroundFillWhitespace(true);
    standardAttribute->setBackground(Qt::red);
  }
  return standardAttribute;
}
*/

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

void ContextBrowserPlugin::changeHighlight( KTextEditor::SmartRange* range, bool highlight, bool /*declaration*/, bool mouseHighlight ) {
  if( !range )
    return;

  QMutexLocker lock(&m_backupsMutex);
  
  if( highlight ) {
/*    if( declaration )
      attrib = highlightedDeclarationAttribute();
    else*/

    Attribute::Ptr attrib = highlightedUseAttribute(mouseHighlight, range->attribute() ? range->attribute()->fontBold() : false);

    if( !m_backups.contains(range) ) {
      m_backups[range] = qMakePair(attrib, range->attribute());
      watchRange(range);
    }
    range->setAttribute(attrib);
  }else{
    //Reset old highlighting, if our set attribute is still on the range
    if( m_backups.contains(range) && m_backups[range].first == range->attribute() ) {
      range->setAttribute(m_backups[range].second);
      m_backups.remove(range);
    }
    ignoreRange(range);
  }

}

void ContextBrowserPlugin::changeHighlight( View* view, KDevelop::Declaration* decl, bool highlight, bool mouseHighlight ) {
  if( !view || !decl || !decl->context() ) {
    kDebug() << "invalid view/declaration";
    return;
  }

  KTextEditor::SmartRange* range = decl->smartRange();
  if( range )
    changeHighlight( range, highlight, true, mouseHighlight );


  QList<KTextEditor::SmartRange*> uses;
  {
    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    uses = decl->smartUses();
  }

  foreach(KTextEditor::SmartRange* range, uses)
    changeHighlight( range, highlight, false, mouseHighlight );

  if( FunctionDefinition* def = FunctionDefinition::definition(decl) )
    if( def->smartRange() )
      changeHighlight( def->smartRange(), highlight, false, mouseHighlight );
}

bool ContextBrowserPlugin::findSpecialObject(View* view, const SimpleCursor& position, ILanguage*& pickedLanguage)
{
      SmartInterface* iface = dynamic_cast<SmartInterface*>(view->document());
      if (!iface) return false;

      if(m_highlightedRange.contains(view)) { // remove old highlighting
        QMutexLocker lock(iface->smartMutex());
        //Q_ASSERT(m_highlightedRange[view]->document() == view->document());
        if (m_highlightedRange[view]->document() == view->document()) {
          delete m_highlightedRange[view];
          m_highlightedRange.remove(view);
      } else {
          kDebug() << "m_highlightedRange[view]->document() != view->document()";	
        }
      }

      KUrl viewUrl = view->document()->url();
      QList<ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(viewUrl);

      SimpleRange r = SimpleRange::invalid();
      foreach( ILanguage* language, languages) {
        r = language->languageSupport()->specialLanguageObjectRange(viewUrl, position);
        if(r.isValid()) {
          pickedLanguage = language;
          break;
        }
      }

      if (r.isValid()) {
        m_highlightedRange[view] = iface->newSmartRange( r.textRange(), m_highlightedRange[view] );//iface->newSmartRange( view->document()->documentRange() );
        iface->addHighlightToDocument(m_highlightedRange[view]);
        m_highlightedRange[view]->setAttribute( highlightedSpecialObjectAttribute() );
        watchRange(m_highlightedRange[view]);
        return true;
      } else {
        pickedLanguage = 0;
        return false;
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
      }
      return foundDeclaration;
}

bool ContextBrowserPlugin::showDeclarationView(View* view, const SimpleCursor& position, Declaration* foundDeclaration, DUContext* ctx)
{
        Q_UNUSED(position);
        Q_ASSERT(foundDeclaration);
        bool success = false;
        foreach(ContextBrowserView* contextView, m_views) {
          if(masterWidget(contextView) == masterWidget(view)) {
              contextView->setDeclaration(foundDeclaration, ctx->topContext());
              success = true;
          }
        }
        return success;
}

bool ContextBrowserPlugin::showSpecialObjectView(View* view, const SimpleCursor& position, ILanguage* pickedLanguage, DUContext* /*ctx*/)
{
        //Q_ASSERT(pickedLanguage != 0); // specialObject was found, pickedLanguage must have been set
        if (!pickedLanguage) {
          kDebug() << "Special's object language turned null.";
          return false;
        }
        bool success = false;
        foreach(ContextBrowserView* contextView, m_views) {
          if(masterWidget(contextView) == masterWidget(view)) {
              ILanguageSupport* ls = pickedLanguage->languageSupport();
              QWidget* w = ls->specialLanguageObjectNavigationWidget(view->document()->url(), position);
              contextView->setSpecialNavigationWidget(w);
              success = true;
            }
        }
        return success;
}

void ContextBrowserPlugin::showContextView(View* view, const SimpleCursor& position, DUContext* ctx)
{
    Q_UNUSED(position);
    Q_ASSERT(ctx);
    foreach(ContextBrowserView* contextView, m_views) {
      if(masterWidget(contextView) == masterWidget(view)) {
        contextView->setContext(SpecializationStore::self().applySpecialization(ctx, ctx->topContext()));
      }
    }
}

DUContext* contextAt(const SimpleCursor& position, TopDUContext* topContext)
{
      DUContext* ctx = topContext->findContextAt(position);
      while(ctx && (ctx->type() == DUContext::Template || ctx->type() == DUContext::Helper || ctx->localScopeIdentifier().isEmpty()) && ctx->parentContext())
        ctx = ctx->parentContext();
      return ctx;
}


void ContextBrowserPlugin::unHighlightAll(KTextEditor::View* selectView)
{
    foreach(KTextEditor::View* view, m_highlightedDeclarations.keys()) {
      if(selectView && selectView != view)
        continue;
      
      KTextEditor::SmartInterface* smart = dynamic_cast<KTextEditor::SmartInterface*>(view->document());
    
      QMutexLocker smartLock(smart->smartMutex());
      QMutexLocker lock(&m_backupsMutex);

      QMap< KTextEditor::SmartRange*, QPair< Attribute::Ptr, Attribute::Ptr > >::iterator it = m_backups.begin();
      while(it != m_backups.end()) {
        if(it.key()->document() == view->document()) {
          if(it.key()->attribute() == it->first)
            it.key()->setAttribute(it->second); //Set the backed up attribute, if it wasn't changed yet
          ignoreRange(it.key());
          it = m_backups.erase(it);
        } else {
          ++it;
        }
      }
      
      changeHighlight( view, m_highlightedDeclarations[view].data(), false, false );
    }

    m_highlightedDeclarations.clear();
}

void ContextBrowserPlugin::updateBrowserWidgetFor(View* view)
{
    bool keptHighlightedDeclaration = m_keepHighlightedDeclaration.contains(view) && m_highlightedDeclarations[view].data();
    m_keepHighlightedDeclaration.remove(view);
    
    bool mouseHighlight =
      (view->document()->url() == m_mouseHoverDocument) &&
      (m_mouseHoverCursor.isValid());

    SimpleCursor position;
    if (mouseHighlight) {
     position = m_mouseHoverCursor;
    } else {
     position = SimpleCursor(view->cursorPosition());
    }

    ///First: Check whether there is a special language object
    ///@todo Maybe make this optional, because it can be slow
    ILanguage* pickedLanguage = 0;
    bool foundSpecialObject = 0;
    if(!keptHighlightedDeclaration)
      foundSpecialObject  = findSpecialObject(view,position, pickedLanguage);

    KDevelop::DUChainReadLocker lock( DUChain::lock(), 100 );
    if(!lock.locked()) {
      kDebug() << "Failed to lock du-chain in time";
      return;
    }

    if(!keptHighlightedDeclaration)
      unHighlightAll();

    TopDUContext* topContext = DUChainUtils::standardContextForUrl(view->document()->url());
    if (!topContext) return;
    DUContext* ctx = contextAt(position, topContext);
    if (!ctx) return;
    Declaration* foundDeclaration = 0;
    
    //Only update the history if this context is around the text cursor
    if(core()->documentController()->activeDocument() && position == SimpleCursor(view->cursorPosition()) && view->document() == core()->documentController()->activeDocument()->textDocument())
    {
      foreach(ContextBrowserView* contextView, m_views)
        if(masterWidget(contextView) == masterWidget(view))
          contextView->updateHistory(ctx, position);
    }
    
    if(keptHighlightedDeclaration) {
      foundDeclaration = m_highlightedDeclarations[view].data();
      kDebug() << "keeping";
    }
    
    if(!foundSpecialObject && !foundDeclaration)
      foundDeclaration = findDeclaration(view, position, mouseHighlight);
    
    if( foundDeclaration ) {
      m_lastHighlightedDeclaration = IndexedDeclaration(foundDeclaration);
      m_highlightedDeclarations[view] = foundDeclaration;
      changeHighlight( view, foundDeclaration, true, mouseHighlight );
    }else{
      m_highlightedDeclarations.remove(view);
    }
    
    bool addedWidget = false;
    // try to add a declaration navigation widget
    if(foundDeclaration || foundSpecialObject) {
      IDocument* doc = core()->documentController()->activeDocument();
      if(mouseHighlight || (view->isActiveView() && doc && doc->textDocument() == view->document())) {
          addedWidget = foundSpecialObject ?
            showSpecialObjectView(view, position, pickedLanguage, ctx) :
            showDeclarationView(view, position, foundDeclaration, ctx);
      }
    }

    // if that failed, try to add a context navigation widget
    if(!addedWidget && !mouseHighlight) {
        showContextView(view, position, ctx);
    }
}

void ContextBrowserPlugin::updateViews()
{
  foreach( View* view, m_updateViews ) {
    updateBrowserWidgetFor(view);
  }
  m_updateViews.clear();
  m_useDeclaration = IndexedDeclaration();
}

void ContextBrowserPlugin::declarationSelectedInUI(DeclarationPointer decl)
{
  m_useDeclaration = IndexedDeclaration(decl.data());
  if(core()->documentController()->activeDocument() && core()->documentController()->activeDocument()->textDocument() && core()->documentController()->activeDocument()->textDocument()->activeView())
    m_updateViews << core()->documentController()->activeDocument()->textDocument()->activeView();

  m_updateTimer->start(highlightingTimeout); // triggers updateViews()
}

void ContextBrowserPlugin::parseJobFinished(KDevelop::ParseJob* job)
{
  if(job->duChain() && job->duChain()->smartRange()) {
    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    registerAsRangeWatcher(job->duChain());
  }
  
  for(QMap< View*, DeclarationPointer >::const_iterator it = m_highlightedDeclarations.constBegin(); it != m_highlightedDeclarations.constEnd(); ++it) {
    if(it.key()->document()->url() == job->document().toUrl()) {
      if(m_updateViews.isEmpty())
        m_updateTimer->start(highlightingTimeout);
      
      if(!m_updateViews.contains(it.key())) {
        kDebug() << "adding view for update";
        m_updateViews << it.key();
        m_keepHighlightedDeclaration.insert(it.key());
      }
    }
  }
}

void ContextBrowserPlugin::registerAsRangeWatcher(KDevelop::DUChainBase* base)
{
  if(base->smartRange()) {
    watchRange(base->smartRange());
}
}

void ContextBrowserPlugin::registerAsRangeWatcher(KDevelop::DUContext* ctx)
{
  if(!ctx || !ctx->smartRange())
    return;

  QMutex* lockSmartMutex = 0;

  if(!ctx->parentContext()) {
    KTextEditor::SmartInterface* smart = dynamic_cast<KTextEditor::SmartInterface*>(ctx->smartRange()->document());
    Q_ASSERT(smart);
    lockSmartMutex = smart->smartMutex();
  }
  
  QMutexLocker lock(lockSmartMutex);
  
  if(dynamic_cast<TopDUContext*>(ctx) && static_cast<TopDUContext*>(ctx)->parsingEnvironmentFile() && static_cast<TopDUContext*>(ctx)->parsingEnvironmentFile()->isProxyContext() && !ctx->importedParentContexts().isEmpty())
    return registerAsRangeWatcher(ctx->importedParentContexts()[0].context(0));

  foreach(Declaration* decl, ctx->localDeclarations())
    registerAsRangeWatcher(decl);

  for(int a = 0; a < ctx->usesCount(); ++a) {
    KTextEditor::SmartRange* range = ctx->useSmartRange(a);
    if(range)
      watchRange(range);
  }

  foreach(DUContext* child, ctx->childContexts())
    registerAsRangeWatcher(child);
}

void ContextBrowserPlugin::textDocumentCreated( KDevelop::IDocument* document )
{
  Q_ASSERT(document->textDocument());

  connect( document->textDocument(), SIGNAL(destroyed( QObject* )), this, SLOT( documentDestroyed( QObject* ) ) );
  connect( document->textDocument(), SIGNAL( viewCreated( KTextEditor::Document* , KTextEditor::View* ) ), this, SLOT( viewCreated( KTextEditor::Document*, KTextEditor::View* ) ) );

  foreach( View* view, document->textDocument()->views() )
    viewCreated( document->textDocument(), view );

  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  QList<TopDUContext*> chains = DUChain::self()->chainsForDocument( document->url() );

  foreach( TopDUContext* chain, chains )
    registerAsRangeWatcher( chain );
}

void ContextBrowserPlugin::documentClosed( KDevelop::IDocument* /*document*/ )
{
}

void ContextBrowserPlugin::documentDestroyed( QObject* /*obj*/ )
{
}

void ContextBrowserPlugin::viewDestroyed( QObject* obj )
{
  m_highlightedDeclarations.remove(static_cast<KTextEditor::View*>(obj));
  m_updateViews.remove(static_cast<View*>(obj));
}

void ContextBrowserPlugin::cursorPositionChanged( View* view, const KTextEditor::Cursor& newPosition )
{
  if(view->document() == m_lastInsertionDocument && newPosition == m_lastInsertionPos)
  {
    //Do not update the highlighting while typing
    m_lastInsertionDocument = 0;
    m_lastInsertionPos = KTextEditor::Cursor();
    m_keepHighlightedDeclaration.insert(view);
  }else{
    m_keepHighlightedDeclaration.remove(view);
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
  disconnect( v, SIGNAL( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ), this, SLOT( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ) ); ///Just to make sure that multiple connections don't happen
  connect( v, SIGNAL( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ), this, SLOT( cursorPositionChanged( KTextEditor::View*, const KTextEditor::Cursor& ) ) );
  connect( v, SIGNAL(destroyed( QObject* )), this, SLOT( viewDestroyed( QObject* ) ) );
  disconnect( v->document(), SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), this, SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
  connect( v->document(), SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), this, SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));

  KTextEditor::TextHintInterface *iface = dynamic_cast<KTextEditor::TextHintInterface*>(v);
  if( !iface )
      return;

  iface->enableTextHints(highlightingTimeout);

  connect(v, SIGNAL(needTextHint(const KTextEditor::Cursor&, QString&)), this, SLOT(textHintRequested(const KTextEditor::Cursor&, QString&)));
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
    KDevelop::SimpleCursor c(doc->activeView()->cursorPosition());


    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    KDevelop::TopDUContext* chosen = DUChainUtils::standardContextForUrl(doc->url());

    if( chosen )
    {
      Declaration* decl = 0;
      //If we have a locked declaration, use that for jumping
      foreach(ContextBrowserView* view, m_views) {
        decl = view->lockedDeclaration().data(); ///@todo Somehow match the correct context-browser view if there is multiple
        if(decl)
          break;
      }
      
      if(!decl) //Try finding a declaration under the cursor
        decl = DUChainUtils::itemUnderCursor(doc->url(), c);
      
      if(decl) {
        
        Declaration* target = 0;
        if(forward)
          //Try jumping from definition to declaration
          target = DUChainUtils::declarationForDefinition(decl, chosen);
        else if(decl->url().toUrl() == doc->url() && decl->range().contains(c))
          //Try jumping from declaration to definition
          target = FunctionDefinition::definition(decl);
        
          if(target && target != decl) {
            SimpleCursor jumpTo = target->range().start;
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
              QList<SimpleRange> useRanges = allUses(top, decl, true);
              qSort(useRanges);
              if(!useRanges.isEmpty()) {
                KUrl url = top->url().toUrl();
                SimpleRange selectUse = forward ? useRanges.first() : useRanges.back();
                lock.unlock();
                core()->documentController()->openDocument(url, cursorToRange(selectUse.start));
              }
            }
          }
          return;
        }
        //Check whether we are within a use
        QList<SimpleRange> localUses = allUses(chosen, decl, true);
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
                SimpleRange range = decl->range();
                range.end = range.start;
                lock.unlock();
                core()->documentController()->openDocument(u, range.textRange());
                return;
              }else{
                TopDUContext* nextTop = usingFiles[nextFile].data();
                
                KUrl u(nextTop->url().str());
                
                QList<SimpleRange> nextTopUses = allUses(nextTop, decl, true);
                qSort(nextTopUses);
                
                if(!nextTopUses.isEmpty()) {
                  SimpleRange range = forward ? nextTopUses.front() : nextTopUses.back();
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
              SimpleRange range = localUses[nextUse];
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

QWidget* ContextBrowserPlugin::toolbarWidgetForMainWindow(QWidget* widgetInWindow)
{
  QWidget* master = masterWidget(widgetInWindow);
  
  for(QList< QPointer< QWidget > >::const_iterator it = m_toolbarWidgets.constBegin(); it != m_toolbarWidgets.constEnd(); ++it) {
    if((*it) && masterWidget(*it) == master) {
      return *it;
    }
  }
  m_toolbarWidgets.append(new QWidget(master));
  m_toolbarWidgets.back()->setHidden(true);
  return m_toolbarWidgets.back();
}

#include "contextbrowser.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
