/*
 * file     : cppsupportwidget.h
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@gmx.net
 * license  : gpl version >= 2
 */

#ifndef __CPPSUPPORTWIDGET_H__
#define __CPPSUPPORTWIDGET_H__

#include <keditcl.h>

class QString;
class CppSupportPart;

/**
 * this class provides access to the code-hinting widget
 */
class CppSupportWidget : public KEdit
{
    Q_OBJECT
    
    public:
	CppSupportWidget( CppSupportPart* part );
	~CppSupportWidget( );

	void setCHText( const QString& text );
	
    private:
	CppSupportPart* m_part;
};

#endif
