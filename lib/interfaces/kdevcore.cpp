
#include <qdir.h>

#include "KDevCoreIface.h"
#include "kdevcore.h"


KDevCore::KDevCore(QObject *parent, const char *name)
    : QObject(parent, name)
{
  new KDevCoreIface(this);
}

KDevCore::~KDevCore()
{
}

FileContext::FileContext( const KURL::List &someURLs  )
    : Context("file"), m_urls(someURLs)
{
    m_fileName = (m_urls.first()).path();

	QFileInfo fi(m_fileName);
	this->m_isDirectory = fi.isDir();
}


#include "kdevcore.moc"
