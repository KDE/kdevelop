/*
 *  Copyright (C) 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_EDITORCHOOSER_H__
#define __KDEVPART_EDITORCHOOSER_H__


class KDialogBase;


#include <kdevplugin.h>


class EditorChooserPart : public KDevPlugin
{
  Q_OBJECT

public:
   
  EditorChooserPart(QObject *parent, const char *name, const QStringList &);
  ~EditorChooserPart();

  
private slots:

  void configWidget(KDialogBase *dlg);
  
};


#endif
