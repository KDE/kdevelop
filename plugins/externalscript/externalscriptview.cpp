/*
    This plugin is part of KDevelop.

    Copyright (C) 2010 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "externalscriptview.h"

#include "externalscriptplugin.h"
#include "externalscriptitem.h"
#include "editexternalscript.h"

#include <KLocalizedString>

#include <QAction>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMouseEvent>
#include <KMessageBox>
#include <QMenu>

ExternalScriptView::ExternalScriptView( ExternalScriptPlugin* plugin, QWidget* parent )
    : QWidget( parent ), m_plugin( plugin )
{
  Ui::ExternalScriptViewBase::setupUi( this );

  setWindowTitle( i18n( "External Scripts" ) );

  m_model = new QSortFilterProxyModel( this );
  m_model->setSourceModel( m_plugin->model() );
  m_model->setDynamicSortFilter( true );
  m_model->sort( 0 );
  connect( filterText, &QLineEdit::textEdited,
           m_model, &QSortFilterProxyModel::setFilterWildcard );

  scriptTree->setModel( m_model );
  scriptTree->setContextMenuPolicy( Qt::CustomContextMenu );
  scriptTree->viewport()->installEventFilter( this );
  scriptTree->header()->hide();
  connect(scriptTree, &QTreeView::customContextMenuRequested,
          this, &ExternalScriptView::contextMenu);

  m_addScriptAction = new QAction(QIcon::fromTheme("document-new"), i18n("Add External Script"), this);
  connect(m_addScriptAction, &QAction::triggered, this, &ExternalScriptView::addScript);
  addAction(m_addScriptAction);
  m_editScriptAction = new QAction(QIcon::fromTheme("document-edit"), i18n("Edit External Script"), this);
  connect(m_editScriptAction, &QAction::triggered, this, &ExternalScriptView::editScript);
  addAction(m_editScriptAction);
  m_removeScriptAction = new QAction(QIcon::fromTheme("document-close"), i18n("Remove External Script"), this);
  connect(m_removeScriptAction, &QAction::triggered, this, &ExternalScriptView::removeScript);
  addAction(m_removeScriptAction);

  connect(scriptTree->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &ExternalScriptView::validateActions);

  validateActions();
}

ExternalScriptView::~ExternalScriptView()
{

}

ExternalScriptItem* ExternalScriptView::currentItem() const
{
  return itemForIndex( scriptTree->currentIndex() );
}

ExternalScriptItem* ExternalScriptView::itemForIndex( const QModelIndex& index ) const
{
  if ( !index.isValid() ) {
    return 0;
  }

  const QModelIndex mappedIndex = m_model->mapToSource( index );
  return static_cast<ExternalScriptItem*>( m_plugin->model()->itemFromIndex( mappedIndex ) );
}


void ExternalScriptView::validateActions()
{
  bool itemSelected = currentItem();

  m_removeScriptAction->setEnabled( itemSelected );
  m_editScriptAction->setEnabled( itemSelected );
}

void ExternalScriptView::contextMenu( const QPoint& pos )
{
  QMenu menu;
  menu.addActions( actions() );

  menu.exec( scriptTree->mapToGlobal( pos ) );
}

bool ExternalScriptView::eventFilter( QObject* obj, QEvent* e )
{
  // no, listening to activated() is not enough since that would also trigger the edit mode which we _dont_ want here
  // users may still rename stuff via select + F2 though
  if ( obj == scriptTree->viewport() ) {
//     const bool singleClick = KGlobalSettings::singleClick();
    const bool singleClick = true; //FIXME: enable singleClick for the sake of porting, should find a proper way
    if ( ( !singleClick && e->type() == QEvent::MouseButtonDblClick ) || ( singleClick && e->type() == QEvent::MouseButtonRelease ) ) {
      QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(e);
      Q_ASSERT( mouseEvent );
      ExternalScriptItem* item = itemForIndex( scriptTree->indexAt( mouseEvent->pos() ) );
      if ( item ) {
        m_plugin->execute( item );
        e->accept();
        return true;
      }
    }
  }
  return QObject::eventFilter( obj, e );
}

void ExternalScriptView::addScript()
{
  ExternalScriptItem* item = new ExternalScriptItem;
  EditExternalScript dlg( item, this );
  int ret = dlg.exec();
  if ( ret == KDialog::Accepted || ret == KDialog::Apply ) {
    m_plugin->model()->appendRow( item );
  } else {
    delete item;
  }
}

void ExternalScriptView::removeScript()
{
  ExternalScriptItem* item = currentItem();
  if ( !item ) {
    return;
  }

  int ret = KMessageBox::questionYesNo( this, 
    i18n("<p>Do you really want to remove the external script configuration for <i>%1</i>?</p>"
         "<p><i>Note:</i> The script itself will not be removed.</p>", item->text()),
    i18n("Confirm External Script Removal")
  );
  if ( ret == KMessageBox::Yes ) {
    m_plugin->model()->removeRow(
        m_plugin->model()->indexFromItem( item ).row()
    );
  }
}

void ExternalScriptView::editScript()
{
  ExternalScriptItem* item = currentItem();
  if ( !item ) {
    return;
  }

  EditExternalScript dlg( item, this );
  int ret = dlg.exec();
  if ( ret == KDialog::Accepted || ret == KDialog::Apply ) {
    item->save();
  }
}


// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on;
