/***************************************************************************
 *   Copyright (C) 2001 by Richard Dale                                    *
 *   Richard_Dale@tipitina.demon.co.uk                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <kinstance.h>
#include <kstandarddirs.h>

#include "javasupportfactory.h"
#include "classparser.h"

#include <qtjava/QtSupport.h>


extern "C" {

    void *init_libkdevjavasupport()
    {
        return new JavaSupportFactory;
    }

void JNICALL
Java_JavaClassParser_newJavaClassParser__Lorg_kde_koala_ClassStore_2(JNIEnv *env, jobject obj, jobject classstore)
{
	if (QtSupport::getQt(env, obj) == 0) {
		QtSupport::setQt(env, obj, new JavaClassParser((ClassStore*) QtSupport::getQt(env, classstore)));
		QtSupport::setObjectForQtKey(env, obj, QtSupport::getQt(env, obj));
	}
	return;
}

void JNICALL
Java_JavaClassParser_finalize(JNIEnv *env, jobject obj)
{
	if (QtSupport::allocatedInJavaWorld(env, obj)) {
		delete (JavaClassParser*)QtSupport::getQt(env, obj);
		QtSupport::setQt(env, obj, 0);
	}
	return;
}

jboolean JNICALL
Java_JavaClassParser_parse(JNIEnv *env, jobject obj, jstring file)
{
static QString * _qstring_file = 0;
	return (jboolean) ((JavaClassParser*) QtSupport::getQt(env, obj))->parse((QString&) * (QString *) QtSupport::toQString(env, file, &_qstring_file));
}

void JNICALL
Java_JavaClassParser_wipeout(JNIEnv *env, jobject obj)
{
	((JavaClassParser*) QtSupport::getQt(env, obj))->wipeout();
	return;
}

void JNICALL
Java_JavaClassParser_out(JNIEnv *env, jobject obj)
{
	((JavaClassParser*) QtSupport::getQt(env, obj))->out();
	return;
}

void JNICALL
Java_JavaClassParser_removeWithReferences(JNIEnv *env, jobject obj, jstring aFile)
{
static QCString * _qstring_aFile = 0;
	((JavaClassParser*) QtSupport::getQt(env, obj))->removeWithReferences((char*) QtSupport::toCharString(env, aFile, &_qstring_aFile));
	return;
}

void JNICALL
Java_JavaSupportFactory_newJavaSupportFactory__Lorg_kde_qt_QObject_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jobject parent, jstring name)
{
static QCString * _qstring_name = 0;
	if (QtSupport::getQt(env, obj) == 0) {
		QtSupport::setQt(env, obj, new JavaSupportFactory((QObject*) QtSupport::getQt(env, parent), (char*) QtSupport::toCharString(env, name, &_qstring_name)));
		QtSupport::setObjectForQtKey(env, obj, QtSupport::getQt(env, obj));
	}
	return;
}

void JNICALL
Java_JavaSupportFactory_newJavaSupportFactory__(JNIEnv *env, jobject obj)
{
	if (QtSupport::getQt(env, obj) == 0) {
		QtSupport::setQt(env, obj, new JavaSupportFactory(0, 0));
		QtSupport::setObjectForQtKey(env, obj, QtSupport::getQt(env, obj));
	}

	return;
}

void JNICALL
Java_JavaSupportFactory_finalize(JNIEnv *env, jobject obj)
{
	if (QtSupport::allocatedInJavaWorld(env, obj)) {
		delete (JavaSupportFactory*)QtSupport::getQt(env, obj);
		QtSupport::setQt(env, obj, 0);
	}
	return;
}

jobject JNICALL
Java_JavaSupportFactory_createPartObject(JNIEnv *env, jobject obj, jobject api, jobject parent, jobjectArray args)
{
static QStringList * _qlist_args = 0;
	return (jobject) QtSupport::objectForQtKey(env, (void *)((JavaSupportFactory*) QtSupport::getQt(env, obj))->createPartObject((KDevApi*) QtSupport::getQt(env, api), (QObject*) QtSupport::getQt(env, parent), (QStringList&) * (QStringList *) QtSupport::toQStringList(env, args, &_qlist_args)), "org.kde.koala.KDevPart");
}

jobject JNICALL
Java_JavaSupportFactory_instance(JNIEnv *env, jclass obj)
{
	return (jobject) QtSupport::objectForQtKey(env, (void *)JavaSupportFactory::instance(), "org.kde.koala.KInstance");
}

};


JavaSupportFactory::JavaSupportFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
	JNINativeMethod	nm;
	JNIEnv *		env;
	jclass			urlLoaderClass;
	jclass			factoryClass;
	jclass			partClass;
	jclass			parserClass;
	JavaVM *		jvm;
	jint			res;
	QString			classPath;

	// Set up the classpath and start the Java VM
	classPath = "-Djava.class.path=";
	classPath += instance()->dirs()->findResourceDir("data", "kdevjavasupport/kdevjavasupport.jar");
	classPath += "kdevjavasupport:";
	classPath += instance()->dirs()->findResourceDir("lib", "java/qtjava.jar");
	classPath += "java/qtjava.jar:";
	classPath += instance()->dirs()->findResourceDir("lib", "java/koala.jar");
	classPath += "java/koala.jar";

	JavaVMInitArgs	vm_args;
	JavaVMOption options[1];
	options[0].optionString = (char *) (const char *) classPath;
	vm_args.version = JNI_VERSION_1_2;
	vm_args.options = options;
	vm_args.nOptions = 1;
	vm_args.ignoreUnrecognized = JNI_TRUE;
	res = JNI_CreateJavaVM(&jvm, (void **) &env, &vm_args);
	if (res < 0) {
		kdDebug(9013) << "Can't create JVM" << endl;
		return;
	}

	kdDebug(9013) << "Created JVM, classpath: " << classPath << endl;
	
	urlLoaderClass = env->FindClass("kdevjavapluginapi");

	if (urlLoaderClass == 0) {
		kdDebug(9013) << "kdevjavapluginapi class not found" << endl;
		env->ExceptionDescribe();
		return;
	}

	partClass = env->FindClass("JavaSupportPart");

	if (partClass == 0) {
		kdDebug(9013) << "JavaSupportPart class not found" << endl;
		env->ExceptionDescribe();
		return;
	}

	factoryClass = env->FindClass("JavaSupportFactory");

	if (factoryClass == 0) {
		kdDebug(9013) << "JavaSupportFactory class not found" << endl;
		env->ExceptionDescribe();
		return;
	}

	parserClass = env->FindClass("JavaClassParser");

	if (parserClass == 0) {
		kdDebug(9013) << "JavaClassParser class not found" << endl;
		env->ExceptionDescribe();
		return;
	}

	nm.name = (char *) "newJavaSupportFactory";
	nm.signature = (char *) "(Lorg/kde/qt/QObject;Ljava/lang/String;)V";
	nm.fnPtr = (void*) Java_JavaSupportFactory_newJavaSupportFactory__Lorg_kde_qt_QObject_2Ljava_lang_String_2;
	env->RegisterNatives(factoryClass, &nm, 1);

	nm.name = (char *) "newJavaSupportFactory";
	nm.signature = (char *) "()V";
	nm.fnPtr = (void*) Java_JavaSupportFactory_newJavaSupportFactory__;
	env->RegisterNatives(factoryClass, &nm, 1);

	nm.name = (char *) "finalize";
	nm.signature = (char *) "()V";
	nm.fnPtr = (void*) Java_JavaSupportFactory_finalize;
	env->RegisterNatives(factoryClass, &nm, 1);

	nm.name = (char *) "createPartObject";
	nm.signature = (char *) "(Lorg/kde/koala/KDevApi;Lorg/kde/qt/QObject;[Ljava/lang/String;)Lorg/kde/koala/KDevPart;";
	nm.fnPtr = (void*) Java_JavaSupportFactory_createPartObject;
	env->RegisterNatives(factoryClass, &nm, 1);

	nm.name = (char *) "instance";
	nm.signature = (char *) "()Lorg/kde/koala/KInstanceInterface;";
	nm.fnPtr = (void*) Java_JavaSupportFactory_instance;
	env->RegisterNatives(factoryClass, &nm, 1);

	nm.name = (char *) "finalize";
	nm.signature = (char *) "()V";
	nm.fnPtr = (void*) Java_JavaClassParser_finalize;
	env->RegisterNatives(parserClass, &nm, 1);

	nm.name = (char *) "newJavaClassParser";
	nm.signature = (char *) "(Lorg/kde/koala/ClassStore;)V";
	nm.fnPtr = (void*) Java_JavaClassParser_newJavaClassParser__Lorg_kde_koala_ClassStore_2;
	env->RegisterNatives(parserClass, &nm, 1);

	nm.name = (char *) "out";
	nm.signature = (char *) "()V";
	nm.fnPtr = (void*) Java_JavaClassParser_out;
	env->RegisterNatives(parserClass, &nm, 1);

	nm.name = (char *) "parse";
	nm.signature = (char *) "(Ljava/lang/String;)Z";
	nm.fnPtr = (void*) Java_JavaClassParser_parse;
	env->RegisterNatives(parserClass, &nm, 1);

	nm.name = (char *) "removeWithReferences";
	nm.signature = (char *) "(Ljava/lang/String;)V";
	nm.fnPtr = (void*) Java_JavaClassParser_removeWithReferences;
	env->RegisterNatives(parserClass, &nm, 1);

	nm.name = (char *) "wipeout";
	nm.signature = (char *) "()V";
	nm.fnPtr = (void*) Java_JavaClassParser_wipeout;
	env->RegisterNatives(parserClass, &nm, 1);
}


JavaSupportFactory::~JavaSupportFactory()
{
	JNIEnv *	env;
	JavaVM *	jvm;
	int			result;

	delete s_instance;
    s_instance = 0;

	env = QtSupport::GetEnv();
	if (env == 0) {
		return;
    }

	result = env->GetJavaVM(&jvm);
	if (result == 0) {
		jvm->DestroyJavaVM();
	}
}


KDevPart *JavaSupportFactory::createPartObject(KDevApi *api, QObject *parent,
                                              const QStringList &/*args*/)
{
    kdDebug(9013) << "Building JavaSupport" << endl;
	JNIEnv *	env;
	jclass		partClass;
	jobject		result;
	jmethodID	cid;
	
	env = QtSupport::GetEnv();
	
	if (env == 0) {
		kdDebug(9013) << "Can't find Java env" << endl;
		return 0;
	}

	partClass = env->FindClass("JavaSupportPart");

	if (partClass == 0) {
		kdDebug(9013) << "JavaSupportPart class not found" << endl;
		return 0;
	}

	cid = env->GetMethodID(partClass, "<init>", "(Lorg/kde/koala/KDevApi;Lorg/kde/qt/QObject;Ljava/lang/String;)V");
	
	if (cid == 0) {
		kdDebug(9013) << "Can't find method id for JavaSupportPart constructor" << endl;
		return 0;
    }

	result = env->NewObject(	partClass,
								cid,
								QtSupport::objectForQtKey(env, api, "org.kde.koala.KDevApi"),
								QtSupport::objectForQtKey(env, parent, "org.kde.qt.QObject"),
								env->NewStringUTF("java support part") );
	
	if (result == 0) {
		kdDebug(9013) << "Can't create JavaSupportPart instance" << endl;
		env->ExceptionDescribe();
		return 0;
	}

	return (KDevPart *) QtSupport::getQt(env, result);
}


KInstance *JavaSupportFactory::s_instance = 0;
KInstance *JavaSupportFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevjavasupport");

    return s_instance;
}

#include "javasupportfactory.moc"
