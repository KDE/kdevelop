/***************************************************************************
                          phpcodecompletion.h  -  description
                             -------------------
    begin                : Tue Jul 17 2001
    copyright            : (C) 2001 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PHPCODECOMPLETION_H
#define PHPCODECOMPLETION_H

#include <qobject.h>
#include "keditor/editor.h"
#include "keditor/edit_iface.h"

/**
 *@author Sandy Meier
 */

class PHPCodeCompletion : public QObject {
  Q_OBJECT

public: 
  PHPCodeCompletion(KEditor::Editor* editor);
  ~PHPCodeCompletion();
protected slots:  
  void documentActivated(KEditor::Document* doc);
  void cursorPositionChanged(KEditor::Document *doc, int line, int col);

 private:
  KEditor::Editor* m_editor;
};

#endif
