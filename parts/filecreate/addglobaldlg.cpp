#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './addglobaldlg.ui'
**
** Created: Сбт Лют 8 20:51:03 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "addglobaldlg.h"

#include <qvariant.h>
#include <q3header.h>
#include <q3listview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QGridLayout>

#include <klocale.h>

/*
 *  Constructs a AddGlobalDlg as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AddGlobalDlg::AddGlobalDlg( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent, name, modal, fl )

{
    if ( !name )
	setName( "AddGlobalDlg" );
    setSizeGripEnabled( TRUE );
    AddGlobalDlgLayout = new QGridLayout( this, 1, 1, 11, 6, "AddGlobalDlgLayout");

    Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1");

    buttonHelp = new KPushButton( this, "buttonHelp" );
    buttonHelp->setAutoDefault( TRUE );
    Layout1->addWidget( buttonHelp );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    buttonOk = new KPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout1->addWidget( buttonOk );

    buttonCancel = new KPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );

    AddGlobalDlgLayout->addLayout( Layout1, 1, 0 );

    fcglobal_view = new Q3ListView( this, "fcglobal_view" );
    fcglobal_view->addColumn( i18n( "Type Extension" ) );
    fcglobal_view->addColumn( i18n( "Type Name" ) );
    fcglobal_view->addColumn( i18n( "Template Location" ) );
    fcglobal_view->addColumn( i18n( "Icon" ) );
    fcglobal_view->addColumn( i18n( "Description" ) );
    fcglobal_view->setResizePolicy( Q3ListView::AutoOne );
    fcglobal_view->setAllColumnsShowFocus( TRUE );
    fcglobal_view->setRootIsDecorated( TRUE );
    fcglobal_view->setResizeMode( Q3ListView::AllColumns );

    AddGlobalDlgLayout->addWidget( fcglobal_view, 0, 0 );
    languageChange();
    resize( QSize(511, 282).expandedTo(minimumSizeHint()) );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AddGlobalDlg::~AddGlobalDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AddGlobalDlg::languageChange()
{
    setCaption( i18n( "Select Global File Types" ) );
    buttonHelp->setGuiItem( KStdGuiItem::help() );
    buttonHelp->setAccel( QKeySequence( tr2i18n( "F1" ) ) );
    buttonOk->setGuiItem( KStdGuiItem::ok() );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setGuiItem( KStdGuiItem::cancel() );
    buttonCancel->setAccel( QKeySequence( QString::null ) );
    fcglobal_view->header()->setLabel( 0, i18n( "Type extension:" ) );
    fcglobal_view->header()->setLabel( 1, i18n( "Type name:" ) );
    fcglobal_view->header()->setLabel( 2, i18n( "Template location:" ) );
    fcglobal_view->header()->setLabel( 3, i18n( "Icon:" ) );
    fcglobal_view->header()->setLabel( 4, i18n( "Description:" ) );
}

#include "addglobaldlg.moc"
