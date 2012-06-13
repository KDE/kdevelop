/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVELOP_TEMPLATECLASSASSISTANT_H
#define KDEVELOP_TEMPLATECLASSASSISTANT_H

#include "createclass.h"
#include "codedescription.h"

class QItemSelection;
namespace KDevelop
{
    
    class TemplateClassAssistant;
    
class KDEVPLATFORMLANGUAGE_EXPORT TemplateSelectionPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString selectedTemplate READ selectedTemplate)
    
public:
    explicit TemplateSelectionPage (TemplateClassAssistant* parent, Qt::WindowFlags f = 0);
    virtual ~TemplateSelectionPage();
    
    QString selectedTemplate() const;
    
private:
    class TemplateSelectionPagePrivate* const d;
    
private slots:
    void currentTemplateChanged (const QModelIndex& index);
    void currentLanguageChanged (const QModelIndex& index);
    void getMoreClicked ();
    void loadFileClicked ();
    
    void saveConfig();
};

class KDEVPLATFORMLANGUAGE_EXPORT TemplateOptionsPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariantHash templateOptions READ templateOptions)
    
public:
    explicit TemplateOptionsPage (TemplateClassAssistant* parent, Qt::WindowFlags f = 0);
    virtual ~TemplateOptionsPage();
    
    void loadXML(const QByteArray& contents);
    QVariantHash templateOptions() const;
    
private:
    class TemplateOptionsPagePrivate* const d;
};

class ClassMembersPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(ClassDescription description READ description WRITE setDescription)
    
public:
    explicit ClassMembersPage (TemplateClassAssistant* parent);
    virtual ~ClassMembersPage();
    
    ClassDescription description() const;
    void setDescription(const ClassDescription& description);
    
    void moveRowTo(int destination, bool relative);
    
private:
    int rows();
    
private Q_SLOTS:
    void moveTop();
    void moveUp();
    void moveDown();
    void moveBottom();
    void currentSelectionChanged(const QItemSelection& current);
    void addItem();
    void removeItem();
    
private:
    class ClassMembersPagePrivate* const d;
};

class KDEVPLATFORMLANGUAGE_EXPORT TemplateClassAssistant : public CreateClassAssistant
{
    Q_OBJECT
public:
    TemplateClassAssistant (QWidget* parent, const KUrl& baseUrl = KUrl());
    virtual ~TemplateClassAssistant();
    
    virtual void setup();
    
    virtual TemplateSelectionPage* newTemplateSelectionPage();
    virtual ClassIdentifierPage* newIdentifierPage();
    virtual OverridesPage* newOverridesPage();
    virtual ClassMembersPage* newMembersPage();
    
    virtual void next();
    virtual void accept();
    
private Q_SLOTS:
    void updateTemplateOptions();
    
private:
    class TemplateClassAssistantPrivate* const d;
};

}

#endif // KDEVELOP_TEMPLATECLASSASSISTANT_H
