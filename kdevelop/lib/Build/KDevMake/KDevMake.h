/***************************************************************************
                          KDevMake.h  -  description
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

#ifndef KDEVMAKE_H
#define KDEVMAKE_H

#include <qstring.h>
#include <qwidget.h>


/**
  *@author Omid Givi
  */


// A Make object
class KDevMake : public QObject{
  Q_OBJECT
public:
	KDevMake();
	~KDevMake();
	KDevMake(const QString &name);
	KDevMake(const QString &name, const QString &icon);
	KDevMake(const QString &name, const QString &icon, QWidget &qw);

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
	QString* MakeName;
	QString* MakeFlags;
	QString* MakeIcon;
	QWidget* optionsPageWidget;
};

#endif
