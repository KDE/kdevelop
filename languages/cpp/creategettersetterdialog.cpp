//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "creategettersetterdialog.h"

#include "cppsupportpart.h"
#include <klineedit.h>
#include <qcheckbox.h>
#include <qregexp.h>

#include "creategettersetterconfiguration.h"

CreateGetterSetterDialog::CreateGetterSetterDialog( CppSupportPart* part, ClassDom aClass,
	                                                VariableDom aVar, QWidget *parent, const char *pName )
: CreateGetterSetterDialogBase( parent, pName ), m_part( part ), m_class( aClass ), m_var( aVar )
{
	QString name = aVar->name();
	setCaption( "Create methods for " + name );

	if ( aVar->type().startsWith( "const" ) && !aVar->type().endsWith( "*" ) )
	{
		m_chkSet->setChecked( false );
		m_chkSet->setEnabled( false );
	}

	CreateGetterSetterConfiguration* config = m_part->createGetterSetterConfiguration();
	if ( config == 0 )
		return ;

	QStringList prefixes = config->prefixVariable();
	unsigned int len = 0;

	QStringList::ConstIterator theend = prefixes.end(); //find longest fitting prefix and remove it
	for ( QStringList::ConstIterator ci = prefixes.begin(); ci != theend; ++ci )
	{
		if ( name.startsWith( *ci ) && ( *ci ).length() > len )
			len = ( *ci ).length();
	}

	if ( len > 0 )
		name.remove( 0, len );

	m_edtGet->setText( name );

	QString getName = name;
	if ( ! config->prefixGet().isEmpty() )
		getName[ 0 ] = getName[ 0 ].upper();

	QString setName = name;
	if ( ! config->prefixSet().isEmpty() )
		setName[ 0 ] = setName[ 0 ].upper();

	bool getIsChecked = config->isInlineGet();
	bool setIsChecked = config->isInlineSet();
	m_chkInlineGet->setChecked( getIsChecked );
	m_chkInlineSet->setChecked( setIsChecked );
	
	m_edtGet->setText( config->prefixGet() + getName );
	m_edtSet->setText( config->prefixSet() + setName );
}

void CreateGetterSetterDialog::accept( )
{
	CreateGetterSetterConfiguration * config = m_part->createGetterSetterConfiguration();
	
	if ( config == 0 )
		return ;
	
	if ( m_chkGet->isChecked() && !m_edtGet->text().isEmpty() )
		m_part->addMethod( m_class, m_edtGet->text(), m_var->type(), "", 
		                   CodeModelItem::Public, true, m_chkInlineGet->isChecked(),
		                   false, false, "\treturn " + m_var->name() + ";" );
	
	if ( m_chkSet->isChecked() && !m_edtSet->text().isEmpty() )
	{
		QString parameterStr;
		
		if ( m_var->type().endsWith( "*" ) )
		{
			parameterStr = m_var->type() + " " + config->parameterName();
		}
		else
		{
			QRegExp basicTypes( "((unsigned)?\\s*(char|byte|short|int|long))|double|float|bool" );
			if ( basicTypes.exactMatch( m_var->type() ) )
				parameterStr = m_var->type() + " " + config->parameterName();
			else
				parameterStr = "const " + m_var->type() + "& " + config->parameterName();
		}
		m_part->addMethod( m_class, m_edtSet->text(), "void", parameterStr, CodeModelItem::Public,
		                   false, m_chkInlineSet->isChecked(), false, false,
		                   "\t" + m_var->name() + " = " + config->parameterName() + ";" );
	}
	//@todo illegale eingaben nicht akzeptieren wie z.b. int& ...
	QDialog::accept();
}

/**
 * store current settings wether get/set methods should be created inline.
 * this is done everytime one changes this behaviour in the dialog.
 */
void CreateGetterSetterDialog::slotInlineChanged( )
{
	CreateGetterSetterConfiguration * config = m_part->createGetterSetterConfiguration();
	if ( config == 0 )
		return ;

	config->setInlineGet( m_chkInlineGet->isChecked() );
	config->setInlineSet( m_chkInlineSet->isChecked() );
	config->store();
}

#include "creategettersetterdialog.moc"
//kate: indent-mode csands; tab-width 4; space-indent off;
