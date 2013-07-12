/*
 * KDevelop Generic Code Completion Support
 *
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVPLATFORM_CODECOMPLETION_H
#define KDEVPLATFORM_CODECOMPLETION_H

#include <QtCore/QObject>
#include "../languageexport.h"

namespace KParts { class Part; }
namespace KTextEditor { class Document; class View; class CodeCompletionModel; 
}

namespace KDevelop
{

class IDocument;
class ILanguage;

class KDEVPLATFORMLANGUAGE_EXPORT CodeCompletion : public QObject
{
  Q_OBJECT

  public:
    /** CodeCompletion will be the @p aModel parent.
      *  If @p language is empty, the completion model will work for all files,
      *  otherwise only for ones that contain the selected language.
    */
    CodeCompletion(QObject* parent, KTextEditor::CodeCompletionModel* aModel, const QString& language);
    virtual ~CodeCompletion();

  private Q_SLOTS:
    void textDocumentCreated(KDevelop::IDocument*);
    void viewCreated(KTextEditor::Document *document, KTextEditor::View *view);
    void documentUrlChanged(KDevelop::IDocument*);

    /**
     * check already opened documents,
     * needs to be done via delayed call to prevent infinite loop in
     * checkDocument() -> load lang plugin -> register CodeCompletion -> checkDocument() -> ...
     */
    void checkDocuments();

  private:
    
    void unregisterDocument(KTextEditor::Document*);
    void checkDocument(KTextEditor::Document*);
    
    KTextEditor::CodeCompletionModel* m_model;
    QString m_language;
};

}

#endif

