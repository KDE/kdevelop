/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo <adymo@kdevelop.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "profileeditor.h"

#include <QLayout>
#include <QLineEdit>
#include <q3textedit.h>
#include <qpalette.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kdebug.h>
#include <kpushbutton.h>
#include <klistbox.h>
#include <k3listview.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdialogbase.h>
#include <kglobalsettings.h>
#include <kdeversion.h>
#include <kservicetypetrader.h>
#include <profile.h>

#include "addprofilewidget.h"

class ProfileItem: public K3ListViewItem {
public:
    ProfileItem(K3ListView *parent, Profile *profile)
        :K3ListViewItem(parent), m_profile(profile)
    {
        setText(0, profile->genericName());
        setText(1, profile->description());
    }

    ProfileItem(K3ListViewItem *parent, Profile *profile)
        : K3ListViewItem(parent), m_profile(profile)
    {
        setText(0, profile->genericName());
        setText(1, profile->description());
    }

    Profile *profile() const { return m_profile; }

private:
    Profile *m_profile;
};

class EDListItem: public K3ListViewItem{
public:
    EDListItem(K3ListView *parent, const QString &text, bool derived)
        : K3ListViewItem(parent, text), m_derived(derived)
    {
    }

    bool isDerived() const { return m_derived; }

    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
    {
        QColorGroup cgNew = cg;
        if (m_derived)
            cgNew.setColor(QColorGroup::Text, KGlobalSettings::inactiveTextColor());
        K3ListViewItem::paintCell(p, cgNew, column, width, alignment);
    }

private:
    bool m_derived;
};


class ProfileListBuilding {
public:
    ProfileItem * operator() (ProfileItem *parent, Profile *profile)
    {
        parent->setOpen(true);
        return new ProfileItem(parent, profile);
    }
};


ProfileEditor::ProfileEditor(QWidget *parent, const char *name)
    :ProfileEditorBase(parent, name)
{
    refresh();
}

void ProfileEditor::refresh()
{
    profilesList->clear();

    ProfileItem *item = new ProfileItem(profilesList, engine.rootProfile());
    ProfileListBuilding op;
    engine.walkProfiles<ProfileListBuilding, ProfileItem>(op, item, engine.rootProfile());

    profilesList->setSelected(item, true);
    profilesList->setCurrentItem(item);

    refreshAvailableList();
    refreshPropertyCombo();
}

void ProfileEditor::refreshPropertyCombo()
{
	KService::List list = KServiceTypeTrader::self()->query(QLatin1String("KDevelop/Plugin"));
    QStringList props;
    for (KService::List::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        QStringList currProps = (*it)->property("X-KDevelop-Properties").toStringList();
        for (QStringList::const_iterator p = currProps.constBegin();
                p != currProps.constEnd(); ++p)
            if (!props.contains(*p))
                props.append(*p);
    }
    props.sort();
    propertyCombo->insertStringList(props);
    propertyCombo->setCurrentText("");
}

void ProfileEditor::refreshAvailableList()
{
    //filling a list of available plugins
    allList->clear();
    allCore = new K3ListViewItem(allList, i18n("Core"));
    allCore->setOpen(true);
    allGlobal = new K3ListViewItem(allList, i18n("Global"));
    allGlobal->setOpen(true);
    allProject = new K3ListViewItem(allList, i18n("Project"));
    allProject->setOpen(true);

	KService::List olist = engine.allOffers(ProfileEngine::Core);
    for (KService::List::iterator it = olist.begin(); it != olist.end(); ++it)
        new K3ListViewItem(allCore, (*it)->desktopEntryName(), (*it)->genericName());
    olist = engine.allOffers(ProfileEngine::Global);
    for (KService::List::iterator it = olist.begin(); it != olist.end(); ++it)
        new K3ListViewItem(allGlobal, (*it)->desktopEntryName(), (*it)->genericName());
    olist = engine.allOffers(ProfileEngine::Project);
    for (KService::List::iterator it = olist.begin(); it != olist.end(); ++it)
        new K3ListViewItem(allProject, (*it)->desktopEntryName(), (*it)->genericName());
}

void ProfileEditor::profileExecuted(Q3ListViewItem *item)
{
    if (!item || item->text(0) == "KDevelop")
        removeProfileButton->setEnabled(false);
    else
        removeProfileButton->setEnabled(true);

    fillPropertyList(currentProfile());
    fillEDLists(currentProfile());
    fillPluginsList(currentProfile());
}

void ProfileEditor::fillPropertyList(Profile *profile)
{
    derivedPropertiesBox->clear();
    ownPropertiesBox->clear();

    Profile::EntryList list = profile->list(Profile::Properties);
    for (Profile::EntryList::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        if ((*it).derived)
            derivedPropertiesBox->insertItem((*it).name);
        else
            ownPropertiesBox->insertItem((*it).name);
    }
}

void ProfileEditor::fillEDLists(Profile *profile)
{
    //filling a list of enabled plugins
    enabledList->clear();
    Profile::EntryList list = profile->list(Profile::ExplicitEnable);
    for (Profile::EntryList::const_iterator it = list.begin(); it != list.end(); ++it)
        new EDListItem(enabledList, (*it).name, (*it).derived);

    //filling a list of disabled plugins
    disabledList->clear();
    list = profile->list(Profile::ExplicitDisable);
    for (Profile::EntryList::const_iterator it = list.begin(); it != list.end(); ++it)
        new EDListItem(disabledList, (*it).name, (*it).derived);
}

void ProfileEditor::fillPluginsList(Profile *profile)
{
    pluginsView->clear();

    K3ListViewItem *core = new K3ListViewItem(pluginsView, i18n("Core Plugins"));
    core->setOpen(true);
    K3ListViewItem *global = new K3ListViewItem(pluginsView, i18n("Global Plugins"));
    global->setOpen(true);
    K3ListViewItem *project = new K3ListViewItem(pluginsView, i18n("Project Plugins"));
    project->setOpen(true);

	KService::List coreOffers = engine.offers(profile->name(), ProfileEngine::Core);
    for (KService::List::const_iterator it = coreOffers.constBegin();
            it != coreOffers.constEnd(); ++it)
        new K3ListViewItem(core, (*it)->desktopEntryName(), (*it)->genericName(),
            (*it)->property("X-KDevelop-Properties").toStringList().join(", "));

	KService::List globalOffers = engine.offers(profile->name(), ProfileEngine::Global);
    for (KService::List::const_iterator it = globalOffers.constBegin();
            it != globalOffers.constEnd(); ++it)
        new K3ListViewItem(global, (*it)->desktopEntryName(), (*it)->genericName(),
            (*it)->property("X-KDevelop-Properties").toStringList().join(", "));

	KService::List projectOffers = engine.offers(profile->name(), ProfileEngine::Project);
    for (KService::List::const_iterator it = projectOffers.constBegin();
            it != projectOffers.constEnd(); ++it)
        new K3ListViewItem(project, (*it)->desktopEntryName(), (*it)->genericName(),
            (*it)->property("X-KDevelop-Properties").toStringList().join(", "));
}

void ProfileEditor::propertyExecuted(Q3ListBoxItem *item)
{
    removePropertyButton->setEnabled(item != 0);
}

void ProfileEditor::addProfile()
{
    if (!profilesList->currentItem())
        return;

    KDialogBase dlg(KDialogBase::Plain, i18n("Add Profile"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok);
// ### KDE4-PORTING    dlg.plainPage()->setMargin(0);
    (new QVBoxLayout(dlg.plainPage(), 0, 0))->setAutoAdd(true);
    AddProfileWidget *prof = new AddProfileWidget(dlg.plainPage());
    prof->nameEdit->setFocus();
    if (dlg.exec() == QDialog::Accepted)
    {
        Profile *profile = new Profile(currentProfile(), prof->nameEdit->text(),
            prof->genericNameEdit->text(),
            prof->descriptionEdit->text());
        profilesList->currentItem()->setOpen(true);
        new ProfileItem(static_cast<K3ListViewItem*>(profilesList->currentItem()), profile);
    }
}

void ProfileEditor::removeProfile()
{
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
    if (KMessageBox::warningContinueCancel(this, i18n("Remove selected profile and all its subprofiles?"),
        i18n("Remove Profile"),KStdGuiItem::del()) == KMessageBox::Continue)
#else
    if (KMessageBox::warningContinueCancel(this, i18n("Remove selected profile and all its subprofiles?"),
        i18n("Remove Profile"),KGuiItem( i18n( "&Delete" ), "editdelete", i18n( "Delete item(s)" ))) == KMessageBox::Continue)
#endif
    {
        Profile *profile = currentProfile();
        if (profile->remove())
        {
            Q3ListViewItem *item = profilesList->currentItem();
            profilesList->setCurrentItem(item->parent());
            profile->detachFromParent();
            delete profile;
            delete item;
        }
        else
            KMessageBox::error(this, i18n("Cannot remove this profile because it is not a local (user-created) profile."), i18n("Remove Profile"));
    }
}

void ProfileEditor::addProperty()
{
    if ( (!propertyCombo->currentText().isEmpty()) &&
        (ownPropertiesBox->findItem(propertyCombo->currentText()) == 0) &&
        (derivedPropertiesBox->findItem(propertyCombo->currentText()) == 0) )
    {
        ownPropertiesBox->insertItem(propertyCombo->currentText());

        currentProfile()->addEntry(Profile::Properties, propertyCombo->currentText());
        currentProfile()->save();
    }

    fillPluginsList(currentProfile());
}

void ProfileEditor::removeProperty()
{
    currentProfile()->removeEntry(Profile::Properties, ownPropertiesBox->currentText());
    currentProfile()->save();

    ownPropertiesBox->removeItem(ownPropertiesBox->currentItem());

    fillPluginsList(currentProfile());
}

Profile *ProfileEditor::currentProfile()
{
    ProfileItem *item = dynamic_cast<ProfileItem*>(profilesList->currentItem());
    if (!item)
        return 0;
    return item->profile();
}

void ProfileEditor::accept()
{
}

void ProfileEditor::addEnabled()
{
    if (!allList->currentItem() && allEdit->text().isEmpty())
        return;
    QString text;
    if (!allEdit->text().isEmpty())
    {
        text = allEdit->text();
        allEdit->clear();
    }
    else
    {
        if ((allList->currentItem() == allGlobal) || (allList->currentItem() == allProject))
            return;
        text = allList->currentItem()->text(0);
    }

    if (enabledList->findItem(text, 0) != 0)
        return;
    if (disabledList->findItem(text, 0) != 0)
    {
        KMessageBox::error(this, i18n("This plugin is already contained in the list of disabled plugins."),
            i18n("Enable Plugin"));
        return;
    }
    currentProfile()->addEntry(Profile::ExplicitEnable, text);
    currentProfile()->save();
    fillPluginsList(currentProfile());
    new EDListItem(enabledList, text, false);
}

void ProfileEditor::delEnabled()
{
    if (!enabledList->currentItem())
        return;

    EDListItem *item = dynamic_cast<EDListItem*>(enabledList->currentItem());
    if (item && !item->isDerived())
    {
        currentProfile()->removeEntry(Profile::ExplicitEnable, enabledList->currentItem()->text(0));
        currentProfile()->save();
        fillPluginsList(currentProfile());
        delete enabledList->currentItem();
    }
}

void ProfileEditor::addDisabled()
{
    if (!allList->currentItem() && allEdit->text().isEmpty())
        return;
    QString text;
    if (!allEdit->text().isEmpty())
    {
        text = allEdit->text();
        allEdit->clear();
    }
    else
    {
        if ((allList->currentItem() == allGlobal) || (allList->currentItem() == allProject))
            return;
        text = allList->currentItem()->text(0);
    }

    if (disabledList->findItem(text, 0) != 0)
        return;
    if (enabledList->findItem(text, 0) != 0)
    {
        KMessageBox::error(this, i18n("This plugin is already contained in the list of enabled plugins."),
            i18n("Disable Plugin"));
        return;
    }
    currentProfile()->addEntry(Profile::ExplicitDisable, text);
    currentProfile()->save();
    fillPluginsList(currentProfile());
    new EDListItem(disabledList, text, false);
}

void ProfileEditor::delDisabled()
{
    if (!disabledList->currentItem())
        return;

    EDListItem *item = dynamic_cast<EDListItem*>(disabledList->currentItem());
    if (item && !item->isDerived())
    {
        currentProfile()->removeEntry(Profile::ExplicitDisable, disabledList->currentItem()->text(0));
        delete disabledList->currentItem();
        currentProfile()->save();
        fillPluginsList(currentProfile());
    }
}

#include "profileeditor.moc"
