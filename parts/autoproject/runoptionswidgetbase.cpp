#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './runoptionswidgetbase.ui'
**
** Created: Fri Apr 12 20:37:02 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "runoptionswidgetbase.h"

#include <qvariant.h>
#include <qcheckbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a RunOptionsWidgetBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
RunOptionsWidgetBase::RunOptionsWidgetBase( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "run_options_widget" );
    resize( 439, 318 ); 
    setCaption( tr2i18n( "Run options" ) );
    run_options_widgetLayout = new QGridLayout( this, 1, 1, 11, 6, "run_options_widgetLayout"); 

    mainprogram_edit = new QLineEdit( this, "mainprogram_edit" );

    run_options_widgetLayout->addMultiCellWidget( mainprogram_edit, 1, 1, 1, 2 );

    progargs_label_3 = new QLabel( this, "progargs_label_3" );
    progargs_label_3->setText( tr2i18n( "Program arguments:" ) );

    run_options_widgetLayout->addMultiCellWidget( progargs_label_3, 3, 3, 0, 3 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
    run_options_widgetLayout->addItem( spacer, 5, 1 );

    mainprogram_label_3 = new QLabel( this, "mainprogram_label_3" );
    mainprogram_label_3->setText( tr2i18n( "Main program (relative to project directory):" ) );

    run_options_widgetLayout->addMultiCellWidget( mainprogram_label_3, 0, 0, 0, 3 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
    run_options_widgetLayout->addItem( spacer_2, 2, 1 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding );
    run_options_widgetLayout->addItem( spacer_3, 10, 2 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
    run_options_widgetLayout->addItem( spacer_4, 8, 2 );

    envvars_label = new QLabel( this, "envvars_label" );
    envvars_label->setText( tr2i18n( "Environment variables:" ) );

    run_options_widgetLayout->addMultiCellWidget( envvars_label, 6, 6, 0, 3 );
    QSpacerItem* spacer_5 = new QSpacerItem( 16, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    run_options_widgetLayout->addItem( spacer_5, 1, 0 );
    QSpacerItem* spacer_6 = new QSpacerItem( 16, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    run_options_widgetLayout->addItem( spacer_6, 4, 0 );
    QSpacerItem* spacer_7 = new QSpacerItem( 16, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    run_options_widgetLayout->addItem( spacer_7, 7, 0 );

    progargs_edit = new QLineEdit( this, "progargs_edit" );

    run_options_widgetLayout->addMultiCellWidget( progargs_edit, 4, 4, 1, 2 );

    startinterminal_box = new QCheckBox( this, "startinterminal_box" );
    startinterminal_box->setText( tr2i18n( "Start in external terminal" ) );

    run_options_widgetLayout->addMultiCellWidget( startinterminal_box, 9, 9, 0, 2 );

    Layout8_3 = new QVBoxLayout( 0, 0, 6, "Layout8_3"); 
    QSpacerItem* spacer_8 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout8_3->addItem( spacer_8 );

    addvar_button = new QPushButton( this, "addvar_button" );
    addvar_button->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, addvar_button->sizePolicy().hasHeightForWidth() ) );
    addvar_button->setText( tr2i18n( "Add..." ) );
    Layout8_3->addWidget( addvar_button );

    removevar_button = new QPushButton( this, "removevar_button" );
    removevar_button->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, removevar_button->sizePolicy().hasHeightForWidth() ) );
    removevar_button->setText( tr2i18n( "Remove" ) );
    Layout8_3->addWidget( removevar_button );
    QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout8_3->addItem( spacer_9 );

    run_options_widgetLayout->addLayout( Layout8_3, 7, 4 );
    QSpacerItem* spacer_10 = new QSpacerItem( 20, 20, QSizePolicy::Preferred, QSizePolicy::Minimum );
    run_options_widgetLayout->addMultiCell( spacer_10, 4, 4, 3, 4 );
    QSpacerItem* spacer_11 = new QSpacerItem( 20, 20, QSizePolicy::Preferred, QSizePolicy::Minimum );
    run_options_widgetLayout->addMultiCell( spacer_11, 1, 1, 3, 4 );

    listview = new QListView( this, "listview" );
    listview->addColumn( tr2i18n( "Name" ) );
    listview->addColumn( tr2i18n( "Value" ) );
    listview->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)7, 0, 0, listview->sizePolicy().hasHeightForWidth() ) );

    run_options_widgetLayout->addMultiCellWidget( listview, 7, 7, 1, 3 );

    // signals and slots connections
    connect( addvar_button, SIGNAL( clicked() ), this, SLOT( addVarClicked() ) );
    connect( removevar_button, SIGNAL( clicked() ), this, SLOT( removeVarClicked() ) );

    // tab order
    setTabOrder( mainprogram_edit, progargs_edit );
    setTabOrder( progargs_edit, listview );
    setTabOrder( listview, addvar_button );
    setTabOrder( addvar_button, removevar_button );
    setTabOrder( removevar_button, startinterminal_box );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
RunOptionsWidgetBase::~RunOptionsWidgetBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void RunOptionsWidgetBase::addVarClicked()
{
    qWarning( "RunOptionsWidgetBase::addVarClicked(): Not implemented yet!" );
}

void RunOptionsWidgetBase::removeVarClicked()
{
    qWarning( "RunOptionsWidgetBase::removeVarClicked(): Not implemented yet!" );
}

#include "runoptionswidgetbase.moc"
