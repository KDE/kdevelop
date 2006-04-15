/* This file is part of the KDE project
   Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SAVEALLDIALOG_H
#define SAVEALLDIALOG_H

#include <QStringList>

#include <kdialog.h>

class QTreeWidget;
class KDevDocument;

/**
@file ksavealldialog.h
Dialogs to save multiple files.
*/

/**
Dialog to save selected files.
*/
class KSaveSelectDialog : public KDialog
{
	Q_OBJECT
	
public:
	KSaveSelectDialog( const QList<KDevDocument*>& filelist, const QList<KDevDocument*>& ignorelist, QWidget * parent );
	virtual ~KSaveSelectDialog();
	
	QList<KDevDocument*> filesToSave();
	QList<KDevDocument*> filesNotToSave();
	
private slots:
	void saveNone();
	void save();
	void cancel();
	
private:
	QTreeWidget * _listview;
};


/**
Dialog to save all files.
*/
class KSaveAllDialog : public KDialog
{
  Q_OBJECT

public:
  enum SaveAllResult{ SaveAll, Cancel, Revert };

  KSaveAllDialog( const QStringList& filenames, QWidget* parent );
  virtual ~KSaveAllDialog();

  SaveAllResult result() const { return m_result; }

private slots:
  void revert();
  void saveAll();
  void cancel();

private:
  SaveAllResult m_result;

};

#endif
