/*
 * Copyright (C) 2001 Cornelius Schumacher
 */

#ifndef _NEDITFACTORY_H_
#define _NEDITFACTORY_H_


#include <kparts/part.h>
#include <kparts/factory.h>


class KInstance;
class KAboutData;

class NeditPartFactory : public KParts::Factory
{
  Q_OBJECT
public:
  NeditPartFactory();
  virtual ~NeditPartFactory();
  virtual QObject* createObject(QObject *parent, const char *name,
                                const char *classname, const QStringList &args );
  static KInstance* instance();

private:
  static KInstance* s_instance;
  static KAboutData* s_about;
};

#endif
