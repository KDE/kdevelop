#ifndef _KDEV_KURL_H_
#define _KDEV_KURL_H_

#include <kdeversion.h>
#if (KDE_VERSION_MINOR==0) && (KDE_VERSION_MAJOR==3)
#include <kurl.h>
class KdevKURL : public KURL {
public:
  KdevKURL();
  KdevKURL( const KURL& u );
  KdevKURL( const QString& url );
  KdevKURL( const QString& url, int encoding_hint = 0 );
  static KdevKURL fromPathOrURL(const QString& text );
};

KdevKURL::KdevKURL() : KURL() {};
KdevKURL::KdevKURL( const KURL& u ) : KURL(u) {};
KdevKURL::KdevKURL( const QString& url ) : KURL(url) {};
KdevKURL::KdevKURL( const QString& url, int encoding_hint /*= 0*/ ) : KURL(url, encoding_hint) {};

KdevKURL KdevKURL::fromPathOrURL(const QString& text )
  {
    if ( text.isEmpty() )
      return KdevKURL();
    KdevKURL url;
    if ( text[0] == '/' )
      url.setPath( text );
    else
      (KURL)url = text;
    return url;
  };
#endif

#endif // _KDEV_KURL_H_
