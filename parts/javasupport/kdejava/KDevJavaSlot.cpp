/***************************************************************************
                          KDevJavaSlot.cpp  -  description
                             -------------------
    begin                : Fri Dec 21 2001
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

#include "KDevJavaSlot.moc"
#include <qtjava/QtSupport.h>
#include <kdejava/KDESupport.h>

/*
 Converts a Java type signature, from a signal or slot, to the corresponding C++ KDE type.
 Used to connect a Java slot to a signal emitted by the underlying C++ KDE instance.
 Note that all Java slots and signals are of type jobjectArray, and so no conversion is needed for them.
*/
static const char * const javaToQtTypeSignatureMap[][2] = {
	{"(QPopupMenu,Context)",									"(QPopupMenu*,const Context*)"},
	{"(Document)",												"(KEditor::Document*)"},
	{"(Document,boolean)",										"(KEditor::Document*,bool)"},
	{"(Document,int)",											"(KEditor::Document*,int)"},
	{"(Document,int,int)",										"(KEditor::Document*,int,int)"},
	{"(Document,String)",										"(KEditor::Document*,const QString&)"}
};

KDevJavaSlot::KDevJavaSlot(JNIEnv * env, jobject receiver, jstring slot)
	: KDEJavaSlot(env, receiver, slot)
{
}

KDevJavaSlot::~KDevJavaSlot()
{
}


const char *
KDevJavaSlot::javaToQtSignalType(const char * signalName, const char * javaTypeSignature, QMetaObject * smeta)
{
static char qtSignalString[200];

	for (	unsigned int index = 0;
			index < sizeof(javaToQtTypeSignatureMap)/sizeof(*javaToQtTypeSignatureMap);
			index++ )
	{
		if (strcmp(javaTypeSignature, javaToQtTypeSignatureMap[index][0]) == 0) {
			(void) sprintf(qtSignalString, "%s%s", signalName, javaToQtTypeSignatureMap[index][1]);
			
			if (smeta == 0 || smeta->findSignal(((const char *) qtSignalString + 1), TRUE) >= 0) {
				return qtSignalString;
			}
		}
	}

	return KDEJavaSlot::javaToQtSignalType(signalName, javaTypeSignature, smeta);
}

const char *
KDevJavaSlot::javaToQtSlotType(const char * javaTypeSignature, const char * signalString)
{
	for (	unsigned int index = 0;
			index < sizeof(javaToQtTypeSignatureMap)/sizeof(*javaToQtTypeSignatureMap);
			index++ )
	{
		if (	strcmp(javaTypeSignature, javaToQtTypeSignatureMap[index][0]) == 0
				&& (	signalString == 0
						|| QObject::checkConnectArgs(signalString, (const QObject *) 0, javaToQtTypeSignatureMap[index][1]) ) )
		{
			return javaToQtTypeSignatureMap[index][1];
		}
	}

	return KDEJavaSlot::javaToQtSlotType(javaTypeSignature, signalString);
}

void
KDevJavaSlot::invoke(QPopupMenu *arg1, const Context *arg2)
{
	JNIEnv *	env;
	jclass		cls;
	jmethodID	mid;
	jobject		result;

	env = QtSupport::GetEnv();
	cls = env->GetObjectClass(invocation);
	mid = env->GetMethodID(cls, "invoke", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	if (mid == NULL) {
		return;
	}
	result = env->CallObjectMethod(	invocation, 
									mid, 
									QtSupport::objectForQtKey(env, (void *) arg1, "org.kde.qt.QPopupMenu"),
									QtSupport::objectForQtKey(env, (void *) arg1, "org.kde.koala.Context") );
}

void
KDevJavaSlot::invoke(KEditor::Document *doc)
{
	JNIEnv *	env;
	jclass		cls;
	jmethodID	mid;
	jobject		result;

	env = QtSupport::GetEnv();
	cls = env->GetObjectClass(invocation);
	mid = env->GetMethodID(cls, "invoke", "(Ljava/lang/Object;)Ljava/lang/Object;");
	if (mid == NULL) {
		return;
	}
	result = env->CallObjectMethod(invocation, mid, QtSupport::objectForQtKey(env, (void *) doc, "org.kde.koala.Document"));
}

void
KDevJavaSlot::invoke(KEditor::Document *doc, bool available)
{
	JNIEnv *	env;
	jclass		cls;
	jmethodID	mid;
	jobject		result;

	env = QtSupport::GetEnv();
	cls = env->GetObjectClass(invocation);
	mid = env->GetMethodID(cls, "invoke", "(Ljava/lang/Object;Z)Ljava/lang/Object;");
	if (mid == NULL) {
		return;
	}
	result = env->CallObjectMethod(invocation, mid, QtSupport::objectForQtKey(env, (void *) doc, "org.kde.koala.Document"), (jboolean) available);
}

void
KDevJavaSlot::invoke(KEditor::Document *doc, int line)
{
	JNIEnv *	env;
	jclass		cls;
	jmethodID	mid;
	jobject		result;

	env = QtSupport::GetEnv();
	cls = env->GetObjectClass(invocation);
	mid = env->GetMethodID(cls, "invoke", "(Ljava/lang/Object;I)Ljava/lang/Object;");
	if (mid == NULL) {
		return;
	}
	result = env->CallObjectMethod(invocation, mid, QtSupport::objectForQtKey(env, (void *) doc, "org.kde.koala.Document"), (jint) line);
}

void
KDevJavaSlot::invoke(KEditor::Document *doc, int line, int col)
{
	JNIEnv *	env;
	jclass		cls;
	jmethodID	mid;
	jobject		result;

	env = QtSupport::GetEnv();
	cls = env->GetObjectClass(invocation);
	mid = env->GetMethodID(cls, "invoke", "(Ljava/lang/Object;II)Ljava/lang/Object;");
	if (mid == NULL) {
		return;
	}
	result = env->CallObjectMethod(	invocation, mid, 
									QtSupport::objectForQtKey(env, (void *) doc, "org.kde.koala.Document"),
									(jint) line,
									(jint) col );
}

void
KDevJavaSlot::invoke(KEditor::Document *doc, const QString &text)
{
	JNIEnv *	env;
	jclass		cls;
	jmethodID	mid;
	jobject		result;

	env = QtSupport::GetEnv();
	cls = env->GetObjectClass(invocation);
	mid = env->GetMethodID(cls, "invoke", "(Ljava/lang/Object;II)Ljava/lang/Object;");
	if (mid == NULL) {
		return;
	}
	result = env->CallObjectMethod(	invocation, mid, 
									QtSupport::objectForQtKey(env, (void *) doc, "org.kde.koala.Document"),
									QtSupport::fromQString(env, (QString *) &text) );
}

KDevJavaSlotFactory::KDevJavaSlotFactory()
	: KDEJavaSlotFactory()
{
}

JavaSlot *
KDevJavaSlotFactory::createJavaSlot(JNIEnv * env, jobject receiver, jstring slot)
{
	return new KDevJavaSlot(env, receiver, slot);
}


