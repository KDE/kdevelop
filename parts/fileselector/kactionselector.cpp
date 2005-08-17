/* This file is part of the KDE project
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include "kactionselector.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kdialog.h> // for spacingHint()
#include <kdebug.h>
#include <qapplication.h>

#include <q3listbox.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qevent.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

class KActionSelectorPrivate {
  public:
  Q3ListBox *availableListBox, *selectedListBox;
  QToolButton *btnAdd, *btnRemove, *btnUp, *btnDown;
  QLabel *lAvailable, *lSelected;
  bool moveOnDoubleClick, keyboardEnabled;
  KActionSelector::ButtonIconSize iconSize;
  QString addIcon, removeIcon, upIcon, downIcon;
  KActionSelector::InsertionPolicy availableInsertionPolicy, selectedInsertionPolicy;
  bool showUpDownButtons;
};

//BEGIN Constructor/destructor

KActionSelector::KActionSelector( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  d = new KActionSelectorPrivate();
  d->moveOnDoubleClick = true;
  d->keyboardEnabled = true;
  d->iconSize = SmallIcon;
  d->addIcon = QApplication::reverseLayout() ? "back" : "forward";
  d->removeIcon = QApplication::reverseLayout() ? "forward" : "back";
  d->upIcon = "up";
  d->downIcon = "down";
  d->availableInsertionPolicy = Sorted;
  d->selectedInsertionPolicy = BelowCurrent;
  d->showUpDownButtons = true;

  //int isz = IconSize( KIcon::Small );

  QHBoxLayout *lo = new QHBoxLayout( this );
  lo->setSpacing( KDialog::spacingHint() );

  QVBoxLayout *loAv = new QVBoxLayout( lo );
  d->lAvailable = new QLabel( i18n("&Available:"), this );
  loAv->addWidget( d->lAvailable );
  d->availableListBox = new Q3ListBox( this );
  loAv->addWidget( d->availableListBox );
  d->lAvailable->setBuddy( d->availableListBox );

  QVBoxLayout *loHBtns = new QVBoxLayout( lo );
  loHBtns->addStretch( 1 );
  d->btnAdd = new QToolButton( this );
  loHBtns->addWidget( d->btnAdd );
  d->btnRemove = new QToolButton( this );
  loHBtns->addWidget( d->btnRemove );
  loHBtns->addStretch( 1 );

  QVBoxLayout *loS = new QVBoxLayout( lo );
  d->lSelected = new QLabel( i18n("&Selected:"), this );
  loS->addWidget( d->lSelected );
  d->selectedListBox = new Q3ListBox( this );
  loS->addWidget( d->selectedListBox );
  d->lSelected->setBuddy( d->selectedListBox );

  QVBoxLayout *loVBtns = new QVBoxLayout( lo );
  loVBtns->addStretch( 1 );
  d->btnUp = new QToolButton( this );
  loVBtns->addWidget( d->btnUp );
  d->btnDown = new QToolButton( this );
  loVBtns->addWidget( d->btnDown );
  loVBtns->addStretch( 1 );

  loadIcons();

  connect( d->btnAdd, SIGNAL(clicked()), this, SLOT(buttonAddClicked()) );
  connect( d->btnRemove, SIGNAL(clicked()), this, SLOT(buttonRemoveClicked()) );
  connect( d->btnUp, SIGNAL(clicked()), this, SLOT(buttonUpClicked()) );
  connect( d->btnDown, SIGNAL(clicked()), this, SLOT(buttonDownClicked()) );
  connect( d->availableListBox, SIGNAL(doubleClicked(Q3ListBoxItem*)),
           this, SLOT(itemDoubleClicked(Q3ListBoxItem*)) );
  connect( d->selectedListBox, SIGNAL(doubleClicked(Q3ListBoxItem*)),
           this, SLOT(itemDoubleClicked(Q3ListBoxItem*)) );
  connect( d->availableListBox, SIGNAL(currentChanged(Q3ListBoxItem*)),
           this, SLOT(slotCurrentChanged(Q3ListBoxItem *)) );
  connect( d->selectedListBox, SIGNAL(currentChanged(Q3ListBoxItem*)),
           this, SLOT(slotCurrentChanged(Q3ListBoxItem *)) );

  d->availableListBox->installEventFilter( this );
  d->selectedListBox->installEventFilter( this );
}

KActionSelector::~KActionSelector()
{
		delete d;
}

//END Constructor/destroctor

//BEGIN Public Methods

Q3ListBox *KActionSelector::availableListBox()
{
  return d->availableListBox;
}

Q3ListBox *KActionSelector::selectedListBox()
{
  return d->selectedListBox;
}

void KActionSelector::setButtonIcon( const QString &icon, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    d->addIcon = icon;
    d->btnAdd->setIconSet( SmallIconSet( icon, d->iconSize ) );
    break;
    case ButtonRemove:
    d->removeIcon = icon;
    d->btnRemove->setIconSet( SmallIconSet( icon, d->iconSize ) );
    break;
    case ButtonUp:
    d->upIcon = icon;
    d->btnUp->setIconSet( SmallIconSet( icon, d->iconSize ) );
    break;
    case ButtonDown:
    d->downIcon = icon;
    d->btnDown->setIconSet( SmallIconSet( icon, d->iconSize ) );
    break;
    default:
    kdDebug()<<"KActionSelector::setButtonIcon: DAINBREAD!"<<endl;
  }
}

void KActionSelector::setButtonIconSet( const QIcon &iconset, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    d->btnAdd->setIconSet( iconset );
    break;
    case ButtonRemove:
    d->btnRemove->setIconSet( iconset );
    break;
    case ButtonUp:
    d->btnUp->setIconSet( iconset );
    break;
    case ButtonDown:
    d->btnDown->setIconSet( iconset );
    break;
    default:
    kdDebug()<<"KActionSelector::setButtonIconSet: DAINBREAD!"<<endl;
  }
}

void KActionSelector::setButtonTooltip( const QString &tip, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    d->btnAdd->setTextLabel( tip );
    break;
    case ButtonRemove:
    d->btnRemove->setTextLabel( tip );
    break;
    case ButtonUp:
    d->btnUp->setTextLabel( tip );
    break;
    case ButtonDown:
    d->btnDown->setTextLabel( tip );
    break;
    default:
    kdDebug()<<"KActionSelector::setButtonToolTip: DAINBREAD!"<<endl;
  }
}

void KActionSelector::setButtonWhatsThis( const QString &text, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    Q3WhatsThis::add( d->btnAdd, text );
    break;
    case ButtonRemove:
    Q3WhatsThis::add( d->btnRemove, text );
    break;
    case ButtonUp:
    Q3WhatsThis::add( d->btnUp, text );
    break;
    case ButtonDown:
    Q3WhatsThis::add( d->btnDown, text );
    break;
    default:
    kdDebug()<<"KActionSelector::setButtonWhatsThis: DAINBREAD!"<<endl;
  }
}

void KActionSelector::setButtonsEnabled()
{
  d->btnAdd->setEnabled( d->availableListBox->currentItem() > -1 );
  d->btnRemove->setEnabled( d->selectedListBox->currentItem() > -1 );
  d->btnUp->setEnabled( d->selectedListBox->currentItem() > 0 );
  d->btnDown->setEnabled( d->selectedListBox->currentItem() > -1 &&
                          d->selectedListBox->currentItem() < (int)d->selectedListBox->count() - 1 );
}

//END Public Methods

//BEGIN Properties

bool KActionSelector::moveOnDoubleClick() const
{
  return d->moveOnDoubleClick;
}

void KActionSelector::setMoveOnDoubleClick( bool b )
{
  d->moveOnDoubleClick = b;
}

bool KActionSelector::keyboardEnabled() const
{
  return d->keyboardEnabled;
}

void KActionSelector::setKeyboardEnabled( bool b )
{
  d->keyboardEnabled = b;
}

QString KActionSelector::availableLabel() const
{
  return d->lAvailable->text();
}

void KActionSelector::setAvailableLabel( const QString &text )
{
  d->lAvailable->setText( text );
}

QString KActionSelector::selectedLabel() const
{
  return d->lSelected->text();
}

void KActionSelector::setSelectedLabel( const QString &text )
{
  d->lSelected->setText( text );
}

KActionSelector::ButtonIconSize KActionSelector::buttonIconSize() const
{
  return d->iconSize;
}

void KActionSelector::setButtonIconSize( ButtonIconSize size )
{
  d->iconSize = size;
  // reload icons
  loadIcons();
}

KActionSelector::InsertionPolicy KActionSelector::availableInsertionPolicy() const
{
  return d->availableInsertionPolicy;
}

void KActionSelector::setAvailableInsertionPolicy( InsertionPolicy p )
{
  d->availableInsertionPolicy = p;
}

KActionSelector::InsertionPolicy KActionSelector::selectedInsertionPolicy() const
{
  return d->selectedInsertionPolicy;
}

void KActionSelector::setSelectedInsertionPolicy( InsertionPolicy p )
{
  d->selectedInsertionPolicy = p;
}

bool KActionSelector::showUpDownButtons() const
{
  return d->showUpDownButtons;
}

void KActionSelector::setShowUpDownButtons( bool show )
{
  d->showUpDownButtons = show;
  if ( show )
  {
    d->btnUp->show();
    d->btnDown->show();
  }
  else
  {
    d->btnUp->hide();
    d->btnDown->hide();
  }
}

//END Properties

//BEGIN Public Slots

void KActionSelector::polish()
{
  setButtonsEnabled();
}

//END Public Slots

//BEGIN Protected
void KActionSelector::keyPressEvent( QKeyEvent *e )
{
  if ( ! d->keyboardEnabled ) return;
  if ( (e->state() & Qt::ControlModifier) )
  {
    switch ( e->key() )
    {
      case Qt::Key_Right:
      buttonAddClicked();
      break;
      case Qt::Key_Left:
      buttonRemoveClicked();
      break;
      case Qt::Key_Up:
      buttonUpClicked();
      break;
      case Qt::Key_Down:
      buttonDownClicked();
      break;
      default:
      e->ignore();
      return;
    }
  }
}

bool KActionSelector::eventFilter( QObject *o, QEvent *e )
{
  if ( d->keyboardEnabled && e->type() == QEvent::KeyPress )
  {
    if  ( (((QKeyEvent*)e)->state() & Qt::ControlModifier) )
    {
      switch ( ((QKeyEvent*)e)->key() )
      {
        case Qt::Key_Right:
        buttonAddClicked();
        break;
        case Qt::Key_Left:
        buttonRemoveClicked();
        break;
        case Qt::Key_Up:
        buttonUpClicked();
        break;
        case Qt::Key_Down:
        buttonDownClicked();
        break;
        default:
        return QWidget::eventFilter( o, e );
        break;
      }
      return true;
    }
    else if ( o->inherits( "QListBox" ) )
    {
      switch ( ((QKeyEvent*)e)->key() )
      {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        Q3ListBox *lb = (Q3ListBox*)o;
        int index = lb->currentItem();
        if ( index < 0 ) break;
        moveItem( lb->item( index ) );
        return true;
      }
    }
  }
  return QWidget::eventFilter( o, e );
}

//END Protected

//BEGIN Private Slots

void KActionSelector::buttonAddClicked()
{
  // move all selected items from available to selected listbox
  Q3ListBoxItem *item = d->availableListBox->firstItem();
  while ( item ) {
    if ( item->isSelected() ) {
      d->availableListBox->takeItem( item );
      d->selectedListBox->insertItem( item, insertionIndex( d->selectedListBox, d->selectedInsertionPolicy ) );
      d->selectedListBox->setCurrentItem( item );
      emit added( item );
    }
    item = item->next();
  }
  if ( d->selectedInsertionPolicy == Sorted )
    d->selectedListBox->sort();
  d->selectedListBox->setFocus();
}

void KActionSelector::buttonRemoveClicked()
{
  // move all selected items from selected to available listbox
  Q3ListBoxItem *item = d->selectedListBox->firstItem();
  while ( item ) {
    if ( item->isSelected() ) {
      d->selectedListBox->takeItem( item );
      d->availableListBox->insertItem( item, insertionIndex( d->availableListBox, d->availableInsertionPolicy ) );
      d->availableListBox->setCurrentItem( item );
      emit removed( item );
    }
    item = item->next();
  }
  if ( d->availableInsertionPolicy == Sorted )
    d->availableListBox->sort();
  d->availableListBox->setFocus();
}

void KActionSelector::buttonUpClicked()
{
  int c = d->selectedListBox->currentItem();
  if ( c < 0 ) return;
  Q3ListBoxItem *item = d->selectedListBox->item( c );
  d->selectedListBox->takeItem( item );
  d->selectedListBox->insertItem( item, c-1 );
  d->selectedListBox->setCurrentItem( item );
  emit movedUp( item );
}

void KActionSelector::buttonDownClicked()
{
  int c = d->selectedListBox->currentItem();
  if ( c < 0 ) return;
  Q3ListBoxItem *item = d->selectedListBox->item( c );
  d->selectedListBox->takeItem( item );
  d->selectedListBox->insertItem( item, c+1 );
  d->selectedListBox->setCurrentItem( item );
  emit movedDown( item );
}

void KActionSelector::itemDoubleClicked( Q3ListBoxItem *item )
{
  if ( d->moveOnDoubleClick )
    moveItem( item );
}

//END Private Slots

//BEGIN Private Methods

void KActionSelector::loadIcons()
{
  d->btnAdd->setIconSet( SmallIconSet( d->addIcon, d->iconSize ) );
  d->btnRemove->setIconSet( SmallIconSet( d->removeIcon, d->iconSize ) );
  d->btnUp->setIconSet( SmallIconSet( d->upIcon, d->iconSize ) );
  d->btnDown->setIconSet( SmallIconSet( d->downIcon, d->iconSize ) );
}

void KActionSelector::moveItem( Q3ListBoxItem *item )
{
  Q3ListBox *lbFrom = item->listBox();
  Q3ListBox *lbTo;
  if ( lbFrom == d->availableListBox )
    lbTo = d->selectedListBox;
  else if ( lbFrom == d->selectedListBox )
    lbTo = d->availableListBox;
  else  //?! somewhat unlikely...
    return;

  InsertionPolicy p = ( lbTo == d->availableListBox ) ?
                        d->availableInsertionPolicy : d->selectedInsertionPolicy;

  lbFrom->takeItem( item );
  lbTo->insertItem( item, insertionIndex( lbTo, p ) );
  lbTo->setFocus();
  lbTo->setCurrentItem( item );

  if ( p == Sorted )
    lbTo->sort();
  if ( lbTo == d->selectedListBox )
    emit added( item );
  else
    emit removed( item );
}

int KActionSelector::insertionIndex( Q3ListBox *lb, InsertionPolicy policy )
{
  int index;
  switch ( policy )
  {
    case BelowCurrent:
    index = lb->currentItem();
    if ( index > -1 ) index += 1;
    break;
    case AtTop:
    index = 0;
    break;
    default:
    index = -1;
  }
  return index;
}

//END Private Methods
#include "kactionselector.moc"
