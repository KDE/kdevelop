
#include <kdebug.h>

#include "kdevversioncontrol.h"

static KDevVersionControl::VersionControlMap m_registeredVcs;

KDevVersionControl::KDevVersionControl( const QString& pluginName, const QString& icon, QObject *parent, const char *name )
	: KDevPlugin( pluginName, icon, parent, name )
{
	kdDebug( 9000 ) << "  ** Registering Version Control System: " << this->pluginName() << endl;
    m_registeredVcs.insert( this->pluginName(), this );
}

KDevVersionControl::~KDevVersionControl()
{
	m_registeredVcs.remove( pluginName() );
}

const KDevVersionControl::VersionControlMap &KDevVersionControl::getRegisteredVCS()
{
	return m_registeredVcs;
}


#include "kdevversioncontrol.moc"
