/*
 *  Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_ASTYLE_H__
#define __KDEVPART_ASTYLE_H__

class KDialogBase;
#include <kaction.h>

#include <kdevplugin.h>


class AStylePart : public KDevPlugin
{
  Q_OBJECT

public:
   
  AStylePart(QObject *parent, const char *name, const QStringList &);
  ~AStylePart();


private slots:

  void activePartChanged(KParts::Part *part);
		  
  void beautifySource();
 
  void configWidget(KDialogBase *dlg);


private:

  KAction *_action;

};


#endif
