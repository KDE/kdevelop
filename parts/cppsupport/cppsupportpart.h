/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CPPSUPPORTPART_H_
#define _CPPSUPPORTPART_H_

#include "kdevcore.h"

#include "kdevlanguagesupport.h"

#include <kdialogbase.h>
#include <qguardedptr.h>
#include "cppsupportwidget.h"
#include <qstring.h>

class ParsedMethod;
class ParsedAttribute;
class ClassStore;
class Context;
class CClassParser;
class CppCodeCompletion;
class ProblemReporter;
class FileRepository;
class QLabel;
class QProgressBar;

namespace KParts { class Part; }
namespace KTextEditor { class EditInterface; };


class CppSupportPart : public KDevLanguageSupport
{
    Q_OBJECT

public:
    CppSupportPart( QObject *parent, const char *name, const QStringList &args );
    ~CppSupportPart();

    void setEnableCC( bool b ){ m_bEnableCC = b;    };
    bool getEnableCC( void   ){ return m_bEnableCC; };

    /**
     * returns pointer to Code Hinting Widget
     */
    CppSupportWidget* getCHWidget( ){ return m_pCHWidget; };
    
    ProblemReporter* problemReporter() { return m_problemReporter; }
    FileRepository* fileRepository() { return m_fileRepository; }

    /*
     * void slotCompleteText();
     * void slotTypeOfExpression();
     */

    /**
     * returns persistant classstore File Extension
     * meant for the project's own classes
     */
    static QString pcsFileExt( ){ return ".pcs"; };

    /**
     * returns pre-parsing File Extension
     * meant for extern includes
     */
    static QString ppFileExt( ){ return "-pp.pcs"; };


    QStringList fileExtensions( ) const;

protected:
    virtual KDevLanguageSupport::Features features();
    virtual QStringList fileFilters();
    virtual QString formatClassName(const QString &name);
    virtual QString unformatClassName(const QString &name);
    virtual void addMethod(const QString &className);
    virtual void addAttribute(const QString &className);
    virtual void implementVirtualMethods( const QString& className );

private slots:
    void activePartChanged(KParts::Part *part);
    void projectOpened();
    void projectClosed();
    void savedFile(const QString &fileName);
    void projectConfigWidget(KDialogBase *dlg);
    void contextMenu(QPopupMenu *popup, const Context *context);
    void addedFileToProject(const QString &fileName);
    void removedFileFromProject(const QString &fileName);

    void slotNewClass();
    void slotSwitchHeader();
    void slotGotoIncludeFile();
    void slotCompleteText();
    void slotTypeOfExpression();

    // code completion related slots - called from config-widget
    void slotEnablePersistantClassStore( bool setEnable );
    void slotEnablePreParsing( bool setEnable );
    void slotChangedPreParsingPath( );
    void slotEnableCodeCompletion( bool setEnabled );
    void slotEnableCodeHinting( bool setEnabled, bool setOutputView );

    /**
     * loads, parses and creates both classstores needed
     */
    void initialParse( );

private:

    /**
     * loads a file into given classstore
     */
    bool restorePreParsedClassStore( ClassStore* cs, const QString fileToLoad );

    /**
     * only parses the current project
     */
    bool parseProject( );

    /**
     * parses the current project and saves classstore into given file
     */
    bool createProjectPCS( const QString fileToSave );

    /**
     * parses directories selected in project options into given file
     */
    bool createPreParsePCS( const QString fileToSave );

    /**
     * checks if a file has to be parsed
     */
    void maybeParse( const QString fileName, ClassStore *store, CClassParser *parser );

    /**
     * parses a directory incl. subdirectories if wanted
     * param directory start directory
     * param withSubDirectoreis true or false
     * param bar progressbar
     * param label for debug output on screen
     */
    void parseDirectory( const QString &directory, bool withSubDirectories,
                         QProgressBar *bar, QLabel *label );

    QString asHeaderCode(ParsedMethod *pm);
    QString asCppCode(ParsedMethod *pm);
    QString asHeaderCode(ParsedAttribute *pa);

    CClassParser *m_pParser;
    CppCodeCompletion* m_pCompletion;
    CClassParser* m_pCCParser;

    bool withcpp;
    QString m_contextFileName;

    bool m_bEnableCC;
    QGuardedPtr< CppSupportWidget > m_pCHWidget;
    QGuardedPtr< ProblemReporter > m_problemReporter;
    QGuardedPtr< FileRepository> m_fileRepository;
};

#endif
