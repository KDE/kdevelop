/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
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

#include <ibasicversioncontrol.h>
#include <vcslocation.h>
#include <iplugin.h>
#include <qobject.h>

class CvsProxy;

/**
 * This is the main class of KDevelop's CVS plugin.
 *
 * It implements the IVersionControl interface.
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsPlugin: public KDevelop::IPlugin , public KDevelop::IBasicVersionControl
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IBasicVersionControl )

friend class CvsProxy;

public:
    CvsPlugin( QObject *parent, const QVariantList & args = QVariantList() );
    virtual ~CvsPlugin();

    virtual QString name() const;
    virtual KDevelop::VcsImportMetadataWidget* importMetadataWidget( QWidget* parent );

    // From KDevelop::IPlugin
    QPair<QString,QList<QAction*> > requestContextMenuActions( KDevelop::Context* );

    // Begin:  KDevelop::IBasicVersionControl
    virtual bool isVersionControlled( const KUrl& localLocation );
    virtual KDevelop::VcsJob* repositoryLocation( const KUrl& localLocation );
    virtual KDevelop::VcsJob* add( const KUrl::List& localLocations,
                KDevelop::IBasicVersionControl::RecursionMode recursion );
    virtual KDevelop::VcsJob* remove( const KUrl::List& localLocations );
    virtual KDevelop::VcsJob* edit( const KUrl& localLocation );
    virtual KDevelop::VcsJob* unedit( const KUrl& localLocation );
    virtual KDevelop::VcsJob* status( const KUrl::List& localLocations,
                KDevelop::IBasicVersionControl::RecursionMode recursion );
    virtual KDevelop::VcsJob* localRevision( const KUrl& localLocation,
                KDevelop::VcsRevision::RevisionType );
    virtual KDevelop::VcsJob* copy( const KUrl& localLocationSrc,
                const KUrl& localLocationDstn );
    virtual KDevelop::VcsJob* move( const KUrl& localLocationSrc,
                const KUrl& localLocationDst );
    virtual KDevelop::VcsJob* revert( const KUrl::List& localLocations,
                KDevelop::IBasicVersionControl::RecursionMode recursion );
    virtual KDevelop::VcsJob* update( const KUrl::List& localLocations,
                const KDevelop::VcsRevision& rev,
                KDevelop::IBasicVersionControl::RecursionMode recursion );
    virtual KDevelop::VcsJob* commit( const QString& message,
                const KUrl::List& localLocations,
                KDevelop::IBasicVersionControl::RecursionMode recursion );
    virtual KDevelop::VcsJob* diff( const KDevelop::VcsLocation& localOrRepoLocationSrc,
                const KDevelop::VcsLocation& localOrRepoLocationDst,
                const KDevelop::VcsRevision& srcRevision,
                const KDevelop::VcsRevision& dstRevision,
                KDevelop::VcsDiff::Type,
                KDevelop::IBasicVersionControl::RecursionMode = KDevelop::IBasicVersionControl::Recursive );
    virtual KDevelop::VcsJob* log( const KUrl& localLocation,
                const KDevelop::VcsRevision& rev,
                unsigned long limit );
    virtual KDevelop::VcsJob* log( const KUrl& localLocation,
                const KDevelop::VcsRevision& rev,
                const KDevelop::VcsRevision& limit );
    virtual KDevelop::VcsJob* annotate( const KUrl& localLocation,
                const KDevelop::VcsRevision& rev );
    virtual KDevelop::VcsJob* merge( const KDevelop::VcsLocation& localOrRepoLocationSrc,
                const KDevelop::VcsLocation& localOrRepoLocationDst,
                const KDevelop::VcsRevision& srcRevision,
                const KDevelop::VcsRevision& dstRevision,
                const KUrl& localLocation );
    virtual KDevelop::VcsJob* resolve( const KUrl::List& localLocations,
                KDevelop::IBasicVersionControl::RecursionMode recursion );
    virtual KDevelop::VcsJob* import( const KUrl& localLocation,
                const QString& repositoryLocation,
                KDevelop::IBasicVersionControl::RecursionMode recursion );
    virtual KDevelop::VcsJob* checkout( const KDevelop::VcsMapping& mapping );
    // End:  KDevelop::IBasicVersionControl

    CvsProxy* proxy();

    const KUrl urlFocusedDocument() const;

public slots:
    // slots for context menu
    void ctxCommit();
    void ctxAdd();
    void ctxRemove();
    void ctxUpdate();
    void ctxLog();
    void ctxAnnotate();
    void ctxRevert();
    void ctxDiff();
    void ctxEdit();
    void ctxUnEdit();
    void ctxEditors();

    // slots for menu
    void slotImport();
    void slotCheckout();
    void slotStatus();

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
     * CvsPlugin will connect the newly created view to the result() signal
     * of a job. So the new view will show the output of that job as
     * soon as it has finished.
     */
    void addNewTabToMainView(QWidget* tab, QString label);

private:
    class CvsPluginPrivate* d;

    void setupActions();
};

#endif
