/*
 *  Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_ASTYLE_H__
#define __KDEVPART_ASTYLE_H__

class KDialogBase;
#include <kaction.h>

#include <kdevpart.h>


#include "keditor/editor.h"


class AStylePart : public KDevPart
{
  Q_OBJECT

public:
   
  AStylePart(KDevApi *api, QObject *parent=0, const char *name=0);
  ~AStylePart();


private slots:

  void activePartChanged(KParts::Part *part);
		  
  void beautifySource();
 
  void configWidget(KDialogBase *dlg);


private:

  KAction *_action;

};


#endif
