/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
*                                                                         *
*   Copyright (C) 2002 by Victor Röder                                    *
*   victor_roeder@gmx.de                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _AUTOPROJECTPART_H_
#define _AUTOPROJECTPART_H_

#include <qdict.h>
#include <qguardedptr.h>
#include <kgenericfactory.h>

#include "kdevproject.h"

class QDomElement;
class QStringList;
class KDialogBase;
class AutoProjectWidget;
class KSelectAction;


class AutoProjectPart : public KDevProject
{
    Q_OBJECT

public:
    AutoProjectPart( QObject *parent, const char *name, const QStringList &args );
    ~AutoProjectPart();

    /**
     * Implementation of the KDevProject interface.
     */
    virtual QString projectDirectory();
    virtual QString projectName();
    virtual QString mainProgram();
    virtual QString activeDirectory();
    virtual QStringList allFiles();
    virtual void addFile(const QString &fileName);
	virtual void addFiles ( const QStringList& fileList );
    virtual void removeFile(const QString &fileName);
	virtual void removeFiles ( const QStringList& fileList );
	
	/**
	 * Implementation of the KDevPlugin interface.
	 */
	virtual void restorePartialProjectSession ( const QDomElement* el );
	virtual void savePartialProjectSession ( QDomElement* el );

    /**
     * automake specific methods.
     */
    QStringList allBuildConfigs();
    QString currentBuildConfig();
    QString buildDirectory();
    QString topsourceDirectory();
    void startMakeCommand(const QString &dir, const QString &target);

protected:
    /**
     * Reimplemented from KDevProject. These methods are only
     * for use by the application core.
     */
    virtual void openProject(const QString &dirName, const QString &projectName);
    virtual void closeProject();

private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void slotAddTranslation();
    void slotBuild();
    void slotCompileFile();
    void slotClean();
    void slotDistClean();
    void slotInstall();
    void slotMakefilecvs();
    void slotMakeMessages();
    void slotConfigure();
    void slotExecute();
    void slotBuildConfigChanged(const QString &config);
    void slotBuildConfigAboutToShow();
    //void slotImportExisting();
    
private:
    QGuardedPtr<AutoProjectWidget> m_widget;
    QString m_projectName;
    QString m_projectPath;
    KSelectAction *buildConfigAction;
    QString makeEnvironment();
    void setWantautotools();
    QString makefileCvsCommand();
    QString configureCommand();

    // Enble AutoProjectWidget to emit our signals
    friend class AutoProjectWidget;
};

typedef KGenericFactory<AutoProjectPart> AutoProjectFactory;

#endif
