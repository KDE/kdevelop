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

#include <qlist.h>

#include "toplevel.h"
#include "kdevcore.h"

class KDevCoreIface;
class KDevApi;
class KDevFactory;
class DocumentationPart;

#ifdef NEW_EDITOR
#include "keditor/editor.h"
#else
class TextEditorDocument;
class EditorPart;
#endif


class Core : public KDevCore
{
    Q_OBJECT
    
public:
    Core();
    ~Core();

protected:
    virtual void embedWidget(QWidget *w, Role role, const QString &shortCaption);
    virtual void raiseWidget(QWidget *w);
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
    virtual void message(const QString &str);
    virtual KParts::PartManager *partManager() const { return manager; };

#ifdef NEW_EDITOR
    KEditor::Editor *editor();
#endif
	    
private slots:
    void activePartChanged(KParts::Part *part);
    void docPartDestroyed();
    void docContextMenu(QPopupMenu *popup, const QString &url, const QString &selection);
    void editorPartDestroyed();
    void editorContextMenu(QPopupMenu *popup, const QString &linestr, int col);
    void wantsToQuit();
    void openFileInteractionFinished(const QString &fileName);
    void saveFileInteractionFinished(const QString &fileName);

    void slotOpenFile();
    void slotSaveFile();
    void slotSaveFileAs();
    void slotCloseWindow();
    void slotSplitVertically();
    void slotSplitHorizontally();
    void slotKillBuffer();
    void slotQuit();
    void slotProjectOpen();
    void slotProjectClose();
    void slotProjectImport();
    void slotBufferSelected();
    void slotSettingsCustomize();
    void slotStop();

private:
    KActionCollection *actionCollection();
    void initActions();
    void initComponent(KDevPart *comp);
    void removeComponent(KDevPart *comp);
    void initComponents();
    void removeComponents();
    DocumentationPart *createDocumentationPart();
#ifdef NEW_EDITOR
    KEditor::Document *createDocument(const KURL &url);
#else
    EditorPart *createEditorPart();
#endif
    void updateBufferMenu();
    void splitWindow(Orientation orient);
 
    void openProject();
    void closeProject();

    TopLevel *win;
    KParts::PartManager *manager;
    KParts::Part *activePart;
    KDevCoreIface *dcopIface;
    KDevApi *api;
    QList<KDevPart> components;
    QList<KDevPart> runningComponents;
    QList<DocumentationPart> docParts;
#ifdef NEW_EDITOR
    KEditor::Editor *_editor;
#else
    QList<EditorPart> editorParts;
    QList<TextEditorDocument> editedDocs;
    QList<KAction> bufferActions;
#endif
    QString projectFile;
};

#endif
