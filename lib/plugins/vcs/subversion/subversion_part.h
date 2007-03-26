#ifndef SUBVERSIONPART_H
#define SUBVERSIONPART_H

#include "iversioncontrol.h"
#include <iplugin.h>
#include <kdevcontext.h>
#include "subversion_core.h"

class KMenu;
class QMenu;
class KUrl;

using namespace KDevelop;

class KDevSubversionPart: public KDevelop::IPlugin, public KDevelop::IVersionControl
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IVersionControl )
public:
    KDevSubversionPart( QObject *parent, const QStringList & );
    virtual ~KDevSubversionPart();
    
    // reimplement to use IVersionControl
//     virtual void registerExtensions();
//     virtual void unregisterExtensions();
//     virtual QStringList extensions() const;
    
    // IVersionControl interfaces
    virtual void createNewProject(const KUrl& dir){};
    virtual bool fetchFromRepository(){return false;};
    virtual bool isValidDirectory(const KUrl &dirPath) const {return false;}
    virtual const VcsFileInfoMap& statusSync(const KUrl &dirPath, bool recursive ){ return VcsFileInfoMap();}
    virtual bool statusASync( const KUrl &dirPath, bool recursive, VcsFileInfoMap &map );
    virtual void fillContextMenu( const KUrl &ctxUrl, QMenu &ctxMenu );
    
    virtual void checkout( const KUrl &repository, const KUrl &targetDir, bool recurse );
    virtual void add( const KUrl::List &wcPaths );
    virtual void remove( const KUrl::List &paths_or_urls );
    virtual void commit( const KUrl::List &wcPaths );
    virtual void update( const KUrl::List &wcPaths );
    virtual void logview( const KUrl &wcPath_or_url );
    virtual void annotate( const KUrl &path_or_url );

    // SubversionPart internal methods
    SubversionCore* svncore();
    bool urlFocusedDocument( KUrl &url );

public Q_SLOTS:
    // invoked by menubar, editor-context menu
    void checkout();
    void add();
    void remove();
    void commit();
    void update();
    void logView();
    void blame();
    // invoked by context-menu
    void ctxLogView();
    
Q_SIGNALS:
    void finishedFetching(KUrl destinationDir);
    void statusReady(const VcsFileInfoMap &fileInfoMap);
private:

    struct KDevSubversionPartPrivate * const d;
};
#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
