/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
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
