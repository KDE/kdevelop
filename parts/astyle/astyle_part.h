/*
 *  Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>   
 */
 

#ifndef __KDEVPART_ASTYLE_H__
#define __KDEVPART_ASTYLE_H__

class KDialogBase;
#include <kaction.h>

#include <kdevpart.h>


#ifdef NEW_EDITOR
#include "keditor/editor.h"
#else
namespace KEditor { class Document; }
#endif


class AStylePart : public KDevPart
{
  Q_OBJECT

public:
   
  AStylePart(KDevApi *api, QObject *parent=0, const char *name=0);
  ~AStylePart();


private slots:

  void activePartChanged(KParts::Part *newPart);
  void documentActivated(KEditor::Document *doc);
		  
  void beautifySource();
 
  void configWidget(KDialogBase *dlg);


private:

  KAction *_action;

};


#endif
