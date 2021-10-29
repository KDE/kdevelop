/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CODECOMPLETION_H
#define KDEVPLATFORM_CODECOMPLETION_H

#include <QObject>
#include <language/languageexport.h>

namespace KTextEditor {
class Document; class View; class CodeCompletionModel;
}

namespace KDevelop {
class IDocument;
class ILanguage;

// TODO: cleanup this class for 5.1
class KDEVPLATFORMLANGUAGE_EXPORT CodeCompletion
    : public QObject
{
    Q_OBJECT

public:
    /** CodeCompletion will be the @p aModel parent.
     *  If @p language is empty, the completion model will work for all files,
     *  otherwise only for ones that contain the selected language.
     */
    CodeCompletion(QObject* parent, KTextEditor::CodeCompletionModel* aModel, const QString& language);
    ~CodeCompletion() override;

private Q_SLOTS:
    void textDocumentCreated(KDevelop::IDocument*);
    void viewCreated(KTextEditor::Document* document, KTextEditor::View* view);
    void documentUrlChanged(KDevelop::IDocument*);

    /**
     * check already opened documents,
     * needs to be done via delayed call to prevent infinite loop in
     * checkDocument() -> load lang plugin -> register CodeCompletion -> checkDocument() -> ...
     */
    void checkDocuments();

Q_SIGNALS:
    void registeredToView(KTextEditor::View* view);
    void unregisteredFromView(KTextEditor::View* view);

private:

    void unregisterDocument(KTextEditor::Document*);
    void checkDocument(KTextEditor::Document*);

    KTextEditor::CodeCompletionModel* m_model;
    QString m_language;
};
}

#endif
