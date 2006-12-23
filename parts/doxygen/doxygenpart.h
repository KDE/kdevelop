/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2004 by Jonas Jacobi                                    *
 *   jonas.jacobi@web.de                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOXYGENPART_H_
#define _DOXYGENPART_H_

#include <qguardedptr.h>
#include <kdialogbase.h>
#include <kprocess.h>
#include <ktempdir.h>
#include <qstring.h>

#include "kdevplugin.h"

class DoxygenDialog;
class QPopupMenu;
class Context;
class KAction;
class KDialogBase;
class ConfigWidgetProxy;

namespace KParts{
    class Part;
}
namespace KTextEditor{
    class ViewCursorInterface;
    class EditInterface;
}

class DoxygenPart : public KDevPlugin
{
    Q_OBJECT

public:
    DoxygenPart( QObject *parent, const char *name, const QStringList & );
    ~DoxygenPart();

private slots:
    /**
     * Configuration dialog under "Project Options..." that alows
     * to specify doxygen configuration options and store them to
     * a configuration file. The file name is "Doxyfile" and it's stored
     * in the project root.
     */
//    void projectConfigWidget(KDialogBase *dlg);
	void insertConfigWidget( const KDialogBase* dlg, QWidget * page, unsigned int );

    /** run doxygen to generate API documentation */
    void slotDoxygen();

    /** clean the html API docs (delete the generated html files) */
    void slotDoxClean();

    /**
     * Gets called, when the Doxygen process for previewing is finished
     * and shows its output then.
     */
    void slotPreviewProcessExited();

    /**
      * If the current part is KTextEditor::Document, run Doxygen over it.
      * When the process exited slotPreviewProcessExited gets called.
      */
    void slotRunPreview();

    /**
     * Gets called when the currently active part changed.
     *  When the new part is a KTextEditor::Document the filepath gets
     * internally stored and gets processed if slotRunPreview() is called.
     * @see KTextEditor::Document
     */
    void slotActivePartChanged(KParts::Part*);

    /**
     * Inserts a documentation template above a function declaration/definition.
     */
    void slotDocumentFunction();


private:

    /**
     * If a Doxygen configuration file doesn't exist, create one.
     * And copy some of the project settings to it.
     */
    void adjustDoxyfile();

    DoxygenDialog *m_dialog;
	ConfigWidgetProxy * _configProxy;

    //needed for doxygen preview
    QString m_file;
    KTempDir m_tmpDir;
    KAction* m_action;

    //needed for documentFunction
    KAction* m_actionDocumentFunction;
    KAction* m_actionPreview;
    KTextEditor::EditInterface* m_activeEditor;
    KTextEditor::ViewCursorInterface* m_cursor;
};

#endif
