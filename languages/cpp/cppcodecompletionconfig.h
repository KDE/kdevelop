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
		Alphabetic
	};

	CompletionBoxItemOrder completionBoxItemOrder() const {
		///not yet used
		return ByAccessLevel;
	}

	bool showEvaluationContextMenu() const {
		///used
		return true;
	}

	bool showCommentWithArgumentHint() const {
		///used
		return true;
	}

	bool statusBarTypeEvaluation() const {
		///used
		return true;
	}

	QString namespaceAliases() const {
		///used
		return "std=_GLIBCXX_STD";
	}

	bool processPrimaryTypes() const { ///The "do complete return-type-evaluation" entry. However it should be renamed, because it also covers Variable-types and typedefs!
		///used
		return true;
	}

	bool processFunctionArguments() const { ///The "do complete argument-type-evaluation" entry
		///used
		return true; ///@todo: link this to the GUI-Configuration
	}
	
	bool automaticCodeCompletion() const
	{
		///used
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

	static QString defaultPath;
};

#endif 
// kate: indent-mode csands; tab-width 4;
