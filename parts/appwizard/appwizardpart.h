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

#ifndef _APPWIZARDPART_H_
#define _APPWIZARDPART_H_

#include <qguardedptr.h>
#include "kdevpart.h"

class AppWizardDialog;


class AppWizardPart : public KDevPart
{
    Q_OBJECT

public:
    AppWizardPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~AppWizardPart();

private slots:
    void slotNewProject();
    
private:
    AppWizardDialog *m_dialog;
};

#endif
