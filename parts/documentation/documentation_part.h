/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __KDEVPART_DOCUMENTATION_H__
#define __KDEVPART_DOCUMENTATION_H__

#include <qguardedptr.h>
#include <kdevplugin.h>


class FindDocumentation;
class DocumentationWidget;
class KListBox;
class KConfig;
class KDialogBase;
class ConfigWidgetProxy;
class DocumentationPlugin;
class KURL;
class Context;
class QPopupMenu;

/*
 Please read the README.dox file for more info about this part
 */
class DocumentationPart : public KDevPlugin
{
    Q_OBJECT
public:
    enum ContextFeature { Finder, IndexLookup, FullTextSearch, GotoMan, GotoInfo };

    DocumentationPart(QObject *parent, const char *name, const QStringList &);
    ~DocumentationPart();
    
    void emitIndexSelected(KListBox *indexBox);
    void emitBookmarkLocation(const QString &title, const KURL &url);
    bool configure(int page = 0);
    
    bool hasContextFeature(ContextFeature feature);
    void setContextFeature(ContextFeature feature, bool b);

public slots:
    void lookInDocumentationIndex();
    void lookInDocumentationIndex(const QString &term);
    void contextLookInDocumentationIndex();
    void contextFindDocumentation();
    void searchInDocumentation();
    void searchInDocumentation(const QString &term);
    void contextSearchInDocumentation();
    void manPage();
    void manPage(const QString &term);
    void contextManPage();
    void infoPage();
    void infoPage(const QString &term);
    void contextInfoPage();
    
signals:
    void indexSelected(KListBox *indexBox);
    void bookmarkLocation(const QString &title, const KURL &url);
    
protected:
    void loadDocumentationPlugins();
    KConfig *config();
    void setupActions();
    
protected slots:
    void insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo);
    void contextMenu(QPopupMenu *popup, const Context *context);
    
private:
    QGuardedPtr<DocumentationWidget> m_widget;
    ConfigWidgetProxy *m_configProxy;
    
    QValueList<DocumentationPlugin*> m_plugins;
    
    QString m_contextStr;
    
friend class DocGlobalConfigWidget;
friend class SearchView;
friend class FindDocumentation;
};

#endif
