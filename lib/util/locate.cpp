//
//
// C++ Implementation for module: Locate
//
// Description: 
//
//
// Author: Eray Ozkural (exa) <erayo@cs.bilkent.edu.tr>
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "locate.h"

#include <qdir.h>

Locate::Locate(KDevPlugin& _part)
 : part(_part)
{
}

QString Locate::relativeName(const QString file)
{
  // file is a URL
  QDir dir(file);
  return QString::null;
}

QString Locate::source(const QString /*file*/)
{
  return QString::null;
}

