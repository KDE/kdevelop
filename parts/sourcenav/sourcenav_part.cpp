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

#include "kdevcore.h"
#include "kdevpartcontroller.h"

typedef KGenericFactory<SourceNavPart> SourceNavFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsourcenav, SourceNavFactory( "kdevsourcenav" ) );

SourceNavPart::SourceNavPart(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin(parent, name)
{
  setInstance(SourceNavFactory::instance());
  setXMLFile("kdevpart_sourcenav.rc");

  connect( partController(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotPartAdded(KParts::Part*)) );

  navForward = new KAction( i18n("&Navigate Forward"), "1rightarrow", 0,
                            this, SLOT(slotNavForward()),
                            actionCollection(), "navForward" );
  navBack = new KAction( i18n("&Navigate Back"), "1leftarrow", 0,
                            this, SLOT(slotNavBack()),
                            actionCollection(), "navBack" );

  navForward->setEnabled( false );
  navBack->setEnabled( false );
}


SourceNavPart::~SourceNavPart()
{
}

void SourceNavPart::slotPartAdded( KParts::Part *part )
{
  if ( !part )
    return;
 
  KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(part);
  if ( !doc )
    return;

  kdDebug() << "SOURCENAV: found KTextEditor::Document" << endl;

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
  if ( !doc )
    return Anchor();

  // hm, let's hope that this is the currently active view...
  KTextEditor::ViewCursorInterface *cur = dynamic_cast<KTextEditor::ViewCursorInterface*>(doc->widget());
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
  if ( pos1.url() == pos2.url() && QABS((int)pos1.line() - (int)pos2.line()) < 5 )
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

  enableActions();
//  kdDebug() << "SourceNavPart::slotTextChanged: " << cur.url().prettyURL() << ":" << cur.line() << endl;
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

#include "sourcenav_part.moc"

