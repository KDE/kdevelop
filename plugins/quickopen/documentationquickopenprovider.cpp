/*
 * This file is part of KDevelop
 *
 * Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "documentationquickopenprovider.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/idocumentationprovider.h>
#include <KLocalizedString>
#include <QAbstractItemModel>
#include <QTime>
#include <QIcon>

using namespace KDevelop;

class DocumentationQuickOpenItem : public QuickOpenDataBase
{
public:
    DocumentationQuickOpenItem(const QModelIndex& data, IDocumentationProvider* p)
    : QuickOpenDataBase()
    , m_data(data)
    , m_provider(p)
    {}

    virtual QString text() const override
    {
        return m_data.data().toString();
    }
    virtual QString htmlDescription() const override
    {
        return i18n("Documentation in the %1", m_provider->name());
    }
    virtual bool execute(QString&) override
    {
        IDocumentation::Ptr docu = m_provider->documentationForIndex(m_data);
        if(docu) {
            ICore::self()->documentationController()->showDocumentation(docu);
        }
        return docu;
    }
    virtual QIcon icon() const override
    {
        return m_provider->icon();
    }

private:
    QModelIndex m_data;
    IDocumentation::Ptr m_ptr;
    IDocumentationProvider* m_provider;
};

namespace {
    uint recursiveRowCount(const QAbstractItemModel* m, const QModelIndex& idx)
    {
        uint rows = m->rowCount(idx);
        uint ret = rows;

        for(uint i = 0; i < rows; i++) {
            ret += recursiveRowCount(m, m->index(i, 0, idx));
        }
        return ret;
    }

    void matchingIndexes(const QAbstractItemModel* m, const QString& match, const QModelIndex& idx, QList<QModelIndex>& ret, int& preferred)
    {
        if(m->hasChildren(idx)) {
            for(int i = 0, rows = m->rowCount(); i < rows; i++) {
                matchingIndexes(m, match, m->index(i, 0, idx), ret, preferred);
            }
        } else {
            int index = idx.data().toString().indexOf(match, 0, Qt::CaseInsensitive);
            if(index == 0) {
                ret.insert(preferred++, idx);
            } else if(index > 0) {
                ret.append(idx);
            }
        }
    }
}

DocumentationQuickOpenProvider::DocumentationQuickOpenProvider()
{
}

void DocumentationQuickOpenProvider::setFilterText(const QString& text)
{
    if(text.size() < 2) {
        return;
    }
    m_results.clear();
    int split = 0;
    QList<IDocumentationProvider*> providers = ICore::self()->documentationController()->documentationProviders();
    foreach(IDocumentationProvider* p, providers) {
        QList<QModelIndex> idxs;
        int internalSplit = 0;
        int i = 0;
        matchingIndexes(p->indexModel(), text, QModelIndex(), idxs, internalSplit);
        foreach(const QModelIndex& idx, idxs) {
            m_results.insert(split+i, QuickOpenDataPointer(new DocumentationQuickOpenItem(idx, p)));
            i++;
        }
        split += internalSplit;
    }
}

uint DocumentationQuickOpenProvider::unfilteredItemCount() const
{
    uint ret = 0;
    QList<IDocumentationProvider*> providers = ICore::self()->documentationController()->documentationProviders();
    foreach(IDocumentationProvider* p, providers) {
        ret += recursiveRowCount(p->indexModel(), QModelIndex());
    }
    return ret;
}

QuickOpenDataPointer DocumentationQuickOpenProvider::data(uint row) const
{
    return m_results.at(row);
}

uint DocumentationQuickOpenProvider::itemCount() const
{
    return m_results.size();
}

void DocumentationQuickOpenProvider::reset()
{
    m_results.clear();
}
