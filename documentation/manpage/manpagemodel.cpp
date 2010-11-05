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

#include "manpagemodel.moc"
