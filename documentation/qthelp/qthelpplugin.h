/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QTHELPPLUGIN_H
#define QTHELPPLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/idocumentationproviderprovider.h>
#include <kpluginfactory.h>

class QtHelpProvider;
class QtHelpQtDoc;
class QtHelpDocumentation;

class QtHelpPlugin : public KDevelop::IPlugin, public KDevelop::IDocumentationProviderProvider
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IDocumentationProviderProvider )
    public:
        QtHelpPlugin(QObject *parent, const QVariantList & args);
        ~QtHelpPlugin();

        static QtHelpPlugin *self() { return s_plugin; }

        virtual QList<KDevelop::IDocumentationProvider*> providers();
        QList<QtHelpProvider*> qtHelpProviderLoaded();
        bool qtHelpQtDocLoaded();

    public slots:
        void readConfig();
    signals:
        void changedProvidersList() const;
    private:
        void loadQtDocumentation(bool loadQtDoc);
        void searchHelpDirectory(QStringList& pathList, QStringList& nameList, QStringList& iconList, const QString& searchDir);
        void loadQtHelpProvider(QStringList pathList, QStringList nameList, QStringList iconList);

        static QtHelpPlugin *s_plugin;
        QList<QtHelpProvider*> m_qtHelpProviders;
        QtHelpQtDoc* m_qtDoc;
};

K_PLUGIN_FACTORY_DECLARATION(QtHelpFactory)

#endif // QTHELPPLUGIN_H
