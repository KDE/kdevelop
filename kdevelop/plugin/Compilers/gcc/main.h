/***************************************************************************
                          KDevCompilerGCC/main.h  -  description
                             -------------------
    begin                : Thu Feb 1 2001
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

#ifndef _GCCCOMPILERFACTORY_H_
#define _GCCCOMPILERFACTORY_H_

#include <klibloader.h>


class GccCompilerFactory : public KLibFactory
{
  Q_OBJECT

public:
  GccCompilerFactory( QObject *parent=0, const char *name=0 );
  ~GccCompilerFactory();

  virtual QObject* create( QObject *parent, const char *name,
                           const char *classname, const QStringList &args);
  static KInstance *instance();

private:
  static KInstance *s_instance;
};

#endif
