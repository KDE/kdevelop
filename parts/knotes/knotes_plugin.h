/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */
#ifndef KNOTES_PLUGIN_H
#define KNOTES_PLUGIN_H


#include <kdevplugin.h>

class KNotesPart;

class KNotesPlugin : public KDevPlugin
{
  Q_OBJECT
public:
  KNotesPlugin(QObject* parent, const char *name, const QStringList &);
  ~KNotesPlugin();

  KParts::Part* part();
  
private slots:
  void slotNewNote();
  void slotShowNotes();
  void slotPartRemoved( KParts::Part* part );

private:
  KNotesPart *m_part;
  
};


#endif
