/*
 * Copyright (C) 2001  <kurt@granroth.org>
 */

#ifndef _QEDITORFACTORY_H_
#define _QEDITORFACTORY_H_


#include <kparts/part.h>
#include <klibloader.h>


class KInstance;
class KAboutData;


class QEditorPartFactory : public KLibFactory
{
  Q_OBJECT

public:

  QEditorPartFactory();
  virtual ~QEditorPartFactory();

  virtual QObject* createObject(QObject *parent, const char *name,
                                 const char *classname, const QStringList &args );

  static KInstance* instance();

private:
  static KInstance* s_instance;
  static KAboutData* s_about;
};

#endif // _QEDITORPART_H_
