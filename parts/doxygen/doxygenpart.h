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
    /** Configuration dialog under "Project Options..." that alows
      * to specify doxygen configuration options and store them to
      * a configuration file. The file name is Doxyfile and it's stored
      * in the project root.
      */
    void projectConfigWidget(KDialogBase *dlg);

    /** run doxygen to generate API documentation */
    void slotDoxygen();

    /** clean the html API docs (delete the generated html files) */
    void slotDoxClean();

private:

    /** If a Doxygen configuration file doesn't exist, create one.
      * And copy some of the project settings to it.
      */
    void adjustDoxyfile();

    DoxygenDialog *m_dialog;
};

#endif
