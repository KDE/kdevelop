/***************************************************************************
                          KDevLinker.h  -  description
                             -------------------
    begin                : Tue Jan 23 2001
    copyright            : (C) 2001 by Omid Givi
    email                : omid@givi.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVLINKER_H
#define KDEVLINKER_H

#include <qstring.h>
#include <qwidget.h>


/**
  *@author Omid Givi
  */


// A Linker object
class KDevLinker : public QObject{
  Q_OBJECT
public:
	KDevLinker();
	~KDevLinker();
	KDevLinker(const QString &name);
	KDevLinker(const QString &name, const QString &icon);
	KDevLinker(const QString &name, const QString &icon, QWidget &qw);

	QString* name();
	QString* flags();
	QWidget* optionsWidget();
	QString* icon();
	void setName(const QString &name);
	void setFlags(const QString &flags);
	void setIcon(const QString &icon);
	void setOptionsWidget(QWidget &qw);
  virtual QWidget* initOptionsWidget(QWidget* parent=0, QWidget* pdlg=0);
		
private:
	QString* LinkerName;
	QString* LinkerFlags;
	QString* LinkerIcon;
	QWidget* optionsPageWidget;
};

#endif
