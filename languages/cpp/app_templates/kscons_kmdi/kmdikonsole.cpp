//Added by qt3to4:
#include <QShowEvent>
%{CPP_TEMPLATE}

#include <qfileinfo.h>
#include <q3frame.h>

#include <kparts/part.h>
#include <kate/document.h>
#include <kate/view.h>
#include <klocale.h>
#include <klibloader.h>
#include <kurl.h>

#include "kmdikonsole.h"

kmdikonsole::kmdikonsole(QWidget *parent, const char *name) : Q3VBox(parent, name)
{
	m_haskonsole = false;
	respawn();
}

kmdikonsole::~kmdikonsole()
{
}

void kmdikonsole::respawn()
{
	KLibFactory *factory = KLibLoader::self()->factory("libkonsolepart");

	if (!factory) return;
	m_part = (KParts::ReadOnlyPart *) factory->create(this);

	if (!m_part) return;

	if (m_part->widget()->inherits("QFrame"))
		((Q3Frame*)m_part->widget())->setFrameStyle(Q3Frame::Panel|Q3Frame::Sunken);

	m_haskonsole=true;
	connect( m_part, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );

	m_part->widget()->show();
	show();
}

void kmdikonsole::setDirectory(const QString &dirname)
{
	if (m_haskonsole)
	{
		KURL url(dirname);
		if (m_part->url() != url)
			m_part->openURL(url);
	}
}

void kmdikonsole::showEvent(QShowEvent *ev)
{
	QWidget::showEvent(ev);
	activate();
}

void kmdikonsole::activate()
{
	if (m_haskonsole)
	{
		m_part->widget()->show();
		setFocusProxy(m_part->widget());
		m_part->widget()->setFocus();
	}
}

void kmdikonsole::slotDestroyed()
{
	m_haskonsole=false;
	respawn();
}

#include "kmdikonsole.moc"
