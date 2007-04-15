/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVS_PART_H
#define CVS_PART_H

#include <KUrl>
#include <KJob>

#include <iversioncontrol.h>
#include <iplugin.h>
#include <qobject.h>

class KDevCvsViewFactory;
class CvsProxy;

/**
 * This is the main class of KDevelop's CVS plugin.
 * 
 * It implements the IVersionControl interface.
 * 
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsPart: public KDevelop::IPlugin , public KDevelop::IVersionControl
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IVersionControl )

friend class CvsProxy;

public:
    CvsPart( QObject *parent, const QStringList & );
    virtual ~CvsPart();

    // BEGIN: IVersionControl interfaces
    virtual void createNewProject(const KUrl& dir);
    virtual bool fetchFromRepository();
    virtual bool isValidDirectory(const KUrl &dirPath) const;
    virtual const QList<KDevelop::VcsFileInfo>& statusSync(const KUrl &dirPath, KDevelop::IVersionControl::WorkingMode mode );
    virtual bool statusASync( const KUrl &dirPath, WorkingMode mode, const QList<KDevelop::VcsFileInfo> &infos );
    virtual void fillContextMenu( const KUrl &ctxUrl, QMenu &ctxMenu );
    virtual void fillContextMenu( const KDevelop::ProjectBaseItem *prjItem, QMenu &ctxMenu );

    virtual void checkout( const KUrl &repository, const KUrl &targetDir, KDevelop::IVersionControl::WorkingMode mode );
    virtual void add( const KUrl::List &urls );
    virtual void remove( const KUrl::List &urls );
    virtual void commit( const KUrl::List &urls );
    virtual void update( const KUrl::List &urls );
    virtual void logview( const KUrl &url );
    virtual void annotate( const KUrl &url );
    // END: IVersionControl interfaces

    void diff(const KUrl& url);
    void edit(const KUrl::List& urls);
    void editors(const KUrl::List& urls);
    void unEdit(const KUrl::List& urls);

    CvsProxy* proxy();

    const KUrl urlFocusedDocument() const;

public slots:
    void slotLog();
    void slotDiff();
    void slotEdit();
    void slotUnEdit();
    void slotEditors();
    void slotAdd();
    void slotRemove();
    void slotCommit();
    void slotUpdate();
    void slotImport();

// Signals inherited from IVersionControl
Q_SIGNALS:
    virtual void finishedFetching( const KUrl& destinationDir);
    virtual void statusReady(const QList<KDevelop::VcsFileInfo> &infos);

signals:
    /**
     * Some actions like commit, add, remove... will connect the job's
     * result() signal to this signal. Anybody, like for instance the 
     * CvsMainView class, that is interested in getting notified about
     * jobs that finished can connect to this signal.
     * @see class CvsMainView
     */
    void jobFinished(KJob* job);

    /**
     * Gets emmited when a job like log, editors... was created.
     * CvsPart will connect the newly created view to the result() signal 
     * of a job. So the new view will show the ouput of that job as
     * soon as it has finished.
     */
    void addNewTabToMainView(QWidget* tab, QString label);

private:
    class CvsPartPrivate* d;

    void setupActions();
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
