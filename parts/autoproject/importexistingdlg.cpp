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

ImportExistingDialog::ImportExistingDialog( AutoProjectPart* part, KFile::Mode mode, const QString& destLabel, QWidget* parent, const char* name, bool modal, WFlags fl )
    : ImportExistingDlgBase ( parent, name, modal, fl )
{
    setCaption ( name );

    sourceSelector = new FileSelectorWidget ( part, mode, sourceGroupBox, "source file selector" );
    sourceGroupBoxLayout->addWidget ( sourceSelector );

    importView = new KIconView ( destGroupBox, "destination icon view" );
    importView->setMode ( KIconView::Select );
    destGroupBoxLayout->addWidget ( importView );

    destGroupBox->setTitle( destLabel );

    setIcon ( SmallIcon ( "fileimport.png" ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ImportExistingDialog::~ImportExistingDialog()
{
    // no need to delete child widgets, Qt does it all for us
}
