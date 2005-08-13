//
// C++ Interface: creategettersetterconfiguration
//
// Description: 
//
//
// Author: Jonas Jacobi <j.jacobi@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CREATEGETTERSETTERCONFIGURATION_H
#define CREATEGETTERSETTERCONFIGURATION_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

class CppSupportPart;
class QDomDocument;

/**
 * Class containing the settings for the creation of get/set methods for class attributes.
 * It contains several attributes:
 * - prefixGet is the prefix which is put in front of the attributename for the getmethod.
 * - prefixSet is the prefix which is put in front of the attributename for the setmethod.
 * - prefixVariable is a StringList containing prefixes which should be removed from the attributename
 *   when creating the get/set method names
 * - parameterName is the name of the parameter containing the value in the setmethod.
 * - inlineGet true if getmethod should be created inline, false otherwise
 * - inlineSet true if setmethod should be created inline, false otherwise
 * 
 * The settings are stored per project under /kdevcppsupport/creategettersetter/.
 * @author Jonas Jacobi <j.jacobi@gmx.de>
 */
class CreateGetterSetterConfiguration : public QObject{
Q_OBJECT
public:
    CreateGetterSetterConfiguration(CppSupportPart* part);
    ~CreateGetterSetterConfiguration();
	
public slots:
	void init();
	void store();

public:
	void setPrefixGet(const QString& theValue)
	{
		m_prefixGet = theValue;
	}
	
	QString prefixGet() const
	{
		return m_prefixGet;
	}
	void setPrefixSet(const QString& theValue)
	{
		m_prefixSet = theValue;
	}
	
	QString prefixSet() const
	{
		return m_prefixSet;
	}
	void setPrefixVariable(const QStringList& theValue)
	{
		m_prefixVariable = theValue;
	}
	
	QStringList prefixVariable() const
	{
		return m_prefixVariable;
	}
	void setParameterName(const QString& theValue)
	{
		m_parameterName = theValue;
	}
	
	QString parameterName() const
	{
		return m_parameterName;
	}
	void setInlineGet(bool theValue)
	{
		m_isInlineGet = theValue;
	}
	
	bool isInlineGet() const
	{
		return m_isInlineGet;
	}
	void setInlineSet(bool theValue)
	{
		m_isInlineSet = theValue;
	}
	
	bool isInlineSet() const
	{
		return m_isInlineSet;
	}
	
private:
	CppSupportPart* m_part;
	QDomDocument* m_settings;
	
	QString m_prefixGet;
	QString m_prefixSet;
	QStringList m_prefixVariable;
	QString m_parameterName;
	bool m_isInlineGet;
	bool m_isInlineSet;
	
private:
	static QString defaultPath;
};

#endif
