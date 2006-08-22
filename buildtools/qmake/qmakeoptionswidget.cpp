/***************************************************************************
*   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
*   jsgaarde@tdcspace.dk                                                  *
*   Copyright (C) 2002-2003 by Alexander Dymo                             *
*   cloudtemple@mksat.net                                                 *
*   Copyright (C) 2003 by Thomas Hasart                                   *
*   thasart@gmx.de                                                        *
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qmakeoptionswidget.h"

#include <qcheckbox.h>
#include <kcombobox.h>
#include <qbuttongroup.h>

#include <domutil.h>
#include <kdebug.h>

QMakeOptionsWidget::QMakeOptionsWidget( QDomDocument &dom, const QString &configGroup,
                                      QWidget *parent, const char *name )
        : QMakeOptionsWidgetBase( parent, name ),
        m_dom( dom ), m_configGroup( configGroup )
{
    groupBehaviour->setButton( DomUtil::readIntEntry( dom, configGroup+"/qmake/savebehaviour", 2) );
}


QMakeOptionsWidget::~QMakeOptionsWidget()
{}


void QMakeOptionsWidget::accept()
{
    DomUtil::writeIntEntry( m_dom, m_configGroup + "/qmake/savebehaviour", groupBehaviour->selectedId() );
}

#include "qmakeoptionswidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
