#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './addexistingdlgbase.ui'
**
** Created: Sam Dez 21 17:57:15 2002
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "addexistingdlgbase.h"

#include <qvariant.h>
#include <kprogress.h>
#include <ksqueezedtextlabel.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a AddExistingDlgBase as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AddExistingDlgBase::AddExistingDlgBase( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )

{
    if ( !name )
	setName( "AddExistingDlgBase" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    AddExistingDlgBaseLayout = new QVBoxLayout( this, 11, 6, "AddExistingDlgBaseLayout"); 

    infoGroupBox = new QGroupBox( this, "infoGroupBox" );
    infoGroupBox->setMaximumSize( QSize( 32767, 32767 ) );
    infoGroupBox->setFrameShape( QGroupBox::Box );
    infoGroupBox->setFrameShadow( QGroupBox::Sunken );
    infoGroupBox->setColumnLayout(0, Qt::Vertical );
    infoGroupBox->layout()->setSpacing( 6 );
    infoGroupBox->layout()->setMargin( 11 );
    infoGroupBoxLayout = new QHBoxLayout( infoGroupBox->layout() );
    infoGroupBoxLayout->setAlignment( Qt::AlignTop );

    infoLayout1 = new QVBoxLayout( 0, 0, 6, "infoLayout1"); 

    directoryStaticLabel = new QLabel( infoGroupBox, "directoryStaticLabel" );
    directoryStaticLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, directoryStaticLabel->sizePolicy().hasHeightForWidth() ) );
    QFont directoryStaticLabel_font(  directoryStaticLabel->font() );
    directoryStaticLabel_font.setBold( TRUE );
    directoryStaticLabel->setFont( directoryStaticLabel_font ); 
    infoLayout1->addWidget( directoryStaticLabel );

    targetStaticLabel = new QLabel( infoGroupBox, "targetStaticLabel" );
    targetStaticLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, targetStaticLabel->sizePolicy().hasHeightForWidth() ) );
    QFont targetStaticLabel_font(  targetStaticLabel->font() );
    targetStaticLabel_font.setBold( TRUE );
    targetStaticLabel->setFont( targetStaticLabel_font ); 
    infoLayout1->addWidget( targetStaticLabel );
    infoGroupBoxLayout->addLayout( infoLayout1 );

    infoLayout2 = new QVBoxLayout( 0, 0, 6, "infoLayout2"); 

    directoryLabel = new KSqueezedTextLabel( infoGroupBox, "directoryLabel" );
    directoryLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, directoryLabel->sizePolicy().hasHeightForWidth() ) );
    infoLayout2->addWidget( directoryLabel );

    targetLabel = new QLabel( infoGroupBox, "targetLabel" );
    infoLayout2->addWidget( targetLabel );
    infoGroupBoxLayout->addLayout( infoLayout2 );
    AddExistingDlgBaseLayout->addWidget( infoGroupBox );

    Splitter2 = new QSplitter( this, "Splitter2" );
    Splitter2->setOrientation( QSplitter::Vertical );

    QWidget* privateLayoutWidget = new QWidget( Splitter2, "Layout11" );
    Layout11 = new QHBoxLayout( privateLayoutWidget, 0, 6, "Layout11"); 

    sourceGroupBox = new QGroupBox( privateLayoutWidget, "sourceGroupBox" );
    sourceGroupBox->setMinimumSize( QSize( 240, 250 ) );
    sourceGroupBox->setMaximumSize( QSize( 32767, 32767 ) );
    sourceGroupBox->setColumnLayout(0, Qt::Vertical );
    sourceGroupBox->layout()->setSpacing( 6 );
    sourceGroupBox->layout()->setMargin( 11 );
    sourceGroupBoxLayout = new QVBoxLayout( sourceGroupBox->layout() );
    sourceGroupBoxLayout->setAlignment( Qt::AlignTop );
    Layout11->addWidget( sourceGroupBox );

    Layout10 = new QVBoxLayout( 0, 0, 6, "Layout10"); 
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout10->addItem( spacer );

    addAllButton = new QPushButton( privateLayoutWidget, "addAllButton" );
    Layout10->addWidget( addAllButton );

    addSelectedButton = new QPushButton( privateLayoutWidget, "addSelectedButton" );
    Layout10->addWidget( addSelectedButton );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout10->addItem( spacer_2 );
    Layout11->addLayout( Layout10 );

    QWidget* privateLayoutWidget_2 = new QWidget( Splitter2, "Layout13" );
    Layout13 = new QHBoxLayout( privateLayoutWidget_2, 0, 6, "Layout13"); 

    Layout9 = new QVBoxLayout( 0, 0, 6, "Layout9"); 

    Layout8 = new QHBoxLayout( 0, 0, 6, "Layout8"); 

    destStaticLabel = new QLabel( privateLayoutWidget_2, "destStaticLabel" );
    QFont destStaticLabel_font(  destStaticLabel->font() );
    destStaticLabel_font.setBold( TRUE );
    destStaticLabel->setFont( destStaticLabel_font ); 
    Layout8->addWidget( destStaticLabel );

    destLabel = new KSqueezedTextLabel( privateLayoutWidget_2, "destLabel" );
    Layout8->addWidget( destLabel );
    Layout9->addLayout( Layout8 );

    destGroupBox = new QGroupBox( privateLayoutWidget_2, "destGroupBox" );
    destGroupBox->setMinimumSize( QSize( 140, 100 ) );
    destGroupBox->setMaximumSize( QSize( 32767, 32767 ) );
    destGroupBox->setColumnLayout(0, Qt::Vertical );
    destGroupBox->layout()->setSpacing( 6 );
    destGroupBox->layout()->setMargin( 11 );
    destGroupBoxLayout = new QVBoxLayout( destGroupBox->layout() );
    destGroupBoxLayout->setAlignment( Qt::AlignTop );
    Layout9->addWidget( destGroupBox );
    Layout13->addLayout( Layout9 );

    Layout10_2 = new QVBoxLayout( 0, 0, 6, "Layout10_2"); 
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout10_2->addItem( spacer_3 );

    removeAllButton = new QPushButton( privateLayoutWidget_2, "removeAllButton" );
    Layout10_2->addWidget( removeAllButton );

    removeSelectedButton = new QPushButton( privateLayoutWidget_2, "removeSelectedButton" );
    Layout10_2->addWidget( removeSelectedButton );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout10_2->addItem( spacer_4 );
    Layout13->addLayout( Layout10_2 );
    AddExistingDlgBaseLayout->addWidget( Splitter2 );

    progressBar = new KProgress( this, "progressBar" );
    progressBar->setEnabled( TRUE );
    AddExistingDlgBaseLayout->addWidget( progressBar );

    layout10 = new QHBoxLayout( 0, 0, 6, "layout10"); 
    QSpacerItem* spacer_5 = new QSpacerItem( 317, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout10->addItem( spacer_5 );

    okButton = new QPushButton( this, "okButton" );
    okButton->setDefault( TRUE );
    layout10->addWidget( okButton );

    cancelButton = new QPushButton( this, "cancelButton" );
    layout10->addWidget( cancelButton );
    AddExistingDlgBaseLayout->addLayout( layout10 );
    languageChange();
    resize( QSize(497, 544).expandedTo(minimumSizeHint()) );

    // signals and slots connections
    connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );

    // tab order
    setTabOrder( okButton, cancelButton );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AddExistingDlgBase::~AddExistingDlgBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AddExistingDlgBase::languageChange()
{
    setCaption( tr2i18n( "ImportExistingDlgBase" ) );
    infoGroupBox->setTitle( tr2i18n( "Subproject Information" ) );
    directoryStaticLabel->setText( tr2i18n( "Directory:" ) );
    targetStaticLabel->setText( tr2i18n( "Target:" ) );
    directoryLabel->setText( tr2i18n( "[DIRECTORY]" ) );
    targetLabel->setText( tr2i18n( "[TARGET]" ) );
    sourceGroupBox->setTitle( tr2i18n( "&Source Directory" ) );
    QToolTip::add( sourceGroupBox, QString::null );
    addAllButton->setText( tr2i18n( "A&dd All" ) );
    QToolTip::add( addAllButton, tr2i18n( "Import by creating symbolic links (recommended)" ) );
    addSelectedButton->setText( tr2i18n( "&Add Selected" ) );
    QToolTip::add( addSelectedButton, tr2i18n( "Import by copying (not recommended)" ) );
    destStaticLabel->setText( tr2i18n( "[INFO]" ) );
    destLabel->setText( tr2i18n( "[DESTINATION]" ) );
    destGroupBox->setTitle( tr2i18n( "Add &Following" ) );
    removeAllButton->setText( tr2i18n( "R&emove All" ) );
    QToolTip::add( removeAllButton, tr2i18n( "Removes all added files." ) );
    removeSelectedButton->setText( tr2i18n( "&Remove Selected" ) );
    QToolTip::add( removeSelectedButton, tr2i18n( "Removes the selected files." ) );
    okButton->setText( tr2i18n( "&OK" ) );
    cancelButton->setText( tr2i18n( "&Cancel" ) );
}

#include "addexistingdlgbase.moc"
