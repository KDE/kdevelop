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

#ifndef _CPPSUPPORTPART_H_
#define _CPPSUPPORTPART_H_

#include <kdevcore.h>
#include <kdevlanguagesupport.h>

#include <kdialogbase.h>
#include <qguardedptr.h>
#include <qstring.h>
#include <qwaitcondition.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qprogressbar.h>

class Context;
class CppCodeCompletion;
class CppCodeCompletionConfig;
class ProblemReporter;
class BackgroundParser;
class Catalog;
class QLabel;
class QProgressBar;
class QStringList;
class QListViewItem;
class QTimer;
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

class CppSupportPart : public KDevLanguageSupport
{
    Q_OBJECT

public:
    CppSupportPart( QObject *parent, const char *name, const QStringList &args );
    virtual ~CppSupportPart();

    bool isValid() const { return m_valid; }
    
    QString specialHeaderName( bool local=false ) const;
    void updateParserConfiguration();

	// @fixme - isValid is used to avoid using the problem reporter
	// when a project is first parsed. This because the problem reporter
	// is currently a great slowdown for large projects (see bug #73671)
    ProblemReporter* problemReporter() { return isValid() ? static_cast<ProblemReporter *>(m_problemReporter) : 0; }
    
    BackgroundParser* backgroundParser() { return m_backgroundParser; }
    CppCodeCompletion* codeCompletion() { return m_pCompletion; }
    CppCodeCompletionConfig* codeCompletionConfig() { return m_pCompletionConfig; }

    const QPtrList<Catalog>& catalogList() { return m_catalogList; }
    void addCatalog( Catalog* catalog );
    void removeCatalog( const QString& dbName );

    bool isValidSource( const QString& fileName ) const;

    virtual void customEvent( QCustomEvent* ev );

    virtual QStringList subclassWidget(const QString& formName);
    virtual QStringList updateWidget(const QString& formName, const QString& fileName);
    
    FunctionDefinitionDom currentFunctionDefinition();
    FunctionDefinitionDom functionDefinitionAt( int line, int column );
    FunctionDefinitionDom functionDefinitionAt( NamespaceDom ns, int line, int column );
    FunctionDefinitionDom functionDefinitionAt( ClassDom klass, int line, int column );
    FunctionDefinitionDom functionDefinitionAt( FunctionDefinitionDom fun, int line, int column );

    KTextEditor::Document* findDocument( const KURL& url );

    static KConfig *config();

    virtual QString formatTag( const Tag& tag );
    virtual QString formatModelItem( const CodeModelItem *item, bool shortDescription=false );
    virtual void addClass();

    QString extractInterface( const ClassDom& klass );

    bool isHeader(const QString& fileName) const;
    bool isSource(const QString& fileName) const;

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
    void savedFile(const QString &fileName);
    void configWidget(KDialogBase *dlg);
    void projectConfigWidget(KDialogBase *dlg);
    void contextMenu(QPopupMenu *popup, const Context *context);
    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);
    void changedFilesInProject( const QStringList & fileList );
    void slotProjectCompiled();
    void setupCatalog();
    void codeCompletionConfigStored();
    void recomputeCodeModel( const QString& fileName );
    void slotNewClass();
    void slotSwitchHeader();
    void slotGotoIncludeFile();
    void slotCompleteText();
    void slotMakeMember();
    void slotExtractInterface();
    void slotCursorPositionChanged();
    void slotFunctionHint();
    void gotoLine( int line );
    void gotoDeclarationLine( int line );
    void emitFileParsed();
    void slotParseFiles();

    void slotNeedTextHint( int, int, QString& );

    /**
     * loads, parses and creates both classstores needed
     */
    void initialParse( );

    /**
     * only parses the current project
     */
    bool parseProject( bool force=false );

private:

    /**
     * checks if a file has to be parsed
     */
    void maybeParse( const QString& fileName );
    void removeWithReferences( const QString& fileName );
    void createIgnorePCSFile();

    void MakeMemberHelper(QString& text, int& atline, int& atcol);

    QString sourceOrHeaderCandidate();

    QStringList modifiedFileList();
    QString findSourceFile();
    int pcsVersion();
    void setPcsVersion( int version );

    void saveProjectSourceInfo();
    static QStringList reorder(const QStringList& list );
    static QString findHeader( const QStringList&list, const QString& header );

    CppCodeCompletion* m_pCompletion;
    CppCodeCompletionConfig* m_pCompletionConfig;

    bool withcpp;
    QString m_contextFileName;

    QGuardedPtr< ProblemReporter > m_problemReporter;
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

    QPtrList<Catalog> m_catalogList;
    Driver* m_driver;
    QString m_projectDirectory;
    QStringList m_projectFileList;

    ClassDom m_activeClass;
    FunctionDom m_activeFunction;
    VariableDom m_activeVariable;
    
    QTimer* m_functionHintTimer;
    
    static QStringList m_sourceMimeTypes;
    static QStringList m_headerMimeTypes;

    static QStringList m_sourceExtensions;
    static QStringList m_headerExtensions;
    
    friend class KDevCppSupportIface;
    friend class CppDriver;

	struct JobData
	{
		QDir dir;
		QProgressBar * progressBar;
		QStringList::Iterator it;
		QStringList files;
		QMap< QString, QPair<uint, Q_LONG> > pcs;
		QDataStream stream;
		QFile file;
		
		~JobData()
		{
			delete progressBar;
		}
	};
	
	JobData * _jd;
};

#endif
