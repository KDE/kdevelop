// Here resides the C/C++ code completion parser

CppCodeCompletionParser::CppCodeCompletionParser ( KEditor::EditDocumentIface* pEditIface, int nLine, int nCol );
{
	m_pEditIface = pEditIface;
	m_bIsPointer = false;

	m_nLine = nLine; m_nCol = nCol;
}

CppCodeCompletionParser::~CppCodeCompletionParser()
{

}

QString CppCodeCompletionParser::getTypeOf ( QString& strObject );
{
	return NULL;
}

bool CppCodeCompletionParser::isPointer ( QString& strObject );
{
	if ( m_strCurObject == strObject )
		return m_bIsPointer;

	getTypeOf ( strObject );

	return m_bIsPointer;
}
