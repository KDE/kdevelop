/***************************************************************************
 *   Copyright (C) 2001 by Daniel Engelschalt                              *
 *   daniel.engelschalt@gmx.net                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// qt includes
#include <qtabwidget.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qmultilineedit.h>
#include <qslider.h>
#include <qheader.h>

// kde includes
#include <kdevproject.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kfileitem.h>
#include <kurlrequester.h>
#include <klistview.h>
#include <knuminput.h>
#include <kmainwindow.h>

// kdevelop includes
#include <domutil.h>
#include <kdevcoderepository.h>
#include <kdevmainwindow.h>
#include <catalog.h>

#include "ccconfigwidget.h"
#include "cppsupportpart.h"
#include "cppcodecompletionconfig.h"
#include "createpcsdialog.h"

using namespace std;


CCConfigWidget::CCConfigWidget( CppSupportPart* part, QWidget* parent, const char* name )
    : CCConfigWidgetBase( parent, name )
{
    m_pPart = part;
    connect( m_pPart->codeRepository(), SIGNAL(catalogRegistered(Catalog* )), this, SLOT(catalogRegistered(Catalog* )) );
    connect( m_pPart->codeRepository(), SIGNAL(catalogUnregistered(Catalog* )), this, SLOT(catalogUnregistered(Catalog* )) );

    initFileTemplatesTab( );
    initCodeCompletionTab( );

    inputCodeCompletion->setRange( 0, 2000, 100 );
    inputArgumentsHint->setRange( 0, 2000, 100 );
}

void CCConfigWidget::initFileTemplatesTab( )
{
    QDomDocument dom = *m_pPart->projectDom();
    interface_suffix->setText(DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/interfacesuffix", ".h"));
    implementation_suffix->setText(DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/implementationsuffix", ".cpp"));
}

CCConfigWidget::~CCConfigWidget( )
{
}

void CCConfigWidget::accept( )
{
    saveFileTemplatesTab();
    saveCodeCompletionTab();
}

void CCConfigWidget::saveFileTemplatesTab( )
{
    QDomDocument dom = *m_pPart->projectDom();
    DomUtil::writeEntry(dom, "/cppsupportpart/filetemplates/interfacesuffix",interface_suffix->text());
    DomUtil::writeEntry(dom, "/cppsupportpart/filetemplates/implementationsuffix",implementation_suffix->text());
}

void CCConfigWidget::initCodeCompletionTab( )
{
    advancedOptions->header()->hide();

    CppCodeCompletionConfig* c = m_pPart->codeCompletionConfig();

    inputCodeCompletion->setValue( c->codeCompletionDelay() );
    inputArgumentsHint->setValue( c->argumentsHintDelay() );
    checkAutomaticCodeCompletion->setChecked( c->automaticCodeCompletion() );
    checkAutomaticArgumentsHint->setChecked( c->automaticArgumentsHint() );

    QListViewItem* codeCompletionOptions = new QListViewItem( advancedOptions, i18n("Code Completion Options") );
    codeCompletionOptions->setExpandable( true );

    //QListViewItem* argumentsHintOptions = new QListViewItem( advancedOptions, i18n("Arguments Hint Options") );

    m_includeGlobalFunctions = new QCheckListItem( codeCompletionOptions, i18n("Include Global Functions"), QCheckListItem::CheckBox );
    m_includeGlobalFunctions->setOn( c->includeGlobalFunctions() );

    m_includeTypes = new QCheckListItem( codeCompletionOptions, i18n("Include Types"), QCheckListItem::CheckBox );
    m_includeTypes->setOn( c->includeTypes() );

    m_includeEnums = new QCheckListItem( codeCompletionOptions, i18n("Include Enums"), QCheckListItem::CheckBox );
    m_includeEnums->setOn( c->includeEnums() );

    m_includeTypedefs = new QCheckListItem( codeCompletionOptions, i18n("Include Typedefs"), QCheckListItem::CheckBox );
    m_includeTypedefs->setOn( c->includeTypedefs() );
    
    m_pcsOptions = new QListViewItem( advancedOptions, i18n("Persistant Class Store") );
    QValueList<Catalog*> catalogs = m_pPart->codeRepository()->registeredCatalogs();
    for( QValueList<Catalog*>::Iterator it=catalogs.begin(); it!=catalogs.end(); ++it )
    {
	Catalog* c = *it;
	QFileInfo dbInfo( c->dbName() );
	QCheckListItem* item = new QCheckListItem( m_pcsOptions, dbInfo.baseName(), QCheckListItem::CheckBox );
	item->setOn( c->enabled() );
	
	m_catalogs[ item ] = c;
    }
}

void CCConfigWidget::saveCodeCompletionTab( )
{
    CppCodeCompletionConfig* c = m_pPart->codeCompletionConfig();

    c->setCodeCompletionDelay( inputCodeCompletion->value() );
    c->setArgumentsHintDelay( inputArgumentsHint->value() );

    c->setAutomaticCodeCompletion( checkAutomaticCodeCompletion->isChecked() );
    c->setAutomaticArgumentsHint( checkAutomaticArgumentsHint->isChecked() );

    c->setIncludeGlobalFunctions( m_includeGlobalFunctions->isOn() );
    c->setIncludeTypes( m_includeTypes->isOn() );
    c->setIncludeEnums( m_includeEnums->isOn() );
    c->setIncludeTypedefs( m_includeTypedefs->isOn() );

    for( QMap<QCheckListItem*, Catalog*>::Iterator it=m_catalogs.begin(); it!=m_catalogs.end(); ++it )
    {
	it.data()->setEnabled( it.key()->isOn() );
    }
    
    c->store();
}

void CCConfigWidget::slotNewPCS( )
{
    CreatePCSDialog dlg( m_pPart, m_pPart->mainWindow()->main() );
    dlg.importerListView->setFocus();
    dlg.exec();
}

void CCConfigWidget::catalogRegistered( Catalog * c )
{
    QFileInfo dbInfo( c->dbName() );
    QCheckListItem* item = new QCheckListItem( m_pcsOptions, dbInfo.baseName(), QCheckListItem::CheckBox );
    item->setOn( c->enabled() );

    m_catalogs[ item ] = c;
}

void CCConfigWidget::catalogUnregistered( Catalog * c )
{
    for( QMap<QCheckListItem*, Catalog*>::Iterator it=m_catalogs.begin(); it!=m_catalogs.end(); ++it )
    {
	if( it.data() == c ){
	    QCheckListItem* item = it.key();
	    delete( item );
	    m_catalogs.remove( it );
	    break;
	}
    }
}

#include "ccconfigwidget.moc"
