/***************************************************************************
 *   Copyright (C) 2002 by Harald Fernengel and the KDevelop Team          *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SAVEALLDIALOG_H
#define SAVEALLDIALOG_H

#include <qstringlist.h>
#include <kdialogbase.h>
#include <kurl.h>

class KListView;

class KSaveSelectDialog : public KDialogBase
{
	Q_OBJECT
	
public:
	KSaveSelectDialog( KURL::List const & filelist, KURL::List const & ignorelist, QWidget * parent );
	virtual ~KSaveSelectDialog();
	
	KURL::List filesToSave();
	KURL::List filesNotToSave();
	
private slots:
	void saveNone();
	void save();
	void cancel();
	
private:
	KListView * _listview;

};


class KSaveAllDialog : public KDialogBase
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
