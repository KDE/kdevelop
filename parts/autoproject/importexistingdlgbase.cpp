#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './importexistingdlgbase.ui'
**
** Created: Wed Apr 24 21:00:15 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "importexistingdlgbase.h"

#include <qvariant.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a ImportExistingDlgBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ImportExistingDlgBase::ImportExistingDlgBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "ImportExistingDlgBase" );
    resize( 600, 450 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 600, 450 ) );
    setCaption( tr2i18n( "ImportExistingDlgBase" ) );
    ImportExistingDlgBaseLayout = new QVBoxLayout( this, 11, 6, "ImportExistingDlgBaseLayout"); 

    viewLayout = new QHBoxLayout( 0, 0, 6, "viewLayout"); 

    sourceGroupBox = new QGroupBox( this, "sourceGroupBox" );
    sourceGroupBox->setMinimumSize( QSize( 330, 0 ) );
    sourceGroupBox->setTitle( tr2i18n( "Source directory" ) );
    sourceGroupBox->setColumnLayout(0, Qt::Vertical );
    sourceGroupBox->layout()->setSpacing( 6 );
    sourceGroupBox->layout()->setMargin( 11 );
    sourceGroupBoxLayout = new QVBoxLayout( sourceGroupBox->layout() );
    sourceGroupBoxLayout->setAlignment( Qt::AlignTop );
    viewLayout->addWidget( sourceGroupBox );

    destGroupBox = new QGroupBox( this, "destGroupBox" );
    destGroupBox->setTitle( tr2i18n( "[DESTINATION DIRECTORY]" ) );
    destGroupBox->setColumnLayout(0, Qt::Vertical );
    destGroupBox->layout()->setSpacing( 6 );
    destGroupBox->layout()->setMargin( 11 );
    destGroupBoxLayout = new QVBoxLayout( destGroupBox->layout() );
    destGroupBoxLayout->setAlignment( Qt::AlignTop );
    viewLayout->addWidget( destGroupBox );
    ImportExistingDlgBaseLayout->addLayout( viewLayout );

    buttonLayout = new QHBoxLayout( 0, 0, 6, "buttonLayout"); 

    helpButton = new QPushButton( this, "helpButton" );
    helpButton->setText( tr2i18n( "Help" ) );
    buttonLayout->addWidget( helpButton );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonLayout->addItem( spacer );

    okButton = new QPushButton( this, "okButton" );
    okButton->setText( tr2i18n( "OK" ) );
    buttonLayout->addWidget( okButton );

    cancelButton = new QPushButton( this, "cancelButton" );
    cancelButton->setText( tr2i18n( "Cancel" ) );
    buttonLayout->addWidget( cancelButton );
    ImportExistingDlgBaseLayout->addLayout( buttonLayout );

    // signals and slots connections
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );

    // tab order
    setTabOrder( okButton, cancelButton );
    setTabOrder( cancelButton, helpButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ImportExistingDlgBase::~ImportExistingDlgBase()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "importexistingdlgbase.moc"
