#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qslider.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kapp.h>
#include <kstddirs.h>
#include <ktextbrowser.h>
#include <kconfig.h>
#include <kfileviewitem.h>

#include <kdevcore.h>
#include <kdevapi.h>
#include <kdevcore.h>
#include <ktip.h>

#include "fileselector_factory.h"
#include "fileselectorwidget.h"
#include "fileselector_part.h"


FileSelectorPart::FileSelectorPart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
	setInstance(FileSelectorFactory::instance());

	m_filetree = new FileSelectorWidget(this);

	connect( m_filetree->dirOperator(),
			  SIGNAL(fileSelected(const KFileViewItem*)),
			  this,
			  SLOT(fileSelected(const KFileViewItem*)));

    m_filetree->setCaption(i18n("File Selector"));
	core()->embedWidget(m_filetree, KDevCore::SelectView, i18n("FileSel"));

}

void FileSelectorPart::fileSelected(const KFileViewItem *file)
{
  KURL u(file->urlString());

  core()->gotoSourceFile( u );
}

#include "fileselector_part.moc"
