#include <qdir.h>

#include "urlfileutilities.h"

QString UrlFileUtilities::extractPathNameRelative(const KURL &baseDirUrl, const KURL &url )
{
	QString absBase = extractPathNameAbsolute( baseDirUrl ),
		absRef = extractPathNameAbsolute( url );
	int i = absRef.find( absBase, 0, true );

	if (i == -1)
		return QString();

	return absRef.replace( 0, absBase.length(), QString() );
}

QString UrlFileUtilities::extractPathNameRelative(const QString &basePath, const KURL &url )
{
	KURL baseDirUrl;
	baseDirUrl.setPath( basePath );
	return extractPathNameRelative( baseDirUrl, url );
}

QString UrlFileUtilities::extractPathNameAbsolute( const KURL &url )
{
	if (isDirectory( url ))
		return url.path( +1 ); // with trailing "/" if none is present
	else
	{
		// Ok, this is an over-tight pre-condition on "url" since I hope nobody will never
		// stress this function with absurd cases ... but who knows?
		QString path = url.path();
		QFileInfo fi( path );  // Argh: QFileInfo is back ;))
		return ( fi.exists()? path : QString() );
	}
}

bool UrlFileUtilities::isDirectory( const KURL &url )
{
	return QDir( url.path() ).exists();
}
