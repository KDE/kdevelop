/**
 * The interface to a diff frontend
 */

#ifndef _KDEVDIFFFRONTEND_H_
#define _KDEVDIFFFRONTEND_H_

#include <kurl.h>
#include "kdevplugin.h"


class KDevDiffFrontend : public KDevPlugin
{
    Q_OBJECT

public:

    KDevDiffFrontend( const QString& pluginName, const QString& icon, QObject *parent=0, const char *name=0 );
    virtual ~KDevDiffFrontend();

    /**
     * displays the patch.
     */
    virtual void showDiff( const QString& diff ) = 0;

    /**
     * displays a patch file
     */
    virtual void openURL( const KURL &url ) = 0;

    /**
     * displays the difference between the two files
     */
    virtual void showDiff( const KURL &url1, const KURL &url2 ) = 0;

};

#endif
