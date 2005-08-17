//Added by qt3to4:
#include <QHBoxLayout>
#include <QBoxLayout>
%{CPP_TEMPLATE}

#include <qpixmap.h>
#include <qlayout.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kmdichildview.h>
#include <kdebug.h>
#include <klibloader.h>
#include <kmessagebox.h>

#include "%{APPNAMELC}kmdiView.h"


	%{APPNAMELC}kmdiView::%{APPNAMELC}kmdiView( QWidget *parentWidget, const char *name )
: KMdiChildView( parentWidget, name )
{
	m_part = NULL;

	// this routine will find and load our Part.  it finds the Part by
	// name which is a bad idea usually.. but it's alright in this
	// case since our Part is made for this Shell
	KLibFactory *factory = KLibLoader::self()->factory("lib%{APPNAMELC}part");
	//KLibFactory *factory = KLibLoader::self()->factory("libkatepart");
	if (factory)
	{
		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
		m_part = static_cast<KParts::ReadWritePart *>(factory->create(
			this, "katepart", "KParts::ReadWritePart" ));

		if (m_part)
		{
			part()->widget()->setFocus();
			setFocusProxy(part()->widget());

			QBoxLayout* layout = new QHBoxLayout(this, 0, -1, "kdissertviewlayout" );
			part()->widget()->reparent( this, QPoint(0, 0) );
			layout->addWidget( part()->widget() );

			// connect( m_part, SIGNAL(stateChanged()), this, SLOT(updateCaption()) );
		}
		else
		{
			KMessageBox::error(this, i18n("Could not find our part."));
		}
	}
	else
	{
		// if we couldn't find our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, i18n("Could not find our part."));
		return;
	}
}

%{APPNAMELC}kmdiView::~%{APPNAMELC}kmdiView()
{
	delete m_part->widget();
}

#include "%{APPNAMELC}kmdiView.moc"
