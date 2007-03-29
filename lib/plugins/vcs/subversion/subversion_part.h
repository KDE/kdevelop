#ifndef SUBVERSIONPART_H
#define SUBVERSIONPART_H

#include "iversioncontrol.h"
#include <iplugin.h>
#include <kdevcontext.h>
#include "subversion_core.h"
#include <qwidget.h>

class KMenu;
class QMenu;
class KUrl;
class QWidget;

class KDevSubversionPart: public KDevelop::IPlugin, public KDevelop::IVersionControl
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IVersionControl )
public:
    KDevSubversionPart( QObject *parent, const QStringList & );
    virtual ~KDevSubversionPart();

    // IVersionControl interfaces
    virtual void createNewProject(const KUrl& dir){};
    virtual bool fetchFromRepository(){return false;};
    virtual bool isValidDirectory(const KUrl &dirPath) const {return false;}
    virtual const QList<KDevelop::VcsFileInfo>& statusSync(const KUrl &dirPath, KDevelop::IVersionControl::WorkingMode mode );
    virtual bool statusASync( const KUrl &dirPath, WorkingMode mode, const QList<KDevelop::VcsFileInfo> &infos );
    virtual void fillContextMenu( const KUrl &ctxUrl, QMenu &ctxMenu );

    virtual void checkout( const KUrl &repository, const KUrl &targetDir, KDevelop::IVersionControl::WorkingMode mode );
    virtual void add( const KUrl::List &wcPaths );
    virtual void remove( const KUrl::List &paths_or_urls );
    virtual void commit( const KUrl::List &wcPaths );
    virtual void update( const KUrl::List &wcPaths );
    virtual void logview( const KUrl &wcPath_or_url );
    virtual void annotate( const KUrl &path_or_url );

    // SubversionPart internal methods
    SubversionCore* svncore();
    const KUrl urlFocusedDocument( );

public Q_SLOTS:
    // invoked by menubar, editor-context menu
    void checkout();
    void add();
    void remove();
    void commit();
    void update();
    void logView();
    void blame();
    void statusSync();
    void statusASync();
    // invoked by context-menu
    void ctxLogView();

Q_SIGNALS:
    void finishedFetching(const KUrl& destinationDir);
    void statusReady(const QList<KDevelop::VcsFileInfo> &infos);

private Q_SLOTS:
    void slotJobFinished( SubversionJob *job );
private:
//     const QList<KDevelop::VcsFileInfo>& statusSync( const KUrl &dirPath,
//                     KDevelop::IVersionControl::WorkingMode mode,
//                     bool recurse, bool getAll, bool update, bool noIgnore, bool ignoreExternals );

    struct KDevSubversionPartPrivate * const d;
};
#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
