/*
 *  Copyright (C) 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_UICHOOSER_H__
#define __KDEVPART_UICHOOSER_H__


class KDialogBase;


#include <kdevpart.h>


class UIChooserPart : public KDevPart
{
  Q_OBJECT

public:
   
  UIChooserPart(KDevApi *api, QObject *parent=0, const char *name=0);
  ~UIChooserPart();

  
private slots:

  void configWidget(KDialogBase *dlg);
  
};


#endif
