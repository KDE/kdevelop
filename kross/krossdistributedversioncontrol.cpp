/* KDevPlatform Kross Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "krossdistributedversioncontrol.h"
#include <KDebug>
#include <KAction>
#include <KLocale>
#include <QMenu>

#include <iplugin.h>
#include <krossvcsjob.h>
#include <kross/core/action.h>
#include <context.h>
#include <projectmodel.h>
#include "dvcsadaptors.h"

using namespace KDevelop;

class DVCSCallbacks : public QObject
{
    Q_OBJECT
    public:
        DVCSCallbacks(QObject* parent, KrossDistributedVersionControl* self) : QObject(parent), m_self(self) {}
    public slots:
        void ctxCommit() {}
        void ctxAdd() {}
        void ctxRemove() {}
        void ctxUpdate() {}
        void ctxRevert() {}
        void ctxDiffHead() {}
        void ctxDiffBase() {}
        void ctxCopy() {}
        void ctxMove() {}
        void ctxHistory() {}
        void slotInit() {}

    private:
        KrossDistributedVersionControl* m_self;
};

KrossDistributedVersionControl::KrossDistributedVersionControl(KDevelop::IPlugin* plugin)
    : action(0), m_plugin(plugin)
{
    m_callbacks=new DVCSCallbacks(plugin, this);
}

KrossDistributedVersionControl::~KrossDistributedVersionControl()
{
    delete m_callbacks;
}

KDevelop::ContextMenuExtension KrossDistributedVersionControl::distributedMenuExtension(KDevelop::Context* context)
{
    KUrl::List ctxUrlList;
    if( context->type() == KDevelop::Context::ProjectItemContext )
    {
        KDevelop::ProjectItemContext *itemCtx = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        if( itemCtx )
        {
            QList<KDevelop::ProjectBaseItem *> baseItemList = itemCtx->items();

            // now general case
            foreach( KDevelop::ProjectBaseItem* _item, baseItemList )
            {
                if( _item->folder() ){
                    KDevelop::ProjectFolderItem *folderItem = dynamic_cast<KDevelop::ProjectFolderItem*>(_item);
                    ctxUrlList << folderItem->url();
                }
                else if( _item->file() ){
                    KDevelop::ProjectFileItem *fileItem = dynamic_cast<KDevelop::ProjectFileItem*>(_item);
                    ctxUrlList << fileItem->url();
                }
            }
        }
    }
    else if( context->type() == KDevelop::Context::EditorContext )
    {
        KDevelop::EditorContext *itemCtx = dynamic_cast<KDevelop::EditorContext*>(context);
        ctxUrlList << itemCtx->url();
    }
    else if( context->type() == KDevelop::Context::FileContext )
    {
        KDevelop::FileContext *itemCtx = dynamic_cast<KDevelop::FileContext*>(context);
        ctxUrlList += itemCtx->urls();
    }

    m_ctxUrls = ctxUrlList;
    KDevelop::ContextMenuExtension menuExt;

    bool hasVersionControlledEntries = false;
    foreach(KUrl url, ctxUrlList)
    {
        if(isVersionControlled( url ) )
        {
            hasVersionControlledEntries = true;
            break;
        }
    }

    KAction *_action = 0;
    kDebug() << "version controlled?" << hasVersionControlledEntries;
    if(hasVersionControlledEntries)
    {
/*
        _action = new KAction(i18n("Commit..."), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxCommit()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );

        _action = new KAction(i18n("Add"), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxAdd()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );

        _action = new KAction(i18n("Remove"), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxRemove()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );

        _action = new KAction(i18n("Update to Head"), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxUpdate()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );

        _action = new KAction(i18n("Revert"), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxRevert()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );

        _action = new KAction(i18n("Diff to Head"), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxDiffHead()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );

        _action = new KAction(i18n("Diff to Base"), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxDiffBase()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );

        _action = new KAction(i18n("Copy..."), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxCopy()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );

        _action = new KAction(i18n("Move..."), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxMove()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );

        _action = new KAction(i18n("History..."), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxHistory()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );

        _action = new KAction(i18n("Annotation..."), m_callbacks);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(ctxBlame()) );
        menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, action );
*/
    }
    else
    {
        QMenu* menu = new QMenu(name());
/*
        _action = new KAction(i18n("Init..."), menu);
        QObject::connect( _action, SIGNAL(triggered()), m_callbacks, SLOT(slotInit()) );
        menu->addAction( _action );
*/
        menuExt.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, menu->menuAction() );
    }

    return menuExt;
}

void KrossDistributedVersionControl::setActionDistributed(Kross::Action* anAction)
{
    action=anAction;
}

QString KrossDistributedVersionControl::name() const
{
    QVariantList args;
    QVariant result=action->callFunction("name", args );
    kDebug() << "retrieving name" << result.toString();
    return result.toString();
}

KDevelop::VcsImportMetadataWidget * KrossDistributedVersionControl::createImportMetadataWidget(QWidget *)
{
    return 0;
}

bool KrossDistributedVersionControl::isVersionControlled(const KUrl & url)
{
    QVariantList args;
    args << QUrl(url);
    QVariant result=action->callFunction( "isVersionControlled", args);
    return result.toBool();
}

KDevelop::VcsJob * KrossDistributedVersionControl::add(const KUrl::List & files, KDevelop::IBasicVersionControl::RecursionMode mode)
{
    QVariant param(files);
    QVariant param1(mode==Recursive);
    
    return new KrossVcsJob("add", QVariantList() << param << param1, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::remove(const KUrl::List & files)
{
    QVariant param(files);
    
    return new KrossVcsJob("remove", QVariantList() << param, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::copy(const KUrl & file, const KUrl & newfile)
{
    QVariant from(file);
    QVariant to(newfile);
    
    return new KrossVcsJob("copy", QVariantList() << from << to, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::move(const KUrl & file, const KUrl & newfile)
{
    QVariant from(file);
    QVariant to(newfile);
    
    return new KrossVcsJob("move", QVariantList() << from << to, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::status(const KUrl::List & files, KDevelop::IBasicVersionControl::RecursionMode mode)
{
    QVariant param(files);
    QVariant param1(mode);
    
    return new KrossVcsJob("status", QVariantList() << param << param1, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::revert(const KUrl::List & files, KDevelop::IBasicVersionControl::RecursionMode mode)
{
    QVariant param(files);
    QVariant param1(mode==Recursive);
    
    return new KrossVcsJob("revert", QVariantList() << param << param1, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::update(const KUrl::List & localLocations, const KDevelop::VcsRevision & rev,
                                                            KDevelop::IBasicVersionControl::RecursionMode rec)
{
/*
    const VcsRevisionAdaptor *revadapt=new VcsRevisionAdaptor(&rev, action);
    QVariant revision;
    revision.setValue((QObject*) revadapt);
    
    QVariantList parameters;
    parameters.append(QVariant(localLocations));
    parameters.append(revision);
    parameters.append(QVariant(rec));
    
    return new KrossVcsJob("update", parameters, action, m_plugin, action);
*/
    return 0;
}

KDevelop::VcsJob * KrossDistributedVersionControl::commit(const QString & msg, const KUrl::List & files,
                                                          KDevelop::IBasicVersionControl::RecursionMode mode)
{
    QVariant message(msg);
    QVariant file(files);
    QVariant recursion(mode);
    
    return new KrossVcsJob("commit", QVariantList() << message << file << mode, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::diff(const KUrl::List & files, KDevelop::VcsRevision & rev,
                                                        KDevelop::IBasicVersionControl::RecursionMode mode)
{
/*
    const VcsRevisionAdaptor *revadapt=new VcsRevisionAdaptor(&rev, action);
    QVariant revision;
    revision.setValue((QObject*) revadapt);
    
    return new KrossVcsJob("commit", QVariantList() << files << revision << mode, action, m_plugin, action);
*/
    return 0;
}

KDevelop::VcsJob * KrossDistributedVersionControl::repositoryLocation(const KUrl::List & files)
{
    return new KrossVcsJob("repositoryLocation", QVariantList() << files, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::repositoryLocation(const KUrl & localLocation)
{
    return new KrossVcsJob("repositoryLocation", QVariantList() << localLocation, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::diff(const KDevelop::VcsLocation & localOrRepoLocationSrc,
                                                        const KDevelop::VcsLocation & localOrRepoLocationDst,
                                                        const KDevelop::VcsRevision & srcRevision,
                                                        const KDevelop::VcsRevision & dstRevision,
                                                        KDevelop::VcsDiff::Type t,
                                                        KDevelop::IBasicVersionControl::RecursionMode recursion)
{
/*
    VcsRevisionAdaptor *srcrevadapt=new VcsRevisionAdaptor(&srcRevision, action);
    QVariant srcrev;
    srcrev.setValue((QObject*) srcrevadapt);
    
    VcsRevisionAdaptor *dstrevadapt=new VcsRevisionAdaptor(&dstRevision, action);
    QVariant dstrev;
    dstrev.setValue((QObject*) dstrevadapt);
    
    VcsLocationAdaptor *srclocadapt=new VcsLocationAdaptor(&localOrRepoLocationSrc, action);
    QVariant srcloc;
    srcloc.setValue((QObject*) srclocadapt);
    
    VcsLocationAdaptor *dstlocadapt=new VcsLocationAdaptor(&localOrRepoLocationDst, action);
    QVariant dstloc;
    dstloc.setValue((QObject*) dstlocadapt);
    
    return new KrossVcsJob("repositoryLocation",
                                         QVariantList() << srcloc << dstloc << srcrev << dstrev << QVariant(t) << QVariant(recursion), action, m_plugin, action);
*/
    return 0;
}

KDevelop::VcsJob * KrossDistributedVersionControl::log(const KUrl & file, const KDevelop::VcsRevision & rev, long unsigned int limit)
{
/*
    VcsRevisionAdaptor *revadapt=new VcsRevisionAdaptor(&rev, action);
    QVariant revision;
    revision.setValue((QObject*) &revadapt);

    return new KrossVcsJob("log",
                                         QVariantList() << file << revision << qlonglong(limit), action, m_plugin, action);
*/
    return 0;
}

VcsJob* KrossDistributedVersionControl::log(const KUrl& file, const VcsRevision& rev, const VcsRevision& lim)
{
/*
    VcsRevisionAdaptor *revadapt=new VcsRevisionAdaptor(&rev, action);
    QVariant revision;
    revision.setValue((QObject*) revadapt);

    VcsRevisionAdaptor *limitadapt=new VcsRevisionAdaptor(&lim, action);
    QVariant limit;
    limit.setValue((QObject*) limitadapt);

    return new KrossVcsJob("log",
                                         QVariantList() << file << revision << limit, action, m_plugin, action);
*/
    return 0;
}

VcsJob* KrossDistributedVersionControl::annotate(const KUrl& file, const VcsRevision& rev)
{
/*
    VcsRevisionAdaptor *revadapt=new VcsRevisionAdaptor(&rev, action);
    QVariant revision;
    revision.setValue((QObject*) revadapt);
    
    return new KrossVcsJob("annotate", QVariantList() << file << revision, action, m_plugin, action);
*/
    return 0;
}

VcsJob* KrossDistributedVersionControl::merge(const VcsLocation& localOrRepoLocationSrc,
                                              const VcsLocation& localOrRepoLocationDst,
                                              const VcsRevision& srcRevision,
                                              const VcsRevision& dstRevision,
                                              const KUrl& localLocation )
{
/*
    VcsRevisionAdaptor *srcrevadapt=new VcsRevisionAdaptor(&srcRevision, action);
    QVariant srcrev;
    srcrev.setValue((QObject*) srcrevadapt);
    
    VcsRevisionAdaptor *dstrevadapt=new VcsRevisionAdaptor(&dstRevision, action);
    QVariant dstrev;
    dstrev.setValue((QObject*) dstrevadapt);
    
    VcsLocationAdaptor *srclocadapt=new VcsLocationAdaptor(&localOrRepoLocationSrc, action);
    QVariant srcloc;
    srcloc.setValue((QObject*) &srclocadapt);
    
    VcsLocationAdaptor *dstlocadapt=new VcsLocationAdaptor(&localOrRepoLocationDst, action);
    QVariant dstloc;
    dstloc.setValue((QObject*) dstlocadapt);
    
    return new KrossVcsJob("merge",
                                         QVariantList() << srcloc << dstloc << srcrev << dstrev << localLocation, action, m_plugin, action);
*/
    return 0;
}

VcsJob* KrossDistributedVersionControl::resolve( const KUrl::List& localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion )
{
    return new KrossVcsJob("resolve", QVariantList() << localLocations << recursion, action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::init(const KUrl& location)
{
    return new KrossVcsJob("init", QVariantList() << location, action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::clone(const KDevelop::VcsLocation& localOrRepoLocationSrc, const KUrl& localRepositoryLocationSrc)
{
    return new KrossVcsJob("clone", QVariantList() << localOrRepoLocationSrc.localUrl() << localRepositoryLocationSrc, action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::push(const KUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst)
{
    return new KrossVcsJob("push", QVariantList() << localRepositoryLocation << localOrRepoLocationDst.localUrl(), action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const KUrl& localRepositoryLocation)
{
    return new KrossVcsJob("pull", QVariantList() << localOrRepoLocationSrc.localUrl() << localRepositoryLocation, action, m_plugin, action);
}

#include "krossdistributedversioncontrol.moc"
