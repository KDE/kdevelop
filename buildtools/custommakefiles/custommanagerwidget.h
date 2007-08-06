/***************************************************************************
 *   Copyright (C) 2007 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CUSTOMMANAGERWIDGET_H
#define CUSTOMMANAGERWIDGET_H

#include "custommanagerwidgetbase.h"


#include <qdom.h>

class CustomProjectPart;
class KEditListBox;

class CustomManagerWidget : public CustomManagerWidgetBase
{
Q_OBJECT

public:
    CustomManagerWidget( CustomProjectPart* part, QWidget* parent );
    ~CustomManagerWidget();
public slots:
    void checkUrl(const QString& url);
    void accept();
private:
    CustomProjectPart* m_part;
    QDomDocument& m_dom;
    KEditListBox* m_blacklistBox;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
