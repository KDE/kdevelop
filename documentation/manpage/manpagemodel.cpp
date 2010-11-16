/*  This file is part of KDevelop

    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "manpagemodel.h"

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include <language/duchain/types/structuretype.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>

#include <KStandardDirs>
#include <KLocalizedString>

#include <KIO/TransferJob>
#include <KIO/Job>
#include <kio/jobclasses.h>

#include <QtDebug>

using namespace KDevelop;

ManPageModel::ManPageModel(QObject* parent)
    : QAbstractListModel(parent), m_internalFunctionsFile("")
{
    fillModel();
}

const KDevelop::IndexedString& ManPageModel::internalFunctionFile() const
{
    return m_internalFunctionsFile;
}

void ManPageModel::slotParseJobFinished( ParseJob* job )
{
    if ( job->document() == m_internalFunctionsFile ) {
        disconnect(ICore::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)),
                   this, SLOT(slotParseJobFinished(KDevelop::ParseJob*)));
        fillModel();
    }
}

void ManPageModel::fillModel()
{
    DUChainReadLocker lock(DUChain::self()->lock());

    TopDUContext* top = DUChain::self()->chainForDocument(m_internalFunctionsFile);
    if ( !top ) {
        qWarning() << "could not find DUChain for internal function file, connecting to background parser";
        connect(ICore::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)),
                this, SLOT(slotParseJobFinished(KDevelop::ParseJob*)));
        return;
    }

    kDebug() << "filling model";
    typedef QPair<Declaration*, int> DeclDepthPair;
    foreach ( const DeclDepthPair& declpair, top->allDeclarations(top->range().end, top) ) {
        if ( declpair.first->abstractType() && declpair.first->abstractType()->modifiers() & AbstractType::ConstModifier ) {
            // filter global constants, since they are hard to find in the documentation
            continue;
        }
        m_declarations << DeclarationPointer(declpair.first);

        if ( StructureType::Ptr type = declpair.first->type<StructureType>() ) {
            foreach ( Declaration* dec, type->internalContext(top)->localDeclarations() ) {
                m_declarations << DeclarationPointer(dec);
            }
        }
    }
}

bool ManPageModel::hasChildren(const QModelIndex& parent) const
{
    return parent == QModelIndex();
}

QVariant ManPageModel::data(const QModelIndex& index, int role) const
{
    return QVariant();
}

int ManPageModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_declarations.count();
}

bool ManPageModel::canFetchMore(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return false;
}

DeclarationPointer ManPageModel::declarationForIndex(const QModelIndex& index) const
{
    Q_ASSERT(m_declarations.size() > index.row());

    return m_declarations[index.row()];
}

void ManPageModel::getManPage(const KUrl& page){
    KIO::TransferJob  * transferJob = NULL;
    //page = KUrl("man:/usr/share/man/man3/a64l.3.gz");

    transferJob = KIO::get(KUrl("man:/usr/share/man/man3/a64l.3.gz"), KIO::NoReload, KIO::HideProgressInfo);
    connect( transferJob, SIGNAL( data  (  KIO::Job *, const QByteArray &)),
             this, SLOT( readDataFromManPage( KIO::Job *, const QByteArray & ) ) );
    connect( transferJob, SIGNAL( result  (  KIO::Job *)),
             this, SLOT( jobDone( KIO::Job *) ) );

    transferJob->start();
}

void ManPageModel::getManMainIndex(){
    KIO::TransferJob  * transferJob = NULL;

    transferJob = KIO::get(KUrl("man://"), KIO::NoReload, KIO::HideProgressInfo);
    connect( transferJob, SIGNAL( data  (  KIO::Job *, const QByteArray &)),
             this, SLOT( readDataFromMainIndex( KIO::Job *, const QByteArray & ) ) );
    connect( transferJob, SIGNAL( result  (  KIO::Job *)),
             this, SLOT( jobDone( KIO::Job *) ) );

    transferJob->start();
}

void ManPageModel::getManSectionIndex(const QString section){
    KIO::TransferJob  * transferJob = NULL;

    transferJob = KIO::get(KUrl("man:(" + section + ")"), KIO::NoReload, KIO::HideProgressInfo);
    connect( transferJob, SIGNAL( data  (  KIO::Job *, const QByteArray &)),
             this, SLOT( readDataFromMainIndex( KIO::Job *, const QByteArray & ) ) );
    connect( transferJob, SIGNAL( result  (  KIO::Job *)),
             this, SLOT( jobDone( KIO::Job *) ) );

    transferJob->start();
}

void ManPageModel::readDataFromManPage(KIO::Job * job, const QByteArray &data){
     qDebug() << "readDataFromManPage";
     m_manPageBuffer.append(data);
      qDebug() << data;
}

void ManPageModel::readDataFromMainIndex(KIO::Job * job, const QByteArray &data){
     qDebug() << "readDataFromMainIndex";
     m_manMainIndexBuffer.append(data);
}

void ManPageModel::readDataFromSectionIndex(KIO::Job * job, const QByteArray &data){
     qDebug() << "readDataFromSectionIndex";
     m_manSectionIndexBuffer.append(data);
}

void ManPageModel::jobDone(KIO::Job *){
     qDebug() << "jobDone";
}

#include "manpagemodel.moc"
