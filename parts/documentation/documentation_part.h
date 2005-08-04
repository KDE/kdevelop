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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef __KDEVPART_DOCUMENTATION_H__
#define __KDEVPART_DOCUMENTATION_H__

#include <qpointer.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PopupMenu>
#include <Q3CString>
#include <kdevplugin.h>


class FindDocumentation;
class DocumentationWidget;
class IndexBox;
class KConfig;
class KDialogBase;
class ConfigWidgetProxy;
class DocumentationPlugin;
class ProjectDocumentationPlugin;
class KURL;
class Context;
class Q3PopupMenu;

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
    
    void emitIndexSelected(IndexBox *indexBox);
    void emitBookmarkLocation(const QString &title, const KURL &url);
    bool configure(int page = 0);
    
    bool hasContextFeature(ContextFeature feature);
    void setContextFeature(ContextFeature feature, bool b);
    
    bool isAssistantUsed() const;
    void setAssistantUsed(bool b);

public slots:
    void lookInDocumentationIndex();
    void lookInDocumentationIndex(const QString &term);
    void contextLookInDocumentationIndex();
    void contextFindDocumentation();
    void findInDocumentation();
    void findInDocumentation(const QString &term);
    void searchInDocumentation();
    void searchInDocumentation(const QString &term);
    void contextSearchInDocumentation();
    void manPage();
    void manPage(const QString &term);
    void contextManPage();
    void infoPage();
    void infoPage(const QString &term);
    void contextInfoPage();
    void projectOpened();
    void projectClosed();
    
signals:
    void indexSelected(IndexBox *indexBox);
    void bookmarkLocation(const QString &title, const KURL &url);
    
protected:
    void loadDocumentationPlugins();
    KConfig *config();
    void setupActions();
    void loadSettings();
    void saveProjectDocumentationInfo();
    
    Q3CString startAssistant();
    void activateAssistantWindow(const Q3CString &ref);
    void callAssistant(const Q3CString &interface, const Q3CString &method);
    void callAssistant(const Q3CString &interface, const Q3CString &method, const QString &dataStr);
    
protected slots:
    void insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo);
    void contextMenu(Q3PopupMenu *popup, const Context *context);
    
private slots:
    void init();
    
private:
    QPointer<DocumentationWidget> m_widget;
    ConfigWidgetProxy *m_configProxy;
    
    Q3ValueList<DocumentationPlugin*> m_plugins;
    ProjectDocumentationPlugin *m_projectDocumentationPlugin;
    ProjectDocumentationPlugin *m_userManualPlugin;
    
    QString m_contextStr;
    bool m_hasIndex;
    bool m_assistantUsed;
    
friend class DocGlobalConfigWidget;
friend class DocProjectConfigWidget;
friend class SearchView;
friend class FindDocumentation;
};

#endif
