/*
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentationquickopenprovider.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/idocumentationprovider.h>
#include <KLocalizedString>
#include <QAbstractItemModel>
#include <QIcon>

using namespace KDevelop;

class DocumentationQuickOpenItem
    : public QuickOpenDataBase
{
public:
    DocumentationQuickOpenItem(const QModelIndex& data, IDocumentationProvider* p)
        : QuickOpenDataBase()
        , m_data(data)
        , m_provider(p)
    {}

    QString text() const override
    {
        return m_data.data().toString();
    }
    QString htmlDescription() const override
    {
        return i18n("Documentation in the %1", m_provider->name());
    }
    bool execute(QString&) override
    {
        IDocumentation::Ptr docu = m_provider->documentationForIndex(m_data);
        if (docu) {
            ICore::self()->documentationController()->showDocumentation(docu);
        }
        return docu;
    }
    QIcon icon() const override
    {
        return m_provider->icon();
    }

private:
    const QModelIndex m_data;
    IDocumentationProvider* const m_provider;
};

namespace {
uint recursiveRowCount(const QAbstractItemModel* m, const QModelIndex& idx)
{
    uint rows = m->rowCount(idx);
    uint ret = rows;

    for (uint i = 0; i < rows; i++) {
        ret += recursiveRowCount(m, m->index(i, 0, idx));
    }

    return ret;
}

void matchingIndexes(const QAbstractItemModel* m, const QString& match, const QModelIndex& idx, QList<QModelIndex>& ret, int& preferred)
{
    if (m->hasChildren(idx)) {
        for (int i = 0, rows = m->rowCount(); i < rows; i++) {
            matchingIndexes(m, match, m->index(i, 0, idx), ret, preferred);
        }
    } else {
        int index = idx.data().toString().indexOf(match, 0, Qt::CaseInsensitive);
        if (index == 0) {
            ret.insert(preferred++, idx);
        } else if (index > 0) {
            ret.append(idx);
        }
    }
}
}

DocumentationQuickOpenProvider::DocumentationQuickOpenProvider()
{
    connect(ICore::self()->documentationController(), &IDocumentationController::providersChanged,
            this, &DocumentationQuickOpenProvider::reset);
}

void DocumentationQuickOpenProvider::setFilterText(const QString& text)
{
    if (text.size() < 2) {
        return;
    }
    m_results.clear();
    int split = 0;
    const QList<IDocumentationProvider*> providers = ICore::self()->documentationController()->documentationProviders();
    for (IDocumentationProvider* p : providers) {
        QList<QModelIndex> idxs;
        int internalSplit = 0;
        int i = 0;
        matchingIndexes(p->indexModel(), text, QModelIndex(), idxs, internalSplit);
        for (const QModelIndex& idx : std::as_const(idxs)) {
            m_results.insert(split + i, QuickOpenDataPointer(new DocumentationQuickOpenItem(idx, p)));
            i++;
        }

        split += internalSplit;
    }
}

uint DocumentationQuickOpenProvider::unfilteredItemCount() const
{
    uint ret = 0;
    const QList<IDocumentationProvider*> providers = ICore::self()->documentationController()->documentationProviders();
    for (IDocumentationProvider* p : providers) {
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

#include "moc_documentationquickopenprovider.cpp"
