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

#ifndef KNOTE_PART_H
#define KNOTE_PART_H

#include <qmap.h>
#include <kparts/part.h>


typedef QMap<int, QString> NotesMap;

class KIconView;
class QIconViewItem;
class QPoint;

class KNotesPart : public KParts::ReadOnlyPart
{
  Q_OBJECT

public:

  KNotesPart(QObject *parent=0, const char *name=0);

  bool openFile();

public slots:

  void slotNewNote();

protected slots:
  void slotNoteRenamed(QIconViewItem *item, const QString& text);
  void slotPopupRMB(QIconViewItem *item, const QPoint& pos);
  void slotRemoveCurrentNote();
  void slotRenameCurrentNote();
  void slotOpenNote(QIconViewItem* item);

protected:

  void initKNotes();
  NotesMap fetchNotes();

private:

  KIconView  *m_iconView;
  QPopupMenu *m_popupMenu;

};


#endif // KNOTE_PART_H
