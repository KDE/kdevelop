/*
 * Copyright (C) 2001  <kurt@granroth.org>
 */

#ifndef _EDITORTESTFACTORY_H_
#define _EDITORTESTFACTORY_H_


#include <kparts/part.h>
#include <kparts/factory.h>


class KInstance;
class KAboutData;


class EditorTestPartFactory : public KParts::Factory
{
  Q_OBJECT
          
public:
          
  EditorTestPartFactory();
  virtual ~EditorTestPartFactory();
  
  virtual QObject* createObject(QObject *parent, const char *name,
                                 const char *classname, const QStringList &args );

  static KInstance* instance();

private:
  static KInstance* s_instance;
  static KAboutData* s_about;
};

#endif // _EDITORTESTPART_H_
