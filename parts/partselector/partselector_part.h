/*
 *  Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_PARTSELECTOR_H__
#define __KDEVPART_PARTSELECTOR_H__


#include <kdevpart.h>


class KDialogBase;


class PartSelectorPart : public KDevPart
{
  Q_OBJECT

public:
   
  PartSelectorPart(KDevApi *api, QObject *parent=0, const char *name=0);
  ~PartSelectorPart();


private slots:

  void configWidget(KDialogBase *dlg);

};


#endif
