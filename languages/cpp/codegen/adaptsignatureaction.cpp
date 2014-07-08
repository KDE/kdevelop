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

#include "adaptsignatureaction.h"

#include <language/assistant/renameaction.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <KLocalizedString>
#include <KMessageBox>

#include "languages/cpp/cppduchain/cppduchain.h"

using namespace KDevelop;
using namespace Cpp;

QString makeSignatureString(const Signature& signature, DUContext* visibilityFrom) {
  QString ret;
  int pos = 0;
  foreach(const ParameterItem& item, signature.parameters) {
    if(!ret.isEmpty())
      ret += ", ";

    ///TODO: merge common code with helpers.cpp::createArgumentList
    AbstractType::Ptr type = item.first.abstractType();

    QString arrayAppendix;
    ArrayType::Ptr arrayType;
    while ((arrayType = type.cast<ArrayType>())) {
      type = arrayType->elementType();
      //note: we have to prepend since we iterate from outside, i.e. from right to left.
      if (arrayType->dimension()) {
        arrayAppendix.prepend(QString("[%1]").arg(arrayType->dimension()));
      } else {
        // dimensionless
        arrayAppendix.prepend("[]");
      }
    }
    ret += Cpp::simplifiedTypeString(type,  visibilityFrom);

    if(!item.second.isEmpty())
      ret += " " + item.second;

    ret += arrayAppendix;

    if (signature.defaultParams.size() > pos && !signature.defaultParams[pos].isEmpty())
      ret += " = " + signature.defaultParams[pos];

    ++pos;
  }
  return ret;
}

AdaptSignatureAction::AdaptSignatureAction(
  const DeclarationId& definitionId,
  ReferencedTopDUContext definitionContext,
  const Signature& oldSignature,
  const Signature& newSignature,
  bool editingDefinition,
  QList<RenameAction*> renameActions
)
: m_otherSideId(definitionId),
m_otherSideTopContext(definitionContext),
m_oldSignature(oldSignature),
m_newSignature(newSignature),
m_editingDefinition(editingDefinition),
m_renameActions(renameActions) {
}

AdaptSignatureAction::~AdaptSignatureAction()
{
  qDeleteAll(m_renameActions);
}

QString AdaptSignatureAction::description() const {
  return m_editingDefinition ? i18n("Update declaration signature") : i18n("Update definition signature");
}

QString AdaptSignatureAction::toolTip() const {
  DUChainReadLocker lock;
  return i18n("Update %1\nfrom: %2(%3)%4\nto: %2(%5)%6",
              m_editingDefinition ? i18n("declaration") : i18n("definition"),
              m_otherSideId.qualifiedIdentifier().toString(),
              makeSignatureString(m_oldSignature, m_otherSideTopContext.data()),
              m_oldSignature.isConst ? " const" : "",
              makeSignatureString(m_newSignature, m_otherSideTopContext.data()),
              m_newSignature.isConst ? " const" : "");
}

void AdaptSignatureAction::execute() {
  DUChainReadLocker lock;
  IndexedString url = m_otherSideTopContext->url();
  lock.unlock();
  m_otherSideTopContext = DUChain::self()->waitForUpdate(url, TopDUContext::AllDeclarationsContextsAndUses);
  if(!m_otherSideTopContext) {
    kDebug() << "failed to update" << url.str();
    return;
  }

  lock.lock();

  Declaration* otherSide = m_otherSideId.getDeclaration(m_otherSideTopContext.data());
  if(!otherSide) {
    kDebug() << "could not find definition";
    return;
  }

  DUContext* functionContext = DUChainUtils::getFunctionContext(otherSide);
  if(!functionContext) {
    kDebug() << "no function context";
    return;
  }

  if(!functionContext || functionContext->type() != DUContext::Function) {
    kDebug() << "no correct function context";
    return;
  }

  DocumentChangeSet changes;
  DocumentChange changeParameters(functionContext->url(), functionContext->rangeInCurrentRevision(), QString(), makeSignatureString(m_newSignature, m_otherSideTopContext.data()));
  changeParameters.m_ignoreOldText = true;
  changes.addChange( changeParameters );
  CodeRepresentation::Ptr document = createCodeRepresentation(functionContext->url());
  const int l = functionContext->range().start.line;
  const QString line = document->line(l);
  if (m_oldSignature.isConst != m_newSignature.isConst) {
    ///TODO: also use code representation here
    QRegExp pattern( QLatin1String("\\)\\s*") );
    RangeInRevision range = functionContext->range();
    // go after closing paren
    int i = line.indexOf(pattern, range.end.column);
    if (i != -1) {
      range.start.column = i;
      // we replace the full regexp match
      QString oldText = pattern.cap();
      range.end.column = i + oldText.length();
      // by the trimmed version (whitespaces are added below as neede)
      QString newText = pattern.cap().trimmed();

      if (m_oldSignature.isConst) {
        // remove const token
        oldText += "const";
        range.end.column += 5;
      } else {
        // add it
        newText += " const";
      }
      DocumentChange changeConstness(functionContext->url(), range.castToSimpleRange(), oldText, newText);
      changes.addChange(changeConstness);
    }
  }
  if (m_oldSignature.returnType != m_newSignature.returnType) {
    QRegExp exe( QString("^(\\s*)(.+)\\s+(?:\\w+::)*\\b%1\\s*\\(").arg(otherSide->identifier().toString()), Qt::CaseSensitive, QRegExp::RegExp2 );
    int pos = exe.indexIn(line);
    if (pos != -1) {
      QString oldText = exe.cap(2);
      SimpleRange range = SimpleRange(l, exe.cap(1).length(), l, exe.cap(1).length() + oldText.length());
      QString newText = Cpp::simplifiedTypeString(m_newSignature.returnType.abstractType(), functionContext->parentContext());
      DocumentChange changeRetType(functionContext->url(), range, oldText, newText);
      changes.addChange(changeRetType);
    }
  }
  changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
  DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
  if(!result) {
    KMessageBox::error(0, i18n("Failed to apply changes: %1", result.m_failureReason));
  }

  emit executed(this);

  foreach(RenameAction *renAct, m_renameActions)
    renAct->execute();
}

