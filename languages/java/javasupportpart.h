/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002-2003 by Roberto Raggi                              *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _JAVASUPPORTPART_H_
#define _JAVASUPPORTPART_H_

#include <kdevcore.h>
#include <kdevlanguagesupport.h>

#include <kdialogbase.h>
#include <qpointer.h>
#include <qstring.h>
#include <qwaitcondition.h>
#include <qdatetime.h>
//Added by qt3to4:
#include <QCustomEvent>
#include <QLabel>
#include <Q3PopupMenu>
#include <Q3PtrList>

class Context;
class ProblemReporter;
class BackgroundParser;
class Catalog;
class QLabel;
class Q3ProgressBar;
class QStringList;
class Q3ListViewItem;
class KListView;
class Driver;

namespace KParts { class Part; }
namespace KTextEditor
{
    class Document;
    class View;
    class EditInterface;
    class SelectionInterface;
    class ViewCursorInterface;
}

class JavaSupportPart : public KDevLanguageSupport
{
    Q_OBJECT

public:
    JavaSupportPart( QObject *parent, const char *name, const QStringList &args );
    virtual ~JavaSupportPart();

    bool isValid() const { return m_valid; }

    ProblemReporter* problemReporter() { return m_problemReporter; }
    BackgroundParser* backgroundParser() { return m_backgroundParser; }

    const Q3PtrList<Catalog>& catalogList() { return m_catalogList; }

    bool isValidSource( const QString& fileName ) const;
    QStringList fileExtensions( ) const;

    virtual void customEvent( QCustomEvent* ev );

    virtual QStringList subclassWidget(const QString& formName);
    virtual QStringList updateWidget(const QString& formName, const QString& fileName);

    KTextEditor::Document* findDocument( const KURL& url );

    static KConfig *config();

    virtual QString formatTag( const Tag& tag );
    virtual QString formatModelItem( const CodeModelItem *item, bool shortDescription=false );
    virtual void addClass();

signals:
    void fileParsed( const QString& fileName );

protected:
    virtual KDevLanguageSupport::Features features();
    virtual KMimeType::List mimeTypes();
    virtual QString formatClassName(const QString &name);
    virtual QString unformatClassName(const QString &name);
    virtual void addMethod( ClassDom klass );
    virtual void addAttribute( ClassDom klass );

private slots:
    void activePartChanged(KParts::Part *part);
    void partRemoved( KParts::Part* part );
    void projectOpened();
    void projectClosed();
    void savedFile(const KURL &fileName);
    void configWidget(KDialogBase *dlg);
    void projectConfigWidget(KDialogBase *dlg);
    void contextMenu(Q3PopupMenu *popup, const Context *context);
    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);
    void changedFilesInProject( const QStringList & fileList );
    void slotProjectCompiled();
    void setupCatalog();

    void slotNewClass();

    void slotNeedTextHint( int, int, QString& );

    /**
     * loads, parses and creates both classstores needed
     */
    void initialParse( );

    /**
     * only parses the current project
     */
    bool parseProject( );

private:

    /**
     * checks if a file has to be parsed
     */
    void maybeParse( const QString& fileName );
    void removeWithReferences( const QString& fileName );

    QStringList modifiedFileList();
    QString findSourceFile();
    int pcsVersion();
    void setPcsVersion( int version );

    void saveProjectSourceInfo();

    QString m_contextFileName;

    QPointer< ProblemReporter > m_problemReporter;
    BackgroundParser* m_backgroundParser;

    KTextEditor::Document* m_activeDocument;
    KTextEditor::View* m_activeView;
    KTextEditor::SelectionInterface* m_activeSelection;
    KTextEditor::EditInterface* m_activeEditor;
    KTextEditor::ViewCursorInterface* m_activeViewCursor;
    QString m_activeFileName;

    QWaitCondition m_eventConsumed;
    bool m_projectClosed;

    QMap<QString, QDateTime> m_timestamp;
    bool m_valid;

    Q3PtrList<Catalog> m_catalogList;
    Driver* m_driver;
    QString m_projectDirectory;

    ClassDom m_activeClass;
    FunctionDom m_activeFunction;
    VariableDom m_activeVariable;

    friend class KDevJavaSupportIface;
    friend class JavaDriver;
};

#endif
