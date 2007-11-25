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
#include <kparts/mainwindow.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kshortcut.h>
#include <kdebug.h>

#include <icore.h>
#include <iuicontroller.h>

#include "expandingtree/expandingdelegate.h"
#include "ui_quickopen.h"
#include "quickopenmodel.h"
#include "project_file_quickopen.h"

K_PLUGIN_FACTORY(KDevQuickOpenFactory, registerPlugin<QuickOpenPlugin>(); )
K_EXPORT_PLUGIN(KDevQuickOpenFactory("kdevquickopen"))

QuickOpenWidgetHandler::QuickOpenWidgetHandler( QDialog* d, QuickOpenModel* model, const QStringList& initialItems, const QStringList& initialScopes ) : QObject( d ), m_dialog(d), m_model(model) {

  o.setupUi( d );
  o.list->header()->hide();
  o.list->setRootIsDecorated( false );
  o.list->setItemDelegate( new ExpandingDelegate( m_model, o.list ) );

  QStringList allTypes = m_model->allTypes();
  QStringList allScopes = m_model->allScopes();

  QVBoxLayout *itemsLayout = new QVBoxLayout;
  
  foreach( QString type, allTypes )
  {
    QCheckBox* check = new QCheckBox( type );
    itemsLayout->addWidget( check );

    if( initialItems.isEmpty() || initialItems.contains( type ) )
      check->setCheckState( Qt::Checked );
  
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
  
    connect( check, SIGNAL(stateChanged(int)), this, SLOT(updateProviders()) );
  }

  scopesLayout->addStretch( 1 );
  o.scopeGroup->setLayout( scopesLayout );
  
  o.searchLine->installEventFilter( this );
  o.list->installEventFilter( this );

  connect( o.searchLine, SIGNAL(textChanged( const QString& )), this, SLOT(textChanged( const QString& )) );
  connect( d, SIGNAL(accepted()), this, SLOT(accept()) );

  connect( o.list, SIGNAL(doubleClicked( const QModelIndex& )), this, SLOT(doubleClicked( const QModelIndex& )) );
  
  updateProviders();
  
  m_model->restart();
  m_model->setTreeView( o.list );

  o.list->setModel( m_model );

  o.list->setColumnWidth( 0, 20 );
  
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
          } else {
            if( w ) {
              QMetaObject::invokeMethod( w, "previous" );
              kDebug() << "dynamic-cast fail" << typeid(*w).name();
              return true;
            }
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
          kDebug() << "dynamic-cast success";
            interface->next();
            return true;
          }else{
            if( w ) {
              QMetaObject::invokeMethod( w, "next" );
              kDebug() << "dynamic-cast fail" << typeid(*w).name();
              return true;
            }
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
          } else {
            ///@todo this is a bug in gcc or whatever, somehow it cannot dynamic_cast to the object. For now, workaround it using invokeMethod
            if( w ) {
              QMetaObject::invokeMethod( w, "accept" );
              kDebug() << "dynamic-cast fail" << typeid(*w).name();
              return true;
            }
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
    ///@todo Make the whole thing work using the own action-collection, it simply doesn't
    m_model = new QuickOpenModel( 0 );

    KActionCollection* actions = actionCollection();

    ///@todo All the locally created actions do not work,, also not if they are put into actionCollection()
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

    m_projectFileData = new ProjectFileDataProvider(core());

    {
      QStringList scopes;
      scopes << i18n("Project");
      m_model->registerProvider( scopes, i18n("Files"), m_projectFileData );
    }
}

QuickOpenPlugin::~QuickOpenPlugin()
{
  delete m_model;
  delete m_projectFileData;
}

void QuickOpenPlugin::unload()
{
}

void QuickOpenPlugin::showQuickOpen( ModelTypes modes )
{
  kDebug() <<  "showing quickopen";
  QDialog* d = new QDialog( core()->uiController()->activeMainWindow() );

  d->setAttribute( Qt::WA_DeleteOnClose, true );

  QStringList initialItems;
  if( modes & All ) {
  } else if( modes & Files ) {
    initialItems << i18n("Files");
  } else if( modes & Functions ) {
    initialItems << i18n("Functions");
  } else if( modes & Classes ) {
    initialItems << i18n("Classes");
  }
  
  new QuickOpenWidgetHandler( d, m_model, initialItems, QStringList() );
}


void QuickOpenPlugin::quickOpen()
{
  showQuickOpen( All );
}

void QuickOpenPlugin::quickOpenFile()
{
  showQuickOpen( Files );
}

void QuickOpenPlugin::quickOpenFunction()
{
  showQuickOpen( Functions );
}

void QuickOpenPlugin::quickOpenClass()
{
  showQuickOpen( Classes );
}

void QuickOpenPlugin::registerProvider( const QStringList& scope, const QString& type, KDevelop::QuickOpenDataProviderBase* provider )
{
  m_model->registerProvider( scope, type, provider );
}

bool QuickOpenPlugin::removeProvider( KDevelop::QuickOpenDataProviderBase* provider )
{
  m_model->removeProvider( provider );
  return true;
}


#include "quickopenplugin.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
