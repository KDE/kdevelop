

#include "librarydocdlg.h"

#include <klineedit.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include "doctreeviewfactory.h"
#include <kconfig.h>
#include <kdebug.h>

LibraryDocDlg::LibraryDocDlg( QWidget* parent, const char* name, const QString& path, const QString& _default)
    : LibraryDocDlgBase( parent, name )
{
    docLocal->fileDialog()->setMode( KFile::Directory );
    mPath = path;
    mName = name;
    mDef = _default;
    libName->setText( name );
    docLocal->setURL(mPath);
}

LibraryDocDlg::~LibraryDocDlg()
{
}

void LibraryDocDlg::accept()
{
//	kdDebug(9002) << "accept" << endl;
    KConfig *config = DocTreeViewFactory::instance()->config();
    config->setGroup("TocDirs");
    config->writeEntry( mName, docLocal->url() );

    QDialog::accept();
}

void LibraryDocDlg::slotDefault()
{
    mPath = mDef;
    docLocal->setURL(mPath);
}

#include "librarydocdlg.moc"
