#ifndef _CIMPORT_H
#define _CIMPORT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>
#include <stdlib.h>
#include <ctype.h>
#include <qglobal.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qregexp.h>
#include <qstrlist.h>

#include "cproject.h"

class CImport
{
public:
 CImport();
 virtual ~CImport();

 QString getValues(QFile&,const QString&);
 QStrList makeStringList(const QString&);
 void outputInfo(QFile&,const QString&,QFile&);
 int analyze(QFile&,const QString&);
};

#endif // _CIMPORT_H
