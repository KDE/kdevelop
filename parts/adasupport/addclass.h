#ifndef __ADDCLASS_H__
#define __ADDCLASS_H__


#include <qstring.h>
#include <qstringlist.h>


class AddClassInfo
{
public:

  AddClassInfo();

  enum Visibility { PublicClass, ProtectedClass, PrivateClass};
	
  QString className;
  QString extends;
  bool    interfaceOpt, abstractOpt, finalOpt;
  QString projectDir, sourceDir;
  Visibility visibility;
  QStringList implements;
  bool    createConstructor, createMain;
  QString documentation;
  QString license;

  QString adaFileName() const;

};


class AddClass
{
public:

  AddClass();

  void setInfo(const AddClassInfo &info);
  AddClassInfo &info();

  void setBaseClasses(const QStringList &classes);
  bool showDialog();

  bool generate();

	
private:

  AddClassInfo m_info;
  QStringList  m_baseClasses;

};


#endif
