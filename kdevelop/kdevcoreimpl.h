/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef kdevcoreimpl_h
#define kdevcoreimpl_h

#include "kdevcore.h"

class CKDevelop;
class KDevCoreIface;

class KDevCoreImpl: public KDevCore{
    Q_OBJECT
public:
    KDevCoreImpl( CKDevelop* =0 );
    virtual ~KDevCoreImpl();

    /**
     * Embed the given widget into the user interface.
     */
    virtual void embedWidget(QWidget *, Role, const QString&);
    /**
     * Make the given widget visible. Useful e.g. for the compiler
     * frontend which wants to be shown in the foreground when
     * a command is started.
     */
    virtual void raiseWidget(QWidget *);

    /**
     * Remove the given widget from the user interface
     */
    virtual void removeWidget( QWidget*, Role );

    /**
     * This method should be called by a part that wants to show a
     * context menu. The parameter context should be filled with
     * information about the context in which this happens (see
     * EditorContext, DocumentationContext, ClassContext, ...).
     * Essentially, this method emits the signal contextMenu()
     * which other parts can use to hook in.
     */
    virtual void fillContextMenu(QPopupMenu *popup, const Context *context);

    /**
       close the current project and opens the new one
    */
    virtual void openProject(const QString& projectFileName);
    /**
     * "Goes" to a file. This is a generic method that is used
     * e.g. by file trees. For non-text files (i.e. files which
     * have a mime-type that doesn't start with "text/") it runs
     * the respective application. Text files are loaded into an
     * editor via # gotoSourceFile().
     */
    virtual void gotoFile(const KURL &url);
    /**
     * Loads a file into the HTML viewer.
     */
    virtual void gotoDocumentationFile(const KURL& url,
                                       Embedding embed=Replace);
    /**
     * Loads a file into the editor and jump to a line number.
     */
    virtual void gotoSourceFile(const KURL& url, int lineNum=0,
                                Embedding embed=Replace);
    /**
     * Goes to a given location in a source file and marks the line.
     * If fileName is null, the mark is cleared.
     * This is used by the debugger to mark the location where the
     * the debugger has stopped.
     */
    virtual void gotoExecutionPoint(const QString &fileName, int lineNum=0);
    /**
     * Saves all modified buffers.
     */
    virtual void saveAllFiles();
    /**
     * Reverts all modified buffers to their version on disk.
     */
    virtual void revertAllFiles();
    /**
     * Sets a breakpoint in the editor document belong to fileName.
     * If id==-1, the breakpoint is deleted.
     */
    virtual void setBreakpoint(const QString &fileName, int lineNum,
                               int id, bool enabled, bool pending);
    /**
     * Marks the component as running (or not running). As long as at least one
     * component is running, the stop button is enabled. When it is pressed,
     * all components get a stopButtonClicked().
     */
    virtual void running(KDevPart *which, bool runs);
    /**
     * Puts a message in the status bar.
     */
    virtual void message(const QString &str);


    /**
     * The manager used to manage the parts.
     */
    virtual KParts::PartManager *partManager() const;


    /**
     * The interface to the editor used
     */
    virtual KEditor::Editor *editor();


    /**
     * The statusbar used
     */
    virtual QStatusBar *statusBar() const;

protected:
    void initPart( KDevPart* );
    void removePart( KDevPart* );
    void initGlobalParts();
    void removeGlobalParts();

private:
    CKDevelop* m_pDevelop;
    KDevApi* api;
    KDevCoreIface* dcopIface;
    KParts::PartManager* manager;

    // All loaded parts
    QPtrList<KDevPart> parts;
    // The global, i.e. not project-related parts
    QPtrList<KDevPart> globalParts;
    // The local, i.e. project-related parts
    QPtrList<KDevPart> localParts;
};

#endif
