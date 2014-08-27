/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#ifndef CPP_MISSINGINCLUDEASSISTANT_H
#define CPP_MISSINGINCLUDEASSISTANT_H

#include <interfaces/iassistant.h>
#include <serialization/indexedstring.h>
#include <language/duchain/problem.h>
#include <kurl.h>


namespace KDevelop {
  class IProject;
}

namespace Cpp {

class AddCustomIncludePathAction : public KDevelop::IAssistantAction {
  Q_OBJECT

public:
  AddCustomIncludePathAction(const KDevelop::IndexedString& url);
  virtual QString description() const override;
  virtual void execute() override;

private:
  KDevelop::IndexedString m_url;
};

class OpenProjectForFileAssistant : public KDevelop::IAssistantAction {
public:
  OpenProjectForFileAssistant(const KUrl&);
  virtual QString description() const override;
  virtual void execute() override;

private:
  KUrl m_url;
};

class MissingIncludePathAssistant : public KDevelop::IAssistant {
public:
  MissingIncludePathAssistant(const KDevelop::IndexedString& url, const QString& directive);
  virtual QString title() const override;
  virtual void createActions() override;
private:
  KDevelop::IndexedString m_url;
  QString m_directive;
};

class MissingIncludePathProblem : public KDevelop::Problem
{
public:
    using Ptr = QExplicitlySharedDataPointer<MissingIncludePathProblem>;

    MissingIncludePathProblem() = default;
    MissingIncludePathProblem(KDevelop::ProblemData& data)
      : KDevelop::Problem(data)
    {}

    virtual QExplicitlySharedDataPointer<KDevelop::IAssistant> solutionAssistant() const;

    void setSolutionAssistant(const QExplicitlySharedDataPointer<KDevelop::IAssistant>& assistant);

private:
    /// FIXME: persist the data for the MissingIncludePathAssistant!
    QExplicitlySharedDataPointer<KDevelop::IAssistant> m_solution;
};

}

#endif // CPP_MISSINGINCLUDEASSISTANT_H
