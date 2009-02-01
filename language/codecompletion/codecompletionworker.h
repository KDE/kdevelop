/*
 * KDevelop Generic Code Completion Support
 *
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
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

#ifndef KDEV_CODECOMPLETIONWORKER_H
#define KDEV_CODECOMPLETIONWORKER_H

#include <QtCore/QThread>
#include <QtCore/QList>

#include <KDE/KTextEditor/Cursor>
#include <KDE/KTextEditor/Range>

#include "../languageexport.h"
#include "../duchain/duchainpointer.h"
#include "../codecompletion/codecompletioncontext.h"

class QMutex;

namespace KTextEditor {
  class Range;
  class View;
  class Cursor;
}

namespace KDevelop
{

class CodeCompletion;
class CompletionTreeElement;
class CodeCompletionModel;


class KDEVPLATFORMLANGUAGE_EXPORT CodeCompletionWorker : public QObject
{
  Q_OBJECT

  public:
    CodeCompletionWorker(QObject* parent);
    virtual ~CodeCompletionWorker();

    void abortCurrentCompletion();

    void setFullCompletion(bool);
    bool fullCompletion() const;

    KDevelop::CodeCompletionModel* model() const;

  Q_SIGNALS:
    void foundDeclarations(QList<KSharedPtr<CompletionTreeElement> >, KSharedPtr<CodeCompletionContext> completionContext);

  protected:
    
    virtual void computeCompletions(DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view, const KTextEditor::Range& contextRange, const QString& contextText);
    virtual QList<KSharedPtr<CompletionTreeElement> > computeGroups(QList<CompletionTreeItemPointer> items, KSharedPtr<CodeCompletionContext> completionContext);
    virtual KDevelop::CodeCompletionContext* createCompletionContext(KDevelop::DUContextPointer context, const QString &contextText, const QString &followingText) const = 0;

    bool& aborting();
    
  protected Q_SLOTS:
    void computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view);

  private:
    QMutex* m_mutex;
    bool m_abort;
    bool m_fullCompletion;
};

}

#endif // KDEVCODECOMPLETIONWORKER_H
