/***************************************************************************
                          KDevCompiler.h  -  description
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

#ifndef KDEVCOMPILER_H
#define KDEVCOMPILER_H

#include <qstring.h>
#include <qwidget.h>


/**
  *@author Omid Givi
  */


// A compiler object
class KDevCompiler : public QObject{
  Q_OBJECT
public:
	KDevCompiler();
	~KDevCompiler();
	KDevCompiler(const QString &name);
	KDevCompiler(const QString &name, const QString &icon);
	KDevCompiler(const QString &name, const QString &icon, QWidget &qw);

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
	QString* compilerName;
	QString* compilerFlags;
	QString* compilerIcon;
	QWidget* optionsPageWidget;
};

#endif
