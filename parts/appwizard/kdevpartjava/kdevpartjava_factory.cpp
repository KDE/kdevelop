#include <kdebug.h>
#include <kinstance.h>
#include <kstandarddirs.h>

#include <jni.h>
#include <qtjava/QtSupport.h>
#include "$APPNAMELC$_factory.h"

extern "C" 
{

  void *init_libkdev$APPNAMELC$()
    {
      return new $APPNAME$Factory;
    }


};

$APPNAME$Factory::$APPNAME$Factory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
	JNINativeMethod	nm;
	JNIEnv *		env;
	jclass			cls;
	jclass			urlLoaderClass;
	jmethodID       addURLmethod;
	QString			classPath;

	env = QtSupport::GetEnv();
	if (env == 0) {
		// The javasupport part hasn't been loaded yet
		return;
	}

	urlLoaderClass = env->FindClass("kdevjavapluginapi");
	if (urlLoaderClass == 0) {
		return;
	}

	addURLmethod = env->GetStaticMethodID(urlLoaderClass, "addURLString", "(Ljava/lang/String;)V");
	if (addURLmethod == 0) {
		kdDebug(9013) << "addURLString(): no such method" << endl;
		return;
	}

	classPath = "file:";
	classPath += instance()->dirs()->findResourceDir("data", "kdev$APPNAMELC$/kdev$APPNAMELC$.jar");
	classPath += "kdev$APPNAMELC$/kdev$APPNAMELC$.jar";

	env->CallStaticVoidMethod(	urlLoaderClass,
								addURLmethod,
								env->NewStringUTF((const char *) classPath) );

	cls = env->FindClass("$APPNAME$Factory");
	if (cls == 0) {
		return;
	}
}


$APPNAME$Factory::~$APPNAME$Factory()
{
}


KDevPart *$APPNAME$Factory::createPartObject(KDevApi *api, QObject *parent, 
											   const QStringList &/*args*/)
{
	JNIEnv *	env;
	jclass		cls;
	jobject		result;
	jmethodID	cid;

	env = QtSupport::GetEnv();
	if (env == 0) {
		// The javasupport part hasn't been loaded yet
		return 0;
	}

	cls = env->FindClass("$APPNAME$Part");
	if (cls == 0) {
		return 0;
	}

	cid = env->GetMethodID(cls, "<init>", "(Lorg/kde/koala/KDevApi;Lorg/kde/qt/QObject;Ljava/lang/String;)V");
	if (cid == 0) {
		return 0;
    }

	result = env->NewObject(	cls,
								cid,
								QtSupport::objectForQtKey(env, api, "org.kde.koala.KDevApi"),
								QtSupport::objectForQtKey(env, parent, "org.kde.qt.QObject"),
								env->NewStringUTF("$APPNAMELC$") );

	return (KDevPart *) QtSupport::getQt(env, result);
}

KInstance *$APPNAME$Factory::s_instance = 0;

KInstance *$APPNAME$Factory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdev$APPNAMELC$");

  return s_instance;
}

#include "$APPNAMELC$_factory.moc"



