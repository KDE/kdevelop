//
//
// C++ Interface: $MODULE$
//
// Description:
//
//
// Author: Roberto Raggi <robertol@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CPPCODECOMPLETIONCONFIG_H
#define CPPCODECOMPLETIONCONFIG_H

#include <qobject.h>

class CppSupportPart;
class QDomDocument;


/**
@author Roberto Raggi
*/
class CppCodeCompletionConfig : public QObject
{
	Q_OBJECT
public:
	CppCodeCompletionConfig( CppSupportPart* part, QDomDocument* dom );
	virtual ~CppCodeCompletionConfig();

// 	bool includeGlobalFunctions() const
// 	{
// 		return m_includeGlobalFunctions;
// 	}
// 
// 	void setIncludeGlobalFunctions( bool b );
// 
// 	bool includeTypes() const
// 	{
// 		return m_includeTypes;
// 	}
// 
// 	void setIncludeTypes( bool b );
// 
// 	bool includeEnums() const
// 	{
// 		return m_includeEnums;
// 	}
// 
// 	void setIncludeEnums( bool b );
// 
// 	bool includeTypedefs() const
// 	{
// 		return m_includeTypedefs;
// 	}
// 
// 	void setIncludeTypedefs( bool b );

	enum CompletionBoxItemOrder {
		ByAccessLevel,
		ByClass,
		ByAlphabet
	};

    ///Whether the completion-cache should be kept alive as long as the context does not change
    bool usePermanentCaching() const {
        return m_usePermanentCaching;
    }
    
    void setUsePermanentCaching(bool b)  {
        m_usePermanentCaching = b;
    }
    
	///Whether higher namespaces(like the global one) should be included while computing the
	///completion-list when using automatic code-completion. Should get an option in the interface(slows down the completion).
    bool alwaysIncludeNamespaces() const {
        return m_alwaysIncludeNamespaces;
    }

    void setAlwaysIncludeNamespaces(bool b) {
        m_alwaysIncludeNamespaces = b;
    }
    
    ///Whether all included headers should be processed by the preprocessor(makes macros work across header-files)
    void setPreProcessAllHeaders(bool b) {
        m_preProcessAllHeaders = b;
    }

    ///Whether headers that were found and were not yet parsed in their active state should be parsed into the code-model
    void setParseMissingHeaders(bool b) {
        m_parseMissingHeaders = b;
    }

    ///Additional include-paths defined by the user
    void  setCustomIncludePaths(QString str) {
        m_includePaths = str;
    }

    ///Whether we should try to automatically find out the used include-path from Makefiles etc.
    void setResolveIncludePaths(bool b) {
        m_resolveIncludePaths = b;
    }
    
	///Whether higher namespaces(like the global one) should be included while computing the
	///completion-list when using automatic code-completion. Should get an option in the interface(slows down the completion).
    
    ///Whether all included headers should be processed by the preprocessor(makes macros work across header-files)
    bool preProcessAllHeaders() const {
        return m_preProcessAllHeaders;
    }
    
    ///Whether headers that were found and were not yet parsed in their active state should be parsed into the code-model
    bool parseMissingHeaders() const {
        return m_parseMissingHeaders;
    }
    
    ///Additional include-paths defined by the user
    QString customIncludePaths() const {
        return m_includePaths;
    }
    
    ///Whether we should try to automatically find out the used include-path from Makefiles etc.
    bool resolveIncludePaths() const {
        return m_resolveIncludePaths;
    }
    
    bool alwaysParseInBackground() const {
        return m_alwaysParseInBackground;
    }
    
    void setAlwaysParseInBackground(bool b)  {
        m_alwaysParseInBackground = b;
    }

	bool showNamespaceAppearances() const;

	bool showOnlyAccessibleItems() const {
		///not yet used
		return m_showOnlyAccessibleItems;
	}

	CompletionBoxItemOrder completionBoxItemOrder() const {
		///not yet used
		return m_completionBoxItemOrder;
	}

	bool showEvaluationContextMenu() const {
		///used
		return m_showEvaluationContextMenu;
	}

	bool showCommentWithArgumentHint() const {
		///used
		return m_showCommentWithArgumentHint;
	}

	bool statusBarTypeEvaluation() const {
		///used
		return m_statusBarTypeEvaluation;
	}

	QString namespaceAliases() const {
		///used
		return m_namespaceAliases;
	}

	bool processPrimaryTypes() const { ///The "do complete return-type-evaluation" entry. However it should be renamed, because it also covers Variable-types and typedefs!
		///used
		return m_processPrimaryTypes;
	}

	bool processFunctionArguments() const { ///The "do complete argument-type-evaluation" entry
		///used
		return m_processFunctionArguments;
	}


	void setShowOnlyAccessibleItems( bool b ) {
		///not yet used
		m_showOnlyAccessibleItems = b;
	}
	
	void setCompletionBoxItemOrder( CompletionBoxItemOrder b ) {
		///not yet used
		m_completionBoxItemOrder = b;
	}
	
	void setShowEvaluationContextMenu( bool b ) {
		///used
		m_showEvaluationContextMenu = b;
	}
	
	void setShowCommentWithArgumentHint( bool b ) {
		///used
		m_showCommentWithArgumentHint = b;
	}
	
	void setStatusBarTypeEvaluation( bool b ) {
		///used
		m_statusBarTypeEvaluation = b;
	}
	
	void setNamespaceAliases( QString n ) {
		///used
		m_namespaceAliases = n;
	}
	
	void setProcessPrimaryTypes( bool b ) { ///The "do complete return-type-evaluation" entry. However it should be renamed, because it also covers Variable-types and typedefs!
		///used
		m_processPrimaryTypes = b;
	}
	
	void setProcessFunctionArguments( bool b ) { ///The "do complete argument-type-evaluation" entry
		///used
		m_processFunctionArguments = b;
	}

	bool automaticCodeCompletion() const {
		return m_automaticCodeCompletion;
	}
	
	
	void setAutomaticCodeCompletion( bool b );

	bool automaticArgumentsHint() const
	{
		///used
		return m_automaticArgumentsHint;
	}

	void setAutomaticArgumentsHint( bool b );

	bool automaticHeaderCompletion() const
	{
		///used
		return m_automaticHeaderCompletion;
	}

	void setAutomaticHeaderCompletion( bool b );

	int codeCompletionDelay() const
	{
		///used
		return m_codeCompletionDelay;
	}

	void setCodeCompletionDelay( int delay );

	int argumentsHintDelay() const
	{
		///used
		return m_argumentsHintDelay;
	}

	void setArgumentsHintDelay( int delay );

	int headerCompletionDelay() const
	{
		///used
		return m_headerCompletionDelay;
	}

	void setHeaderCompletionDelay( int delay );

public slots:
	void store();

signals:
	void stored();

private:
	void init();

private:
	CppSupportPart* m_part;
	QDomDocument* m_dom;
/*	bool m_includeGlobalFunctions;
	bool m_includeTypes;
	bool m_includeEnums;
	bool m_includeTypedefs;*/
	bool m_automaticCodeCompletion;
	bool m_automaticArgumentsHint;
	bool m_automaticHeaderCompletion;
	int m_codeCompletionDelay;
	int m_argumentsHintDelay;
	int m_headerCompletionDelay;

    bool m_preProcessAllHeaders;
    bool m_parseMissingHeaders;
    bool m_resolveIncludePaths;
    bool m_alwaysIncludeNamespaces;
    bool m_alwaysParseInBackground;
    bool m_usePermanentCaching;
    QString m_includePaths;
    
	bool m_showOnlyAccessibleItems;
	CompletionBoxItemOrder m_completionBoxItemOrder;
	bool m_showEvaluationContextMenu;
	bool m_showCommentWithArgumentHint;
	bool m_statusBarTypeEvaluation;
	QString m_namespaceAliases;
	bool m_processPrimaryTypes;
	bool m_processFunctionArguments;

	static QString defaultPath;
};

#endif 
// kate: indent-mode csands; tab-width 4;
