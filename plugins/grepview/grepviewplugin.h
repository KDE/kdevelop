/***************************************************************************
 *   Copyright 1999-2001 by Bernd Gehrmann                                 *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GREPVIEWPART_H_
#define GREPVIEWPART_H_

#include <interfaces/iplugin.h>
#include <interfaces/contextmenuextension.h>

#include <QtCore/QVariant>

class GrepViewPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    GrepViewPlugin( QObject *parent, const QVariantList & = QVariantList() );
    ~GrepViewPlugin();

    void rememberSearchDirectory(QString const & directory);
    virtual KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);
    
private Q_SLOTS:
    void showDialog();

private:
    QString m_directory;
};

#endif
