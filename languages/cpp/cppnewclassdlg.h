/***************************************************************************
 *   Copyright (C) 1998 by Sandy Meier                                     *
 *   smeier@rz.uni-potsdam.de                                              *
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CPPNEWCLASSDLG_H_
#define _CPPNEWCLASSDLG_H_

#include <qlineedit.h>
#include <qlistview.h>
#include <qwidgetstack.h>

#include "codemodel.h"
#include "cppnewclassdlgbase.h"

class CppSupportPart;
class KDevProject;
class QPopupMenu;
class KCompletion;


template<class T>
class PCheckListItem: public QCheckListItem{
public:

    PCheckListItem ( T item, QCheckListItem * parent, const QString & text, Type tt = Controller ):
	QCheckListItem ( parent, text, tt ), m_item(item) {}

    PCheckListItem ( T item, QCheckListItem * parent, QListViewItem * after, const QString & text, Type tt = Controller ):
	QCheckListItem ( parent, after, text, tt ), m_item(item) {}

    PCheckListItem ( T item,  QListViewItem * parent, const QString & text, Type tt = Controller ):
	QCheckListItem ( parent, text, tt ), m_item(item) {}

    PCheckListItem ( T item, QListViewItem * parent, QListViewItem * after, const QString & text, Type tt = Controller ):
	QCheckListItem ( parent, after, text, tt ), m_item(item) {}

    PCheckListItem ( T item, QListView * parent, const QString & text, Type tt = Controller ):
	QCheckListItem ( parent, text, tt ), m_item(item) {}

    PCheckListItem ( T item, QListView * parent, QListViewItem * after, const QString & text, Type tt = Controller ):
	QCheckListItem ( parent, after, text, tt ), m_item(item) {}

    PCheckListItem ( T item, QListViewItem * parent, const QString & text, const QPixmap & p ):
	QCheckListItem ( parent, text, p ), m_item(item) {}

    PCheckListItem ( T item, QListView * parent, const QString & text, const QPixmap & p ):
	QCheckListItem ( parent, text, p ), m_item(item) {}

    T item()
    {
	return m_item;
    }

    QString templateAddition;

private:
    T m_item;
};

template<class T>
class PListViewItem: public QListViewItem{
public:

    PListViewItem ( T item, QListViewItem * parent, QListViewItem * after, const QString & text ):
	QListViewItem ( parent, after, text ), m_item(item) {}

    PListViewItem ( T item,  QListViewItem * parent, const QString & text ):
	QListViewItem ( parent, text ), m_item(item) {}

    PListViewItem ( T item, QListView * parent, const QString & text ):
	QListViewItem ( parent, text ), m_item(item) {}

    PListViewItem ( T item, QListView * parent, QListViewItem * after, const QString & text ):
	QListViewItem ( parent, after, text ), m_item(item) {}

    T item()
    {
	return m_item;
    }

    QString templateAddition;
private:
    T m_item;
};


class CppNewClassDialog : public CppNewClassDialogBase
{
    Q_OBJECT

public:
    CppNewClassDialog(CppSupportPart *part, QWidget *parent=0, const char *name=0);
    ~CppNewClassDialog();

protected:
    virtual void accept();
    virtual void classNameChanged(const QString &text);
    virtual void classNamespaceChanged(const QString &text);
    virtual void headerChanged();
    virtual void implementationChanged();
    virtual void nameHandlerChanged(const QString &text);
    virtual void baseclassname_changed(const QString &text);
    virtual void baseIncludeChanged(const QString &text);
    
    virtual void addBaseClass();
    virtual void remBaseClass();
    virtual void remBaseClassOnly();
    virtual void currBaseNameChanged(const QString &text);
    virtual void currBasePrivateSet();
    virtual void currBaseProtectedSet();
    virtual void currBasePublicSet();
    virtual void currBaseVirtualChanged(int val);
    virtual void currBaseSelected(QListViewItem *it);
    virtual void scopeboxActivated(int value);

    virtual void checkObjCInheritance(int val);
    virtual void checkQWidgetInheritance(int val);

    virtual void upbaseclass_button_clicked();
    virtual void downbaseclass_button_clicked();
    virtual void baseclasses_view_selectionChanged();

    virtual void newTabSelected(const QString &text);
    virtual void newTabSelected(QWidget *w);
    virtual void access_view_mouseButtonPressed( int button, QListViewItem * item, const QPoint &p, int c );
    virtual void changeToPrivate();
    virtual void changeToProtected();
    virtual void changeToPublic();
    virtual void changeToInherited();
    virtual void methods_view_mouseButtonPressed(int button ,QListViewItem * item, const QPoint&p ,int c);
    virtual void extendFunctionality();
    virtual void replaceFunctionality();
    virtual void to_constructors_list_clicked();
    virtual void clear_selection_button_clicked();
    virtual void selectall_button_clicked();
    virtual void gtk_box_stateChanged(int val);
    virtual void qobject_box_stateChanged(int val);

    void reloadAdvancedInheritance(bool clean = false);
    void parseClass(QString clName, QString inheritance);
    void parsePCSClass(QString clName, QString inheritance);
    void addToConstructorsList(QCheckListItem *myClass, FunctionDom method);
    void addToMethodsList(QListViewItem *parent, FunctionDom method);
    void addToUpgradeList(QListViewItem *parent, FunctionDom method, QString modifier);
    void addToUpgradeList(QListViewItem *parent, VariableDom attr, QString modifier);
    void clearConstructorsList(bool clean = false);
    void clearMethodsList(bool clean = false);
    void clearUpgradeList(bool clean = false);
    bool isConstructor(QString className, const FunctionDom &method);
    bool isDestructor(QString className, const FunctionDom &method);

private:

    bool headerModified;
    bool baseincludeModified;
    bool implementationModified;
    QString m_parse;
    QPopupMenu *accessMenu;
    QPopupMenu *overMenu;
    CppSupportPart *m_part;
    CodeModel *myModel;

    // configuration variables
    QString interface_url;
    QString implementation_url;
    QString interface_suffix;
    QString implementation_suffix;
    QStringList currNamespace;
    bool lowercase_filenames;
    QStringList currBaseClasses;
    KCompletion * compBasename;
    KCompletion * compNamespace;

    void setCompletionBasename(CodeModel *model);
    void addCompletionBasenameNamespacesRecursive(const NamespaceDom & namespaceDom, const QString & namespaceParent = "");
    void setCompletionNamespaceRecursive(const NamespaceDom & namespaceDom, const QString & namespaceParent = "");
    void setStateOfInheritanceEditors(bool state, bool hideList = true);
    void setAccessForBase(QString baseclass, QString newAccess);
    void setAccessForItem(QListViewItem *curr, QString newAccess, bool isPublic);
    void remClassFromAdv(QString text);
    void checkUpButtonState();
    void checkDownButtonState();
    void updateConstructorsOrder();

    QString classNameFormatted();
    QString templateStrFormatted();
    QString templateParamsFormatted();
    QString classNameFormatted(const QString &);
    QString templateStrFormatted(const QString &);
    QString templateParamsFormatted(const QString &);
    QString templateActualParamsFormatted(const QString &);
    void removeTemplateParams(QString &);

    friend class ClassGenerator;

    //! The class that translates UI input to a class
    class ClassGenerator {

    public:
      ClassGenerator(CppNewClassDialog& _dlg) : dlg(_dlg) {}
      bool generate();

    private:
      bool validateInput();
      void common_text();
      void gen_implementation();
      void gen_interface();
      void genMethodDeclaration(FunctionDom method, QString className, QString templateStr,
        QString *adv_h, QString *adv_cpp, bool extend, QString baseClassName );

      void beautifyHeader(QString &templ, QString &headerGuard,
        QString &includeBaseHeader, QString &author, QString &doc, QString &className, QString &templateStr,
        QString &baseclass, QString &inheritance, QString &qobjectStr, QString &args,
        QString &header, QString &namespaceBeg, QString &constructors, QString &advH_public, QString &advH_public_slots,
        QString &advH_protected, QString &advH_protected_slots, QString &advH_private, QString &advH_private_slots,
        QString &namespaceEnd);
      void beautifySource(QString &templ, QString &header, QString &className, QString &namespaceBeg,
        QString &constructors, QString &advCpp, QString &namespaceEnd, QString &implementation);

      QString className;
      QString templateStr;
      QString templateParams;
      QString header;
      QString implementation;

      QString advConstructorsHeader;
      QString advConstructorsSource;

      KDevProject *project;
      QString subDir, headerPath, implementationPath;
      QString doc;
      QString namespaceStr;
      bool childClass;
      bool objc;
      bool qobject;
      bool gtk;
      QStringList namespaces;
      QString namespaceBeg, namespaceEnd;
      QString argsH;
      QString argsCpp;

      QString advH_public;
      QString advH_public_slots;
      QString advH_protected;
      QString advH_protected_slots;
      QString advH_private;
      QString advH_private_slots;
      QString advCpp;


      CppNewClassDialog& dlg;
    };


    //! workaround to make gcc 2.95.x happy
    friend class CppNewClassDialog::ClassGenerator;
};

#endif
