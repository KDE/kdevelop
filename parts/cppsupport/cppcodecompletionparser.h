// Here resides the C/C++ code completion parser

#ifndef __CPPCODECOMPLETION_H__
#define __CPPCODECOMPLETION_H__

#include <qstring.h>

#include "keditor/edit_iface.h"

class CppCodeCompletionParser
{
	public:
		CppCodeCompletionParser ( KEditor::EditDocumentIface* pEditIface, int nLine, int nCol );
		virtual ~CppCodeCompletionParser();

	public:
		QString getTypeOf ( QString& strObject );
		bool isPointer ( QString& strObject );

	private:
		KEditor::EditDocumentIface* m_pEditIface;
		QString m_strCurObject;
		bool m_bIsPointer;
		int m_nLine;
		int m_nCol;
};

#endif