#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qslider.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <ktextbrowser.h>
#include <kconfig.h>
#include <kfileitem.h>
#include <kgenericfactory.h>

#include <kdevapi.h>
#include <kdevcore.h>
#include <kdevproject.h>
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"

#include <ktip.h>

#include "fileselectorwidget.h"
#include "fileselector_part.h"

typedef KGenericFactory<FileSelectorPart> FileSelectorFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfileselector, FileSelectorFactory( "kdevfileselector" ) );

FileSelectorPart::FileSelectorPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(parent, name)
{
	setInstance(FileSelectorFactory::instance());

	m_filetree = new FileSelectorWidget(this);

	connect( m_filetree->dirOperator(),
			  SIGNAL(fileSelected(const KFileItem*)),
			  this,
			  SLOT(fileSelected(const KFileItem*)));
	connect(core(),SIGNAL(projectOpened()),this,SLOT(slotProjectOpened()));

    m_filetree->setCaption(i18n("File Selector"));
	topLevel()->embedSelectView(m_filetree, i18n("File Selector"), i18n("file selector"));

}


FileSelectorPart::~FileSelectorPart()
{
  if (m_filetree)
    topLevel()->removeView(m_filetree);
  delete m_filetree;
}


void FileSelectorPart::fileSelected(const KFileItem *file)
{
  KURL u(file->url());

  partController()->editDocument( u );
  topLevel()->lowerView(m_filetree);
}

void FileSelectorPart::slotProjectOpened(){
  KURL u;
  u.setPath(project()->projectDirectory());
  m_filetree->setDir(u);
}

#include "fileselector_part.moc"
