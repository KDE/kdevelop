/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kdevdebugger.h"

KDevDebugger::KDevDebugger(QObject *parent, const char *name)
  : QObject(parent, name)
{
}


KDevDebugger::~KDevDebugger()
{
}

const QPixmap* KDevDebugger::inactiveBreakpointPixmap()
{
  const char*breakpoint_gr_xpm[]={
  "11 16 6 1",
  "c c #c6c6c6",
  "d c #2c2c2c",
  "# c #000000",
  ". c None",
  "a c #ffffff",
  "b c #555555",
  "...........",
  "...........",
  "...#####...",
  "..#aaaaa#..",
  ".#abbbbbb#.",
  "#abbbbbbbb#",
  "#abcacacbd#",
  "#abbbbbbbb#",
  "#abcacacbd#",
  "#abbbbbbbb#",
  ".#bbbbbbb#.",
  "..#bdbdb#..",
  "...#####...",
  "...........",
  "...........",
  "..........."};
  static QPixmap pixmap( breakpoint_gr_xpm );
  return &pixmap;
}

const QPixmap* KDevDebugger::activeBreakpointPixmap()
{
  const char* breakpoint_xpm[]={
  "11 16 6 1",
  "c c #c6c6c6",
  ". c None",
  "# c #000000",
  "d c #840000",
  "a c #ffffff",
  "b c #ff0000",
  "...........",
  "...........",
  "...#####...",
  "..#aaaaa#..",
  ".#abbbbbb#.",
  "#abbbbbbbb#",
  "#abcacacbd#",
  "#abbbbbbbb#",
  "#abcacacbd#",
  "#abbbbbbbb#",
  ".#bbbbbbb#.",
  "..#bdbdb#..",
  "...#####...",
  "...........",
  "...........",
  "..........."};
  static QPixmap pixmap( breakpoint_xpm );
  return &pixmap;
}

const QPixmap* KDevDebugger::reachedBreakpointPixmap()
{
  const char*breakpoint_bl_xpm[]={
  "11 16 7 1",
  "a c #c0c0ff",
  "# c #000000",
  "c c #0000c0",
  "e c #0000ff",
  "b c #dcdcdc",
  "d c #ffffff",
  ". c None",
  "...........",
  "...........",
  "...#####...",
  "..#ababa#..",
  ".#bcccccc#.",
  "#acccccccc#",
  "#bcadadace#",
  "#acccccccc#",
  "#bcadadace#",
  "#acccccccc#",
  ".#ccccccc#.",
  "..#cecec#..",
  "...#####...",
  "...........",
  "...........",
  "..........."};
  static QPixmap pixmap( breakpoint_bl_xpm );
  return &pixmap;
}

const QPixmap* KDevDebugger::disabledBreakpointPixmap()
{
  const char*breakpoint_wh_xpm[]={
  "11 16 7 1",
  "a c #c0c0ff",
  "# c #000000",
  "c c #0000c0",
  "e c #0000ff",
  "b c #dcdcdc",
  "d c #ffffff",
  ". c None",
  "...........",
  "...........",
  "...#####...",
  "..#ddddd#..",
  ".#ddddddd#.",
  "#ddddddddd#",
  "#ddddddddd#",
  "#ddddddddd#",
  "#ddddddddd#",
  "#ddddddddd#",
  ".#ddddddd#.",
  "..#ddddd#..",
  "...#####...",
  "...........",
  "...........",
  "..........."};
  static QPixmap pixmap( breakpoint_wh_xpm );
  return &pixmap;
}

const QPixmap* KDevDebugger::executionPointPixmap()
{
  const char*exec_xpm[]={
  "11 16 4 1",
  "a c #00ff00",
  "b c #000000",
  ". c None",
  "# c #00c000",
  "...........",
  "...........",
  "...........",
  "#a.........",
  "#aaa.......",
  "#aaaaa.....",
  "#aaaaaaa...",
  "#aaaaaaaaa.",
  "#aaaaaaa#b.",
  "#aaaaa#b...",
  "#aaa#b.....",
  "#a#b.......",
  "#b.........",
  "...........",
  "...........",
  "..........."};
  static QPixmap pixmap( exec_xpm );
  return &pixmap;
}

#include "kdevdebugger.moc"
