#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './logform.ui'
**
** Created: Wed Jun 18 16:17:52 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "logform.h"

#include <qvariant.h>
#include <kdialog.h>
#include <qtextbrowser.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include "./logform.ui.h"
/* 
 *  Constructs a LogForm as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
LogForm::LogForm( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "LogForm" );
    LogFormLayout = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint(), "LogFormLayout"); 

    contents = new QTextBrowser( this, "contents" );
    LogFormLayout->addWidget( contents );
    languageChange();
    resize( QSize(801, 642).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
LogForm::~LogForm()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void LogForm::languageChange()
{
    setCaption( tr2i18n( "Log" ) );
    contents->setText( tr2i18n( "none" ) );
}

#include "logform.moc"
