#include <qvariant.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include <kiconview.h>
#include <ksqueezedtextlabel.h>

#include "importexistingdlg.h"
#include "autoprojectwidget.h"
#include "fileselectorwidget.h"


/* 
 *  Constructs a ImportExistingDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */

ImportExistingDialog::ImportExistingDialog( AutoProjectPart* part, SubprojectItem* spitem, TargetItem* titem, QWidget* parent, const char* name, bool modal, WFlags fl )
    : ImportExistingDlgBase ( parent, name, modal, fl )
{
    KFile::Mode mode = KFile::Files;;

    if ( titem && spitem && titem->type() == ProjectItem::Target && spitem->type() == ProjectItem::Subproject )
        {
            destGroupBox->setTitle ( i18n ( "Target: " + titem->name ) );
            introLabel->setText ( i18n ( "<b>Drag one or more files from the left side and drop it at the right side.</b>" ) );
            targetLabel->setText ( titem->name );
            directoryLabel->setText ( spitem->path );
        }

    sourceSelector = new FileSelectorWidget ( part, mode, sourceGroupBox, "source file selector" );
    sourceGroupBoxLayout->addWidget ( sourceSelector );

    importView = new KIconView ( destGroupBox, "destination icon view" );
    importView->setMode ( KIconView::Select );
    destGroupBoxLayout->addWidget ( importView );

    setIcon ( SmallIcon ( "fileimport.png" ) );

}

ImportExistingDialog::ImportExistingDialog ( AutoProjectPart* part, SubprojectItem* spitem, QWidget* parent, const char* name, bool modal, WFlags fl )
    : ImportExistingDlgBase ( parent, name, modal, fl )
{
    setCaption ( name );

    KFile::Mode mode = KFile::Directory;;

    if ( spitem && spitem->type() == ProjectItem::Subproject )
    {
        destGroupBox->setTitle ( i18n ( "Subproject: " + spitem->subdir ) );
        introLabel->setText ( i18n ( "<b>Drag one or more directories with an existing Makefile.am from the left side and drop it at the right side.</b>" ) );
        targetLabel->setText ( i18n ( "none" ) );
        directoryLabel->setText ( i18n ( spitem->path ) );
    }

    sourceSelector = new FileSelectorWidget ( part, mode, sourceGroupBox, "source file selector" );
    sourceGroupBoxLayout->addWidget ( sourceSelector );

    importView = new KIconView ( destGroupBox, "destination icon view" );
    importView->setMode ( KIconView::Select );
    destGroupBoxLayout->addWidget ( importView );

    setIcon ( SmallIcon ( "fileimport.png" ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ImportExistingDialog::~ImportExistingDialog()
{
    // no need to delete child widgets, Qt does it all for us
}
#include "importexistingdlg.moc"
