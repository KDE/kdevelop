/*
 *  Copyright (C) 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_EDITORCHOOSER_H__
#define __KDEVPART_EDITORCHOOSER_H__


class KDialogBase;


#include <kdevpart.h>


class EditorChooserPart : public KDevPart
{
  Q_OBJECT

public:
   
  EditorChooserPart(KDevApi *api, QObject *parent=0, const char *name=0);
  ~EditorChooserPart();

  
private slots:

  void configWidget(KDialogBase *dlg);
  
};


#endif
