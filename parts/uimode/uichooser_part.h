/*
 *  Copyright (C) 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_UICHOOSER_H__
#define __KDEVPART_UICHOOSER_H__


class KDialogBase;


#include <kdevplugin.h>


class UIChooserPart : public KDevPlugin
{
  Q_OBJECT

public:
   
  UIChooserPart(QObject *parent, const char *name, const QStringList &);
  ~UIChooserPart();

  
private slots:

  void configWidget(KDialogBase *dlg);
  
};


#endif
