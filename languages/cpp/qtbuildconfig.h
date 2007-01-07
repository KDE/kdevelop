/*
	Copyright (C) 2005 by Tobias Erbsland <te@profzone.ch>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	version 2, License as published by the Free Software Foundation.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public License
	along with this library; see the file COPYING.LIB.  If not, write to
	the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.
*/
#ifndef QTBUILDCONFIG_H
#define QTBUILDCONFIG_H

#include <qobject.h>
#include <qstringlist.h>

class CppSupportPart;
class QDomDocument;

/**
	@brief The QtBuildConfig class stores all parameters related to the used Qt library

	@author Tobias Erbsland <te@profzone.ch>
*/
class QtBuildConfig : public QObject
{
	Q_OBJECT

public:
	QtBuildConfig( CppSupportPart* part, QDomDocument* dom );
	virtual ~QtBuildConfig();

	inline bool isUsed() const { return m_used; }
	inline int version() const { return m_version; }
	inline int includeStyle() const { return m_includeStyle; }
	inline const QString& root() const { return m_root; }
    inline const QString& qmakePath() const { return m_qmakePath; }
    inline const QString& designerPath() const { return m_designerPath; }
    inline const QStringList& designerPluginPaths() const { return m_designerPluginPaths; }
	inline const QString& designerIntegration() const { return m_designerIntegration; }

	void setUsed( bool used );
	void setVersion( int version );
	void setIncludeStyle( int style );
	void setRoot( const QString& root );
    void setDesignerPath( const QString& path );
    void setDesignerPluginPaths( const QStringList& pfx );
    void setQMakePath( const QString& path );
	void setDesignerIntegration( const QString& designerIntegration );
	void init();

public slots:
	void store();

signals:
	void stored();

private:

    bool isValidQtDir( const QString& ) const;
    void findQtDir();
    QString findExecutable( const QString& ) const;
    void buildBinDirs( QStringList& ) const;
    bool isExecutable( const QString& ) const;

	CppSupportPart* m_part; ///< The cpp support part
	QDomDocument* m_dom; ///< The project configuration

	bool m_used; ///< Flag if qt is used in this project.
	int m_version; ///< The major version of the qt library (3 or 4)
	int m_includeStyle; ///< The type of include style used (qt 3 or 4)
	QString m_root; ///< The root directory of the used qt installation for Qt3
    QString m_designerPath; ///< The path including the binary name of Qt Designer
    QString m_qmakePath; ///< The path including the binary name of QMake
    QStringList m_designerPluginPaths; ///< The Prefix for Designer
	QString m_designerIntegration; ///< The type of designer used, kdevdesigner or qt designer

	static const QString m_configRoot; ///< The root path of the configuration
};

#endif

// kate: indent-mode csands; tab-width 4; space-indent off;
