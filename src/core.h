/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CORE_H_
#define _CORE_H_

#include <qglobal.h>
#if (QT_VERSION >= 300)
#include <qptrlist.h>
#else
#include <qlist.h>
#define QPtrList QList
#define QPtrListIterator QListIterator
#endif

#include "toplevel.h"
#include "kdevcore.h"

class KDevCoreIface;
class KDevApi;
class KDevFactory;
class DocumentationPart;
class KAction; 
class KRecentFilesAction;


#include "keditor/editor.h"

// 2002-02-08 added removeWidget( ) - daniel
class Core : public KDevCore
{
    Q_OBJECT
    
public:
    Core();
    ~Core();

public:
    virtual void embedWidget(QWidget *w, Role role, const QString &shortCaption);    
    virtual void raiseWidget(QWidget *w);
    /**
      * added by daniel
      */
    virtual void removeWidget( QWidget* w, Role role );    
    
    
    virtual void gotoFile(const KURL &url);
    virtual void gotoDocumentationFile(const KURL& url,
                                       Embedding embed=Replace);
    virtual void gotoSourceFile(const KURL& url, int lineNum=0,
                                Embedding embed=Replace);
    virtual void gotoExecutionPoint(const QString &fileName, int lineNum=0);
    virtual void saveAllFiles();
    virtual void revertAllFiles();
    virtual void setBreakpoint(const QString &fileName, int lineNum,
                               int id, bool enabled, bool pending);
    virtual void running(KDevPart *which, bool runs);
    virtual void fillContextMenu(QPopupMenu *popup, const Context *context);
    virtual void message(const QString &str);
    virtual KParts::PartManager *partManager() const { return manager; };
    virtual void openProject(const QString& projectFileName);

    KEditor::Editor *editor();
    
    virtual QStatusBar *statusBar() const;

    
private slots:
    void activePartChanged(KParts::Part *part);
    void partCountChanged();

    void docPartDestroyed();
    void docContextMenu(QPopupMenu *popup, const QString &url, const QString &selection);
    void editorContextMenu(QPopupMenu *popup, const QString &linestr, int col);
    void wantsToQuit();
    void openFileInteractionFinished(const QString &fileName);

    void message(KEditor::Document *doc, const QString &str);
    
    void slotSaveAll();
    void slotRevertAll();    
    void slotOpenFile();
    void slotCloseWindow();
    void slotSplitVertically();
    void slotSplitHorizontally();
    void slotKillBuffer();
    void slotQuit();
    void slotProjectOpen();
    void slotProjectClose();
    void slotBufferSelected();
    void slotDocumentationBufferSelected(const KURL &url);
    void slotProjectOptions();
    void slotSettingsCustomize();
    void slotStop();
    void slotBreakPointToggled(KEditor::Document* doc, int line);
    void slotBreakPointEnabled(KEditor::Document* doc, int line);

    void slotUpdateStatusBar();

    void slotDocumentLoaded(KEditor::Document *doc);
    void slotDocumentSaved(KEditor::Document *doc);

    void recentProjectSelected(const KURL &url);
    

private:
    KActionCollection *actionCollection();
    void initActions();
    
    void initPart(KDevPart *part);
    void removePart(KDevPart *part);
    void initGlobalParts();
    void removeGlobalParts();
    
    DocumentationPart *createDocumentationPart();
    KEditor::Document *createDocument(const KURL &url);
    void updateBufferMenu();
    void splitWindow(Orientation orient);
 
    void openProject();
    void closeProject();

    TopLevel *win;
    KParts::PartManager *manager;
    KParts::Part *activePart;
    KDevCoreIface *dcopIface;
    KDevApi *api;
    // All loaded parts
    QPtrList<KDevPart> parts;
    // The global, i.e. not project-related parts
    QPtrList<KDevPart> globalParts;
    // The local, i.e. project-related parts
    QPtrList<KDevPart> localParts;
    QPtrList<KDevPart> runningParts;
    
    QPtrList<DocumentationPart> docParts;
    
    KEditor::Editor *_editor;

    KURL::List viewedURLs;
    QString projectFile;

    KAction *_saveAll, *_revertAll, *_closeWindow, *_killBuffer;
    KRecentFilesAction *_recentProjectAction;

};

#endif
