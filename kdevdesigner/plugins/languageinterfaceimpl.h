/**********************************************************************
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef LANGUAGEINTERFACEIMPL_H
#define LANGUAGEINTERFACEIMPL_H

#include "../interfaces/languageinterface.h"

class LanguageInterfaceImpl : public LanguageInterface
{
public:
    LanguageInterfaceImpl( QUnknownInterface *outer = 0 );

    ulong addRef();
    ulong release();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );

    void functions( const QString &code, QValueList<Function> *funcs ) const;
    void connections( const QString &, QValueList<Connection> * ) const {};
    QString createFunctionStart( const QString &className, const QString &func,
				 const QString &returnType, const QString &access );
    QStringList definitions() const;
    QStringList definitionEntries( const QString &definition, QUnknownInterface *designerIface ) const;
    void setDefinitionEntries( const QString &definition, const QStringList &entries, QUnknownInterface *designerIface );
    QString createArguments( const QString & ) { return QString::null; }
    QString createEmptyFunction();
    bool supports( Support s ) const;
    QStringList fileFilterList() const;
    QStringList fileExtensionList() const;
    void preferedExtensions( QMap<QString, QString> &extensionMap ) const;
    void sourceProjectKeys( QStringList &keys ) const;
    QString projectKeyForExtension( const QString &extension ) const;
    QString cleanSignature( const QString &sig ) { return sig; } // #### implement me
    void loadFormCode( const QString &, const QString &,
		       QValueList<Function> &,
		       QStringList &,
		       QValueList<Connection> & );
    QString formCodeExtension() const { return ".h"; }
    bool canConnect( const QString &signal, const QString &slot );
    void compressProject( const QString &, const QString &, bool ) {}
    QString uncompressProject( const QString &, const QString & ) { return QString::null; }
    QString aboutText() const { return ""; }

    void addConnection( const QString &, const QString &,
			const QString &, const QString &,
			QString * ) {}
    void removeConnection( const QString &, const QString &,
			   const QString &, const QString &,
			   QString * ) {}
    QStrList signalNames( QObject *obj ) const;

private:
    QUnknownInterface *parent;
    ulong ref;

};

#endif
