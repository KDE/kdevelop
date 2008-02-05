/***************************************************************************
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
#include <kurlrequester.h>

#include <domutil.h>
#include <kdebug.h>

QMakeOptionsWidget::QMakeOptionsWidget( const QString& projectdir, QDomDocument &dom, const QString &configGroup,
                                      QWidget *parent, const char *name )
        : QMakeOptionsWidgetBase( parent, name ),
        m_dom( dom ), m_configGroup( configGroup ), m_projectDir( projectdir )
{
    groupBehaviour->setButton( DomUtil::readIntEntry( dom, configGroup+"/qmake/savebehaviour", 2) );
    checkReplacePaths->setChecked( DomUtil::readBoolEntry( dom, configGroup+"/qmake/replacePaths", false ) );
    checkDisableDefaultOpts->setChecked( DomUtil::readBoolEntry( dom, configGroup+"/qmake/disableDefaultOpts", true ) );
    checkFilenamesOnly->setChecked( DomUtil::readBoolEntry( dom, configGroup+"/qmake/enableFilenamesOnly", false ) );
    showVariablesInTree->setChecked( DomUtil::readBoolEntry( dom, configGroup+"/qmake/showVariablesInTree", true ) );
    checkShowParseErrors->setChecked( DomUtil::readBoolEntry( dom,
            configGroup+"/qmake/showParseErrors", true ) );
    qmakeProjectFile->setURL( DomUtil::readEntry( dom, configGroup+"/qmake/projectfile", "" ) );
    qmakeProjectFile->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
    qmakeProjectFile->setFilter( "*.pro *.pri" );
    if( qmakeProjectFile->url().isEmpty() )
    {
        qmakeProjectFile->setURL( projectdir );
    }
}


QMakeOptionsWidget::~QMakeOptionsWidget()
{}


void QMakeOptionsWidget::accept()
{
    DomUtil::writeIntEntry( m_dom, m_configGroup + "/qmake/savebehaviour", groupBehaviour->selectedId() );
    DomUtil::writeBoolEntry( m_dom, m_configGroup + "/qmake/replacePaths", checkReplacePaths->isChecked() );
    DomUtil::writeBoolEntry( m_dom, m_configGroup + "/qmake/disableDefaultOpts", checkDisableDefaultOpts->isChecked() );
    DomUtil::writeBoolEntry( m_dom, m_configGroup + "/qmake/enableFilenamesOnly", checkFilenamesOnly->isChecked() );
    DomUtil::writeBoolEntry( m_dom, m_configGroup + "/qmake/showVariablesInTree", showVariablesInTree->isChecked() );
    DomUtil::writeBoolEntry( m_dom, m_configGroup + "/qmake/showParseErrors", checkShowParseErrors->isChecked() );
    QString projfile = qmakeProjectFile->url();
    if( projfile != m_projectDir && QFileInfo( projfile ).isFile() && ( projfile.endsWith( ".pro" ) || projfile.endsWith( ".pri" ) ) )
        DomUtil::writeEntry( m_dom, m_configGroup + "/qmake/projectfile", projfile );
}

#include "qmakeoptionswidget.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
