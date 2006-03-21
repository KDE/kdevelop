/*
 *  Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_TIPOFDAY_H__
#define __KDEVPART_TIPOFDAY_H__


#include <kdevplugin.h>


class TipOfDayPart : public KDevPlugin
{
  Q_OBJECT

public:
   
  TipOfDayPart(QObject *parent, const char *name, const QStringList &);


public slots:

  void showTip();
  void showOnStart();

};


#endif
