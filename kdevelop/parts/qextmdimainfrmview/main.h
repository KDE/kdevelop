/***************************************************************************
         mdimainfrmfactory.h  - the library factory
                             -------------------
    begin                : Thu Jul 27 2000
    copyright            : (C) 2000 by Falk Brettschneider
    email                : <Falk Brettschneider> falk@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MDIMAINFRMFACTORY_H_
#define _MDIMAINFRMFACTORY_H_

#include <klibloader.h>


class MdiMainFrmFactory : public KLibFactory
{
  Q_OBJECT

public:
  MdiMainFrmFactory( QObject *parent=0, const char *name=0 );
  ~MdiMainFrmFactory();

  virtual QObject* create( QObject *parent, const char *name,
                           const char *classname, const QStringList &args);
  static KInstance *instance();

private:
  static KInstance *s_instance;
};

#endif
