/*
 *  Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_TIPOFDAY_H__
#define __KDEVPART_TIPOFDAY_H__


#include <qguardedptr.h>
#include <kdevpart.h>


class TipOfDay;
class KTipDatabase;


class TipOfDayPart : public KDevPart
{
  Q_OBJECT

public:
   
  TipOfDayPart(KDevApi *api, QObject *parent=0, const char *name=0);
  ~TipOfDayPart();


public slots:

  void showTip();


private slots:

  void nextTip();
  void toggleOnStart(bool on);


private:

 TipOfDay *_tipWidget;
 KTipDatabase *_tipDatabase;

};


#endif
