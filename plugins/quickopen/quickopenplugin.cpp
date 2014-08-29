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

#include "quickopenplugin.h"

#include <cassert>
#include <typeinfo>
#include <QTreeView>
#include <QHeaderView>
#include <QDialog>
#include <QKeyEvent>
#include <QApplication>
#include <QScrollBar>
#include <QCheckBox>
#include <QMetaObject>
#include <QWidgetAction>

#include <kbuttongroup.h>
#include <KLocalizedString>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <kparts/mainwindow.h>
#include <KConfigCore/ksharedconfig.h>
#include <KConfigCore/KConfigGroup>
#include <kactioncollection.h>
#include <kaction.h>
#include <kshortcut.h>
#include <kdebug.h>

#include <interfaces/ilanguage.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/types/identifiedtype.h>
#include <serialization/indexedstring.h>
#include <language/duchain/types/functiontype.h>

#include "expandingtree/expandingdelegate.h"
#include "ui_quickopen.h"
#include "quickopenmodel.h"
#include "projectfilequickopen.h"
#include "projectitemquickopen.h"
#include "declarationlistquickopen.h"
#include "documentationquickopenprovider.h"
#include <language/duchain/functiondefinition.h>
#include <qmenu.h>
#include <qdesktopwidget.h>
#include <util/activetooltip.h>
#include <qboxlayout.h>
#include <language/util/navigationtooltip.h>
#include <interfaces/contextmenuextension.h>
#include <language/interfaces/codecontext.h>

using namespace KDevelop;

const bool noHtmlDestriptionInOutline = true;

class QuickOpenWidgetCreator {
  public:
    virtual ~QuickOpenWidgetCreator() {
    }
    virtual QuickOpenWidget* createWidget() = 0;
    virtual QString objectNameForLine() = 0;
    virtual void widgetShown() {
    }
};

class StandardQuickOpenWidgetCreator : public QuickOpenWidgetCreator {
  public:
    StandardQuickOpenWidgetCreator(const QStringList& items, const QStringList& scopes)
      : m_items(items)
      , m_scopes(scopes)
    {
    }

    virtual QString objectNameForLine() {
      return "Quickopen";
    }

    void setItems(const QStringList& scopes, const QStringList& items)
    {
      m_scopes = scopes;
      m_items = items;
    }

    virtual QuickOpenWidget* createWidget() {
      QStringList useItems = m_items;
      if(useItems.isEmpty())
        useItems = QuickOpenPlugin::self()->lastUsedItems;
      
      QStringList useScopes = m_scopes;
      if(useScopes.isEmpty())
        useScopes = QuickOpenPlugin::self()->lastUsedScopes;
      
      return new QuickOpenWidget( i18n("Quick Open"), QuickOpenPlugin::self()->m_model, QuickOpenPlugin::self()->lastUsedItems, useScopes, false, true );
    }
    
    QStringList m_items;
    QStringList m_scopes;
};

class QuickOpenDelegate : public ExpandingDelegate {
public:
  QuickOpenDelegate(ExpandingWidgetModel* model, QObject* parent = 0L) : ExpandingDelegate(model, parent) {
  }
  virtual QList<QTextLayout::FormatRange> createHighlighting(const QModelIndex& index, QStyleOptionViewItem& option) const {
    QList<QVariant> highlighting = index.data(KTextEditor::CodeCompletionModel::CustomHighlight).toList();
    if(!highlighting.isEmpty())
      return highlightingFromVariantList(highlighting);
    return ExpandingDelegate::createHighlighting( index, option );
  }

};

class OutlineFilter : public DUChainUtils::DUChainItemFilter {
public:
  enum OutlineMode { Functions, FunctionsAndClasses };
  OutlineFilter(QList<DUChainItem>& _items, OutlineMode _mode = FunctionsAndClasses) : items(_items), mode(_mode) {
  }
  virtual bool accept(Declaration* decl) {
    if(decl->range().isEmpty())
      return false;
    bool collectable = mode == Functions ? decl->isFunctionDeclaration() : (decl->isFunctionDeclaration() || (decl->internalContext() && decl->internalContext()->type() == DUContext::Class));
    if (collectable) {
      DUChainItem item;
      item.m_item = IndexedDeclaration(decl);
      item.m_text = decl->toString();
      items << item;

      return true;
    } else
      return false;
  }
  virtual bool accept(DUContext* ctx) {
    if(ctx->type() == DUContext::Class || ctx->type() == DUContext::Namespace || ctx->type() == DUContext::Global || ctx->type() == DUContext::Other || ctx->type() == DUContext::Helper )
      return true;
    else
      return false;
  }
  QList<DUChainItem>& items;
  OutlineMode mode;
};

K_PLUGIN_FACTORY_WITH_JSON(KDevQuickOpenFactory, "kdevquickopen.json", registerPlugin<QuickOpenPlugin>();)
// K_EXPORT_PLUGIN(KDevQuickOpenFactory(KAboutData("kdevquickopen","kdevquickopen", ki18n("Quick Open"), "0.1", ki18n("This plugin allows quick access to project files and language-items like classes/functions."), KAboutData::License_GPL)))

Declaration* cursorDeclaration() {

  KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
  if(!view)
    return 0;

  KDevelop::DUChainReadLocker lock( DUChain::lock() );

  return DUChainUtils::declarationForDefinition( DUChainUtils::itemUnderCursor( view->document()->url(), KTextEditor::Cursor(view->cursorPosition()) ) );
}

///The first definition that belongs to a context that surrounds the current cursor
Declaration* cursorContextDeclaration() {
  KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
  if(!view)
    return 0;

  KDevelop::DUChainReadLocker lock( DUChain::lock() );

  TopDUContext* ctx = DUChainUtils::standardContextForUrl(view->document()->url());
  if(!ctx)
    return 0;

  KTextEditor::Cursor cursor(view->cursorPosition());

  DUContext* subCtx = ctx->findContext(ctx->transformToLocalRevision(cursor));

  while(subCtx && !subCtx->owner())
    subCtx = subCtx->parentContext();

  Declaration* definition = 0;

  if(!subCtx || !subCtx->owner())
    definition = DUChainUtils::declarationInLine(cursor, ctx);
  else
    definition = subCtx->owner();

  if(!definition)
    return 0;

  return definition;
}

//Returns only the name, no template-parameters or scope
QString cursorItemText() {
  KDevelop::DUChainReadLocker lock( DUChain::lock() );

  Declaration* decl = cursorDeclaration();
  if(!decl)
    return QString();

  IDocument* doc = ICore::self()->documentController()->activeDocument();
  if(!doc)
    return QString();

  TopDUContext* context = DUChainUtils::standardContextForUrl( doc->url() );

  if( !context ) {
    kDebug() << "Got no standard context";
    return QString();
  }

  AbstractType::Ptr t = decl->abstractType();
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(t.data());
  if( idType && idType->declaration(context) )
    decl = idType->declaration(context);

  if(!decl->qualifiedIdentifier().isEmpty())
    return decl->qualifiedIdentifier().last().identifier().str();

  return QString();
}

QuickOpenLineEdit* QuickOpenPlugin::createQuickOpenLineWidget()
{
  return new QuickOpenLineEdit(new StandardQuickOpenWidgetCreator(QStringList(), QStringList()));
}

void QuickOpenWidget::showStandardButtons(bool show)
{
  if(show) {
    o.okButton->show();
    o.cancelButton->show();
  }else{
    o.okButton->hide();
    o.cancelButton->hide();
  }
}

QuickOpenWidget::QuickOpenWidget( QString title, QuickOpenModel* model, const QStringList& initialItems, const QStringList& initialScopes, bool listOnly, bool noSearchField ) : m_model(model), m_expandedTemporary(false) {
  m_filterTimer.setSingleShot(true);
  connect(&m_filterTimer, SIGNAL(timeout()), this, SLOT(applyFilter()));

  Q_UNUSED( title );
  o.setupUi( this );
  o.list->header()->hide();
  o.list->setRootIsDecorated( false );
  o.list->setVerticalScrollMode( QAbstractItemView::ScrollPerItem );
  
  connect(o.list->verticalScrollBar(), SIGNAL(valueChanged(int)), m_model, SLOT(placeExpandingWidgets()));

  o.searchLine->setFocus();

  o.list->setItemDelegate( new QuickOpenDelegate( m_model, o.list ) );

  if(!listOnly) {
    QStringList allTypes = m_model->allTypes();
    QStringList allScopes = m_model->allScopes();

    QMenu* itemsMenu = new QMenu;
    
    foreach( const QString &type, allTypes )
    {
      QAction* action = new QAction(type, itemsMenu);
      action->setCheckable(true);
      action->setChecked(initialItems.isEmpty() || initialItems.contains( type ));
      connect( action, SIGNAL(toggled(bool)), this, SLOT(updateProviders()), Qt::QueuedConnection );
      itemsMenu->addAction(action);
    }

    o.itemsButton->setMenu(itemsMenu);

    QMenu* scopesMenu = new QMenu;

    foreach( const QString &scope, allScopes )
    {
      QAction* action = new QAction(scope, scopesMenu);
      action->setCheckable(true);
      action->setChecked(initialScopes.isEmpty() || initialScopes.contains( scope ) );

      connect( action, SIGNAL(toggled(bool)), this, SLOT(updateProviders()), Qt::QueuedConnection );
      scopesMenu->addAction(action);
    }
    
    o.scopesButton->setMenu(scopesMenu);

  }else{
    o.list->setFocusPolicy(Qt::StrongFocus);
    o.scopesButton->hide();
    o.itemsButton->hide();
    o.label->hide();
    o.label_2->hide();
  }

  showSearchField(!noSearchField);
  
  o.okButton->hide();
  o.cancelButton->hide();
  
  o.searchLine->installEventFilter( this );
  o.list->installEventFilter( this );
  o.list->setFocusPolicy(Qt::NoFocus);
  o.scopesButton->setFocusPolicy(Qt::NoFocus);
  o.itemsButton->setFocusPolicy(Qt::NoFocus);

  connect( o.searchLine, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)) );

  connect( o.list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)) );

  connect(o.okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
  connect(o.okButton, SIGNAL(clicked(bool)), SIGNAL(ready()));
  connect(o.cancelButton, SIGNAL(clicked(bool)), SIGNAL(ready()));
  
  updateProviders();
  updateTimerInterval(true);
  
// no need to call this, it's done by updateProviders already
//   m_model->restart();
}

void QuickOpenWidget::updateTimerInterval(bool cheapFilterChange)
{
  const int MAX_ITEMS = 10000;
  if ( cheapFilterChange && m_model->rowCount(QModelIndex()) < MAX_ITEMS ) {
    // cheap change and there are currently just a few items,
    // so apply filter instantly
    m_filterTimer.setInterval(0);
  } else if ( m_model->unfilteredRowCount() < MAX_ITEMS ) {
    // not a cheap change, but there are generally
    // just a few items in the list: apply filter instantly
    m_filterTimer.setInterval(0);
  } else {
    // otherwise use a timer to prevent sluggishness while typing
    m_filterTimer.setInterval(300);
  }
}

void QuickOpenWidget::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    
    // The column width only has an effect _after_ the widget has been shown
    o.list->setColumnWidth( 0, 20 );
}

void QuickOpenWidget::setAlternativeSearchField(KLineEdit* alterantiveSearchField)
{
    o.searchLine = alterantiveSearchField;
    o.searchLine->installEventFilter( this );
    connect( o.searchLine, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)) );
}


void QuickOpenWidget::showSearchField(bool b)
{
    if(b){
      o.searchLine->show();
      o.searchLabel->show();
    }else{
      o.searchLine->hide();
      o.searchLabel->hide();
    }
}

void QuickOpenWidget::prepareShow()
{
  o.list->setModel( 0 );
  o.list->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  m_model->setTreeView( o.list );
  o.list->setModel( m_model );
  
  m_filterTimer.stop();
  m_filter = QString();
  
  if (!m_preselectedText.isEmpty())
  {
    o.searchLine->setText(m_preselectedText);
    o.searchLine->selectAll();
  }
  
  m_model->restart(false);
  
  connect( o.list->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
           SLOT(callRowSelected()) );
  connect( o.list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           SLOT(callRowSelected()) );
}

void QuickOpenWidgetDialog::run() {
  m_widget->prepareShow();
  m_dialog->show();
}

QuickOpenWidget::~QuickOpenWidget() {
  m_model->setTreeView( 0 );
}


QuickOpenWidgetDialog::QuickOpenWidgetDialog(QString title, QuickOpenModel* model, const QStringList& initialItems, const QStringList& initialScopes, bool listOnly, bool noSearchField)
{
  m_widget = new QuickOpenWidget(title, model, initialItems, initialScopes, listOnly, noSearchField);
  // the QMenu might close on esc and we want to close the whole dialog then
  connect( m_widget, SIGNAL(aboutToHide()), this, SLOT(deleteLater()) );
  
  //KDialog always sets the focus on the "OK" button, so we use QDialog
  m_dialog = new QDialog( ICore::self()->uiController()->activeMainWindow() );
  m_dialog->resize(QSize(800, 400));

  m_dialog->setWindowTitle(title);
  QVBoxLayout* layout = new QVBoxLayout(m_dialog);
  layout->addWidget(m_widget);
  m_widget->showStandardButtons(true);
  connect(m_widget, SIGNAL(ready()), m_dialog, SLOT(close()));
  connect( m_dialog, SIGNAL(accepted()), m_widget, SLOT(accept()) );
}


QuickOpenWidgetDialog::~QuickOpenWidgetDialog()
{
  delete m_dialog;
}

void QuickOpenWidget::setPreselectedText(const QString& text)
{
  m_preselectedText = text;
}

void QuickOpenWidget::updateProviders() {
  if(QAction* action = qobject_cast<QAction*>(sender())) {
    QMenu* menu = qobject_cast<QMenu*>(action->parentWidget());
    if(menu) {
      menu->show();
      menu->setActiveAction(action);
    }
  }
  
  QStringList checkedItems;
  
  if(o.itemsButton->menu()) {
  
    foreach( QObject* obj, o.itemsButton->menu()->children() ) {
      QAction* box = qobject_cast<QAction*>( obj );
      if( box ) {
        if( box->isChecked() )
          checkedItems << box->text().remove('&');
      }
    }
    o.itemsButton->setText(checkedItems.join(", "));
  }

  QStringList checkedScopes;
  
  if(o.scopesButton->menu()) {
    
    foreach( QObject* obj, o.scopesButton->menu()->children() ) {
      QAction* box = qobject_cast<QAction*>( obj );
      if( box ) {
        if( box->isChecked() )
          checkedScopes << box->text().remove('&');
      }
    }
    
    o.scopesButton->setText(checkedScopes.join(", "));
  }

  emit itemsChanged( checkedItems );
  emit scopesChanged( checkedScopes );
  m_model->enableProviders( checkedItems, checkedScopes );
}

void QuickOpenWidget::textChanged( const QString& str )
{
  // "cheap" when something was just appended to the current filter
  updateTimerInterval(str.startsWith(m_filter));
  m_filter = str;
  m_filterTimer.start();
}

void QuickOpenWidget::applyFilter()
{
  m_model->textChanged( m_filter );

  QModelIndex currentIndex = m_model->index(0, 0, QModelIndex());
  o.list->selectionModel()->setCurrentIndex( currentIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows | QItemSelectionModel::Current );

  callRowSelected();
}

void QuickOpenWidget::callRowSelected() {
  QModelIndex currentIndex = o.list->selectionModel()->currentIndex();
  if( currentIndex.isValid() )
    m_model->rowSelected( currentIndex );
  else
    kDebug() << "current index is not valid";
}

void QuickOpenWidget::accept() {
  QString filterText = o.searchLine->text();
  m_model->execute( o.list->currentIndex(), filterText );
}

void QuickOpenWidget::doubleClicked ( const QModelIndex & index ) {
  // crash guard: https://bugs.kde.org/show_bug.cgi?id=297178
  o.list->setCurrentIndex(index);
  QMetaObject::invokeMethod(this, "accept", Qt::QueuedConnection);
  QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

bool QuickOpenWidget::eventFilter ( QObject * watched, QEvent * event )
{
  QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
  
  if( event->type() == QEvent::KeyRelease ) {
    if(keyEvent->key() == Qt::Key_Alt) {
      if((m_expandedTemporary && m_altDownTime.msecsTo( QTime::currentTime() ) > 300) || (!m_expandedTemporary && m_altDownTime.msecsTo( QTime::currentTime() ) < 300 && m_hadNoCommandSinceAlt)) {
        //Unexpand the item
        QModelIndex row = o.list->selectionModel()->currentIndex();
        if( row.isValid() ) {
          row = row.sibling( row.row(), 0 );
          if(m_model->isExpanded( row ))
            m_model->setExpanded( row, false );
        }      
      }
      m_expandedTemporary = false;
    }
  }
  
  if( event->type() == QEvent::KeyPress  ) {
    m_hadNoCommandSinceAlt = false;
    if(keyEvent->key() == Qt::Key_Alt) {
      m_hadNoCommandSinceAlt = true;
      //Expand
      QModelIndex row = o.list->selectionModel()->currentIndex();
      if( row.isValid() ) {
        row = row.sibling( row.row(), 0 );
        m_altDownTime = QTime::currentTime();
        if(!m_model->isExpanded( row )) {
          m_expandedTemporary = true;
          m_model->setExpanded( row, true );
        }
      }      
    }

    switch( keyEvent->key() ) {
      case Qt::Key_Tab:
        if ( keyEvent->modifiers() == Qt::NoModifier ) {
          // Tab should work just like Down
          QCoreApplication::sendEvent(o.list, new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier));
          QCoreApplication::sendEvent(o.list, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Down, Qt::NoModifier));
          return true;
        }
        break;
      case Qt::Key_Backtab:
         if ( keyEvent->modifiers() == Qt::ShiftModifier ) {
          // Shift + Tab should work just like Up
          QCoreApplication::sendEvent(o.list, new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier));
          QCoreApplication::sendEvent(o.list, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Up, Qt::NoModifier));
          return true;
        }
        break;
      case Qt::Key_Down:
      case Qt::Key_Up:
      {
        if( keyEvent->modifiers() == Qt::AltModifier ) {
          QWidget* w = m_model->expandingWidget(o.list->selectionModel()->currentIndex());
          if( KDevelop::QuickOpenEmbeddedWidgetInterface* interface =
              dynamic_cast<KDevelop::QuickOpenEmbeddedWidgetInterface*>( w ) ){
            if( keyEvent->key() == Qt::Key_Down )
              interface->down();
            else
              interface->up();
            return true;
          }
          return false;
        }
      }
      case Qt::Key_PageUp:
      case Qt::Key_PageDown:
        if(watched == o.list )
          return false;
        QApplication::sendEvent( o.list, event );
      //callRowSelected();
        return true;

      case Qt::Key_Left: {
        //Expand/unexpand
        if( keyEvent->modifiers() == Qt::AltModifier ) {
          //Eventually Send action to the widget
          QWidget* w = m_model->expandingWidget(o.list->selectionModel()->currentIndex());
          if( KDevelop::QuickOpenEmbeddedWidgetInterface* interface =
              dynamic_cast<KDevelop::QuickOpenEmbeddedWidgetInterface*>( w ) ){
            interface->previous();
            return true;
          }
        } else {
          QModelIndex row = o.list->selectionModel()->currentIndex();
          if( row.isValid() ) {
            row = row.sibling( row.row(), 0 );

            if( m_model->isExpanded( row ) ) {
              m_model->setExpanded( row, false );
              return true;
            }
          }
        }
        return false;
      }
      case Qt::Key_Right: {
        //Expand/unexpand
        if( keyEvent->modifiers() == Qt::AltModifier ) {
          //Eventually Send action to the widget
          QWidget* w = m_model->expandingWidget(o.list->selectionModel()->currentIndex());
          if( KDevelop::QuickOpenEmbeddedWidgetInterface* interface =
              dynamic_cast<KDevelop::QuickOpenEmbeddedWidgetInterface*>( w ) ){
            interface->next();
            return true;
          }
        } else {
          QModelIndex row = o.list->selectionModel()->currentIndex();
          if( row.isValid() ) {
            row = row.sibling( row.row(), 0 );

            if( !m_model->isExpanded( row ) ) {
              m_model->setExpanded( row, true );
              return true;
            }
          }
        }
        return false;
      }
      case Qt::Key_Return:
      case Qt::Key_Enter: {
        if (m_filterTimer.isActive()) {
          m_filterTimer.stop();
          applyFilter();
        }
        if( keyEvent->modifiers() == Qt::AltModifier ) {
          //Eventually Send action to the widget
          QWidget* w = m_model->expandingWidget(o.list->selectionModel()->currentIndex());
          if( KDevelop::QuickOpenEmbeddedWidgetInterface* interface =
              dynamic_cast<KDevelop::QuickOpenEmbeddedWidgetInterface*>( w ) ){
            interface->accept();
            return true;
          }
        } else {
          QString filterText = o.searchLine->text();
          
          //Safety: Track whether this object is deleted. When execute() is called, a dialog may be opened,
          //which kills the quickopen widget.
          QPointer<QObject> stillExists(this);
          
          if( m_model->execute( o.list->currentIndex(), filterText ) ) {
            
            if(!stillExists)
              return true;
            
            if(!(keyEvent->modifiers() & Qt::ShiftModifier))
              emit ready();
          } else {
            //Maybe the filter-text was changed:
            if( filterText != o.searchLine->text() ) {
              o.searchLine->setText( filterText );
            }
          }
        }
        return true;
      }
    }
  }

  return false;
}


QuickOpenLineEdit* QuickOpenPlugin::quickOpenLine(QString name)
{
  QList< QuickOpenLineEdit* > lines = ICore::self()->uiController()->activeMainWindow()->findChildren<QuickOpenLineEdit*>(name);
  foreach(QuickOpenLineEdit* line, lines) {
    if(line->isVisible()) {
      return line;
    }
  }
  
  return 0;
}

static QuickOpenPlugin* staticQuickOpenPlugin = 0;

QuickOpenPlugin* QuickOpenPlugin::self()
{
  return staticQuickOpenPlugin;
}

void QuickOpenPlugin::createActionsForMainWindow(Sublime::MainWindow* /*window*/, QString& xmlFile, KActionCollection& actions)
{
    xmlFile ="kdevquickopen.rc";

    QAction* quickOpen = actions.addAction("quick_open");
    quickOpen->setText( i18n("&Quick Open") );
    quickOpen->setIcon( QIcon::fromTheme("quickopen") );
    actions.setDefaultShortcut( quickOpen, Qt::CTRL | Qt::ALT | Qt::Key_Q );
    connect(quickOpen, SIGNAL(triggered(bool)), this, SLOT(quickOpen()));

    QAction* quickOpenFile = actions.addAction("quick_open_file");
    quickOpenFile->setText( i18n("Quick Open &File") );
    quickOpenFile->setIcon( QIcon::fromTheme("quickopen-file") );
    actions.setDefaultShortcut( quickOpenFile, Qt::CTRL | Qt::ALT | Qt::Key_O );
    connect(quickOpenFile, SIGNAL(triggered(bool)), this, SLOT(quickOpenFile()));

    QAction* quickOpenClass = actions.addAction("quick_open_class");
    quickOpenClass->setText( i18n("Quick Open &Class") );
    quickOpenClass->setIcon( QIcon::fromTheme("quickopen-class") );
    actions.setDefaultShortcut( quickOpenClass, Qt::CTRL | Qt::ALT | Qt::Key_C );
    connect(quickOpenClass, SIGNAL(triggered(bool)), this, SLOT(quickOpenClass()));

    QAction* quickOpenFunction = actions.addAction("quick_open_function");
    quickOpenFunction->setText( i18n("Quick Open &Function") );
    quickOpenFunction->setIcon( QIcon::fromTheme("quickopen-function") );
    actions.setDefaultShortcut( quickOpenFunction, Qt::CTRL | Qt::ALT | Qt::Key_M );
    connect(quickOpenFunction, SIGNAL(triggered(bool)), this, SLOT(quickOpenFunction()));

    QAction* quickOpenAlreadyOpen = actions.addAction("quick_open_already_open");
    quickOpenAlreadyOpen->setText( i18n("Quick Open &Already Open File") );
    quickOpenAlreadyOpen->setIcon( QIcon::fromTheme("quickopen-file") );
    connect(quickOpenAlreadyOpen, SIGNAL(triggered(bool)), this, SLOT(quickOpenOpenFile()));

    QAction* quickOpenDocumentation = actions.addAction("quick_open_documentation");
    quickOpenDocumentation->setText( i18n("Quick Open &Documentation") );
    quickOpenDocumentation->setIcon( QIcon::fromTheme("quickopen-documentation") );
    actions.setDefaultShortcut( quickOpenDocumentation, Qt::CTRL | Qt::ALT | Qt::Key_D );
    connect(quickOpenDocumentation, SIGNAL(triggered(bool)), this, SLOT(quickOpenDocumentation()));

    m_quickOpenDeclaration = actions.addAction("quick_open_jump_declaration");
    m_quickOpenDeclaration->setText( i18n("Jump to Declaration") );
    m_quickOpenDeclaration->setIcon( QIcon::fromTheme("go-jump-declaration" ) );
    actions.setDefaultShortcut( m_quickOpenDeclaration, Qt::CTRL | Qt::Key_Period );
    connect(m_quickOpenDeclaration, SIGNAL(triggered(bool)), this, SLOT(quickOpenDeclaration()), Qt::QueuedConnection);

    m_quickOpenDefinition = actions.addAction("quick_open_jump_definition");
    m_quickOpenDefinition->setText( i18n("Jump to Definition") );
    m_quickOpenDefinition->setIcon( QIcon::fromTheme("go-jump-definition" ) );
    actions.setDefaultShortcut( m_quickOpenDefinition, Qt::CTRL | Qt::Key_Comma );
    connect(m_quickOpenDefinition, SIGNAL(triggered(bool)), this, SLOT(quickOpenDefinition()), Qt::QueuedConnection);

    QWidgetAction* quickOpenLine = new QWidgetAction(this);
    quickOpenLine->setText( i18n("Embedded Quick Open") );
    //     actions.setDefaultShortcut( quickOpenLine, Qt::CTRL | Qt::ALT | Qt::Key_E );
//     connect(quickOpenLine, SIGNAL(triggered(bool)), this, SLOT(quickOpenLine(bool)));
    quickOpenLine->setDefaultWidget(createQuickOpenLineWidget());
    actions.addAction("quick_open_line", quickOpenLine);

    QAction* quickOpenNextFunction = actions.addAction("quick_open_next_function");
    quickOpenNextFunction->setText( i18n("Next Function") );
    actions.setDefaultShortcut( quickOpenNextFunction, Qt::CTRL| Qt::ALT | Qt::Key_PageDown );
    connect(quickOpenNextFunction, SIGNAL(triggered(bool)), this, SLOT(nextFunction()));

    QAction* quickOpenPrevFunction = actions.addAction("quick_open_prev_function");
    quickOpenPrevFunction->setText( i18n("Previous Function") );
    actions.setDefaultShortcut( quickOpenPrevFunction, Qt::CTRL| Qt::ALT | Qt::Key_PageUp );
    connect(quickOpenPrevFunction, SIGNAL(triggered(bool)), this, SLOT(previousFunction()));

    QAction* quickOpenNavigateFunctions = actions.addAction("quick_open_outline");
    quickOpenNavigateFunctions->setText( i18n("Outline") );
    actions.setDefaultShortcut( quickOpenNavigateFunctions, Qt::CTRL| Qt::ALT | Qt::Key_N );
    connect(quickOpenNavigateFunctions, SIGNAL(triggered(bool)), this, SLOT(quickOpenNavigateFunctions()));   
}

QuickOpenPlugin::QuickOpenPlugin(QObject *parent,
                                 const QVariantList&)
    : KDevelop::IPlugin("kdevquickopen", parent)
{
    staticQuickOpenPlugin = this;
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IQuickOpen )
    m_model = new QuickOpenModel( 0 );

    KConfigGroup quickopengrp = KSharedConfig::openConfig()->group("QuickOpen");
    lastUsedScopes = quickopengrp.readEntry("SelectedScopes", QStringList() << i18n("Project") << i18n("Includes") << i18n("Includers") << i18n("Currently Open") );
    lastUsedItems = quickopengrp.readEntry("SelectedItems", QStringList() );

    {
      m_openFilesData = new OpenFilesDataProvider();
      QStringList scopes, items;
      scopes << i18n("Currently Open");
      items << i18n("Files");
      m_model->registerProvider( scopes, items, m_openFilesData );
    }
    {
      m_projectFileData = new ProjectFileDataProvider();
      QStringList scopes, items;
      scopes << i18n("Project");
      items << i18n("Files");
      m_model->registerProvider( scopes, items, m_projectFileData );
    }
    {
      m_projectItemData = new ProjectItemDataProvider(this);
      QStringList scopes, items;
      scopes << i18n("Project");
      items << ProjectItemDataProvider::supportedItemTypes();
      m_model->registerProvider( scopes, items, m_projectItemData );
    }
    {
      m_documentationItemData = new DocumentationQuickOpenProvider;
      QStringList scopes, items;
      scopes << i18n("Includes");
      items << i18n("Documentation");
      m_model->registerProvider( scopes, items, m_documentationItemData );
    }
}

QuickOpenPlugin::~QuickOpenPlugin()
{
  freeModel();

  delete m_model;
  delete m_projectFileData;
  delete m_projectItemData;
  delete m_openFilesData;
  delete m_documentationItemData;
}

void QuickOpenPlugin::unload()
{
}

ContextMenuExtension QuickOpenPlugin::contextMenuExtension(Context* context)
{
  KDevelop::ContextMenuExtension menuExt = KDevelop::IPlugin::contextMenuExtension( context );

  KDevelop::DeclarationContext *codeContext = dynamic_cast<KDevelop::DeclarationContext*>(context);

  if (!codeContext)
      return menuExt;

  DUChainReadLocker readLock;
  Declaration* decl(codeContext->declaration().data());

  if (decl) {
    const bool isDef = FunctionDefinition::definition(decl);
    if (codeContext->use().isValid() || !isDef) {
      menuExt.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, m_quickOpenDeclaration);
    }

    if(isDef) {
      menuExt.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, m_quickOpenDefinition);
    }
  }

  return menuExt;
}

void QuickOpenPlugin::showQuickOpen(const QStringList& items)
{
  if(!freeModel())
    return;

  QStringList initialItems = items;

  QStringList useScopes = lastUsedScopes;

  if (!useScopes.contains(i18n("Currently Open")))
    useScopes << i18n("Currently Open");

  showQuickOpenWidget(initialItems, useScopes, false);
}

void QuickOpenPlugin::showQuickOpen( ModelTypes modes )
{
  if(!freeModel())
    return;

  QStringList initialItems;
  if( modes & Files || modes & OpenFiles )
    initialItems << i18n("Files");

  if( modes & Functions )
    initialItems << i18n("Functions");

  if( modes & Classes )
    initialItems << i18n("Classes");
  
  QStringList useScopes;
  if ( modes != OpenFiles )
    useScopes = lastUsedScopes;
  
  if((modes & OpenFiles) && !useScopes.contains(i18n("Currently Open")))
    useScopes << i18n("Currently Open");

  bool preselectText = (!(modes & Files) || modes == QuickOpenPlugin::All);
  showQuickOpenWidget(initialItems, useScopes, preselectText);
}

void QuickOpenPlugin::showQuickOpenWidget(const QStringList& items, const QStringList& scopes, bool preselectText)
{
  QuickOpenWidgetDialog* dialog = new QuickOpenWidgetDialog( i18n("Quick Open"), m_model, items, scopes );
  m_currentWidgetHandler = dialog;
  if (preselectText)
  {
    KDevelop::IDocument *currentDoc = core()->documentController()->activeDocument();
    if (currentDoc && currentDoc->isTextDocument())
    {
      QString preselected = currentDoc->textSelection().isEmpty() ? currentDoc->textWord() : currentDoc->textDocument()->text(currentDoc->textSelection());
      dialog->widget()->setPreselectedText(preselected);
    }
  }

  connect( dialog->widget(), SIGNAL(scopesChanged(QStringList)), this, SLOT(storeScopes(QStringList)) );
  //Not connecting itemsChanged to storeItems, as showQuickOpen doesn't use lastUsedItems and so shouldn't store item changes
  //connect( dialog->widget(), SIGNAL(itemsChanged(QStringList)), this, SLOT(storeItems(QStringList)) );
  dialog->widget()->o.itemsButton->setEnabled(false);

  if(quickOpenLine()) {
    quickOpenLine()->showWithWidget(dialog->widget());
    dialog->deleteLater();
  }else{
    dialog->run();
  }
}

void QuickOpenPlugin::storeScopes( const QStringList& scopes )
{
  lastUsedScopes = scopes;
  KConfigGroup grp = KSharedConfig::openConfig()->group("QuickOpen");
  grp.writeEntry( "SelectedScopes", scopes );
}

void QuickOpenPlugin::storeItems( const QStringList& items )
{
  lastUsedItems = items;
  KConfigGroup grp = KSharedConfig::openConfig()->group("QuickOpen");
  grp.writeEntry( "SelectedItems", items );
}

void QuickOpenPlugin::quickOpen()
{
  if(quickOpenLine()) //Same as clicking on Quick Open
    quickOpenLine()->setFocus();
  else
    showQuickOpen( All );
}

void QuickOpenPlugin::quickOpenFile()
{
  showQuickOpen( (ModelTypes)(Files | OpenFiles) );
}

void QuickOpenPlugin::quickOpenFunction()
{
  showQuickOpen( Functions );
}

void QuickOpenPlugin::quickOpenClass()
{
  showQuickOpen( Classes );
}

void QuickOpenPlugin::quickOpenOpenFile()
{
  showQuickOpen( OpenFiles );
}

void QuickOpenPlugin::quickOpenDocumentation()
{
  showQuickOpenWidget(QStringList(i18n("Documentation")), QStringList(i18n("Includes")), true);
}

QSet<KDevelop::IndexedString> QuickOpenPlugin::fileSet() const {
  return m_model->fileSet();
}

void QuickOpenPlugin::registerProvider( const QStringList& scope, const QStringList& type, KDevelop::QuickOpenDataProviderBase* provider )
{
  m_model->registerProvider( scope, type, provider );
}

bool QuickOpenPlugin::removeProvider( KDevelop::QuickOpenDataProviderBase* provider )
{
  m_model->removeProvider( provider );
  return true;
}

void QuickOpenPlugin::quickOpenDeclaration()
{
  if(jumpToSpecialObject())
    return;

  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  Declaration* decl = cursorDeclaration();

  if(!decl) {
    kDebug() << "Found no declaration for cursor, cannot jump";
    return;
  }
  decl->activateSpecialization();

  IndexedString u = decl->url();
  KTextEditor::Cursor c = decl->rangeInCurrentRevision().start();

  if(u.isEmpty()) {
    kDebug() << "Got empty url for declaration" << decl->toString();
    return;
  }

  lock.unlock();
  core()->documentController()->openDocument(u.toUrl(), c);
}

///Returns all languages for that url that have a language support, and prints warnings for other ones.
QList<KDevelop::ILanguage*> languagesWithSupportForUrl(QUrl url) {
  QList<KDevelop::ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(url);
  QList<KDevelop::ILanguage*> ret;
  foreach( KDevelop::ILanguage* language, languages) {
    if(language->languageSupport()) {
      ret << language;
    }else{
      kDebug() << "got no language-support for language" << language->name();
    }
  }
  return ret;
}

QWidget* QuickOpenPlugin::specialObjectNavigationWidget() const
{
  KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
  if( !view )
    return 0;

  QUrl url = ICore::self()->documentController()->activeDocument()->url();

  foreach( KDevelop::ILanguage* language, languagesWithSupportForUrl(url) ) {
    QWidget* w = language->languageSupport()->specialLanguageObjectNavigationWidget(url, KTextEditor::Cursor(view->cursorPosition()) );
    if(w)
      return w;
  }

  return 0;
}

QPair<QUrl, KTextEditor::Cursor> QuickOpenPlugin::specialObjectJumpPosition() const
{
  KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
  if( !view )
    return qMakePair(QUrl(), KTextEditor::Cursor());

  QUrl url = ICore::self()->documentController()->activeDocument()->url();

  foreach( KDevelop::ILanguage* language, languagesWithSupportForUrl(url) ) {
    QPair<QUrl, KTextEditor::Cursor> pos = language->languageSupport()->specialLanguageObjectJumpCursor(url, KTextEditor::Cursor(view->cursorPosition()) );
    if(pos.second.isValid()) {
      return pos;
    }
  }

  return qMakePair(QUrl(), KTextEditor::Cursor::invalid());
}

bool QuickOpenPlugin::jumpToSpecialObject()
{
  QPair<QUrl, KTextEditor::Cursor> pos = specialObjectJumpPosition();
  if(pos.second.isValid()) {
    if(pos.first.isEmpty()) {
      kDebug() << "Got empty url for special language object";
      return false;
    }

    ICore::self()->documentController()->openDocument(pos.first, pos.second);
    return true;
  }
  return false;
}

void QuickOpenPlugin::quickOpenDefinition()
{
  if(jumpToSpecialObject())
    return;

  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  Declaration* decl = cursorDeclaration();

  if(!decl) {
    kDebug() << "Found no declaration for cursor, cannot jump";
    return;
  }

  IndexedString u = decl->url();
  KTextEditor::Cursor c = decl->rangeInCurrentRevision().start();
  if(FunctionDefinition* def = FunctionDefinition::definition(decl)) {
    def->activateSpecialization();
    u = def->url();
    c = def->rangeInCurrentRevision().start();
  }else{
    kDebug() << "Found no definition for declaration";
    decl->activateSpecialization();
  }

  if(u.isEmpty()) {
    kDebug() << "Got empty url for declaration" << decl->toString();
    return;
  }

  lock.unlock();
  core()->documentController()->openDocument(u.toUrl(), c);
}

bool QuickOpenPlugin::freeModel()
{
  if(m_currentWidgetHandler)
    delete m_currentWidgetHandler;
  m_currentWidgetHandler = 0;

  return true;
}

void QuickOpenPlugin::nextFunction()
{
  jumpToNearestFunction(NextFunction);
}

void QuickOpenPlugin::previousFunction()
{
  jumpToNearestFunction(PreviousFunction);
}

void QuickOpenPlugin::jumpToNearestFunction(QuickOpenPlugin::FunctionJumpDirection direction)
{
  IDocument* doc = ICore::self()->documentController()->activeDocument();
  if(!doc) {
    kDebug() << "No active document";
    return;
  }

  KDevelop::DUChainReadLocker lock( DUChain::lock() );

  TopDUContext* context = DUChainUtils::standardContextForUrl( doc->url() );

  if( !context ) {
    kDebug() << "Got no standard context";
    return;
  }

  QList<DUChainItem> items;
  OutlineFilter filter(items, OutlineFilter::Functions);
  DUChainUtils::collectItems( context, filter );

  CursorInRevision cursor = context->transformToLocalRevision(KTextEditor::Cursor(doc->cursorPosition()));
  if (!cursor.isValid())
    return;

  Declaration *nearestDeclBefore = 0;
  int distanceBefore = INT_MIN;
  Declaration *nearestDeclAfter = 0;
  int distanceAfter = INT_MAX;

  for (int i = 0; i < items.count(); ++i) {
    Declaration *decl = items[i].m_item.data();

    int distance = decl->range().start.line - cursor.line;
    if (distance < 0 && distance >= distanceBefore) {
      distanceBefore = distance;
      nearestDeclBefore = decl;
    } else if (distance > 0 && distance <= distanceAfter) {
      distanceAfter = distance;
      nearestDeclAfter = decl;
    }
  }

  CursorInRevision c = CursorInRevision::invalid();
  if (direction == QuickOpenPlugin::NextFunction && nearestDeclAfter)
    c = nearestDeclAfter->range().start;
  else if (direction == QuickOpenPlugin::PreviousFunction && nearestDeclBefore)
    c = nearestDeclBefore->range().start;

  KTextEditor::Cursor textCursor = KTextEditor::Cursor::invalid();
  if (c.isValid())
    textCursor = context->transformFromLocalRevision(c);

  lock.unlock();
  if (textCursor.isValid())
    core()->documentController()->openDocument(doc->url(), textCursor);
  else
    kDebug() << "No declaration to jump to";
}


struct CreateOutlineDialog {
  CreateOutlineDialog() : dialog(0), cursorDecl(0), model(0) {
  }
  
  void start() {
    if(!QuickOpenPlugin::self()->freeModel())
        return;

      IDocument* doc = ICore::self()->documentController()->activeDocument();
      if(!doc) {
        kDebug() << "No active document";
        return;
      }

      KDevelop::DUChainReadLocker lock( DUChain::lock() );

      TopDUContext* context = DUChainUtils::standardContextForUrl( doc->url() );

      if( !context ) {
        kDebug() << "Got no standard context";
        return;
      }

      model = new QuickOpenModel(0);

      OutlineFilter filter(items);

      DUChainUtils::collectItems( context, filter );

      if(noHtmlDestriptionInOutline) {
        for(int a = 0; a < items.size(); ++a)
          items[a].m_noHtmlDestription = true;
      }

      cursorDecl = cursorContextDeclaration();

      model->registerProvider( QStringList(), QStringList(), new DeclarationListDataProvider(QuickOpenPlugin::self(), items, true) );

      dialog = new QuickOpenWidgetDialog( i18n("Outline"), model, QStringList(), QStringList(), true );
      
      model->setParent(dialog->widget());    
  }
  void finish() {
    //Select the declaration that contains the cursor
    if(cursorDecl && dialog) {
      int num = 0;
      foreach(const DUChainItem& item, items) {
        if(item.m_item.data() == cursorDecl) {
          dialog->widget()->o.list->setCurrentIndex( model->index(num,0,QModelIndex()) );
          dialog->widget()->o.list->scrollTo( model->index(num,0,QModelIndex()), QAbstractItemView::PositionAtCenter );
        }
        ++num;
      }
    }
  }
    QPointer<QuickOpenWidgetDialog> dialog;
    Declaration* cursorDecl;
    QList<DUChainItem> items;
    QuickOpenModel* model;
};

class OutlineQuickopenWidgetCreator : public QuickOpenWidgetCreator {
  public:
    OutlineQuickopenWidgetCreator(QStringList /*scopes*/, QStringList /*items*/) : m_creator(0) {
    }
    
    ~OutlineQuickopenWidgetCreator() {
      delete m_creator;
    }
    
    virtual QuickOpenWidget* createWidget() {
      delete m_creator;
      m_creator = new CreateOutlineDialog;
      m_creator->start();
      
      if(!m_creator->dialog)
        return 0;
      
      m_creator->dialog->deleteLater();
      return m_creator->dialog->widget();
    }
    
    virtual void widgetShown() {
      if(m_creator) {
        m_creator->finish();
        delete m_creator;
        m_creator = 0;
      }
    }
    
    virtual QString objectNameForLine() {
      return "Outline";
    }
    
    CreateOutlineDialog* m_creator;
};

void QuickOpenPlugin::quickOpenNavigateFunctions()
{
  CreateOutlineDialog create;
  create.start();
  
  if(!create.dialog)
    return;
  
  m_currentWidgetHandler = create.dialog;
  
  QuickOpenLineEdit* line = quickOpenLine("Outline");
  if(!line)
    line  = quickOpenLine();
  
  if(line) {
    line->showWithWidget(create.dialog->widget());
    create.dialog->deleteLater();
  }else
    create.dialog->run();
  
  create.finish();
}

QuickOpenLineEdit::QuickOpenLineEdit(QuickOpenWidgetCreator* creator) : m_widget(0), m_forceUpdate(false), m_widgetCreator(creator) {
    setMinimumWidth(200);
    setMaximumWidth(400);

    deactivate();
    setDefaultText(i18n("Quick Open..."));
    setToolTip(i18n("Search for files, classes, functions and more,"
                    " allowing you to quickly navigate in your source code."));
    setObjectName(m_widgetCreator->objectNameForLine());
    setFocusPolicy(Qt::ClickFocus);
}

QuickOpenLineEdit::~QuickOpenLineEdit() {
    delete m_widget;
    delete m_widgetCreator;
}

bool QuickOpenLineEdit::insideThis(QObject* object) {
    while (object)
    {
        kDebug() << object;
        if (object == this || object == m_widget)
        {
            return true;
        }
        object = object->parent();
    }
    return false;
}

void QuickOpenLineEdit::widgetDestroyed(QObject* obj)
{
  Q_UNUSED(obj);
  deactivate();
}

void QuickOpenLineEdit::showWithWidget(QuickOpenWidget* widget)
{
  connect(widget, SIGNAL(destroyed(QObject*)), SLOT(widgetDestroyed(QObject*)));
  kDebug() << "storing widget" << widget;
  deactivate();
  if(m_widget) {
    kDebug() << "deleting" << m_widget;
    delete m_widget;
  }
  m_widget = widget;
  m_forceUpdate = true;
  setFocus();
}

void QuickOpenLineEdit::focusInEvent(QFocusEvent* ev) {
    QLineEdit::focusInEvent(ev);
//       delete m_widget;
    kDebug() << "got focus";
    kDebug() << "old widget" << m_widget << "force update:" << m_forceUpdate;
    if (m_widget && !m_forceUpdate)
        return;

    if (!m_forceUpdate && !QuickOpenPlugin::self()->freeModel()) {
        deactivate();
        return;
    }
    
    m_forceUpdate = false;
    
    if(!m_widget)
    {
      m_widget = m_widgetCreator->createWidget();
      if(!m_widget) {
        deactivate();
        return;
      }
    }

    activate();

    m_widget->showStandardButtons(false);
    m_widget->showSearchField(false);
    
    m_widget->setParent(0, Qt::ToolTip);
    m_widget->setFocusPolicy(Qt::NoFocus);
    m_widget->setAlternativeSearchField(this);
    
    QuickOpenPlugin::self()->m_currentWidgetHandler = m_widget;
    connect(m_widget, SIGNAL(ready()), SLOT(deactivate()));

    connect( m_widget, SIGNAL(scopesChanged(QStringList)), QuickOpenPlugin::self(), SLOT(storeScopes(QStringList)) );
    connect( m_widget, SIGNAL(itemsChanged(QStringList)), QuickOpenPlugin::self(), SLOT(storeItems(QStringList)) );
    Q_ASSERT(m_widget->o.searchLine == this);
    m_widget->prepareShow();
    QRect widgetGeometry = QRect(mapToGlobal(QPoint(0, height())), mapToGlobal(QPoint(width(), height() + 400)));
    widgetGeometry.setWidth(700); ///@todo Waste less space
    QRect screenGeom = QApplication::desktop()->screenGeometry(this);
    if (widgetGeometry.right() > screenGeom.right()) {
      widgetGeometry.moveRight(screenGeom.right());
    }
    if (widgetGeometry.bottom() > screenGeom.bottom()) {
      widgetGeometry.moveBottom(mapToGlobal(QPoint(0, 0)).y());
    }
    m_widget->setGeometry(widgetGeometry);
    m_widget->show();
    
    m_widgetCreator->widgetShown();
}

void QuickOpenLineEdit::hideEvent(QHideEvent* ev)
{
    QWidget::hideEvent(ev);
    if(m_widget)
      QMetaObject::invokeMethod(this, "checkFocus", Qt::QueuedConnection);
//       deactivate();
}

bool QuickOpenLineEdit::eventFilter(QObject* obj, QEvent* e) {
    if (!m_widget)
        return false;

    switch (e->type()) {
    case QEvent::KeyPress:
    case QEvent::ShortcutOverride:
      if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Escape) {
          deactivate();
          e->accept();
          return true;
      }
      break;
     case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        kDebug() << "closing because of window activation";
        deactivate();
        break;
    // handle bug 260657 - "Outline menu doesn't follow main window on its move"
    case QEvent::Move: {
          if (QWidget* widget = qobject_cast<QWidget*>(obj)) {
            // close the outline menu in case a parent widget moved
            if (widget->isAncestorOf(this)) {
              kDebug() << "closing because of parent widget move";
              deactivate();
            }
            break;
          }
        }
    case QEvent::FocusIn:
        if (dynamic_cast<QWidget*>(obj)) {
            QFocusEvent* focusEvent = dynamic_cast<QFocusEvent*>(e);
            Q_ASSERT(focusEvent);
            //Eat the focus event, keep the focus
            kDebug() << "focus change" << "inside this: " << insideThis(obj) << "this" << this << "obj" << obj;
            if(obj == this)
              return false;
            
            kDebug() << "reason" << focusEvent->reason();
            if (focusEvent->reason() != Qt::MouseFocusReason && focusEvent->reason() != Qt::ActiveWindowFocusReason) {
                QMetaObject::invokeMethod(this, "checkFocus", Qt::QueuedConnection);
                return false;
            }
            if (!insideThis(obj))
                deactivate();
        }
        break;
    default:
        break;
    }
    return false;
}
void QuickOpenLineEdit::activate() {
    kDebug() << "activating";
    setText("");
    setStyleSheet("");
    qApp->installEventFilter(this);
}
void QuickOpenLineEdit::deactivate() {
    kDebug() << "deactivating";
    
    clear();

    if(m_widget || hasFocus())
      QMetaObject::invokeMethod(this, "checkFocus", Qt::QueuedConnection);
    
   if (m_widget)
        m_widget->deleteLater();
    
    m_widget = 0;
    qApp->removeEventFilter(this);
    
}

void QuickOpenLineEdit::checkFocus()
{
    kDebug() << "checking focus" << m_widget;
    if(m_widget) {
      if(isVisible() && !isHidden())
        setFocus();
      else
        deactivate();
    }else{
       if (ICore::self()->documentController()->activeDocument())
           ICore::self()->documentController()->activateDocument(ICore::self()->documentController()->activeDocument());
       
       //Make sure the focus is somewehre else, even if there is no active document
       setEnabled(false);
       setEnabled(true);
    }
}

IQuickOpenLine* QuickOpenPlugin::createQuickOpenLine(const QStringList& scopes, const QStringList& type, IQuickOpen::QuickOpenType kind)
{
  if(kind == Outline)
    return new QuickOpenLineEdit(new OutlineQuickopenWidgetCreator(scopes, type));
  else
    return new QuickOpenLineEdit(new StandardQuickOpenWidgetCreator(scopes, type));
}

#include "quickopenplugin.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
