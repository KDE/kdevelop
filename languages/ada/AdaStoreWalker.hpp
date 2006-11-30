#ifndef INC_AdaStoreWalker_hpp_
#define INC_AdaStoreWalker_hpp_

#line 1 "expandedada.store.g"

#include <qstring.h>
#include <qstringlist.h>
#include <qfileinfo.h>

#include <codemodel.h>
#include "AdaAST.hpp"
#include "ada_utils.hpp"

#line 15 "AdaStoreWalker.hpp"
#include <antlr/config.hpp>
#include "AdaStoreWalkerTokenTypes.hpp"
/* $ANTLR 2.7.7 (20061129): "expandedada.store.g" -> "AdaStoreWalker.hpp"$ */
#include <antlr/TreeParser.hpp>

#line 10 "expandedada.store.g"

#include <codemodel.h>
#include <kdebug.h>

#line 26 "AdaStoreWalker.hpp"
class CUSTOM_API AdaStoreWalker : public ANTLR_USE_NAMESPACE(antlr)TreeParser, public AdaStoreWalkerTokenTypes
{
#line 25 "expandedada.store.g"

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
#line 30 "AdaStoreWalker.hpp"
public:
	AdaStoreWalker();
	static void initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory );
	int getNumTokens() const
	{
		return AdaStoreWalker::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return AdaStoreWalker::tokenNames[type];
	}
	const char* const* getTokenNames() const
	{
		return AdaStoreWalker::tokenNames;
	}
	public: void compilation_unit(RefAdaAST _t);
	public: void context_items_opt(RefAdaAST _t);
	public: void library_item(RefAdaAST _t);
	public: void subunit(RefAdaAST _t);
	public: void pragma(RefAdaAST _t);
	public: void with_clause(RefAdaAST _t);
	public: void compound_name(RefAdaAST _t);
	public: void use_clause(RefAdaAST _t);
	public: void subtype_mark(RefAdaAST _t);
	public: void lib_subprog_decl_or_rename_or_inst_or_body(RefAdaAST _t);
	public: void def_id(RefAdaAST _t);
	public: void pkg_body_part(RefAdaAST _t);
	public: void generic_inst(RefAdaAST _t);
	public: void pkg_spec_part(RefAdaAST _t);
	public: void renames(RefAdaAST _t);
	public: void generic_decl(RefAdaAST _t);
	public: void subprog_def_id(RefAdaAST _t);
	public: void subprog_decl(RefAdaAST _t);
	public: void formal_part_opt(RefAdaAST _t);
	public: void def_designator(RefAdaAST _t);
	public: void function_tail(RefAdaAST _t);
	public: void definable_operator_symbol(RefAdaAST _t);
	public: void spec_decl_part(RefAdaAST _t);
	public: void basic_declarative_items_opt(RefAdaAST _t);
	public: void generic_formal_part_opt(RefAdaAST _t);
	public: void procedure_body(RefAdaAST _t);
	public: void function_body(RefAdaAST _t);
	public: void subprog_decl_or_rename_or_inst_or_body(RefAdaAST _t);
	public: void subprog_decl_or_body(RefAdaAST _t);
	public: void package_body(RefAdaAST _t);
	public: void pragma_arg(RefAdaAST _t);
	public: void expression(RefAdaAST _t);
	public: void attribute_id(RefAdaAST _t);
	public: void modifiers(RefAdaAST _t);
	public: void value_s(RefAdaAST _t);
	public: void value(RefAdaAST _t);
	public: void ranged_expr_s(RefAdaAST _t);
	public: void ranged_expr(RefAdaAST _t);
	public: void simple_expression(RefAdaAST _t);
	public: void range(RefAdaAST _t);
	public: void range_constraint(RefAdaAST _t);
	public: void range_dots(RefAdaAST _t);
	public: void range_attrib_ref(RefAdaAST _t);
	public: void prefix(RefAdaAST _t);
	public: void parameter_specification(RefAdaAST _t);
	public: void defining_identifier_list(RefAdaAST _t);
	public: void init_opt(RefAdaAST _t);
	public: void name(RefAdaAST _t);
	public: void parenthesized_primary(RefAdaAST _t);
	public: void extension_opt(RefAdaAST _t);
	public: void basic_decl_item(RefAdaAST _t);
	public: void task_type_or_single_decl(RefAdaAST _t);
	public: void prot_type_or_single_decl(RefAdaAST _t);
	public: void decl_common(RefAdaAST _t);
	public: void discrim_part_opt(RefAdaAST _t);
	public: void task_definition_opt(RefAdaAST _t);
	public: void task_items_opt(RefAdaAST _t);
	public: void private_task_items_opt(RefAdaAST _t);
	public: void discriminant_specifications(RefAdaAST _t);
	public: void discriminant_specification(RefAdaAST _t);
	public: void entrydecls_repspecs_opt(RefAdaAST _t);
	public: void entry_declaration(RefAdaAST _t);
	public: void rep_spec(RefAdaAST _t);
	public: void discrete_subtype_def_opt(RefAdaAST _t);
	public: void discrete_subtype_definition(RefAdaAST _t);
	public: void subtype_ind(RefAdaAST _t);
	public: void align_opt(RefAdaAST _t);
	public: void comp_loc_s(RefAdaAST _t);
	public: void local_enum_name(RefAdaAST _t);
	public: void enumeration_aggregate(RefAdaAST _t);
	public: void protected_definition(RefAdaAST _t);
	public: void prot_op_decl_s(RefAdaAST _t);
	public: void prot_member_decl_s(RefAdaAST _t);
	public: void prot_op_decl(RefAdaAST _t);
	public: void comp_decl(RefAdaAST _t);
	public: void component_subtype_def(RefAdaAST _t);
	public: void enum_id_s(RefAdaAST _t);
	public: void range_constraint_opt(RefAdaAST _t);
	public: void array_type_declaration(RefAdaAST _t);
	public: void access_type_declaration(RefAdaAST _t);
	public: void id_and_discrim(RefAdaAST _t);
	public: void record_definition(RefAdaAST _t);
	public: void array_type_definition(RefAdaAST _t);
	public: void enumeration_literal_specification(RefAdaAST _t);
	public: void index_or_discrete_range_s(RefAdaAST _t);
	public: void index_or_discrete_range(RefAdaAST _t);
	public: void constraint_opt(RefAdaAST _t);
	public: void digits_constraint(RefAdaAST _t);
	public: void delta_constraint(RefAdaAST _t);
	public: void index_constraint(RefAdaAST _t);
	public: void discriminant_constraint(RefAdaAST _t);
	public: void discrete_range(RefAdaAST _t);
	public: void discriminant_association(RefAdaAST _t);
	public: void selector_names_opt(RefAdaAST _t);
	public: void selector_name(RefAdaAST _t);
	public: void component_list(RefAdaAST _t);
	public: void component_items(RefAdaAST _t);
	public: void variant_part(RefAdaAST _t);
	public: void discriminant_direct_name(RefAdaAST _t);
	public: void variant_s(RefAdaAST _t);
	public: void variant(RefAdaAST _t);
	public: void choice_s(RefAdaAST _t);
	public: void choice(RefAdaAST _t);
	public: void discrete_with_range(RefAdaAST _t);
	public: void mark_with_constraint(RefAdaAST _t);
	public: void generic_formal_parameter(RefAdaAST _t);
	public: void formal_array_type_declaration(RefAdaAST _t);
	public: void formal_access_type_declaration(RefAdaAST _t);
	public: void id_part(RefAdaAST _t);
	public: void subprogram_default_opt(RefAdaAST _t);
	public: void formal_package_actual_part_opt(RefAdaAST _t);
	public: void body_part(RefAdaAST _t);
	public: void declarative_part(RefAdaAST _t);
	public: void block_body(RefAdaAST _t);
	public: void declarative_item(RefAdaAST _t);
	public: void prot_op_bodies_opt(RefAdaAST _t);
	public: void block_body_opt(RefAdaAST _t);
	public: void handled_stmt_s(RefAdaAST _t);
	public: void entry_body(RefAdaAST _t);
	public: void statements(RefAdaAST _t);
	public: void except_handler_part_opt(RefAdaAST _t);
	public: void statement(RefAdaAST _t);
	public: void def_label_opt(RefAdaAST _t);
	public: void null_stmt(RefAdaAST _t);
	public: void exit_stmt(RefAdaAST _t);
	public: void return_stmt(RefAdaAST _t);
	public: void goto_stmt(RefAdaAST _t);
	public: void delay_stmt(RefAdaAST _t);
	public: void abort_stmt(RefAdaAST _t);
	public: void raise_stmt(RefAdaAST _t);
	public: void requeue_stmt(RefAdaAST _t);
	public: void accept_stmt(RefAdaAST _t);
	public: void select_stmt(RefAdaAST _t);
	public: void if_stmt(RefAdaAST _t);
	public: void case_stmt(RefAdaAST _t);
	public: void loop_stmt(RefAdaAST _t);
	public: void block(RefAdaAST _t);
	public: void call_or_assignment(RefAdaAST _t);
	public: void cond_clause(RefAdaAST _t);
	public: void elsifs_opt(RefAdaAST _t);
	public: void else_opt(RefAdaAST _t);
	public: void condition(RefAdaAST _t);
	public: void alternative_s(RefAdaAST _t);
	public: void case_statement_alternative(RefAdaAST _t);
	public: void iteration_scheme_opt(RefAdaAST _t);
	public: void declare_opt(RefAdaAST _t);
	public: void label_name(RefAdaAST _t);
	public: void entry_body_formal_part(RefAdaAST _t);
	public: void entry_barrier(RefAdaAST _t);
	public: void entry_index_spec_opt(RefAdaAST _t);
	public: void entry_call_stmt(RefAdaAST _t);
	public: void entry_index_opt(RefAdaAST _t);
	public: void triggering_alternative(RefAdaAST _t);
	public: void abortable_part(RefAdaAST _t);
	public: void selective_accept(RefAdaAST _t);
	public: void entry_call_alternative(RefAdaAST _t);
	public: void delay_alternative(RefAdaAST _t);
	public: void stmts_opt(RefAdaAST _t);
	public: void guard_opt(RefAdaAST _t);
	public: void select_alternative(RefAdaAST _t);
	public: void or_select_opt(RefAdaAST _t);
	public: void accept_alternative(RefAdaAST _t);
	public: void exception_handler(RefAdaAST _t);
	public: void identifier_colon_opt(RefAdaAST _t);
	public: void except_choice_s(RefAdaAST _t);
	public: void exception_choice(RefAdaAST _t);
	public: void operator_call(RefAdaAST _t);
	public: void relation(RefAdaAST _t);
	public: void range_or_mark(RefAdaAST _t);
	public: void signed_term(RefAdaAST _t);
	public: void term(RefAdaAST _t);
	public: void factor(RefAdaAST _t);
	public: void primary(RefAdaAST _t);
	public: void name_or_qualified(RefAdaAST _t);
	public: void allocator(RefAdaAST _t);
	public: void subprogram_body(RefAdaAST _t);
	public: void task_body(RefAdaAST _t);
	public: void protected_body(RefAdaAST _t);
public:
	ANTLR_USE_NAMESPACE(antlr)RefAST getAST()
	{
		return ANTLR_USE_NAMESPACE(antlr)RefAST(returnAST);
	}
	
protected:
	RefAdaAST returnAST;
	RefAdaAST _retTree;
private:
	static const char* tokenNames[];
#ifndef NO_STATIC_CONSTS
	static const int NUM_TOKENS = 305;
#else
	enum {
		NUM_TOKENS = 305
	};
#endif
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
	static const unsigned long _tokenSet_2_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_2;
	static const unsigned long _tokenSet_3_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_3;
	static const unsigned long _tokenSet_4_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_4;
};

#endif /*INC_AdaStoreWalker_hpp_*/
