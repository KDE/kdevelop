// Here resides the C/C++ code completion parser

#ifndef __CPPCODECOMPLETIONPARSER_H__
#define __CPPCODECOMPLETIONPARSER_H__

#include <qstring.h>
#include <qvaluelist.h>

#include "keditor/edit_iface.h"
#include "classstore.h"
#include "parsedclass.h"
#include "parsedmethod.h"

class CppCodeCompletionParser
{
	public:
		CppCodeCompletionParser ( KEditor::EditDocumentIface* pEditIface, ClassStore* pStore );
		virtual ~CppCodeCompletionParser();

	public:
		int getNodePos ( int nCol );
		QString getNodeText ( int nNode );
		QString getNodeDelimiter ( int nNode );
		QString getTypeOfObject ( const QString& strObject, int nLine );
		QString getReturnTypeOfMethod ( const QString& strMethod );
		QValueList getEntryListForClass ( const QString& strClass );
		QValueList getEntryListForNamespace ( const QString& strNamespace );
		QValueList getEntryListForStruct ( const QString& strStruct );
		bool isMethod ( const QString& strNodeText );
		bool isObject ( const QString& strNodeText );
		bool isClass ( const QString& strNodeText );
		bool isStruct ( const QString& strNodeText );
		bool isNamespace ( const QString& strNodeText );

		void setLine ( const QString& strLine ) { m_strCurLine = strLine };

	private:
		KEditor::EditDocumentIface* m_pEditIface;
		ClassStore* m_pStore;
		QString m_strCurLine;

};

#endif
