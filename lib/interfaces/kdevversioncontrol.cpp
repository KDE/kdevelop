
#include <kdebug.h>

#include "kdevversioncontrol.h"

KDevVersionControl::KDevVersionControl( const QString& pluginName, const QString& icon, QObject *parent, const char *name )
	: KDevPlugin( pluginName, icon, parent, name )
{
	kdDebug( 9000 ) << "  ** Registering Version Control System: " << this->pluginName() << endl;

	registerVersionControl( this );
}

KDevVersionControl::~KDevVersionControl()
{
	unregisterVersionControl( this );
}

QString KDevVersionControl::uid() const
{
	return this->pluginName();
}

#include "kdevversioncontrol.moc"
