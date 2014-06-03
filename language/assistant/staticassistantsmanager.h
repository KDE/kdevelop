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

#include "staticassistant.h"

#include <language/languageexport.h>
#include <language/duchain/indexedstring.h>

#include <ktexteditor/range.h>
#include <ktexteditor/cursor.h>

class QTimer;

typedef QWeakPointer<KTextEditor::Document> SafeDocumentPointer;

namespace KDevelop {

class IDocument;
class ParseJob;
class DUContext;
class TopDUContext;

/**
 * @brief Class managing instances of StaticAssistant
 *
 * Invokes the appropiate methods on registered StaticAssistant instances, such as StaticAssistant::textChanged
 *
 * @sa StaticAssistant::textChanged
 */
class KDEVPLATFORMLANGUAGE_EXPORT StaticAssistantsManager : public QObject
{
    Q_OBJECT

public:
    StaticAssistantsManager(QObject* parent = 0);
    virtual ~StaticAssistantsManager();

    KSharedPtr<KDevelop::IAssistant> activeAssistant();

    void registerAssistant(const StaticAssistant::Ptr assistant);
    void unregisterAssistant(const StaticAssistant::Ptr assistant);
    QList<StaticAssistant::Ptr> registeredAssistants() const;

public slots:
    void hideAssistant();

private:
    struct Private;
    QScopedPointer<Private> const d;

    Q_PRIVATE_SLOT(d, void documentLoaded(KDevelop::IDocument*));
    Q_PRIVATE_SLOT(d, void textInserted(KTextEditor::Document*, const KTextEditor::Range&));
    Q_PRIVATE_SLOT(d, void textRemoved(KTextEditor::Document*, const KTextEditor::Range&, const QString& removedText));
    Q_PRIVATE_SLOT(d, void parseJobFinished(KDevelop::ParseJob*));
    Q_PRIVATE_SLOT(d, void documentActivated(KDevelop::IDocument*));
    Q_PRIVATE_SLOT(d, void cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&));
    Q_PRIVATE_SLOT(d, void timeout());
    Q_PRIVATE_SLOT(d, void eventuallyStartAssistant());
};

}

#endif // KDEVPLATFORM_STATICASSISTANTSMANAGER_H
