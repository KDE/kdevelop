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

#include "kdevcore.h"

#include "kdevlanguagesupport.h"

#include <kdialogbase.h>
#include <qguardedptr.h>
#include <qstring.h>
#include <qwaitcondition.h>
#include <qdatetime.h>

class ParsedMethod;
class ParsedAttribute;
class ClassStore;
class Context;
class JavaCodeCompletion;
class ProblemReporter;
class BackgroundParser;
class Catalog;
class QLabel;
class QProgressBar;
class QStringList;
class QListViewItem;
class KListView;

namespace KParts { class Part; }
namespace KTextEditor { class EditInterface; class SelectionInterface; class ViewCursorInterface; class Document; };

class JavaSupportPart : public KDevLanguageSupport
{
    Q_OBJECT

public:
    JavaSupportPart( QObject *parent, const char *name, const QStringList &args );
    virtual ~JavaSupportPart();

    bool isValid() const { return m_valid; }

    void setCodeCompletionEnabled( bool b ){ m_bEnableCC = b;    };
    bool codeCompletionEnabled( void   ){ return m_bEnableCC; };

    ProblemReporter* problemReporter() { return m_problemReporter; }
    BackgroundParser* backgroundParser() { return m_backgroundParser; }
#if defined(JAVA_CODECOMPLETION)
    JavaCodeCompletion* codeCompletion() { return m_pCompletion; }
#endif

    const QPtrList<Catalog>& catalogList() { return m_catalogList; }

    QStringList fileExtensions( ) const;

    virtual void customEvent( QCustomEvent* ev );

    virtual QStringList subclassWidget(const QString& formName);
    virtual QStringList updateWidget(const QString& formName, const QString& fileName);

    KTextEditor::Document* findDocument( const KURL& url );

    static KConfig *config();

    void emitFileParsed( const QString& fileName );

    virtual QString formatTag( const Tag& tag );

signals:
    void fileParsed( const QString& fileName );

protected:
    virtual KDevLanguageSupport::Features features();
    virtual KMimeType::List mimeTypes();
    virtual QString formatClassName(const QString &name);
    virtual QString unformatClassName(const QString &name);
    virtual void addMethod(const QString &className);
    virtual void addAttribute(const QString &className);
    virtual void implementVirtualMethods( const QString& className );

private slots:
    void activePartChanged(KParts::Part *part);
    void partRemoved( KParts::Part* part );
    void projectOpened();
    void projectClosed();
    void savedFile(const QString &fileName);
    void configWidget(KDialogBase *dlg);
    void projectConfigWidget(KDialogBase *dlg);
    void contextMenu(QPopupMenu *popup, const Context *context);
    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);
    void changedFilesInProject( const QStringList & fileList );
    void slotProjectCompiled();
    void setupCatalog();

    void slotNewClass();
    void slotCompleteText();

    // code completion related slots - called from config-widget
    void slotEnableCodeCompletion( bool setEnabled );
    void slotNodeSelected( QListViewItem* item );
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

    QStringList modifiedFileList();
    QString findSourceFile();
    int pcsVersion();
    void setPcsVersion( int version );

#if defined(JAVA_CODECOMPLETION)
    JavaCodeCompletion* m_pCompletion;
#endif

    bool withjava;
    QString m_contextFileName;

    bool m_bEnableCC;
    QGuardedPtr< ProblemReporter > m_problemReporter;
    BackgroundParser* m_backgroundParser;

    KTextEditor::SelectionInterface* m_activeSelection;
    KTextEditor::EditInterface* m_activeEditor;
    KTextEditor::ViewCursorInterface* m_activeViewCursor;
    QString m_activeFileName;

    KListView* m_structureView;

    QWaitCondition m_eventConsumed;
    bool m_projectClosed;

    QMap<QString, QDateTime> m_timestamp;
    bool m_valid;

    QPtrList<Catalog> m_catalogList;

    friend class KDevJavaSupportIface;
};

#endif
