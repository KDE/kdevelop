/*
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

#ifndef MISSINGINCLUDECOMPLETIONMODEL_H
#define MISSINGINCLUDECOMPLETIONMODEL_H

#include <language/codecompletion/codecompletionmodel.h>
#include <language/codecompletion/codecompletionworker.h>
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <language/duchain/ducontext.h>

namespace KDevelop {
class ParseJob;
}

namespace Cpp {
class MissingIncludeCompletionWorker;

class MissingIncludeCompletionModel : public KDevelop::CodeCompletionModel
{
  Q_OBJECT
public:
  
  void stop();
  
  MissingIncludeCompletionModel(QObject* parent);
  ~MissingIncludeCompletionModel();

  //Bridge from Cpp::CodeCompletionModel to this model
  void startWithExpression(KDevelop::DUContextPointer context, const QString& prefixExpression, const QString& localExpression);
  
  static MissingIncludeCompletionModel& self();
  
  static void startCompletionAfterParsing(const KDevelop::IndexedString& url);
  
  private slots:
  void parseJobFinished(KDevelop::ParseJob* job);
  
protected:
  
  virtual QString filterString(KTextEditor::View* view, const KTextEditor::Range& range, const KTextEditor::Cursor& position);
  virtual KTextEditor::Range updateCompletionRange(KTextEditor::View* view, const KTextEditor::Range& range);
  MissingIncludeCompletionWorker* worker() const;
  virtual void completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range, KTextEditor::CodeCompletionModel::InvocationType invocationType, const KUrl& url);
  virtual KDevelop::CodeCompletionWorker* createCompletionWorker ();
  virtual KTextEditor::CodeCompletionModelControllerInterface::MatchReaction matchingItem(const QModelIndex& matched);
};

class MissingIncludeCompletionWorker : public KDevelop::CodeCompletionWorker {
  public:
  MissingIncludeCompletionWorker(KDevelop::CodeCompletionModel* model);
  ~MissingIncludeCompletionWorker();
  
  QMutex mutex;
  QWaitCondition wait;
  QString localExpression; //The local expression, that is represented by the completion-range, for example "map" as component of "std::map"
  QString prefixExpression; //The prefix-expression, like "std::"
  //This contains the current surrounding context, and is also used as marker whether this completion is enabled.
  KDevelop::IndexedDUContext context;
  
  void allowCompletion();
  
  bool aborted() {
    return aborting();
  }
  
  virtual void abortCurrentCompletion();
  virtual void doSpecialProcessing(unsigned int data);
  protected:
  virtual void computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, QString followingText, const KTextEditor::Range& contextRange, const QString& contextText);
};

}

#endif // MISSINGINCLUDECOMPLETIONMODEL_H
