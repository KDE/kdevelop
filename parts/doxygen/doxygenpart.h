/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOXYGENPART_H_
#define _DOXYGENPART_H_

#include <qguardedptr.h>
#include <kdialogbase.h>
#include "kdevpart.h"

class DoxygenDialog;


class DoxygenPart : public KDevPart
{
    Q_OBJECT

public:
    DoxygenPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~DoxygenPart();

private slots:
   void projectConfigWidget(KDialogBase *dlg);
    void slotDoxygen();
    
private:
    DoxygenDialog *m_dialog;
};

#endif
