/***************************************************************************
                          KDevMake.cpp  -  description
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

#include "KDevMake.h"

/* Class KDevMake
 * Provide functions to register a Make
 * and its dependant attributes.
 */

// constructors
KDevMake::KDevMake(){
	MakeName = new QString("");
	MakeFlags = new QString("");
}
KDevMake::KDevMake(const QString &name){
	MakeName = new QString(name);
	MakeFlags = new QString("");
}
KDevMake::KDevMake(const QString &name, const QString &icon){
	MakeName = new QString(name);
	MakeIcon = new QString(icon);
	MakeFlags = new QString("");
}

KDevMake::KDevMake(const QString &name, const QString &icon, QWidget &qw){
	MakeName = new QString(name);
	MakeIcon = new QString(icon);
	MakeFlags = new QString("");
	optionsPageWidget = &qw;
}

// destructor
KDevMake::~KDevMake(){
	if (MakeName) delete MakeName;
	if (MakeName) delete MakeFlags;
	if (MakeName) delete MakeIcon;
	if (MakeName) delete optionsPageWidget;
}

// returns the name of the Make
QString* KDevMake::name(){
	return MakeName;
}

// returns the Make flags
QString* KDevMake::flags(){
	return MakeFlags;
}

// returns the icon of the Make
QString* KDevMake::icon(){
	return MakeIcon;
}

// returns the option widget of the Make
QWidget* KDevMake::optionsWidget(){
	return optionsPageWidget;
}

// set the Make Name
void KDevMake::setName(const QString &name){
	if (MakeName)
		*MakeName = *name;
	else
		MakeName = new QString(name);
}

// set the Make flags
void KDevMake::setFlags(const QString &flags){
	if (MakeFlags)
		MakeFlags = new QString(flags);
	else
	  *MakeFlags = *flags;
}

// set the Make icon
void KDevMake::setIcon(const QString &icon){
	if (MakeIcon)
		MakeIcon = new QString(icon);
	else
	  *MakeIcon = *icon;
}

// set the Make widget
void KDevMake::setOptionsWidget(QWidget &qw){
	optionsPageWidget = &qw;
}

QWidget* KDevMake::initOptionsWidget(QWidget* parent, QWidget* pdlg){
	return 0;
}

#include "KDevMake.moc"
