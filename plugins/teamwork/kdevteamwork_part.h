/***************************************************************************
   copyright            : (C) 2006 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KDEVPART_KDEVCODEVIEW_H__
#define __KDEVPART_KDEVCODEVIEW_H__

#include <QtCore/QPointer>

#include <iplugin.h>

class KDevTeamworkViewFactory;
class KUrl;
class QModelIndex;
class KDevTeamwork;
class KDevTeamworkPartFactory;

namespace KDevelop {
  class ICore;
  class IDocumentController;
  class IProject;
}

class KDevTeamworkPart: public KDevelop::IPlugin
{
    Q_OBJECT
public:
    enum RefreshPolicy
    {
        Refresh,
        NoRefresh,
        ForceRefresh
    };

public:
    KDevTeamworkPart( QObject *parent, const QStringList & );
    virtual ~KDevTeamworkPart();

    //KDevPlugin methods
    virtual QWidget* pluginView() const;
    virtual Qt::DockWidgetArea dockWidgetAreaHint() const;

    void import( RefreshPolicy policy = Refresh );
    
    static KDevelop::ICore* staticCore();

    static KDevelop::IDocumentController* staticDocumentController();

    virtual void restorePartialProjectSession(const QDomElement* el);

    virtual void savePartialProjectSession(QDomElement* el);

    void setView( QWidget* view );

      // KDevelop::Plugin methods
    virtual void unload();

    signals:
    void refresh();

private slots:
    void projectOpened( KDevelop::IProject* );
    void projectClosed( KDevelop::IProject* );

private:
    void destroyTeamwork();
    void startTeamwork( KDevelop::IProject* );

    KDevelop::IProject* m_currentProject;
    static KDevTeamworkPart* m_self;
    QPointer<KDevTeamwork> m_teamwork;
    QPointer<QWidget> m_window;
    KDevTeamworkViewFactory* m_factory;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
