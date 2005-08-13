/***************************************************************************
 *   Copyright (C) 2002 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ABBREVCONFIGWIDGET_H_
#define _ABBREVCONFIGWIDGET_H_

#include "abbrevconfigwidgetbase.h"
#include "abbrevpart.h"


class AbbrevConfigWidget : public AbbrevConfigWidgetBase
{
    Q_OBJECT
    
public:
    AbbrevConfigWidget(AbbrevPart *part, QWidget *parent=0, const char *name=0);
    ~AbbrevConfigWidget();

public slots:
    void accept();
    
private:
    virtual void addTemplate();
    virtual void removeTemplate();
    virtual void selectionChanged();
    virtual void codeChanged();

    AbbrevPart *m_part;
};

#endif
