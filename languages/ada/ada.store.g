/* $Id$
 * ANTLR Ada tree walker for building the Kdevelop class store
 * Copyright (C) 2003 Oliver Kellogg  <okellogg@users.sourceforge.net>
 */

header "pre_include_hpp" {
#include <qstring.h>
#include <qstringlist.h>
#include <qfileinfo.h>

#include <codemodel.h>
#include "AdaAST.hpp"
#include "ada_utils.hpp"
}

header "post_include_hpp" {
#include <codemodel.h>
#include <kdebug.h>
}

options {
  language="Cpp";
}

class AdaStoreWalker extends AdaTreeParserSuper;
options {
  importVocab = Ada;
  ASTLabelType = "RefAdaAST";
}

{
private:
    QString m_fileName;
    QValueList<NamespaceDom> m_scopeStack;
    CodeModel* m_model;
    QValueList<QStringList> m_imports;
    NamespaceDom m_currentContainer;
    int m_currentAccess;
    bool m_addToStore; /* auxiliary variable: for the moment, this is `true'
                          only when we are in specs, not bodies.  */
    bool m_isSubprogram;  // auxiliary to def_id()
    FileDom m_file;

public:
    void setCodeModel (CodeModel* model)     { m_model = model; }
    CodeModel* codeModel ()                  { return m_model; }
    const CodeModel* codeModel () const      { return m_model; }

    QString fileName () const                  { return m_fileName; }
    void setFileName (const QString& fileName) { m_fileName = fileName; }

    void init () {
        m_scopeStack.clear ();
        m_imports.clear ();
        m_currentContainer = m_model->globalNamespace ();
        m_scopeStack.append (m_currentContainer);
        m_currentAccess = CodeModelItem::Public;
        m_addToStore = false;
        m_isSubprogram = false;
        if (m_model->hasFile(m_fileName))
            m_model->removeFile (m_model->fileByName(m_fileName));
        m_file = m_model->create<FileModel>();
        m_file->setName(m_fileName);
        m_model->addFile(m_file);
    }

    void wipeout ()            { m_model->wipeout (); }
//    void out ()                { m_store->out (); }
    void removeWithReferences (const QString& fileName) {
        m_model->removeFile (m_model->fileByName(fileName));
    }
    NamespaceDom insertScopeContainer
                (NamespaceDom scope, const QStringList & scopes ) {
        QStringList::ConstIterator it = scopes.begin();
        QString prefix( *it );
        NamespaceDom ns = scope->namespaceByName( prefix );
//        kdDebug() << "insertScopeContainer begin with prefix " << prefix << endl;
        if (!ns.data()) {
//            kdDebug() << "insertScopeContainer: ns is empty" << endl;
            ns = m_model->create<NamespaceModel>();
//            kdDebug() << "insertScopeContainer: ns created" << endl;
            ns->setName( prefix );
//            kdDebug() << "insertScopeContainer: ns name set" << endl;
            scope->addNamespace( ns );
//            kdDebug() << "insertScopeContainer: ns added to a scope" << endl;

            if (scope == m_model->globalNamespace())
                m_file->addNamespace( ns );
        }
//        kdDebug() << "insertScopeContainer: while" << endl;
        while ( ++it != scopes.end() ) {
            QString nameSegment( *it );
            prefix += "." + nameSegment;
//            kdDebug() << "insertScopeContainer: while prefix = " << prefix << endl;
            NamespaceDom inner = scope->namespaceByName( prefix );
            if (!inner.data() ) {
//                kdDebug() << "insertScopeContainer: inner is empty " << endl;
                inner = m_model->create<NamespaceModel>();
//                kdDebug() << "insertScopeContainer: inner created " << endl;
                inner->setName( nameSegment );
                ns->addNamespace( inner );
//                kdDebug() << "insertScopeContainer: inner added " << endl;
            }
            ns = inner;
        }
        return ns;
    }
    NamespaceDom defineScope( RefAdaAST namenode ) {
       QStringList scopes( qnamelist( namenode ) );
//        kdDebug() << "defineScope: " << scopes.join(" ") << endl;
       NamespaceDom psc = insertScopeContainer( m_currentContainer, scopes );
//        kdDebug() << "defineScope psc created" << endl;
       psc->setStartPosition(namenode->getLine(), namenode->getColumn());
//        kdDebug() << "defineScope start position set" << endl;
       psc->setFileName(m_fileName);
//        kdDebug() << "defineScope file name set" << endl;
       // psc->setEndPosition (endLine, 0);
//        kdDebug() << "defineScope return" << endl;
       return psc;
    }
}

/*
 * Only those rules that require specific action for the kdevelop
 * class store are overloaded here.
 */

compilation_unit
	: { init(); }
	  context_items_opt ( library_item | subunit ) ( pragma )*
	;


with_clause
	: #(WITH_CLAUSE ( compound_name
			  // TBD: slurp in the actual files
	                )+ )
	;

/*
compound_name
	: IDENTIFIER
	| #(DOT compound_name IDENTIFIER)
	;
 */

use_clause
	: #(USE_TYPE_CLAUSE ( subtype_mark )+ )
	| #(USE_CLAUSE ( c:compound_name
			{ m_imports.back ().push_back (qtext (#c)); }
		       )+ )
	;


library_item :
	#(LIBRARY_ITEM
		#(MODIFIERS ( PRIVATE { m_currentAccess = CodeModelItem::Protected; } )? )
		( lib_subprog_decl_or_rename_or_inst_or_body
		| #(PACKAGE_BODY pb:def_id pkg_body_part)
		| #(GENERIC_PACKAGE_INSTANTIATION gpi:def_id
		     {
		       defineScope( #gpi );
		     }
		     generic_inst
		   )
		| #(PACKAGE_SPECIFICATION ps:def_id
		     {
		       NamespaceDom psc = defineScope( #ps );
		       m_currentContainer = psc;
		       m_scopeStack.append( psc );
		       m_addToStore = true;
		     }
		    pkg_spec_part
		     {
		       m_scopeStack.remove(m_scopeStack.last());
		       if (m_scopeStack.count() == 0) {
			 kdDebug() << "adastore: m_scopeStack is empty!" << endl;
		         m_scopeStack.append( m_model->globalNamespace() );
		       }
		       m_currentContainer = m_scopeStack.last();
		       // m_currentContainer->setEndPosition (endLine, 0);
		       m_addToStore = false;
		     }
		   )
		| #(PACKAGE_RENAMING_DECLARATION prd:def_id
		     {
		       defineScope( #prd );
		     }
		     renames
		   )
		| generic_decl
		)
	)
	;


subprog_def_id
	: { m_isSubprogram = true; }
	def_id
	  { m_isSubprogram = false; }
	;

subprog_decl
	: #(GENERIC_PROCEDURE_INSTANTIATION subprog_def_id generic_inst)
	| #(PROCEDURE_RENAMING_DECLARATION  subprog_def_id formal_part_opt renames)
	| #(PROCEDURE_DECLARATION           subprog_def_id formal_part_opt)
	| #(PROCEDURE_BODY_STUB             subprog_def_id formal_part_opt)
	| #(ABSTRACT_PROCEDURE_DECLARATION  subprog_def_id formal_part_opt)
	| #(GENERIC_FUNCTION_INSTANTIATION  def_designator generic_inst)
	| #(FUNCTION_RENAMING_DECLARATION   def_designator function_tail renames)
	| #(FUNCTION_DECLARATION            def_designator function_tail)
	| #(FUNCTION_BODY_STUB              def_designator function_tail)
	| #(ABSTRACT_FUNCTION_DECLARATION   subprog_def_id function_tail)
	;


def_id
	: cn:compound_name
	  {
//        kdDebug() << "cn:compound_name started " << endl;
	    if (m_addToStore) {
//          kdDebug() << "cn:compound_name m_addToStore " << endl;
	      if (m_isSubprogram) {
//            kdDebug() << "cn:compound_name m_isSubprogram " << endl;
            FunctionDom method = m_model->create<FunctionModel>();
	        method->setName (qtext (cn));
            method->setFileName(m_fileName);
//            kdDebug() << "cn:compound_name method->setStartPosition(" << endl;
            method->setStartPosition(#cn->getLine(), #cn->getColumn());

            if (m_currentContainer == m_model->globalNamespace())
                m_file->addFunction(method);
            else
                m_currentContainer->addFunction(method);
            //FIXME: adymo: is this valid for CodeModel
/*	        ParsedMethod *old = m_currentContainer->getMethod (method);
	        if (old) {
	          delete (method);
	          method = old;
	        } else {
	          m_currentContainer->addMethod (method);
	        }*/
	      } else {
	        // TBC: what about other declarations?
	      }
	    }
	  }
	;

/*
generic_inst : compound_name ( value_s )?
	;
 */

/*
formal_part_opt : #(FORMAL_PART_OPT ( parameter_specification )* )
	;
 */

/*
parameter_specification
	: #(PARAMETER_SPECIFICATION defining_identifier_list modifiers
		subtype_mark init_opt)
	;
 */

/*
defining_identifier_list : #(DEFINING_IDENTIFIER_LIST ( IDENTIFIER )+ )
	;
 */

/*
renames : CHARACTER_STRING  // CHARACTER_STRING should not really be there.
	| OPERATOR_SYMBOL   // OPERATOR_SYMBOL should be used instead.
	| name
	;
 */

/*
name    : IDENTIFIER
	| #(DOT name
		( ALL
		| IDENTIFIER
		| CHARACTER_LITERAL
		| OPERATOR_SYMBOL
		)
	   )
	| #(INDEXED_COMPONENT name value_s)
	| #(TIC name attribute_id)
	;
 */

def_designator
	: cn:compound_name
	  {
//        kdDebug() << "def_designator cn:compound_name started" << endl;
	    if (m_addToStore) {
//            kdDebug() << "def_designator cn:compound_name m_addToStore" << endl;
            FunctionDom method = m_model->create<FunctionModel>();
            method->setName (qtext (cn));
            method->setFileName(m_fileName);
//            kdDebug() << "def_designator cn:compound_name method->setStartPosition(" << endl;
            method->setStartPosition(#cn->getLine(), #cn->getColumn());

            if (m_currentContainer == m_model->globalNamespace())
                m_file->addFunction(method);
            else
                m_currentContainer->addFunction(method);
            //FIXME: adymo: is this valid for CodeModel
/*	      ParsedMethod *old = m_currentContainer->getMethod (method);
	      if (old) {
		delete method;
		method = old;
	      } else {
		m_currentContainer->addMethod (method);
	      }*/
	    }
	  }
	| definable_operator_symbol
	;

/*
function_tail : formal_part_opt subtype_mark
	;
 */

spec_decl_part
	: #(GENERIC_PACKAGE_INSTANTIATION def_id generic_inst)
	| #(PACKAGE_SPECIFICATION ps:def_id
	     {
	       NamespaceDom psc = defineScope( #ps );
	       m_currentContainer = psc;
	       m_scopeStack.append( psc );
	       m_addToStore = true;
	     }
	    pkg_spec_part
	     {
	       m_scopeStack.remove(m_scopeStack.last());
	       if (m_scopeStack.count() == 0) {
		 kdDebug() << "adastore: m_scopeStack is empty!" << endl;
	         m_scopeStack.append( m_model->globalNamespace() );
	       }
	       m_currentContainer = m_scopeStack.last();
	       // m_currentContainer->setDeclarationEndsOnLine (endLine);
	       m_addToStore = false;
	     }
	   )
	| #(PACKAGE_RENAMING_DECLARATION def_id renames)
	;

pkg_spec_part
	: basic_declarative_items_opt
	  (	{ m_currentAccess = CodeModelItem::Protected; }
		basic_declarative_items_opt
	 	{ m_currentAccess = CodeModelItem::Public; }
	  )?
	;

/*
task_type_or_single_decl
	: #(TASK_TYPE_DECLARATION def_id discrim_part_opt task_definition_opt)
	| #(SINGLE_TASK_DECLARATION def_id task_definition_opt)
	;
 */

/*
discriminant_specification
	: #(DISCRIMINANT_SPECIFICATION defining_identifier_list
		modifiers subtype_mark init_opt)
	;
 */

/*
entry_declaration
	: #(ENTRY_DECLARATION IDENTIFIER
		discrete_subtype_def_opt formal_part_opt)
	;
 */

/*
prot_op_decl
	: entry_declaration
	| #(PROCEDURE_DECLARATION def_id formal_part_opt)
	| #(FUNCTION_DECLARATION def_designator function_tail)
	| rep_spec
	| pragma
	;
 */

/*
prot_member_decl_s
	: #(PROT_MEMBER_DECLARATIONS ( prot_op_decl | comp_decl )* )
	;

comp_decl
	: #(COMPONENT_DECLARATION defining_identifier_list component_subtype_def init_opt)
	;
 */

/*
// decl_common is shared between declarative_item and basic_decl_item.
// decl_common only contains specifications.
decl_common
	: // type_def:
	  #(ENUMERATION_TYPE_DECLARATION IDENTIFIER enum_id_s)
	| #(SIGNED_INTEGER_TYPE_DECLARATION IDENTIFIER range)
	| #(MODULAR_TYPE_DECLARATION IDENTIFIER expression)
	| #(FLOATING_POINT_DECLARATION IDENTIFIER expression range_constraint_opt)
	| #(ORDINARY_FIXED_POINT_DECLARATION IDENTIFIER expression range)
	| #(DECIMAL_FIXED_POINT_DECLARATION IDENTIFIER expression expression range_constraint_opt)
	| array_type_declaration
	| access_type_declaration
	//
	| #(INCOMPLETE_TYPE_DECLARATION IDENTIFIER discrim_part_opt)
	// derived_or_private_or_record
	| #(PRIVATE_EXTENSION_DECLARATION id_and_discrim modifiers subtype_ind)
	| #(DERIVED_RECORD_EXTENSION id_and_discrim modifiers subtype_ind record_definition)
	| #(ORDINARY_DERIVED_TYPE_DECLARATION id_and_discrim subtype_ind)
	| #(PRIVATE_TYPE_DECLARATION id_and_discrim modifiers)
	| #(RECORD_TYPE_DECLARATION id_and_discrim modifiers record_definition)
	//
	| #(SUBTYPE_DECLARATION IDENTIFIER subtype_ind)
	| generic_decl
	| use_clause
	| rep_spec  // enumeration_representation_clause only
	| #(EXCEPTION_RENAMING_DECLARATION def_id compound_name)
	| #(OBJECT_RENAMING_DECLARATION def_id subtype_mark name)
	| #(EXCEPTION_DECLARATION defining_identifier_list)
	| #(NUMBER_DECLARATION defining_identifier_list expression)
	| #(ARRAY_OBJECT_DECLARATION defining_identifier_list modifiers
		array_type_definition init_opt)
	| #(OBJECT_DECLARATION defining_identifier_list modifiers
		subtype_ind init_opt)
	;
 */

/*
id_and_discrim
	: IDENTIFIER discrim_part_opt
	;
 */

/*
enumeration_literal_specification : IDENTIFIER | CHARACTER_LITERAL
	;
 */

/*
array_type_declaration
	: #(ARRAY_TYPE_DECLARATION IDENTIFIER array_type_definition)
	;
 */

/*
access_type_declaration
	: #(ACCESS_TO_PROCEDURE_DECLARATION IDENTIFIER modifiers formal_part_opt)
	| #(ACCESS_TO_FUNCTION_DECLARATION IDENTIFIER modifiers function_tail)
	| #(ACCESS_TO_OBJECT_DECLARATION IDENTIFIER modifiers subtype_ind)
	;
 */


generic_decl
	: #(GENERIC_PACKAGE_RENAMING generic_formal_part_opt def_id renames)
	| #(GENERIC_PACKAGE_DECLARATION generic_formal_part_opt gpd:def_id
		     {
		       NamespaceDom psc = defineScope( #gpd );
		       m_currentContainer = psc;
		       m_scopeStack.append( psc );
		       m_addToStore = true;
		     }
		pkg_spec_part
		     {
		       m_scopeStack.remove(m_scopeStack.last());
		       if (m_scopeStack.count() == 0)
		         m_scopeStack.append( m_model->globalNamespace() );
		       m_currentContainer = m_scopeStack.last();
		       // m_currentContainer->setDeclarationEndsOnLine (endLine);
		       m_addToStore = false;
		     }
		)
	| #(GENERIC_PROCEDURE_RENAMING generic_formal_part_opt def_id
		formal_part_opt renames)
	| #(GENERIC_PROCEDURE_DECLARATION generic_formal_part_opt subprog_def_id
		formal_part_opt)
	| #(GENERIC_FUNCTION_RENAMING generic_formal_part_opt def_designator
		function_tail renames)
	| #(GENERIC_FUNCTION_DECLARATION generic_formal_part_opt subprog_def_id
		function_tail)
	;

/*
generic_formal_part_opt
	: #(GENERIC_FORMAL_PART
		( pragma | use_clause | generic_formal_parameter )*
	   )
	;
 */

/*
generic_formal_parameter
	: // FORMAL_TYPE_DECLARATIONs:
	  #(FORMAL_DISCRETE_TYPE_DECLARATION def_id)
	| #(FORMAL_SIGNED_INTEGER_TYPE_DECLARATION def_id)
	| #(FORMAL_MODULAR_TYPE_DECLARATION def_id)
	| #(FORMAL_DECIMAL_FIXED_POINT_DECLARATION def_id)
	| #(FORMAL_ORDINARY_FIXED_POINT_DECLARATION def_id)
	| #(FORMAL_FLOATING_POINT_DECLARATION def_id)
	| formal_array_type_declaration
	| formal_access_type_declaration
	| #(FORMAL_PRIVATE_TYPE_DECLARATION id_part modifiers)
	| #(FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION id_part subtype_ind)
	| #(FORMAL_PRIVATE_EXTENSION_DECLARATION id_part modifiers subtype_ind)
	| #(FORMAL_PROCEDURE_DECLARATION def_id formal_part_opt
		subprogram_default_opt)
	| #(FORMAL_FUNCTION_DECLARATION def_designator function_tail
		subprogram_default_opt)
	| #(FORMAL_PACKAGE_DECLARATION def_id compound_name formal_package_actual_part_opt)
	| parameter_specification
	;
 */

lib_subprog_decl_or_rename_or_inst_or_body
	: { m_addToStore = true; }
        ( subprog_decl
	| procedure_body
	| function_body
	)
          { m_addToStore = false; }
	;

subprog_decl_or_rename_or_inst_or_body
	: subprog_decl
	| procedure_body
	| function_body
	;


/*
// A declarative_item may appear in the declarative part of any body.
declarative_item
	: #(PACKAGE_BODY_STUB def_id)
	| #(PACKAGE_BODY def_id pkg_body_part)
	| spec_decl_part
	| #(TASK_BODY_STUB def_id)
	| #(TASK_BODY def_id body_part)
	| task_type_or_single_decl
	| #(PROTECTED_BODY_STUB def_id)
	| #(PROTECTED_BODY def_id prot_op_bodies_opt)
	| prot_type_or_single_decl
	| subprog_decl_or_rename_or_inst_or_body
	| decl_common
	;
 */


subprog_decl_or_body
	: procedure_body
	| #(PROCEDURE_DECLARATION subprog_def_id formal_part_opt)
	| function_body
	| #(FUNCTION_DECLARATION def_designator function_tail)
	;

/*
// Temporary, to be turned into just `qualified'.
// We get away with it because `qualified' is always mentioned
// together with `name'.
// Only exception: `code_stmt', which is not yet implemented.
name_or_qualified
	: IDENTIFIER
	| #(DOT name_or_qualified
		( ALL
		| IDENTIFIER
		| CHARACTER_LITERAL
		| OPERATOR_SYMBOL
		)
	   )
	| #(INDEXED_COMPONENT name_or_qualified value_s)
	| #(TIC name_or_qualified
		( parenthesized_primary
		| attribute_id
		)
	   )
	;
 */

package_body
	: #(PACKAGE_BODY id:def_id
		/* TBD
		{ QString name (qtext (id));
		}
		 */
	    pkg_body_part)
	;

/*
task_body : #(TASK_BODY def_id body_part)
	;
 */

/*
protected_body : #(PROTECTED_BODY def_id prot_op_bodies_opt)
	;
 */

