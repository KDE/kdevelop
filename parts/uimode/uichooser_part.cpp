#include "uichooser_part.h"

#include <qvbox.h>

#include <kdialogbase.h>
#include <kdevgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdevcore.h>
#include <configwidgetproxy.h>

#include "uichooser_widget.h"

#define UICHOOSERSETTINGSPAGE 1

typedef KDevGenericFactory<UIChooserPart> UIChooserFactory;
static const KAboutData data("kdevuichooser", I18N_NOOP("User Interface"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevuichooser, UIChooserFactory( &data ) )

UIChooserPart::UIChooserPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin( "UIChooser", "view_choose", parent, name ? name : "UIChooserPart")
{
	setInstance(UIChooserFactory::instance());

	_configProxy = new ConfigWidgetProxy( core() );
	_configProxy->createGlobalConfigPage( i18n("User Interface"), UICHOOSERSETTINGSPAGE, icon() );
	connect( _configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )),
		this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )) );
}


UIChooserPart::~UIChooserPart()
{
}

void UIChooserPart::insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int pagenumber )
{
	kdDebug() << k_funcinfo << endl;

	if ( pagenumber == UICHOOSERSETTINGSPAGE )
	{
		UIChooserWidget * w = new UIChooserWidget( this, page, "UIChooser widget" );
		connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
	}
}




#include "uichooser_part.moc"
