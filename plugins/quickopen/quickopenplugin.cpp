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

#include <typeinfo>
#include <QtGui/QTreeView>
#include <QtGui/QHeaderView>
#include <QDialog>
#include <QKeyEvent>
#include <QApplication>
#include <QCheckBox>
#include <QMetaObject>

#include <kbuttongroup.h>
#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <kparts/mainwindow.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kshortcut.h>
#include <kdebug.h>

#include <ilanguage.h>
#include <icore.h>
#include <iuicontroller.h>
#include <idocumentcontroller.h>
#include <ilanguagecontroller.h>
#include <ilanguagesupport.h>
#include <hashedstring.h>
#include <duchain/duchainutils.h>
#include <duchain/duchainlock.h>
#include <duchain/duchain.h>
#include <duchain/identifiedtype.h>
#include <duchain/typesystem.h>

#include "expandingtree/expandingdelegate.h"
#include "ui_quickopen.h"
#include "quickopenmodel.h"
#include "projectfilequickopen.h"
#include "projectitemquickopen.h"
#include "declarationlistquickopen.h"
#include "customlistquickopen.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevQuickOpenFactory, registerPlugin<QuickOpenPlugin>(); )
K_EXPORT_PLUGIN(KDevQuickOpenFactory("kdevquickopen"))

Declaration* cursorDeclaration() {
  IDocument* doc = ICore::self()->documentController()->activeDocument();
  if(!doc)
    return 0;

  KTextEditor::Document* textDoc = doc->textDocument();
  if(!textDoc)
    return 0;

  KTextEditor::View* view = textDoc->activeView();
  if(!view)
    return 0;

  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  
  return DUChainUtils::declarationForDefinition( DUChainUtils::itemUnderCursor( doc->url(), SimpleCursor(view->cursorPosition()) ) );
}

///The first declaration(or definition's declaration) that belongs to a context that surrounds the current cursor
Declaration* cursorContextDeclaration() {
  IDocument* doc = ICore::self()->documentController()->activeDocument();
  if(!doc)
    return 0;

  KTextEditor::Document* textDoc = doc->textDocument();
  if(!textDoc)
    return 0;

  KTextEditor::View* view = textDoc->activeView();
  if(!view)
    return 0;

  KDevelop::DUChainReadLocker lock( DUChain::lock() );

  TopDUContext* ctx = DUChainUtils::standardContextForUrl(doc->url());
  if(!ctx)
    return 0;
  
  SimpleCursor cursor(view->cursorPosition());

  DUContext* subCtx = ctx->findContext(cursor);

  while(subCtx && !subCtx->owner())
    subCtx = subCtx->parentContext();

  if(!subCtx || !subCtx->owner())
    return 0;

  return DUChainUtils::declarationForDefinition(subCtx->owner());
}

QString cursorItemText() {
  KDevelop::DUChainReadLocker lock( DUChain::lock() );

  Declaration* decl = cursorDeclaration();
  if(!decl)
    return QString();
  
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(decl->abstractType().data());
  if( idType && idType->declaration() )
    decl = idType->declaration();

  return decl->qualifiedIdentifier().toString();
}

QuickOpenWidgetHandler::QuickOpenWidgetHandler( QDialog* d, QuickOpenModel* model, const QStringList& initialItems, const QStringList& initialScopes, bool listOnly, bool noSearchField ) : QObject( d ), m_dialog(d), m_model(model) {

  o.setupUi( d );
  o.list->header()->hide();
  o.list->setRootIsDecorated( false );
  o.list->setVerticalScrollMode( QAbstractItemView::ScrollPerItem );

  o.list->setItemDelegate( new ExpandingDelegate( m_model, o.list ) );

  if(!listOnly) {
    QStringList allTypes = m_model->allTypes();
    QStringList allScopes = m_model->allScopes();

    QVBoxLayout *itemsLayout = new QVBoxLayout;
    
    foreach( QString type, allTypes )
    {
      QCheckBox* check = new QCheckBox( type );
      itemsLayout->addWidget( check );

      if( initialItems.isEmpty() || initialItems.contains( type ) )
        check->setCheckState( Qt::Checked );
      else
        check->setCheckState( Qt::Unchecked );
    
      connect( check, SIGNAL(stateChanged(int)), this, SLOT(updateProviders()) );
    }

    itemsLayout->addStretch( 1 );
    o.itemsGroup->setLayout( itemsLayout );
      
    QVBoxLayout *scopesLayout = new QVBoxLayout;
    
    foreach( QString scope, allScopes )
    {
      QCheckBox* check = new QCheckBox( scope );
      scopesLayout->addWidget( check );
      
      if( initialScopes.isEmpty() || initialScopes.contains( scope ) )
        check->setCheckState( Qt::Checked );
      else
        check->setCheckState( Qt::Unchecked );
    
      connect( check, SIGNAL(stateChanged(int)), this, SLOT(updateProviders()) );
    }

    scopesLayout->addStretch( 1 );
    o.scopeGroup->setLayout( scopesLayout );
  }else{
    o.list->setFocusPolicy(Qt::StrongFocus);
    o.scopeGroup->hide();
    o.itemsGroup->hide();
  }
  
  if( noSearchField ) {
    o.searchLine->hide();
    o.searchLabel->hide();
  }
  o.searchLine->installEventFilter( this );
  o.list->installEventFilter( this );
  o.buttonBox->installEventFilter( this );

  connect( o.searchLine, SIGNAL(textChanged( const QString& )), this, SLOT(textChanged( const QString& )) );
  connect( d, SIGNAL(accepted()), this, SLOT(accept()) );

  connect( o.list, SIGNAL(doubleClicked( const QModelIndex& )), this, SLOT(doubleClicked( const QModelIndex& )) );
  
  updateProviders();
  
  m_model->restart();
  m_model->setTreeView( o.list );

  o.list->setModel( m_model );

  o.list->setColumnWidth( 0, 20 );

  if(!listOnly)
    o.searchLine->setText(cursorItemText());
  d->show();

  connect( o.list->selectionModel(), SIGNAL(currentRowChanged( const QModelIndex&, const QModelIndex& )), this, SLOT(currentChanged( const QModelIndex&, const QModelIndex& )) );
  connect( o.list->selectionModel(), SIGNAL(selectionChanged( const QModelIndex&, const QModelIndex& )), this, SLOT(currentChanged( const QModelIndex&, const QModelIndex& )) );
}

QuickOpenWidgetHandler::~QuickOpenWidgetHandler() {
  if( m_model->treeView() == o.list )
    m_model->setTreeView( 0 );
}

void QuickOpenWidgetHandler::updateProviders() {
  QStringList checkedItems;
  QStringList checkedScopes;
  
  foreach( QObject* obj, o.itemsGroup->children() ) {
    QCheckBox* box = qobject_cast<QCheckBox*>( obj );
    if( box ) {
      if( box->checkState() == Qt::Checked )
        checkedItems << box->text().remove('&');
    }
  }
  
  foreach( QObject* obj, o.scopeGroup->children() ) {
    QCheckBox* box = qobject_cast<QCheckBox*>( obj );
    if( box ) {
      if( box->checkState() == Qt::Checked )
        checkedScopes << box->text().remove('&');
    }
  }

  m_model->enableProviders( checkedItems, checkedScopes );
}


void QuickOpenWidgetHandler::textChanged( const QString& str ) {
  m_model->textChanged( str );
  
  QModelIndex currentIndex = m_model->index(0, 0, QModelIndex());
  o.list->selectionModel()->setCurrentIndex( currentIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows | QItemSelectionModel::Current );
  
  callRowSelected();
}

void QuickOpenWidgetHandler::callRowSelected() {
  QModelIndex currentIndex = o.list->selectionModel()->currentIndex();
  if( currentIndex.isValid() )
    m_model->rowSelected( currentIndex );
  else
    kDebug() << "current index is not valid";
}

void QuickOpenWidgetHandler::currentChanged( const QModelIndex& /*current*/, const QModelIndex& /*previous */) {
  callRowSelected();
}

void QuickOpenWidgetHandler::accept() {
  QString filterText = o.searchLine->text();
  m_model->execute( o.list->currentIndex(), filterText );
}

void QuickOpenWidgetHandler::doubleClicked ( const QModelIndex & index ) {
  QString filterText = o.searchLine->text();
  if(  m_model->execute( index, filterText ) )
    m_dialog->close();
  else if( filterText != o.searchLine->text() )
    o.searchLine->setText( filterText );
}


bool QuickOpenWidgetHandler::eventFilter ( QObject * watched, QEvent * event )
{
  if( event->type() == QEvent::KeyPress  ) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

    switch( keyEvent->key() ) {
      case Qt::Key_Down:
      case Qt::Key_Up:
      {
        if( keyEvent->modifiers() == Qt::ShiftModifier ) {
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
      case Qt::Key_End:
      case Qt::Key_Home:
        if(watched == o.list )
          return false;
        QApplication::sendEvent( o.list, event );
      //callRowSelected();
        return true;
      case Qt::Key_Left: {
        //Expand/unexpand
        if( keyEvent->modifiers() == Qt::ShiftModifier ) {
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
        if( keyEvent->modifiers() == Qt::ShiftModifier ) {
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
        if( keyEvent->modifiers() == Qt::ShiftModifier ) {
          //Eventually Send action to the widget
          QWidget* w = m_model->expandingWidget(o.list->selectionModel()->currentIndex());
          if( KDevelop::QuickOpenEmbeddedWidgetInterface* interface =
              dynamic_cast<KDevelop::QuickOpenEmbeddedWidgetInterface*>( w ) ){
            interface->accept();
            return true;
          }
        } else {
          QString filterText = o.searchLine->text();
          if( m_model->execute( o.list->currentIndex(), filterText ) ) {
            m_dialog->close();
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

QuickOpenPlugin::QuickOpenPlugin(QObject *parent,
                                 const QVariantList&)
    : KDevelop::IPlugin(KDevQuickOpenFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IQuickOpen )
    setXMLFile( "kdevquickopen.rc" );
    m_model = new QuickOpenModel( 0 );

    KActionCollection* actions = actionCollection();

    QAction* quickOpen = actions->addAction("quick_open");
    quickOpen->setText( i18n("&Quick Open") );
    quickOpen->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_Q );
    connect(quickOpen, SIGNAL(triggered(bool)), this, SLOT(quickOpen()));
    
    QAction* quickOpenFile = actions->addAction("quick_open_file");
    quickOpenFile->setText( i18n("Quick Open &File") );
    quickOpenFile->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_O );
    connect(quickOpenFile, SIGNAL(triggered(bool)), this, SLOT(quickOpenFile()));

    QAction* quickOpenClass = actions->addAction("quick_open_class");
    quickOpenClass->setText( i18n("Quick Open &Class") );
    quickOpenClass->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_C );
    connect(quickOpenClass, SIGNAL(triggered(bool)), this, SLOT(quickOpenClass()));

    QAction* quickOpenFunction = actions->addAction("quick_open_function");
    quickOpenFunction->setText( i18n("Quick Open &Function") );
    quickOpenFunction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_M );
    connect(quickOpenFunction, SIGNAL(triggered(bool)), this, SLOT(quickOpenFunction()));

    QAction* quickOpenDeclaration = actions->addAction("quick_open_jump_declaration");
    quickOpenDeclaration->setText( i18n("Jump to Declaration") );
    quickOpenDeclaration->setShortcut( Qt::CTRL | Qt::Key_Period );
    connect(quickOpenDeclaration, SIGNAL(triggered(bool)), this, SLOT(quickOpenDeclaration()));
  
    QAction* quickOpenDefinition = actions->addAction("quick_open_jump_definition");
    quickOpenDefinition->setText( i18n("Jump to Definition") );
    quickOpenDefinition->setShortcut( Qt::CTRL | Qt::Key_Comma );
    connect(quickOpenDefinition, SIGNAL(triggered(bool)), this, SLOT(quickOpenDefinition()));
  
    QAction* quickOpenNavigate = actions->addAction("quick_open_navigate");
    quickOpenNavigate->setText( i18n("Navigate Declaration") );
    quickOpenNavigate->setShortcut( Qt::ALT | Qt::Key_Space );
    connect(quickOpenNavigate, SIGNAL(triggered(bool)), this, SLOT(quickOpenNavigate()));
  
    QAction* quickOpenNavigateFunctions = actions->addAction("quick_open_navigate_function_definitions");
    quickOpenNavigateFunctions->setText( i18n("Navigate Function Definitions") );
    quickOpenNavigateFunctions->setShortcut( Qt::CTRL| Qt::ALT | Qt::Key_N );
    connect(quickOpenNavigateFunctions, SIGNAL(triggered(bool)), this, SLOT(quickOpenNavigateFunctions()));
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
}

QuickOpenPlugin::~QuickOpenPlugin()
{
  delete m_model;
  delete m_projectFileData;
  delete m_projectItemData;
}

void QuickOpenPlugin::unload()
{
}

void QuickOpenPlugin::showQuickOpen( ModelTypes modes )
{
  QDialog* d = new QDialog( core()->uiController()->activeMainWindow() );

  d->setAttribute( Qt::WA_DeleteOnClose, true );

  QStringList initialItems;
  if( modes & Files )
    initialItems << i18n("Files");

  if( modes & Functions )
    initialItems << i18n("Functions");

  if( modes & Classes )
    initialItems << i18n("Classes");
  
  new QuickOpenWidgetHandler( d, m_model, initialItems, QStringList() );
}


void QuickOpenPlugin::quickOpen()
{
  if(!modelIsFree())
    return;
  showQuickOpen( All );
}

void QuickOpenPlugin::quickOpenFile()
{
  if(!modelIsFree())
    return;
  showQuickOpen( Files );
}

void QuickOpenPlugin::quickOpenFunction()
{
  if(!modelIsFree())
    return;
  showQuickOpen( Functions );
}

void QuickOpenPlugin::quickOpenClass()
{
  if(!modelIsFree())
    return;
  showQuickOpen( Classes );
}

QSet<KDevelop::HashedString> QuickOpenPlugin::fileSet() const {
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
  if(!modelIsFree())
    return;

  if(jumpToSpecialObject())
    return;
  
  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  Declaration* decl = cursorDeclaration();

  if(!decl) {
    kDebug() << "Found no declaration for cursor, cannot jump";
    return;
  }

  HashedString u = decl->url();
  SimpleCursor c = decl->range().start;
  
  lock.unlock();
  core()->documentController()->openDocument(KUrl(u.str()), c.textCursor());
}

QWidget* QuickOpenPlugin::specialObjectNavigationWidget() const
{
  if( !ICore::self()->documentController()->activeDocument() || !ICore::self()->documentController()->activeDocument()->textDocument() || !ICore::self()->documentController()->activeDocument()->textDocument()->activeView() )
    return false;

  KUrl url = ICore::self()->documentController()->activeDocument()->url();
  
  QList<KDevelop::ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(url);

  foreach( KDevelop::ILanguage* language, languages) {
    QWidget* w = language->languageSupport()->specialLanguageObjectNavigationWidget(url, SimpleCursor(ICore::self()->documentController()->activeDocument()->textDocument()->activeView()->cursorPosition()) );
    if(w)
      return w;
  }
  return 0;
}

QPair<KUrl, SimpleCursor> QuickOpenPlugin::specialObjectJumpPosition() const {
  if( !ICore::self()->documentController()->activeDocument() || !ICore::self()->documentController()->activeDocument()->textDocument() || !ICore::self()->documentController()->activeDocument()->textDocument()->activeView() )
    return qMakePair(KUrl(), SimpleCursor());

  KUrl url = ICore::self()->documentController()->activeDocument()->url();
  
  QList<KDevelop::ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(url);

  foreach( KDevelop::ILanguage* language, languages) {
    QPair<KUrl, SimpleCursor> pos = language->languageSupport()->specialLanguageObjectJumpCursor(url, SimpleCursor(ICore::self()->documentController()->activeDocument()->textDocument()->activeView()->cursorPosition()) );
    if(pos.second.isValid()) {
      return pos;
    }
  }
  
  return qMakePair(KUrl(), SimpleCursor::invalid());
}

bool QuickOpenPlugin::jumpToSpecialObject()
{
  QPair<KUrl, SimpleCursor> pos = specialObjectJumpPosition();
  if(pos.second.isValid()) {
    ICore::self()->documentController()->openDocument(pos.first, pos.second.textCursor());
    return true;
  }
  return false;
}

void QuickOpenPlugin::quickOpenDefinition()
{
  if(!modelIsFree())
    return;

  if(jumpToSpecialObject())
    return;
  
  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  Declaration* decl = cursorDeclaration();

  if(!decl) {
    kDebug() << "Found no declaration for cursor, cannot jump";
    return;
  }

  HashedString u = decl->url();
  SimpleCursor c = decl->range().start;
  if(decl->definition()) {
    u = decl->definition()->url();
    c = decl->definition()->range().start;
  }else{
    kDebug() << "Found no definition for declaration";
  }

  lock.unlock();
  core()->documentController()->openDocument(KUrl(u.str()), c.textCursor());
}

void QuickOpenPlugin::quickOpenNavigate()
{
  if(!modelIsFree())
    return;
  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  
  QWidget* widget = specialObjectNavigationWidget();
  Declaration* decl = cursorDeclaration();

  if(widget || decl) {
  
    QDialog* d = new QDialog( core()->uiController()->activeMainWindow() );

    d->setAttribute( Qt::WA_DeleteOnClose, true );

    QuickOpenModel* model = new QuickOpenModel( d );
    model->setExpandingWidgetHeightIncrease(200); //Make the widget higher, since it's the only visible item

    if(widget) {
      QPair<KUrl, SimpleCursor> jumpPos = specialObjectJumpPosition();
      
      CustomItem item;
      item.m_widget = widget;
      item.m_executeTargetPosition = jumpPos.second;
      item.m_executeTargetUrl = jumpPos.first;

      QList<CustomItem> items;
      items << item;
    
      model->registerProvider( QStringList(), QStringList(), new CustomItemDataProvider(items) );
    }else{
      DUChainItem item;
      
      item.m_item = DeclarationPointer(decl);
      item.m_text = decl->qualifiedIdentifier().toString();

      QList<DUChainItem> items;
      items << item;

      model->registerProvider( QStringList(), QStringList(), new DeclarationListDataProvider(this, items) );
    }

    //Change the parent so there are no conflicts in destruction order
    model->setParent(new QuickOpenWidgetHandler( d, model, QStringList(), QStringList(), true, true ));

    model->setExpanded(model->index(0,0, QModelIndex()), true);
  }
  
  if(!decl) {
    kDebug() << "Found no declaration for cursor, cannot navigate";
    return;
  }
}

class DUChainItemFilter {
public:
  //Both should return whether processing should be continued
  virtual bool accept(Declaration* decl) = 0;
  virtual bool accept(DUContext* ctx) = 0;
  virtual ~DUChainItemFilter() {
  }
};

void collectItems( QList<DUChainItem>& items, DUContext* context, DUChainItemFilter& filter ) {

  QVector<DUContext*> children = context->childContexts();
  QVector<Declaration*> localDeclarations = context->localDeclarations();

  QVector<DUContext*>::const_iterator childIt = children.begin();
  QVector<Declaration*>::const_iterator declIt = localDeclarations.begin();

  while(childIt != children.end() || declIt != localDeclarations.end()) {

    DUContext* child = 0;
    if(childIt != children.end())
      child = *childIt;

    Declaration* decl = 0;
    if(declIt != localDeclarations.end())
      decl = *declIt;

    if(decl) {
      if(child && child->range().start >= decl->range().start)
        child = 0;
    }
    
    if(child) {
      if(decl && decl->range().start >= child->range().start)
        decl = 0;
    }
    
    if(child) {
      if( filter.accept(child) )
        collectItems(items, child, filter);
      ++childIt;
      continue;
    }

    if(decl) {
      if( filter.accept(decl) ) {
        if(decl->isDefinition() && decl->declaration())
          decl = decl->declaration();
        
        DUChainItem item;
        item.m_item = DeclarationPointer(decl);
        item.m_text = decl->qualifiedIdentifier().toString();
        items << item;
        
        FunctionType* functionType = dynamic_cast<FunctionType*>(decl->abstractType().data());

        if(functionType) {
          item.m_text += functionType->partToString(FunctionType::SignatureArguments);
        }
      }

      ++declIt;
      continue;
    }
  }
}

bool QuickOpenPlugin::modelIsFree() const
{
  if(m_model->treeView()) {
    //We cannot allow more than 1 quickopen widget at a time, because the model is coupled to the view.
    kDebug() << "Only one quickopen-widget at a time is allowed";
    return false;
  }else{
    return true;
  }
}

void QuickOpenPlugin::quickOpenNavigateFunctions()
{
  if(!modelIsFree())
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
  
  QDialog* d = new QDialog( core()->uiController()->activeMainWindow() );

  d->setAttribute( Qt::WA_DeleteOnClose, true );

  QuickOpenModel* model = new QuickOpenModel( d );

  QList<DUChainItem> items;

  class FunctionFilter : public DUChainItemFilter {
  public:
    virtual bool accept(Declaration* decl) {
      if(dynamic_cast<AbstractFunctionDeclaration*>(decl))
        return true;
      else
        return false;
    }
    virtual bool accept(DUContext* ctx) {
      if(ctx->type() == DUContext::Class || ctx->type() == DUContext::Namespace || ctx->type() == DUContext::Global )
        return true;
      else
        return false;
    }
  } filter;
  
  collectItems( items, context, filter );

  Declaration* cursorDecl = cursorContextDeclaration();
  if(!cursorDecl)
    cursorDecl = cursorDeclaration();
  
  model->registerProvider( QStringList(), QStringList(), new DeclarationListDataProvider(this, items, true) );

  //Change the parent so there are no conflicts in destruction order
  QuickOpenWidgetHandler* handler = new QuickOpenWidgetHandler( d, model, QStringList(), QStringList(), true );
  model->setParent(handler);

  //Select the declaration that contains the cursor
  if(cursorDecl) {
    int num = 0;
    foreach(const DUChainItem& item, items) {
      if(item.m_item.data() == cursorDecl)
        handler->o.list->setCurrentIndex( model->index(num,0,QModelIndex()) );
      ++num;
    }
  }
}

#include "quickopenplugin.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
