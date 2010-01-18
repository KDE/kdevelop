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
#include <language/duchain/classdeclaration.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/coderepresentation.h>

#include "../codecompletion/implementationhelperitem.h"
#include "../codecompletion/missingincludeitem.h"

using namespace KDevelop;

CppClassIdentifierPage::CppClassIdentifierPage(QWizard* parent)
  : KDevelop::ClassIdentifierPage(parent)
{
    // Give the user a hint that the access info should be provided here too
    inheritanceLineEdit()->setText("public ");
}

QualifiedIdentifier CppClassIdentifierPage::parseParentClassId(const QString & inheritedObject)
{
  // TODO: properly strip qualifiers
  QString identifier = inheritedObject;
  identifier = identifier.remove("public ", Qt::CaseInsensitive).remove("protected ", Qt::CaseInsensitive).remove("private ", Qt::CaseInsensitive).simplified();

  return QualifiedIdentifier(identifier);
}

CppOverridesPage::CppOverridesPage(QWizard* parent)
  : KDevelop::OverridesPage(parent)
{
}

void CppOverridesPage::addPotentialOverride(QTreeWidgetItem* classItem, KDevelop::DeclarationPointer childDeclaration)
{
  // HACK: filter out Qt's moc calls
  QString id = childDeclaration->identifier().toString();
  if (id == "qt_metacall" || id == "qt_metacast" || id == "metaObject")
    return;

  OverridesPage::addPotentialOverride(classItem, childDeclaration);
}

CppNewClassWizard::CppNewClassWizard(QWidget* parent, CppNewClass* generator, KUrl baseUrl)
  : CreateClassWizard(parent, generator, baseUrl)
{
  setup();
}

CppClassIdentifierPage* CppNewClassWizard::newIdentifierPage()
{
  return new CppClassIdentifierPage(this);
}

CppOverridesPage* CppNewClassWizard::newOverridesPage()
{
  return new CppOverridesPage(this);
}

KDevelop::DocumentChangeSet CppNewClass::generate()
{
  
  KDevelop::DocumentChangeSet changeSet0 = generateHeader();
  KDevelop::DocumentChangeSet changeSet1 = generateImplementation();
  
  return changeSet0 << changeSet1;
}

const QList<KDevelop::DeclarationPointer> & CppNewClass::addBaseClass(const QString & base)
{
  if(base.isEmpty())
    return m_baseClasses;
  //strip access specifier
  QStringList splitBase = base.split(' ', QString::SkipEmptyParts);
  
  //if no access specifier is found use public by default
  if(splitBase.size() == 1)
    m_baseAccessSpecifiers << "public";
  else
    m_baseAccessSpecifiers << splitBase[0];
  
  //Call base function with stripped access specifier
  return ClassGenerator::addBaseClass(splitBase.last());
}

void CppNewClass::clearInheritance()
{
    ClassGenerator::clearInheritance();
    m_baseAccessSpecifiers.clear();
}

void CppNewClass::identifier(const QString & identifier)
{
  QStringList list = identifier.split("::");
  name(list.last());
  m_objectType->setDeclarationId(DeclarationId(QualifiedIdentifier(name())));
  list.pop_back();
  m_namespaces = list;
}

StructureType::Ptr CppNewClass::objectType() const
{
  return StructureType::Ptr::staticCast<CppClassType>(m_objectType);
}

void CppNewClass::setType(Type type)
{
  m_type = type;
}


QString CppNewClass::identifier(void) const
{
  QString identifier = m_namespaces.join("::");
  
  identifier.append(m_namespaces.empty() ? name() : "::" + name());
  return identifier;
}

KUrl CppNewClass::headerUrlFromBase(KUrl baseUrl, bool toLower) {
  KUrl url(baseUrl); ///@todo Add some settings somewhere to set up how this is computed
  if( toLower )
    url.addPath(name().toLower() + ".h");
  else
    url.addPath(name() + ".h");
  return url;
}

KUrl CppNewClass::implementationUrlFromBase(KUrl baseUrl, bool toLower) {
  KUrl url(baseUrl);
  if( toLower )
    url.addPath(name().toLower() + ".cpp");
  else
    url.addPath(name() + ".cpp");
  return url;
}

KDevelop::DocumentChangeSet CppNewClass::generateHeader()
{
  QString header;

  QTextStream output(&header, QIODevice::WriteOnly);
  
  kDebug() << "base-classes:" << m_baseClasses.size();
  
  if(!license().isEmpty())
    output << license() << "\n\n";

  Identifier classId = Identifier(name());

  // Header protector
  //If this is a new file add the header guard
  QString headerGuard;
  bool headerExists;
  {
    QFileInfo info(headerUrl().toLocalFile());
    headerExists = info.exists() || artificialCodeRepresentationExists(IndexedString(headerUrl()));
  }
  if(!headerExists) 
  {
    if(headerPosition() != SimpleCursor())
    {
      kWarning() << "A header position was specified for a new file, ignoring.";
      setHeaderPosition(SimpleCursor());
    }
    
    headerGuard = headerUrl().fileName().toUpper().replace('.', '_').replace('-','_');
    if (m_namespaces.size())
      headerGuard.prepend(m_namespaces.join("_").toUpper() + '_');

    
    output << "#ifndef " << headerGuard << '\n';
    output << "#define " << headerGuard << "\n\n";
  }

  //Add #includes
  
  bool addedIncludes = false;
 
  DUChainReadLocker lock(DUChain::lock());
 
  foreach(DeclarationPointer base, m_baseClasses) {
    if(!base.data())
      continue;
    KSharedPtr<Cpp::MissingIncludeCompletionItem> item = Cpp::includeDirectiveFromUrl(headerUrl(), IndexedDeclaration(base.data()));
    if(item) {
      output << item->lineToInsert() << "\n";
      addedIncludes = true;
    }
  }
  
  if(addedIncludes)
    output << "\n\n";
  
  //Support for nested namespaces
  foreach(QString ns, m_namespaces)
    output << "namespace " << ns << " {\n\n";

  output << (m_type == Class || m_type == DefaultType ? "class " : "struct ");

  //if (useExportMacro) output << macroExportString;

  output << classId.toString();

  Q_ASSERT(m_baseClasses.size() == m_baseAccessSpecifiers.size());
  for(int i = 0; i < m_baseClasses.size(); ++i) {
      if(!m_baseClasses[i].data())
        continue;
      output << (i == 0 ? " : " : ", ") << m_baseAccessSpecifiers[i] << ' ' << m_baseClasses[i]->qualifiedIdentifier().toString() ;
  }

  output << "\n{\n";

  // if (baseClassIsQObjectDerivative) output << "\tQ_OBJECT\n\n";

  Declaration::AccessPolicy ap = m_type == Class ? Declaration::Private : Declaration::Public;

  // Constructor(s)

  // Overrides

  foreach (DeclarationPointer override, m_declarations) {
    if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(override.data())) {
      if (ap != member->accessPolicy()) {
        switch (member->accessPolicy()) {
          case Declaration::DefaultAccess:
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
    
    if(override->type<FunctionType>())
    {
      Cpp::ImplementationHelperItem item(Cpp::ImplementationHelperItem::Override, override);
      output << item.insertionText() << "\n";
    }
    else
      output << override->toString() << ";\n";
  }

  output << "};\n\n";

  for(int i = 0; i < m_namespaces.size(); ++i)
    output << "}\n\n";

  if(!headerExists)
    output << "#endif // " << headerGuard << '\n';
  
  DocumentChangeSet changes;
  changes.addChange(DocumentChange(IndexedString(headerUrl()),
                    SimpleRange(headerPosition(), 0), QString(), header));

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
      KDevelop::BaseClassInstance base;
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

  lock.unlock(); //Never call openDocument while the duchain is locked
  //TODO DocumentChangeSet should take care of opening new documents in the editor
  ICore::self()->documentController()->openDocument(headerUrl());
  
  return changes;
}

KDevelop::DocumentChangeSet CppNewClass::generateImplementation()
{
  QString implementation;

  QTextStream output(&implementation, QIODevice::WriteOnly);

  if(!license().isEmpty())
    output << "/*\n" << license() << "\n*/\n\n";

  // #include our header
  QString path = KUrl::relativePath(implementationUrl().directory(), headerUrl().toLocalFile());
  if(path.startsWith("./"))
    path = path.mid(2);
  output << "#include \"" << path << "\"\n\n";

  // Namespace
  QualifiedIdentifier id(identifier());
  
  Identifier classId = id.last();

  bool ns = false;

  if (id.count() > 1) {
    ns = true;
    id.pop();
  }

  if (ns)
    // Add using statement
    ///@todo make configurable
    output << "using namespace " << id.toString() << ";\n\n";

  // Overrides
  {
  KDevelop::DUChainReadLocker lock( DUChain::lock() );

  foreach (DeclarationPointer override, m_declarations) {
    Cpp::ImplementationHelperItem item(Cpp::ImplementationHelperItem::CreateDefinition, override);

    output << item.insertionText(KUrl(), KDevelop::SimpleCursor(), QualifiedIdentifier(classId)) << "\n";
  }
  }
  
  DocumentChangeSet changes;
  
  changes.addChange(DocumentChange(IndexedString(implementationUrl()), SimpleRange(implementationPosition(), 0),
                                   QString(), implementation));
  
  return changes;
}

#include "cppnewclass.moc"
