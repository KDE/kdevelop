/***************************************************************************
                             customizedlg.h
                             ----------------------
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef CUSTOMIZEDLG_H
#define CUSTOMIZEDLG_H

#include <qtabwidget.h>
#include <kdialogbase.h>


class CustomizeDialog : public KDialogBase
{
public: 
    CustomizeDialog( QWidget *parent, const char *name=0 );
    ~CustomizeDialog();
    QFrame *addPage(const QString &title)
      { return KDialogBase::addPage(title); }
};

#endif
