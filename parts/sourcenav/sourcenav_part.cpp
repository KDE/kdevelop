/***************************************************************************
 *   Copyright (C) 2000-2001 by Harald Fernengel                           *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "sourcenav_part.h"

#include <kdebug.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kguiitem.h>
#include <kpopupmenu.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"

// maximum steps to remember
static const int MAX_HISTORY = 50;
// textchanges after which the QValueLists are cleaned
static const int MAX_CLEANUP = 200;
// maximum number of items to display in Popup
static const int MAX_ITEMS = 20;

static int anchorID = 0;

int Anchor::nextID()
{
  return anchorID++;
}

typedef KGenericFactory<SourceNavPart> SourceNavFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsourcenav, SourceNavFactory( "kdevsourcenav" ) );

SourceNavPart::SourceNavPart(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin(parent, name)
{
  setInstance(SourceNavFactory::instance());
  setXMLFile("kdevpart_sourcenav.rc");

  backPopupVisible = false;
  forwardPopupVisible = false;

  connect( partController(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotPartAdded(KParts::Part*)) );

  navForward = new KToolBarPopupAction( KGuiItem( i18n("Navigate Forward"), "1rightarrow", i18n( "ToolTip" ), i18n( "What's This" ) ),
                                        0, this, SLOT(slotNavForward()), actionCollection(), "navForward" );
  navBack = new KToolBarPopupAction( KGuiItem( i18n("Navigate Backwards"), "1leftarrow", i18n( "ToolTip" ), i18n( "What's This" ) ),
                                     0, this, SLOT(slotNavBack()), actionCollection(), "navBack" );

  connect( navForward->popupMenu(), SIGNAL(aboutToShow()), this, SLOT(fillForwardPopup()) );
  connect( navForward->popupMenu(), SIGNAL(activated(int)), this, SLOT(forwardPopupClicked(int)) );
  connect( navBack->popupMenu(), SIGNAL(aboutToShow()), this, SLOT(fillBackPopup()) );
  connect( navBack->popupMenu(), SIGNAL(activated(int)), this, SLOT(backPopupClicked(int)) );

  navForward->setEnabled( false );
  navBack->setEnabled( false );
}


SourceNavPart::~SourceNavPart()
{
}

void SourceNavPart::backPopupClicked( int id )
{
  navigate( id, navList, forwardList );
}

void SourceNavPart::forwardPopupClicked( int id )
{
  navigate( id, forwardList, navList );
}

void SourceNavPart::fillPopup( const AnchorList& list, QPopupMenu* pop )
{
  if ( !pop )
    return;

  QString item;
  int i = 0;
  pop->clear();
  AnchorList::ConstIterator it = list.begin();
  while ( it != list.end() && i < MAX_ITEMS ) {
    if ( (*it).url().isLocalFile() ) {
      item = (*it).url().fileName();
    } else {
      item = (*it).url().prettyURL();
    }
    item +=  ":" + QString::number( (*it).line() );
    pop->insertItem( item, (*it).id() );
    ++i;
    ++it;
  }
}

void SourceNavPart::fillBackPopup()
{
  fillPopup( navList, navBack->popupMenu() );
}

void SourceNavPart::fillForwardPopup()
{
  fillPopup( forwardList, navForward->popupMenu() );
}

// called when a new document has been added
void SourceNavPart::slotPartAdded( KParts::Part *part )
{
  if ( !part )
    return;
 
  KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(part);
  if ( !doc )
    return;

  KTextEditor::EditInterface *ed = dynamic_cast<KTextEditor::EditInterface*>(doc);
  if ( ed ) {
    connect( doc, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));   
  }  

}

Anchor SourceNavPart::getCurrentPos()
{
  uint line = 0;
  uint col = 0;

  if ( !partController() )
    return Anchor();

  KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(partController()->activePart());
  QWidget *view = partController()->activeWidget();
  if ( !doc || !view )
    return Anchor();

  // hm, let's hope that this is the currently active view...
  KTextEditor::ViewCursorInterface *cur = dynamic_cast<KTextEditor::ViewCursorInterface*>(view);
  if ( cur ) {
    cur->cursorPosition( &line, &col );
  }
  
  return Anchor( doc->url(), line, col );
}

void SourceNavPart::gotoPos( const Anchor& ankh )
{
  if ( !partController() )
    return;
  partController()->editDocument( ankh.url(), ankh.line() );
}

// returns true if pos1 is near to pos2
bool SourceNavPart::isNearby( const Anchor& pos1, const Anchor& pos2 )
{
  // 10 is just a magic number, might need a bit tweaking ;)
  if ( pos1.isValid() && pos2.isValid() && pos1.url() == pos2.url() && QABS((int)pos1.line() - (int)pos2.line()) < 5 )
    return true;
  return false;
}

void SourceNavPart::slotTextChanged()
{
  const Anchor cur = getCurrentPos();
  const Anchor old = navList.isEmpty() ? Anchor() : navList.first();

  if ( !isNearby( cur, old ) ) { 
    navList.push_front( cur );
  }

  if ( cur.id() % MAX_CLEANUP == 0 ) {
    cleanupList( navList );
    cleanupList( forwardList );
  }

  enableActions();
}

void SourceNavPart::navigate( int id, AnchorList& list1, AnchorList& list2 )
{
  Anchor ankh;

  AnchorList::Iterator it = list1.begin();
  while ( it != list1.end() ) {
    ankh = (*it);
    list2.push_front( *it );
    it = list1.remove( it );

    if ( ankh.id() == id ) {
      // found the right position
      it = list1.end();
    }
  }

  if ( ankh.isValid() ) {
    gotoPos( ankh );
  }
  enableActions();
}

void SourceNavPart::navigate( AnchorList& list1, AnchorList& list2 )
{
  if ( list1.isEmpty() )
    return;

  const Anchor cur = getCurrentPos();
  Anchor last = list1.first();

  list2.push_front( last );
  list1.pop_front();

  if ( isNearby( cur, last ) ) {
    // we are currently at the position of the last change, so go to the one before
    if ( list1.isEmpty() ) {
      //nowhere to go...
      enableActions();
      return;
    }
    last = list1.first();
    list2.push_front( last );
    list1.pop_front();
  }

  gotoPos( last );
  enableActions();
}

void SourceNavPart::slotNavBack()
{
  navigate( navList, forwardList );
}

void SourceNavPart::slotNavForward()
{
  navigate( forwardList, navList );
}

void SourceNavPart::enableActions()
{
  navForward->setEnabled( !forwardList.isEmpty() );
  navBack->setEnabled( !navList.isEmpty() );
}

void SourceNavPart::cleanupList( AnchorList& list )
{
  AnchorList::Iterator it = list.at( MAX_HISTORY );
  while ( it != list.end() ) {
    it = list.remove( it );
  }
}

#include "sourcenav_part.moc"

