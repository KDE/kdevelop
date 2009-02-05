/*
 * KDevelop C++ Language Support
 *
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
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

#include "cppnewclass.h"

#include <QFile>
#include <QTextStream>

#include <KLineEdit>
#include <KUrl>
#include <KMessageBox>
#include <KLocale>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <language/duchain/identifier.h>
#include <language/duchain/declaration.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/persistentsymboltable.h>

#include "../cppduchain/classdeclaration.h"
#include <implementationhelperitem.h>

using namespace KDevelop;

CppClassIdentifierPage::CppClassIdentifierPage(QWizard* parent)
  : KDevelop::ClassIdentifierPage(parent)
{
    // Give the user a hint that the access info should be provided here too
    inheritanceLineEdit()->setText("public ");
}

CppOverridesPage::CppOverridesPage(QWizard* parent)
  : KDevelop::OverridesPage(parent)
{
}

void CppOverridesPage::addPotentialOverride(QTreeWidgetItem* classItem, KDevelop::Declaration* childDeclaration)
{
  // HACK: filter out Qt's moc calls
  QString id = childDeclaration->identifier().toString();
  if (id == "qt_metacall" || id == "qt_metacast" || id == "metaObject")
    return;

  OverridesPage::addPotentialOverride(classItem, childDeclaration);
}

CppNewClass::CppNewClass(QWidget* parent, KUrl baseUrl)
  : KDevelop::CreateClass(parent, baseUrl)
{
  setup();
}

CppClassIdentifierPage* CppNewClass::newIdentifierPage()
{
  return new CppClassIdentifierPage(this);
}

CppOverridesPage* CppNewClass::newOverridesPage()
{
  return new CppOverridesPage(this);
}

void CppNewClass::generate()
{
  generateHeader();
  generateImplementation();
}

KUrl CppNewClass::headerUrlFromBase(QString className, KUrl baseUrl) {
  KUrl url(baseUrl); ///@todo Add some settings somewhere to set up how this is computed
  url.addPath(className.toLower() + ".h");
  return url;
}

KUrl CppNewClass::implementationUrlFromBase(QString className, KUrl baseUrl) {
  KUrl url(baseUrl);
  url.addPath(className.toLower() + ".cpp");
  return url;
}

void CppNewClass::generateHeader()
{
  KUrl url = field("headerUrl").value<KUrl>();

  QFile file;
  file.setFileName(url.path());
  if (!file.open(QIODevice::WriteOnly)) {
    KMessageBox::error(this, i18n("Unable to open '%1' to write the new class header.", url.prettyUrl()), i18n("Header generation error"));
    return;
  }

  QTextStream output(&file);

  output << "/*\n" << field("license").toString() << "*/\n\n";

  // Namespace
  QualifiedIdentifier id(field("classIdentifier").toString());
  Identifier classId = id.last();

  bool ns = false;

  if (id.count() > 1) {
    ns = true;
    id.pop();
  }

  // Header protector
  QString headerGuard = url.fileName().toUpper().replace('.', '_');
  if (ns)
    headerGuard.prepend(id.toString().toUpper().replace("::", "_") + '_');

  output << "#ifndef " << headerGuard << '\n';
  output << "#define " << headerGuard << "\n\n";

  if (ns)
    output << "namespace " << id.toString() << " {\n\n";

  output << "class ";

  //if (useExportMacro) output << macroExportString;

  output << classId.toString();

  bool first = true;
  foreach (const QString& inherits, field("classInheritance").toStringList())
    if (first) {
      first = false;
      output << " : " << inherits;
    } else {
      output << ", " << inherits;
    }

  output << "\n{\n";

  // if (baseClassIsQObjectDerivative) output << "\tQ_OBJECT\n\n";

  output << "public:\n";
  Declaration::AccessPolicy ap = Declaration::Public;

  // Constructor(s)

  // Overrides
  {
  KDevelop::DUChainReadLocker lock( DUChain::lock() );

  foreach (const QVariant& override, qvariant_cast<QVariantList>(field("overrides"))) {
    IndexedDeclaration id = qvariant_cast<IndexedDeclaration>(override);
    if (Declaration* d = id.declaration()) {
      if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(d)) {
        if (ap != member->accessPolicy()) {
          switch (member->accessPolicy()) {
            case Declaration::Public:
              output << "\npublic:\n";
              break;

            case Declaration::Protected:
              output << "\nprotected:\n";
              break;

            case Declaration::Private:
              output << "\nprivate:\n";
              break;
          }
          ap = member->accessPolicy();
        }
      }
      
      ImplementationHelperItem item(ImplementationHelperItem::Override, DeclarationPointer(d));
      output << item.insertionText() << "\n";
    }
  }
  }

  output << "};\n\n";

  if (ns) {
    output << "}\n\n";
  }

  output << "#endif // " << headerGuard << '\n';

  file.close();

#if 0
  {
    // New generation code
    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    DUChainChangeSet newHeaderChange(ReferencedTopDUContext());

    QualifiedIdentifier classId(field("classIdentifier").toString());
    DUChainRef* newClass = newHeaderChange.newClass(classId);

    foreach (const QString& inherits, field("classInheritance").toStringList()) {
      // TODO: properly strip qualifiers
      // TODO: move lookup to CppClassIdentifierPage
      Cpp::BaseClassInstance base;
      if (inherits.beginsWith("protected "))
        base.access = Declaration::Protected;
      else if (inherits.beginsWith("private "))
        base.access = Declaration::Private;
      else
        base.access = Declaration::Public;

      if (inherits.contains(" virtual "))
        base.virtualInheritance = true;

      QString identifier = inherits;
      identifier = identifier.remove("public ", Qt::CaseInsensitive).remove("protected ", Qt::CaseInsensitive).remove("private ", Qt::CaseInsensitive).simplified();

      PersistentSymbolTable::Declarations declarations = PersistentSymbolTable::self().getDeclarations( IndexedQualifiedIdentifier( QualifiedIdentifier(identifier)) );

      for (PersistentSymbolTable::Declarations::Iterator it = declarations.iterator(); it; ++it) {
        Declaration* decl = it->declaration();
        if (decl->isForwardDeclaration())
          continue;

        // Check if it's a class/struct/etc
        if (decl->type<StructureType>()) {
          base.baseClass = decl->type<StructureType>();

          newClass->addBaseClass(base);
          break;
        }
      }
    }

    foreach (const QVariant& override, qvariant_cast<QVariantList>(field("overrides"))) {
      IndexedDeclaration id = qvariant_cast<IndexedDeclaration>(override);
      if (id.isValid()) {
        newClass->appendDeclaration( id->declaration() );
      }
    }
  }
#endif

  ICore::self()->documentController()->openDocument(url);
}

void CppNewClass::generateImplementation()
{
  KUrl headerUrl = field("headerUrl").value<KUrl>();
  KUrl url = field("implementationUrl").value<KUrl>();

  QFile file;
  file.setFileName(url.path());
  if (!file.open(QIODevice::WriteOnly)) {
    KMessageBox::error(this, i18n("Unable to open '%1' to write the new class implementation.", url.prettyUrl()), i18n("Implementation generation error"));
    return;
  }

  QTextStream output(&file);

  output << "/*\n" << field("license").toString() << "*/\n\n";

  // #include our header
  output << "#include \"" << KUrl::relativePath(url.directory(), headerUrl.path()) << "\"\n\n";

  // Namespace
  QualifiedIdentifier id(field("classIdentifier").toString());
  Identifier classId = id.last();

  bool ns = false;

  if (id.count() > 1) {
    ns = true;
    id.pop();
  }

  if (ns)
    // Add using statement
    // TODO make configurable
    output << "using namespace " << id.toString() << "\n\n";

  // Overrides
  {
  KDevelop::DUChainReadLocker lock( DUChain::lock() );

  foreach (const QVariant& override, qvariant_cast<QVariantList>(field("overrides"))) {
    IndexedDeclaration indexedDecl = qvariant_cast<IndexedDeclaration>(override);
    if (Declaration* d = indexedDecl.declaration()) {
      ImplementationHelperItem item(ImplementationHelperItem::CreateDefinition, DeclarationPointer(d));

      output << item.insertionText() << "\n";
    }
  }
  }

  file.close();

  ICore::self()->documentController()->openDocument(url);
}

#include "cppnewclass.moc"
