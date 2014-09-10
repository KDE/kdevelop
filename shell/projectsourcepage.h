/***************************************************************************
 *   Copyright (C) 2010 by Aleix Pol Gonzalez <aleixpol@kde.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PROJECTSOURCEPAGE_H
#define KDEVPLATFORM_PROJECTSOURCEPAGE_H

#include <QWidget>

class KJob;
class QUrl;
namespace Ui { class ProjectSourcePage; }
namespace KDevelop
{
    class VcsJob;
    class IProjectProvider;
    class IProjectProviderWidget;
    class IPlugin;
    class IBasicVersionControl;
    class VcsLocationWidget;

class ProjectSourcePage : public QWidget
{
    Q_OBJECT
    public:
        ProjectSourcePage(const QUrl& initial, QWidget* parent = 0);
        virtual ~ProjectSourcePage();

        QUrl workingDir() const;
        
    private slots:
        void setSourceIndex(int index);
        void checkoutVcsProject();
        void projectReceived(KJob* job);
        void reevaluateCorrection();
        void progressChanged(KJob*, unsigned long);
        void infoMessage(KJob*, const QString& text, const QString& rich);
        void locationChanged();
        void projectChanged(const QString& name);
        
    signals:
        void isCorrect(bool);
        
    private:
        void setStatus(const QString& message);
        void clearStatus();
        
        KDevelop::IBasicVersionControl* vcsPerIndex(int index);
        KDevelop::IProjectProvider* providerPerIndex(int index);
        KDevelop::VcsJob* jobPerCurrent();
        
        Ui::ProjectSourcePage* m_ui;
        QList<KDevelop::IPlugin*> m_plugins;
        KDevelop::VcsLocationWidget* m_locationWidget;
        KDevelop::IProjectProviderWidget* m_providerWidget;
};

}
#endif // KDEVPLATFORM_PROJECTSOURCEPAGE_H
