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

class Anchor
{
public:
  Anchor( KURL url = KURL(), uint line = 0, uint col = 0 )
  { 
    _url = url;
    _line = line;
    _col = col;
  }
  KURL url() const { return _url; }
  uint line() const { return _line; }
  uint col() const { return _col; }

private:
  KURL _url;
  uint _line, _col;
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

private:
  Anchor getCurrentPos();
  void   gotoPos( const Anchor& ankh );
  bool   isNearby( const Anchor& pos1, const Anchor& pos2 );
  void   enableActions();
  void   navigate( AnchorList& list1, AnchorList& list2 );

  KAction *navForward, *navBack;
  AnchorList navList;
  AnchorList forwardList;
};


#endif

