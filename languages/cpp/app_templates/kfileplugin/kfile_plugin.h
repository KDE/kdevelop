
#ifndef __KFILE_%{APPNAMEUC}_H__
#define __KFILE_%{APPNAMEUC}_H__

/**
 * Note: For further information look into <$KDEDIR/include/kfilemetainfo.h>
 */
#include <kfilemetainfo.h>

class QStringList;

class %{APPNAME}Plugin: public KFilePlugin
{
    Q_OBJECT
    
public:
    %{APPNAME}Plugin( QObject *parent, const char *name, const QStringList& args );
    
    virtual bool readInfo( KFileMetaInfo& info, uint what);
};

#endif // __KFILE_%{APPNAMEUC}_H__

