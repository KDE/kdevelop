/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DOCLINEEDIT_H
#define DOCLINEEDIT_H

#include <klineedit.h>

class DocLineEdit: public KLineEdit{
    Q_OBJECT
public:
    DocLineEdit( const QString &string, QWidget *parent, const char *name = 0 )
        :KLineEdit(string, parent, name) {}
    DocLineEdit( QWidget *parent=0, const char *name=0 )
        :KLineEdit(parent, name) {}

signals:
    void upPressed();
    void downPressed();
    void pgupPressed();
    void pgdownPressed();
    void homePressed();
    void endPressed();

protected:
    virtual void keyReleaseEvent ( QKeyEvent * e );
};

#endif
