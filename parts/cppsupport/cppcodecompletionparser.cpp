// Here resides the C/C++ code completion parser

CppCodeCompletionParser::CppCodeCompletionParser ( KEditor::EditDocumentIface* pEditIface, ClassStore* pStore );
{
	m_pEditIface = pEditIface;
	m_pStore = pStore;
}

CppCodeCompletionParser::~CppCodeCompletionParser()
{

}

int CppCodeCompletionParser::getNodePos ( int nCol )
{
	return 0;
}

QString CppCodeCompletionParser::getNodeText ( int nNode )
{
	return "";
}

QString CppCodeCompletionParser::getNodeDelimiter ( int nNode )
{
	return "";
}

QString CppCodeCompletionParser::getTypeOfObject ( const QString& strObject, int nLine )
{
	return "";
}

QString CppCodeCompletionParser::getReturnTypeOfMethod ( const QString& strMethod )
{
	return "";
}

QValueList CppCodeCompletionParser::getEntryListForClass ( const QString& strClass )
{
	return NULL;
}

QValueList CppCodeCompletionParser::getEntryListForNamespace ( const QString& strNamespace )
{
	return NULL;
}

QValueList CppCodeCompletionParser::getEntryListForStruct ( const QString& strStruct )
{
	return NULL;
}

bool CppCodeCompletionParser::isMethod ( const QString& strNodeText )
{
	return false;
}

bool CppCodeCompletionParser::isObject ( const QString& strNodeText )
{
	return false;
}

bool CppCodeCompletionParser::isClass ( const QString& strNodeText )
{
	return false;
}

bool CppCodeCompletionParser::isStruct ( const QString& strNodeText )
{
	return false;
}

bool CppCodeCompletionParser::isNamespace ( const QString& strNodeText )
{
	return false;
}
