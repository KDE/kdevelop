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

#ifndef _KDEVDESIGNERPART_H_
#define _KDEVDESIGNERPART_H_

#include <qmap.h>

#include <kparts/part.h>
#include <kparts/factory.h>

#include "designer.h"

class QWidget;
class QPainter;
class KURL;
class MainWindow;
class KAction;
class QAction;
class KRadioAction;

class KDevDesignerPart : public KInterfaceDesigner::Designer
{
    Q_OBJECT
public:
    KDevDesignerPart(QWidget *parentWidget, const char *widgetName,
                    QObject *parent, const char *name, const QStringList &args);
    virtual ~KDevDesignerPart();

    virtual void setReadWrite(bool rw);
    virtual void setModified(bool modified);
    
    void statusMessage(const QString &msg);

    //KInterfaceDesigner::Designer methods
    virtual void openProject(const QString &projectFile);
    virtual KInterfaceDesigner::DesignerType designerType();

    void emitAddedFunction(const QString &form, KInterfaceDesigner::Function func);
    void emitEditedFunction(const QString &form, KInterfaceDesigner::Function oldFunc, KInterfaceDesigner::Function func);
    void emitRemovedFunction(const QString &form, KInterfaceDesigner::Function func);

    void emitEditFunction(const QString &formName, const QString &functionName);
    void emitEditSource(const QString &formName);
    
    void emitNewStatus(const QString &formName, int status);
    
protected:
    virtual bool openFile();
    virtual bool saveFile();
    
    void setupDesignerWindow();
    void setupActions();
    
    void stateSync(KAction *kaction, QAction *qaction);
    void setupToolsAction(KRadioAction *toggle, QAction *action);

protected slots:
    void setToggleActionChecked(bool b);
    void setToggleActionOn(bool b);
    
    void fileNew();
    void fileOpen();
    void fileClose();
    void fileSaveAs();
    void fileCreateTemplate();
    
    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editDelete();
    void editSelectAll();
    void editAccels();
    void editFunctions();
    void editConnections();
    void editFormSettings();
    void editPreferences();
    
    void projectAddFile();
    void projectImageCollection();
    void projectDatabaseCollections();
    void projectSettings();
    
    void toolsConfigureToolbox();
    
    void layoutAdjustSize();
    void layoutHLayout();
    void layoutVLayout();
    void layoutGridLayout();
    void layoutSplitHLayout();
    void layoutSplitVLayout();
    void layoutBreak();
    
    void windowPreview();
    void windowNext();
    void windowPrev();
    
    void formModified(bool b);
private:
    MainWindow *m_widget;
    
    QMap<const QAction*, KRadioAction*> m_actionMap;
    QMap<const KRadioAction*, QAction*> m_actionMap2;
    KRadioAction* pointerAction;
};

class KInstance;
class KAboutData;

class KDevDesignerPartFactory : public KParts::Factory
{
    Q_OBJECT
public:
    KDevDesignerPartFactory();
    virtual ~KDevDesignerPartFactory();
    virtual KParts::Part* createPartObject( QWidget *parentWidget, const char *widgetName,
                                            QObject *parent, const char *name,
                                            const char *classname, const QStringList &args );
    static KInstance* instance();
 
private:
    static KInstance* s_instance;
    static KAboutData* s_about;
};

#endif // _KDEVDESIGNERPART_H_
