#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './addfiledlgbase.ui'
**
** Created: Mon Apr 22 00:10:22 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "addfiledlgbase.h"

#include <qvariant.h>
#include <klineedit.h>
#include <ksqueezedtextlabel.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a AddFileDlgBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AddFileDlgBase::AddFileDlgBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "AddFileDlgBase" );
    resize( 445, 220 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 445, 220 ) );
    setMaximumSize( QSize( 445, 220 ) );
    setBaseSize( QSize( 0, 0 ) );
    setCaption( tr2i18n( "Add new created file to this target" ) );
    AddFileDlgBaseLayout = new QVBoxLayout( this, 11, 6, "AddFileDlgBaseLayout"); 

    targetBox = new QGroupBox( this, "targetBox" );
    targetBox->setTitle( tr2i18n( "Target information" ) );
    targetBox->setColumnLayout(0, Qt::Vertical );
    targetBox->layout()->setSpacing( 6 );
    targetBox->layout()->setMargin( 11 );
    targetBoxLayout = new QHBoxLayout( targetBox->layout() );
    targetBoxLayout->setAlignment( Qt::AlignTop );

    targetLayout = new QGridLayout( 0, 1, 1, 0, 6, "targetLayout"); 

    directoryLabel = new KSqueezedTextLabel( targetBox, "directoryLabel" );
    directoryLabel->setText( tr2i18n( "[TARGET DIRECTORY]" ) );

    targetLayout->addWidget( directoryLabel, 0, 1 );

    targetLabel = new QLabel( targetBox, "targetLabel" );
    targetLabel->setText( tr2i18n( "[TARGET NAME]" ) );

    targetLayout->addWidget( targetLabel, 1, 1 );

    directoryStaticLabel = new QLabel( targetBox, "directoryStaticLabel" );
    directoryStaticLabel->setText( tr2i18n( "<b>Directory:</b" ) );

    targetLayout->addWidget( directoryStaticLabel, 0, 0 );

    targetStaticLabel = new QLabel( targetBox, "targetStaticLabel" );
    targetStaticLabel->setText( tr2i18n( "<b>Target:</b>" ) );

    targetLayout->addWidget( targetStaticLabel, 1, 0 );
    targetBoxLayout->addLayout( targetLayout );
    AddFileDlgBaseLayout->addWidget( targetBox );

    fileGroupBox = new QGroupBox( this, "fileGroupBox" );
    fileGroupBox->setTitle( tr2i18n( "File information" ) );

    fileEdit = new KLineEdit( fileGroupBox, "fileEdit" );
    fileEdit->setGeometry( QRect( 80, 20, 320, 22 ) ); 
    fileEdit->setMinimumSize( QSize( 320, 0 ) );

    fileStaticLabel = new QLabel( fileGroupBox, "fileStaticLabel" );
    fileStaticLabel->setGeometry( QRect( 9, 20, 65, 22 ) ); 
    fileStaticLabel->setMinimumSize( QSize( 65, 0 ) );
    fileStaticLabel->setText( tr2i18n( "<b>File name:</b>" ) );

    templateCheckBox = new QCheckBox( fileGroupBox, "templateCheckBox" );
    templateCheckBox->setGeometry( QRect( 80, 50, 321, 20 ) ); 
    templateCheckBox->setText( tr2i18n( "&Use file template" ) );
    templateCheckBox->setChecked( TRUE );
    AddFileDlgBaseLayout->addWidget( fileGroupBox );

    buttonLayout = new QHBoxLayout( 0, 0, 6, "buttonLayout"); 
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonLayout->addItem( spacer );

    createButton = new QPushButton( this, "createButton" );
    createButton->setText( tr2i18n( "&Create" ) );
    buttonLayout->addWidget( createButton );

    cancelButton = new QPushButton( this, "cancelButton" );
    cancelButton->setText( tr2i18n( "C&ancel" ) );
    buttonLayout->addWidget( cancelButton );
    AddFileDlgBaseLayout->addLayout( buttonLayout );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
AddFileDlgBase::~AddFileDlgBase()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "addfiledlgbase.moc"
