/***************************************************************************
                          KDEJavaSlot.h  -  description
                             -------------------
    begin                : Tue Oct 31 2000
    copyright            : (C) 2000-2001 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVJAVASLOT_H_
#define _KDEVJAVASLOT_H_

#include <jni.h>
#include <qobject.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <kdejava/KDEJavaSlot.h>
#include <kdevcore.h>
#include <keditor/editor.h>

class KDevJavaSlotFactory;

class KDevJavaSlot: public KDEJavaSlot
{
    Q_OBJECT
public:
    KDevJavaSlot(JNIEnv * env, jobject receiver, jstring slot);
    virtual ~KDevJavaSlot();

	/** Converts a Java type signature for a signal to the corresponding C++ Qt type signature */
	virtual const char * javaToQtSignalType(const char * signalName, const char * javaTypeSignature, QMetaObject * smeta);
	
	/** Converts a Java type signature for a slot to the corresponding C++ Qt type signature */
	virtual const char * javaToQtSlotType(const char * javaTypeSignature, const char * signalString);

public slots:
	void invoke(QPopupMenu *arg1, const Context *arg2);
	void invoke(KEditor::Document *doc);
	void invoke(KEditor::Document *doc, bool available);
	void invoke(KEditor::Document *doc, int line);
	void invoke(KEditor::Document *doc, int line, int col);
	void invoke(KEditor::Document *doc, const QString &text);
};

class KDevJavaSlotFactory : public KDEJavaSlotFactory {
public:
	KDevJavaSlotFactory();
	virtual ~KDevJavaSlotFactory() {};
	virtual JavaSlot *	createJavaSlot(JNIEnv * env, jobject receiver, jstring slot);
};

#endif




