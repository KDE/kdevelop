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
#include "kdevplugin.h"

class DoxygenDialog;


class DoxygenPart : public KDevPlugin
{
    Q_OBJECT

public:
    DoxygenPart( QObject *parent, const char *name, const QStringList & );
    ~DoxygenPart();

private slots:
   void projectConfigWidget(KDialogBase *dlg);
    void slotDoxygen();
    
private:
    DoxygenDialog *m_dialog;
};

#endif
