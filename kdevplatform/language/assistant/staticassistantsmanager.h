/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_STATICASSISTANTSMANAGER_H
#define KDEVPLATFORM_STATICASSISTANTSMANAGER_H

#include <QObject>
#include <QPointer>

#include "staticassistant.h"

#include <language/languageexport.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/problem.h>
#include <language/duchain/topducontext.h>
#include <serialization/indexedstring.h>

using SafeDocumentPointer = QPointer<KTextEditor::Document>;

namespace KDevelop {
class IDocument;
class DUContext;
class TopDUContext;
class StaticAssistantsManagerPrivate;

/**
 * @brief Class managing instances of StaticAssistant
 *
 * Invokes the appropriate methods on registered StaticAssistant instances, such as StaticAssistant::textChanged
 *
 * @sa StaticAssistant::textChanged
 */
class KDEVPLATFORMLANGUAGE_EXPORT StaticAssistantsManager
    : public QObject
{
    Q_OBJECT

public:
    explicit StaticAssistantsManager(QObject* parent = nullptr);
    ~StaticAssistantsManager() override;

    void registerAssistant(const StaticAssistant::Ptr& assistant);
    void unregisterAssistant(const StaticAssistant::Ptr& assistant);
    QVector<StaticAssistant::Ptr> registeredAssistants() const;
    void notifyAssistants(const IndexedString& url, const KDevelop::ReferencedTopDUContext& context);

    QVector<KDevelop::Problem::Ptr> problemsForContext(const ReferencedTopDUContext& top) const;

Q_SIGNALS:
    void problemsChanged(const IndexedString& url);

private:
    const QScopedPointer<class StaticAssistantsManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(StaticAssistantsManager)
};
}

#endif // KDEVPLATFORM_STATICASSISTANTSMANAGER_H
