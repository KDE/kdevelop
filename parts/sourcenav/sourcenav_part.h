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
 

#ifndef __KDEVPART_SOURCENAV_H__
#define __KDEVPART_SOURCENAV_H__

#include <qvaluelist.h>

#include <kdevplugin.h>
#include <kurl.h>

namespace KParts {
  class Part;
};

class KAction;
class KToolBarPopupAction;
class QPopupMenu;

class Anchor
{
public:
  Anchor()
  {
    _id = -1;
    _line = _col = 0;
  }
  Anchor( KURL url, uint line = 0, uint col = 0 )
  { 
    _url = url;
    _line = line;
    _col = col;
    _id = nextID();
  }
  KURL url() const { return _url; }
  uint line() const { return _line; }
  uint col() const { return _col; }
  int  id() const { return _id; }
  bool isValid() const { return _id != -1; }

private:
  KURL _url;
  uint _line, _col;
  int  _id;
  static int nextID();
};

typedef QValueList<Anchor> AnchorList;

class SourceNavPart : public KDevPlugin
{
  Q_OBJECT

public:
   
  SourceNavPart(QObject *parent, const char *name, const QStringList&);
  ~SourceNavPart();
 
public slots:
  void slotNavForward();
  void slotNavBack();

private slots:
  void slotPartAdded( KParts::Part *part );
  void slotTextChanged();
  void fillBackPopup();
  void fillForwardPopup();
  void backPopupClicked( int id );
  void forwardPopupClicked( int id );

private:
  Anchor getCurrentPos();
  void   gotoPos( const Anchor& ankh );
  bool   isNearby( const Anchor& pos1, const Anchor& pos2 );
  void   enableActions();
  void   navigate( AnchorList& list1, AnchorList& list2 );
  void   navigate( int id, AnchorList& list1, AnchorList& list2 );
  void   fillPopup( const AnchorList& list, QPopupMenu* pop );
  void   cleanupList( AnchorList& list );

  KToolBarPopupAction *navForward, *navBack;
  AnchorList          navList;
  AnchorList          forwardList;
  bool                backPopupVisible, forwardPopupVisible;
};


#endif

