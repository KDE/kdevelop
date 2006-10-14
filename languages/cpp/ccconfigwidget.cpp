/***************************************************************************
*   Copyright (C) 2001 by Daniel Engelschalt                              *
*   daniel.engelschalt@gmx.net                                            *
*   Copyright (C) 2004 Jonas Jacobi<j.jacobi@gmx.de>                      *
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
#include <qcolor.h>
#include <qlabel.h>

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
#include <klineedit.h>
#include <kdeversion.h> 
// kdevelop includes
#include <domutil.h>
#include <kdevcoderepository.h>
#include <kdevmainwindow.h>
#include <kdevcoderepository.h>
#include <catalog.h>
// std includes
#include <stdlib.h>

#include "cppsupportfactory.h"
#include "ccconfigwidget.h"
#include "qtbuildconfig.h"
#include "cppsupportpart.h"
#include "cppcodecompletionconfig.h"
#include "cppsplitheadersourceconfig.h"
#include "createpcsdialog.h"
#include "creategettersetterconfiguration.h"

using namespace std;


CCConfigWidget::CCConfigWidget( CppSupportPart* part, QWidget* parent, const char* name )
		: CCConfigWidgetBase( parent, name )
{
	m_pPart = part;
	connect( m_pPart->codeRepository(), SIGNAL( catalogRegistered( Catalog* ) ),
	         this, SLOT( catalogRegistered( Catalog* ) ) );
	connect( m_pPart->codeRepository(), SIGNAL( catalogUnregistered( Catalog* ) ),
	         this, SLOT( catalogUnregistered( Catalog* ) ) );

	connect( m_qtDir, SIGNAL(urlSelected(const QString &)), 
		 this, SLOT(isValidQtDir(const QString &)));
	connect( m_qtDir, SIGNAL(textChanged(const QString &)), 
		 this, SLOT(isValidQtDir(const QString &)));

	initGeneralTab( );
	initQtTab();
	initCodeCompletionTab( );
	initGetterSetterTab( );
	initSplitTab();
	inputCodeCompletion->setRange( 0, 2000, 100, false );
	inputArgumentsHint->setRange( 0, 2000, 100, false );
}

void CCConfigWidget::initGeneralTab( )
{
	QDomDocument dom = *m_pPart->projectDom();
	interface_suffix->setText( DomUtil::readEntry( dom, "/cppsupportpart/filetemplates/interfacesuffix", ".h" ) );
	implementation_suffix->setText( DomUtil::readEntry( dom, "/cppsupportpart/filetemplates/implementationsuffix", ".cpp" ) );

	KConfig *config = CppSupportFactory::instance() ->config();
	if ( config )
	{
		config->setGroup( "General" );
		m_switchShouldMatch->setChecked( config->readBoolEntry( "SwitchShouldMatch", true ) );
		m_showContextMenuExplosion->setChecked( config->readBoolEntry( "ShowContextMenuExplosion", false ) );
	}
}

CCConfigWidget::~CCConfigWidget( )
{}

void CCConfigWidget::accept( )
{
	saveFileTemplatesTab();
	saveQtTab();
	saveCodeCompletionTab();
	saveGetterSetterTab();
	saveSplitTab();
}

void CCConfigWidget::saveFileTemplatesTab( )
{
	QDomDocument dom = *m_pPart->projectDom();
	DomUtil::writeEntry( dom, "/cppsupportpart/filetemplates/interfacesuffix", interface_suffix->text() );
	DomUtil::writeEntry( dom, "/cppsupportpart/filetemplates/implementationsuffix", implementation_suffix->text() );

	KConfig *config = CppSupportFactory::instance() ->config();
	if ( config )
	{
		config->setGroup( "General" );
		config->writeEntry( "SwitchShouldMatch", m_switchShouldMatch->isChecked() );
		config->writeEntry( "ShowContextMenuExplosion", m_showContextMenuExplosion->isChecked() );
	}
}

void CCConfigWidget::initCodeCompletionTab( )
{
	advancedOptions->header() ->hide();

	CppCodeCompletionConfig* c = m_pPart->codeCompletionConfig();

	inputCodeCompletion->setValue( c->codeCompletionDelay() );
	inputArgumentsHint->setValue( c->argumentsHintDelay() );
	checkAutomaticCodeCompletion->setChecked( c->automaticCodeCompletion() );
	checkAutomaticArgumentsHint->setChecked( c->automaticArgumentsHint() );

	checkCompleteArgumentType->setChecked( c->processFunctionArguments() );
	checkCompleteReturnType->setChecked( c->processPrimaryTypes() );
	//checkShowOnlyAccessible->setChecked( c->showOnlyAccessibleItems() );

	editNamespaceAlias->setText( c->namespaceAliases() );
	checkBox18->setChecked( c->showEvaluationContextMenu() );
	checkShowTypeEvaluationInStatusBar->setChecked( c->statusBarTypeEvaluation() );
	checkShowCommentInArgumentHint->setChecked( c->showCommentWithArgumentHint() );
	/*
	switch( c->completionBoxItemOrder() ) {
	case CppCodeCompletionConfig::ByAccessLevel:
		radioGroupByAccess->setChecked( true );
		break;
	case CppCodeCompletionConfig::ByClass:
		radioGroupByClass->setChecked( true );
		break;
	case CppCodeCompletionConfig::ByAlphabet:
		radioGroupByAlphabet->setChecked( true );
		break;
	}*/

	QValueList<Catalog*> catalogs = m_pPart->codeRepository() ->registeredCatalogs();
	for ( QValueList<Catalog*>::Iterator it = catalogs.begin(); it != catalogs.end(); ++it )
	{
		Catalog* c = *it;
		QFileInfo dbInfo( c->dbName() );
		QCheckListItem* item = new QCheckListItem( advancedOptions, KURL::decode_string( dbInfo.baseName() ), QCheckListItem::CheckBox );
		item->setOn( c->enabled() );

		m_catalogs[ item ] = c;
	}
	
	checkPreprocessIncludedHeaders->setChecked( c->preProcessAllHeaders() );
	checkListGlobalItems->setChecked( c->alwaysIncludeNamespaces() );
	checkParseMissingHeaders->setChecked( c->parseMissingHeaders() );
	checkResolveIncludePaths->setChecked( c->resolveIncludePaths() );
	checkParseMissingHeaders->hide();
	checkResolveIncludePaths->hide();
	editIncludePaths->setText( c->customIncludePaths() );
}

void CCConfigWidget::saveCodeCompletionTab( )
{
	CppCodeCompletionConfig * c = m_pPart->codeCompletionConfig();

	c->setCodeCompletionDelay( inputCodeCompletion->value() );
	c->setArgumentsHintDelay( inputArgumentsHint->value() );

	c->setAutomaticCodeCompletion( checkAutomaticCodeCompletion->isChecked() );
	c->setAutomaticArgumentsHint( checkAutomaticArgumentsHint->isChecked() );

	c->setProcessFunctionArguments( checkCompleteArgumentType->isChecked() );
	c->setProcessPrimaryTypes( checkCompleteReturnType->isChecked() );
	//c->setShowOnlyAccessibleItems( checkShowOnlyAccessible->isChecked() );
	
	c->setNamespaceAliases( editNamespaceAlias->text() );
	c->setShowEvaluationContextMenu( checkBox18->isChecked() );
	c->setStatusBarTypeEvaluation( checkShowTypeEvaluationInStatusBar->isChecked() );
	c->setShowCommentWithArgumentHint( checkShowCommentInArgumentHint->isChecked() );

	/*if( radioGroupByAccess->isChecked() )
		c->setCompletionBoxItemOrder( CppCodeCompletionConfig::ByAccessLevel );

	if( radioGroupByClass->isChecked() )
		c->setCompletionBoxItemOrder( CppCodeCompletionConfig::ByClass );

	if( radioGroupByAlphabet->isChecked() )
		c->setCompletionBoxItemOrder( CppCodeCompletionConfig::ByAlphabet );*/

	for ( QMap<QCheckListItem*, Catalog*>::Iterator it = m_catalogs.begin(); it != m_catalogs.end(); ++it )
	{
		it.data() ->setEnabled( it.key() ->isOn() );
	}

	c->setPreProcessAllHeaders( checkPreprocessIncludedHeaders->isChecked() );
	c->setAlwaysIncludeNamespaces( checkListGlobalItems->isChecked() );
	c->setParseMissingHeaders( checkParseMissingHeaders->isChecked() );
	c->setResolveIncludePaths( checkResolveIncludePaths->isChecked() );
	c->setCustomIncludePaths( editIncludePaths->text() );
	
	c->store();
}

void CCConfigWidget::slotNewPCS( )
{
	CreatePCSDialog dlg( m_pPart, m_pPart->mainWindow() ->main() );
	dlg.importerListView->setFocus();
	dlg.exec();
}

void CCConfigWidget::slotRemovePCS()
{
	if ( !advancedOptions->selectedItem() )
		return ;

	QString db = advancedOptions->selectedItem() ->text( 0 );
	QString question = i18n( "Are you sure you want to remove the \"%1\" database?" ).arg( db );

	KStandardDirs *dirs = m_pPart->instance() ->dirs();
	QString dbName = dirs->saveLocation( "data", "kdevcppsupport/pcs" ) + KURL::encode_string_no_slash( db ) + ".db";
	
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)

	if ( KMessageBox::Continue == KMessageBox::warningContinueCancel( 0, question, i18n( "Remove Database" ), KStdGuiItem::del() ) )
#else

	if ( KMessageBox::Continue == KMessageBox::warningContinueCancel( 0, question, i18n( "Remove Database" ),
			KGuiItem( i18n( "&Delete" ), "editdelete", i18n( "Delete item(s)" ) ) ) )
#endif

	{
		m_pPart->removeCatalog( dbName );
	}
}

void CCConfigWidget::catalogRegistered( Catalog * c )
{
	QFileInfo dbInfo( c->dbName() );
	QCheckListItem* item = new QCheckListItem( advancedOptions, KURL::decode_string( dbInfo.baseName() ), QCheckListItem::CheckBox );
	item->setOn( c->enabled() );

	m_catalogs[ item ] = c;
}

void CCConfigWidget::catalogUnregistered( Catalog * c )
{
	for ( QMap<QCheckListItem*, Catalog*>::Iterator it = m_catalogs.begin(); it != m_catalogs.end(); ++it )
	{
		if ( it.data() == c )
		{
			QCheckListItem * item = it.key();
			delete( item );
			m_catalogs.remove( it );
			break;
		}
	}
}

void CCConfigWidget::initGetterSetterTab( )
{
	CreateGetterSetterConfiguration * config = m_pPart->createGetterSetterConfiguration();

	if ( config == 0 )
		return ;

	m_edtGet->setText( config->prefixGet() );
	m_edtSet->setText( config->prefixSet() );
	m_edtRemovePrefix->setText( config->prefixVariable().join( "," ) );
	m_edtParameterName->setText( config->parameterName() );

	slotGetterSetterValuesChanged();
}

void CCConfigWidget::slotGetterSetterValuesChanged( )
{
	bool hasError = false;
	if ( m_edtParameterName->text().isEmpty() )
	{
		m_lblParameterName->setPaletteForegroundColor( QColor( "red" ) );
		m_edtExampleGet->setText( "error, missing parametername" );
		m_edtExampleSet->setText( "error, missing parametername" );
		hasError = true;
	}

	QString name = m_edtVariableName->text();
	if ( name.isEmpty() )
	{
		m_lblVariableName->setPaletteForegroundColor( QColor( "red" ) );
		m_edtExampleGet->setText( "error, missing variablename" );
		m_edtExampleSet->setText( "error, missing variablename" );
		hasError = true;
	}

	if ( hasError )
	{
		m_edtExampleGet->setPaletteForegroundColor( QColor( "red" ) );
		m_edtExampleSet->setPaletteForegroundColor( QColor( "red" ) );

		return ;
	}
	else
	{
		m_lblVariableName->setPaletteForegroundColor( QColor( "black" ) );
		m_lblParameterName->setPaletteForegroundColor( QColor( "black" ) );
		m_edtExampleGet->setPaletteForegroundColor( QColor( "black" ) );
		m_edtExampleSet->setPaletteForegroundColor( QColor( "black" ) );
	}

	QStringList prefixes = QStringList::split( ",", m_edtRemovePrefix->text().replace( " ", "" ) );
	unsigned int len = 0;
	QStringList::ConstIterator theend = prefixes.end();
	for ( QStringList::ConstIterator ci = prefixes.begin(); ci != theend; ++ci )
	{
		if ( name.startsWith( *ci ) && ( *ci ).length() > len )
			len = ( *ci ).length();
	}

	if ( len > 0 )
		name.remove( 0, len );

	QString getName = name;
	if ( !m_edtGet->text().isEmpty() )
	{
		getName[ 0 ] = getName[ 0 ].upper();
		getName.prepend( m_edtGet->text() );
	}

	QString setName = name;
	if ( !m_edtSet->text().isEmpty() )
	{
		setName[ 0 ] = setName[ 0 ].upper();
		setName.prepend( m_edtSet->text() );
	}

	m_edtExampleGet->setText( "string " + getName + "() const;" );
	m_edtExampleSet->setText( "void " + setName + "(const string& " + m_edtParameterName->text() + ");" );
}

void CCConfigWidget::saveGetterSetterTab( )
{
	if ( m_edtParameterName->text().isEmpty() || m_edtGet->text() == m_edtSet->text() )
		return ;

	CreateGetterSetterConfiguration* config = m_pPart->createGetterSetterConfiguration();
	if ( config == 0 )
		return ;

	config->setPrefixGet( m_edtGet->text() );
	config->setPrefixSet( m_edtSet->text() );
	config->setPrefixVariable( QStringList::split( ",", m_edtRemovePrefix->text().replace( " ", "" ) ) );
	config->setParameterName( m_edtParameterName->text() );
	config->store();
}

void CCConfigWidget::initSplitTab( )
{
	CppSplitHeaderSourceConfig * config = m_pPart->splitHeaderSourceConfig();
	
	if ( config == 0 )
		return ;
	
	m_splitEnable->setChecked( config->splitEnabled() );
	m_splitSync->setChecked( config->autoSync() );
	
	QString o = config->orientation();
	m_splitVertical->setChecked( o == "Vertical" );
	m_splitHorizontal->setChecked( o == "Horizontal" );
}

void CCConfigWidget::saveSplitTab( )
{
	CppSplitHeaderSourceConfig * config = m_pPart->splitHeaderSourceConfig();
	
	if ( config == 0 )
		return ;
	
	config->setSplitEnable( m_splitEnable->isChecked() );
	config->setAutoSync( m_splitSync->isChecked() );
	
	if ( m_splitVertical->isChecked() )
		config->setOrientation( "Vertical" );
	else if ( m_splitHorizontal->isChecked() )
		config->setOrientation( "Horizontal" );
	
	config->store();
}

void CCConfigWidget::initQtTab()
{
	m_qtDir->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
	
	QtBuildConfig* c = m_pPart->qtBuildConfig();

	m_qtUsed->setChecked( c->isUsed() );
	if( c->version() == 4 )
	{
		m_versionQt4->setChecked( true );
		m_kdevembedded->setEnabled( false );
		m_kdevexternal->setEnabled( false );
		m_qtStyleVersion4->setEnabled( true );
	}
	else
	{
		m_versionQt3->setChecked( true );
		m_kdevembedded->setEnabled( true );
		m_kdevexternal->setEnabled( true );
		m_qtStyleVersion4->setEnabled( false );
	}
	if( c->includeStyle() == 4 )
	{
		m_qtStyleVersion4->setChecked( true );
	}else
	{
		m_qtStyleVersion3->setChecked( true );
	}
	m_qtDir->setURL( c->root() );
	isValidQtDir(m_qtDir->url());
	if ( c->designerIntegration() == "EmbeddedKDevDesigner" )
	{
		m_kdevembedded->setChecked( true );
	}
	else if ( c->designerIntegration() == "ExternalKDevDesigner" )
	{
		m_kdevexternal->setChecked( true );
	}else
	{
		m_qtdesigner->setChecked( true );
	}
}

void CCConfigWidget::saveQtTab()
{
	QtBuildConfig* c = m_pPart->qtBuildConfig();
	
	c->setUsed( m_qtUsed->isChecked() );
	if( m_versionQt4->isChecked() )
	{
		c->setVersion( 4 );
	}
	else
	{
		c->setVersion( 3 );
	}
	if( m_qtStyleVersion4->isChecked() )
	{
		c->setIncludeStyle( 4 );
	}else
	{
		c->setIncludeStyle( 3 );
	}
	c->setRoot( m_qtDir->url() );
	if( m_kdevembedded->isChecked() )
	{
		c->setDesignerIntegration( "EmbeddedKDevDesigner" );
	}
	else if ( m_kdevexternal->isChecked() )
	{
		c->setDesignerIntegration( "ExternalKDevDesigner" );
	}else
	{
		c->setDesignerIntegration( "ExternalDesigner" );
	}
	c->store();
}

void CCConfigWidget::isValidQtDir( const QString &dir )
{
	if ( !QFile::exists( dir + "/include/qt.h" ) && !QFile::exists( dir + "/include/Qt/qglobal.h" ) )
	{
		m_qtDir->lineEdit()->setPaletteForegroundColor(QColor("#ff0000"));
	}else
	{
		m_qtDir->lineEdit()->unsetPalette();
	}
}

void CCConfigWidget::toggleQtVersion( bool )
{
  if ( m_versionQt3->isChecked() )
  {
    m_qtStyleVersion4->setEnabled( false );
    m_qtStyleVersion3->setChecked( true );
    m_kdevembedded->setEnabled( true );
    m_kdevexternal->setEnabled( true );
  }
  if ( m_versionQt4->isChecked() )
  {
    m_qtStyleVersion4->setEnabled( true );
    m_qtdesigner->setChecked( true );
    m_kdevembedded->setEnabled( false );
    m_kdevexternal->setEnabled( false );
  }
}

#include "ccconfigwidget.moc"

//kate: indent-mode csands; tab-width 4; space-indent off;
