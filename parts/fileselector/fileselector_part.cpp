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
#include <kfileitem.h>

#include <kdevapi.h>
#include <kdevcore.h>
#include <kdevproject.h>
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
			  SIGNAL(fileSelected(const KFileItem*)),
			  this,
			  SLOT(fileSelected(const KFileItem*)));
	connect(core(),SIGNAL(projectOpened()),this,SLOT(slotProjectOpened()));

    m_filetree->setCaption(i18n("File Selector"));
	core()->embedWidget(m_filetree, KDevCore::SelectView, i18n("FileSel"));

}

void FileSelectorPart::fileSelected(const KFileItem *file)
{
  KURL u(file->url());

  core()->gotoSourceFile( u );
}
void FileSelectorPart::slotProjectOpened(){
  KURL u;
  u.setPath(project()->projectDirectory());
  m_filetree->setDir(u);
}

#include "fileselector_part.moc"
