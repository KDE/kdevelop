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
    
    bool includeGlobalFunctions() const
    {
	return m_includeGlobalFunctions;
    }
    
    void setIncludeGlobalFunctions( bool b );
    
    bool includeTypes() const
    {
	return m_includeTypes;
    }
    
    void setIncludeTypes( bool b );
    
    bool includeEnums() const
    {
	return m_includeEnums;
    }
    
    void setIncludeEnums( bool b );
    
    bool includeTypedefs() const
    {
	return m_includeTypedefs;
    }
    
    void setIncludeTypedefs( bool b );
    
    bool automaticCodeCompletion() const
    {
	return m_automaticCodeCompletion;
    }
    
    void setAutomaticCodeCompletion( bool b );
    
    bool automaticArgumentsHint() const
    {
	return m_automaticArgumentsHint;
    }
    
    void setAutomaticArgumentsHint( bool b );
    
    int codeCompletionDelay() const
    {
	return m_codeCompletionDelay;
    }
    
    void setCodeCompletionDelay( int delay );
    
    int argumentsHintDelay() const
    {
	return m_argumentsHintDelay;
    }
    
    void setArgumentsHintDelay( int delay );
    
public slots:
    void store();
    
signals:
    void stored();
    
private:
    void init();
    
private:
    CppSupportPart* m_part;
    QDomDocument* m_dom;
    int m_includeGlobalFunctions: 1;
    int m_includeTypes: 1;
    int m_includeEnums: 1;
    int m_includeTypedefs: 1;
    int m_automaticCodeCompletion: 1;
    int m_automaticArgumentsHint: 1;
    int m_codeCompletionDelay;
    int m_argumentsHintDelay;
    
    static QString defaultPath;
};

#endif
