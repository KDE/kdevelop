/*
  $Id$

   Copyright (C) 1998, 1999 Jochen Wilhelmy
                            digisnap@cs.tu-berlin.de

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qkeycode.h>
#include <qobject.h>
#include <qcombobox.h>

#include <klocale.h>

#include "kckey.h"

#include "kguicommand.h"

/*
The following things i have discovered:
- on a pc with num lock unset the key "8" is Up, but Shift-"8" is 8.
- on a pc with num lock set, cursor keys have the X11 modifier 2 set.
- on old Sun workstations without cursor keys the key "8" is Up,
  but Shift-"8" is Shift-F28.
- on old Sun workstations with cursor keys the X11 modifier 3 is set.
*/

const int SHIFT = Qt::SHIFT;
const int ALT = Qt::ALT;
const int CTRL = Qt::CTRL;
const int META = 0x00010000;
//const int MOD2 = 0x00020000;
//const int MOD3 = 0x00040000;
//const int MOD5 = 0x00080000;
const int modifierMask = SHIFT | CTRL | ALT | META;// | MOD2 | MOD3 | MOD5;

int globalModifiers;

bool isSpecialKey(int keyCode) {
  int special[] = {Qt::Key_CapsLock, Qt::Key_NumLock, Qt::Key_ScrollLock, 
    Qt::Key_unknown, 0};
//    Key_Shift, Key_Control, Key_Meta, Key_Alt, 0};
  int *i;

  i = special;
  while (*i != 0) {
    if (*i == keyCode) return true;
    i++;
  }
  return false;
}


int modifier2Flag(int keyCode) {
  if (keyCode == Qt::Key_Shift) return SHIFT;
  if (keyCode == Qt::Key_Control) return CTRL;
  if (keyCode == Qt::Key_Meta) return META;
  if (keyCode == Qt::Key_Alt) return ALT;
  return 0;
}


const QString keyToLanguage(int keyCode) {
  QString s;
  int z;
 	
  if (keyCode & SHIFT) {
    s = i18n("Shift");
  }
  if (keyCode & CTRL) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Ctrl");
  }
  if (keyCode & ALT) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Alt");
  }
  if (keyCode & META) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Meta");
  }
/*  if (keyCode & MOD2) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Mod2");
  }
  if (keyCode & MOD3) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Mod3");
  }
  if (keyCode & MOD5) {
    if (!s.isEmpty()) s += '+';
    s += i18n("Mod5");
  }*/

  keyCode &= ~modifierMask;
  if (keyCode != 0) {
    for (z = 0; z < NB_KEYS; z++) {
      if (keyCode == KKEYS[z].code) {
        if (!s.isEmpty()) s += '+';
        s += i18n(KKEYS[z].name);
        break;
      }
    }
  }
  return s;
}

const QString keyToString(int keyCode) {
  QString s;
  int z;
 	
  if (keyCode & SHIFT) {
    s = "Shift";
  }
  if (keyCode & CTRL) {
    if (!s.isEmpty()) s += '+';
    s += "Ctrl";
  }
  if (keyCode & ALT) {
    if (!s.isEmpty()) s += '+';
    s += "Alt";
  }
  if (keyCode & META) {
    if (!s.isEmpty()) s += '+';
    s += "Meta";
  }
/*  if (keyCode & MOD2) {
    if (!s.isEmpty()) s += '+';
    s += "Mod2";
  }
  if (keyCode & MOD3) {
    if (!s.isEmpty()) s += '+';
    s += "Mod3";
  }
  if (keyCode & MOD5) {
    if (!s.isEmpty()) s += '+';
    s += "Mod5";
  }*/

  keyCode &= ~modifierMask;
  if (keyCode != 0) {
    for (z = 0; z < NB_KEYS; z++) {
      if (keyCode == KKEYS[z].code) {
        if (!s.isEmpty()) s += '+';
        s += KKEYS[z].name;
        break;
      }
    }
  }
  return s;
}

int myStringToKey(QString s) {
  int keyCode, modifier, z;
  QString keyStr;

//  s.detach(); //for QT < 2
//printf("stringtokey %s\n", s);
  keyCode = 0;
  do {
    z = s.find('+');
    keyStr = (z >= 0) ? s.left(z) : s;
    modifier = 0;
    if (keyStr == "Shift") modifier = SHIFT;
    else if (keyStr == "Ctrl") modifier = CTRL;
    else if (keyStr == "Alt") modifier = ALT;
    else if (keyStr == "Meta") modifier = META;
/*    else if (keyStr == "Mod2") modifier = MOD2;
    else if (keyStr == "Mod3") modifier = MOD3;
    else if (keyStr == "Mod5") modifier = MOD5;*/
    keyCode |= modifier;
    s.remove(0, z +1);

    if (z < 0) {
      if (!modifier) {
        for (z = 0; z < NB_KEYS; z++) {
          if (keyStr == KKEYS[z].name) {
            keyCode |= KKEYS[z].code;
            break;
          }
        }
      }
      return keyCode;
    }
  } while (true);
}

QString removeAnd(QString s) {
  int pos;

//  s.detach(); //for QT < 2
  while ((pos = s.find('&')) != -1) s.remove(pos, 1);
  return s;
}


bool KGuiCmdAccel::equals(int kc1, int kc2) {
  return (keyCode1 == kc1 && (keyCode2 == kc2 || keyCode1 == 0));
}

KGuiCmd::KGuiCmd(int cmdNum, const QString &name, KGuiCmdCategory *c)
  : cmdNum(cmdNum), name(name), category(c) {

  int z;
  for (z = 0; z < nAccels; z++) accels[z].keyCode1 = 0;
}

int KGuiCmd::accelCount() {
  int z;

  for (z = 0; z < nAccels; z++) {
    if (accels[z].keyCode1 == 0) break;
  }
  return z;
}

void KGuiCmd::addAccel(int keyCode1, int keyCode2) {
  int z;

  for (z = 0; z < nAccels; z++) {
    if (accels[z].keyCode1 == 0) {
      accels[z].keyCode1 = keyCode1;
      accels[z].keyCode2 = keyCode2;
      break;
    }
  }
}

QString KGuiCmd::getAccelString(int pos) {
  QString res;
  int keyCode;

  keyCode = accels[pos].keyCode1;
  if (keyCode != 0) {
    res = keyToLanguage(keyCode);

    keyCode = accels[pos].keyCode2;
    if (keyCode != 0) {
      res += ", ";
      res += keyToLanguage(keyCode);
    }
  }
  return res;
}

int KGuiCmd::containsAccel(int keyCode1, int keyCode2) {
  int z;

  for (z = 0; z < nAccels; z++) {
    if (accels[z].keyCode1 == 0) break;
    if (accels[z].keyCode1 == keyCode1 && (accels[z].keyCode2 == keyCode2
      || accels[z].keyCode2 == 0 || keyCode2 == 0)) {

      return z;
    }

    if (accels[z].equals(keyCode1, keyCode2)) return z;
  }
  return -1;
}

void KGuiCmd::removeAccel(int z) {
  while (z < nAccels -1) {
    accels[z] = accels[z +1];
    z++;
  }
  accels[z].keyCode1 = 0;
}

void KGuiCmd::removeAccel(int keyCode1, int keyCode2) {
  int n;

  while ((n = containsAccel(keyCode1, keyCode2)) >= 0) {
    removeAccel(n);
  }
}

QString KGuiCmd::getCategoryName() {
  return i18n(category->name);
}

void KGuiCmd::makeDefault() {
  int z;
  for (z = 0; z < nAccels; z++) defaultAccels[z] = accels[z];
}

void KGuiCmd::restoreDefault() {
  int z;
  for (z = 0; z < nAccels; z++) accels[z] = defaultAccels[z];
}

void KGuiCmd::saveAccels() {
  int z;
  for (z = 0; z < nAccels; z++) oldAccels[z] = accels[z];
}

void KGuiCmd::restoreAccels() {
  int z;
  for (z = 0; z < nAccels; z++) accels[z] = oldAccels[z];
}

void KGuiCmd::changeAccels() {
  if (!accels[0].equals(oldAccels[0].keyCode1, oldAccels[0].keyCode2)) {
    emit changed(getAccelString());
  }
}


void KGuiCmd::readConfig(KConfig *config) {
  QString s;
  int z, nextAccel, next;

  s = removeAnd(name);
  if (!config->hasKey(s)) return;
  s = config->readEntry(s);
//  s.detach(); //for QT < 2
  while ((z = s.find(' ')) != -1) s.remove(z, 1);

  if (s == "(Default)") return;

  for (z = 0; z < nAccels; z++) {
    accels[z].keyCode1 = 0;
    accels[z].keyCode2 = 0;
  }


  z = 0;
  while (!s.isEmpty()) {
    nextAccel = s.find(';');
    if (nextAccel == -1) nextAccel = s.length();

    next = s.findRev(',', nextAccel -1);
    if (next != -1) {
      accels[z].keyCode2 = myStringToKey(s.mid(next +1, nextAccel - next -1));
    } else next = nextAccel;
    accels[z].keyCode1 = myStringToKey(s.left(next));
    s.remove(0, nextAccel +1);
    z++;
  }
}

void KGuiCmd::writeConfig(KConfig *config) {
  QString key;
  int count, z;
  bool def;

  key = removeAnd(name);
  count = accelCount();

  //are current accelerators the default accelerators?
  if (count < nAccels && defaultAccels[count].keyCode1 != 0) {
    def = false;
  } else {
    def = true;
    for (z = 0; z < count; z++) {
      if (!defaultAccels[z].equals(accels[z].keyCode1, accels[z].keyCode2)) {
        def = false;
        break;
      }
    }
  }

  if (def) {
    //default
    //is there a possibility to remove a key?
    if (config->hasKey(key)) config->writeEntry(key, "(Default)");
  } else {
    //not default
    QString s;

    for (z = 0; z < count; z++) {
      if (z != 0) s += ';';
      s += keyToString(accels[z].keyCode1);
      if (accels[z].keyCode2 != 0) {
        s += ',';
        s += keyToString(accels[z].keyCode2);
      }
    }
    config->writeEntry(key, s);
  }
}


void KGuiCmd::emitAccelString() {
  emit changed(getAccelString());
}

KGuiCmdCategory::KGuiCmdCategory(int catNum, const QString &name)
  : catNum(catNum), name(name), selectModifiers(0), selectFlag(0),
  mSelectModifiers(0), mSelectFlag(0) {

  commandList.setAutoDelete(true);
}

void KGuiCmdCategory::setSelectModifiers(int selectModifiers, int selectFlag,
  int mSelectModifiers, int mSelectFlag) {

  this->selectModifiers = selectModifiers;
  this->selectFlag = selectFlag;
  this->mSelectModifiers = mSelectModifiers;
  this->mSelectFlag = mSelectFlag;
}

KGuiCmd *KGuiCmdCategory::containsAccel(int keyCode1, int keyCode2) {
  int z;
  KGuiCmd *command;

  for (z = 0; z < (int) commandList.count(); z++) {
    command = commandList.at(z);
    if (command->containsAccel(keyCode1, keyCode2) != -1) return command;
  }
  return 0L;
}

void KGuiCmdCategory::removeAccel(int keyCode1, int keyCode2) {
  int z;

  for (z = 0; z < (int) commandList.count(); z++) {
    commandList.at(z)->removeAccel(keyCode1, keyCode2);
  }
}

void KGuiCmdCategory::addCommand(KGuiCmd *command) {
  commandList.append(command);
}

KGuiCmd *KGuiCmdCategory::setCurrentCommand(int cmdNum) {
  int z;
  KGuiCmd *command;

  for (z = 0; z < (int) commandList.count(); z++) {
    command = commandList.at(z);
    if (command->cmdNum == cmdNum) return command;
  }
  return 0L;
}

void KGuiCmdCategory::addAccel(int keyCode1, int keyCode2) {
  commandList.current()->addAccel(keyCode1, keyCode2);
}
/*
void KGuiCmdCategory::connectCommand(const QObject *receiver, const char *member) {
  connect(commandList.current(), SIGNAL(activated(int)), receiver, member);
}

void KGuiCmdCategory::disconnectCommand(const QObject *receiver, const char *member) {
  disconnect(commandList.current(), SIGNAL(activated(int)), receiver, member);
}
*/
void KGuiCmdCategory::makeDefault() {
  int z;
  for (z = 0; z < (int) commandList.count(); z++) {
    commandList.at(z)->makeDefault();
  }
}

void KGuiCmdCategory::restoreDefault() {
  int z;
  for (z = 0; z < (int) commandList.count(); z++) {
    commandList.at(z)->restoreDefault();
  }
}

void KGuiCmdCategory::saveAccels() {
  int z;
  for (z = 0; z < (int) commandList.count(); z++) {
    commandList.at(z)->saveAccels();
  }
}

void KGuiCmdCategory::restoreAccels() {
  int z;
  for (z = 0; z < (int) commandList.count(); z++) {
    commandList.at(z)->restoreAccels();
  }
}

void KGuiCmdCategory::changeAccels() {
  int z;
  for (z = 0; z < (int) commandList.count(); z++) {
    commandList.at(z)->changeAccels();
  }
}

void KGuiCmdCategory::readConfig(KConfig *config) {
  int z;
  config->setGroup(name);
  for (z = 0; z < (int) commandList.count(); z++) {
    commandList.at(z)->readConfig(config);
  }
}

void KGuiCmdCategory::writeConfig(KConfig *config) {
  int z;
  config->setGroup(name);
  for (z = 0; z < (int) commandList.count(); z++) {
    commandList.at(z)->writeConfig(config);
  }
}




void KGuiCmdCategory::getCommands(KGuiCmdCategoryActivator *ca) {
  int z;

  for (z = 0; z < (int) commandList.count(); z++) {
    ca->addCommand(new KGuiCmdActivator(commandList.at(z), ca));
  }
}


KGuiCmdManager::KGuiCmdManager() : nextId(0) {
  categoryList.setAutoDelete(true);
}

KGuiCmdManager::~KGuiCmdManager() {
}

int KGuiCmdManager::addCategory(const QString &name) {
  nextId--;
  categoryList.append(new KGuiCmdCategory(nextId, name));
  return nextId;
}

int KGuiCmdManager::addCategory(int catNum, const QString &name) {
  categoryList.append(new KGuiCmdCategory(catNum, name));
  return catNum;
}

KGuiCmdCategory *KGuiCmdManager::setCurrentCategory(int catNum) {
  int z;
  KGuiCmdCategory *category;

  for (z = 0; z < (int) categoryList.count(); z++) {
    category = categoryList.at(z);
    if (category->catNum == catNum) return category;
  }
  return 0L;
}

void KGuiCmdManager::setSelectModifiers(int selectModifiers, int selectFlag,
  int mSelectModifiers, int mSelectFlag) {

  categoryList.current()->setSelectModifiers(selectModifiers, selectFlag,
    mSelectModifiers, mSelectFlag);
}

/*
void KGuiCmdManager::connectCategory(const QObject *receiver, const char *member) {
  connect(categoryList.current(), SIGNAL(activated(int, int)), receiver, member);
}

void KGuiCmdManager::disconnectCategory(const QObject *receiver, const char *member) {
  disconnect(categoryList.current(), SIGNAL(activated(int, int)), receiver, member);
}
*/

int KGuiCmdManager::addCommand(const QString &name, int keyCode01,
  int keyCode11, int keyCode21) {

  nextId--;
  addCommand(nextId, name, keyCode01, keyCode11, keyCode21);
  return nextId;
}

int KGuiCmdManager::addCommand(int cmdNum, const QString &name, int keyCode01,
  int keyCode11, int keyCode21) {

  KGuiCmd *command;
  KGuiCmdCategory *category;

  category = categoryList.current();
  command = new KGuiCmd(cmdNum, name, category);
  command->addAccel(keyCode01, 0);
  command->addAccel(keyCode11, 0);
  command->addAccel(keyCode21, 0);
  category->addCommand(command);
  return cmdNum;
}

void KGuiCmdManager::addAccel(int keyCode1, int keyCode2) {
  categoryList.current()->addAccel(keyCode1, keyCode2);
}
/*
void KGuiCmdManager::connectCommand(const QObject *receiver, const char *member) {
  categoryList.current()->connectCommand(receiver, member);
}

void KGuiCmdManager::disconnectCommand(const QObject *receiver, const char *member) {
  categoryList.current()->disconnectCommand(receiver, member);
}
*/


KGuiCmd *KGuiCmdManager::getCommand(int catNum, int cmdNum) {
  KGuiCmdCategory *category;

  category = setCurrentCategory(catNum);
  if (category != 0L) {
    return category->setCurrentCommand(cmdNum);
  }
  return 0L;
}

KGuiCmd *KGuiCmdManager::containsAccel(int keyCode1, int keyCode2) {
  int z;
  KGuiCmd *command;

  for (z = 0; z < (int) categoryList.count(); z++) {
    command = categoryList.at(z)->containsAccel(keyCode1, keyCode2);
    if (command != 0L) return command;
  }
  return 0L;
}

void KGuiCmdManager::removeAccel(int keyCode1, int keyCode2) {
  int z;

  for (z = 0; z < (int) categoryList.count(); z++) {
    categoryList.at(z)->removeAccel(keyCode1, keyCode2);
  }
}

void KGuiCmdManager::makeDefault() {
  int z;
  for (z = 0; z < (int) categoryList.count(); z++) {
    categoryList.at(z)->makeDefault();
  }
}

void KGuiCmdManager::restoreDefault() {
  int z;
  for (z = 0; z < (int) categoryList.count(); z++) {
    categoryList.at(z)->restoreDefault();
  }
}

void KGuiCmdManager::saveAccels() {
  int z;
  for (z = 0; z < (int) categoryList.count(); z++) {
    categoryList.at(z)->saveAccels();
  }
}

void KGuiCmdManager::restoreAccels() {
  int z;
  for (z = 0; z < (int) categoryList.count(); z++) {
    categoryList.at(z)->restoreAccels();
  }
}

void KGuiCmdManager::changeAccels() {
  int z;
  for (z = 0; z < (int) categoryList.count(); z++) {
    categoryList.at(z)->changeAccels();
  }
}

void KGuiCmdManager::readConfig(KConfig *config) {
  int z;
  for (z = 0; z < (int) categoryList.count(); z++) {
    categoryList.at(z)->readConfig(config);
  }
}

void KGuiCmdManager::writeConfig(KConfig *config) {
  int z;
  for (z = 0; z < (int) categoryList.count(); z++) {
    categoryList.at(z)->writeConfig(config);
  }
}


void KGuiCmdManager::getCategories(KGuiCmdDispatcher *cd) {
  int z;

  for (z = 0; z < (int) categoryList.count(); z++) {
    cd->addCategory(new KGuiCmdCategoryActivator(categoryList.at(z)));
  }
}


KGuiCmdActivator::KGuiCmdActivator(KGuiCmd *command,
  KGuiCmdCategoryActivator *categoryActivator) : command(command),
  categoryActivator(categoryActivator) {

  int z;

  for (z = 0; z < KGuiCmd::nAccels; z++) second[z] = false;
}

void KGuiCmdActivator::matchKey(int keyCode, int modifiers, KGuiCmdMatch &match) {
  int z;
  KGuiCmdAccel *a;
  KGuiCmdCategory *category;
  int c, unmatchedModifiers, flagCmd;
  bool toSecond, sec, b;

  category = command->category;
  for (z = 0; z < KGuiCmd::nAccels; z++) {
    a = &command->accels[z];
    if (a->keyCode1 == 0) break;

    sec = second[z];
    second[z] = false;
    c = (sec) ? a->keyCode2 : a->keyCode1;

    if ((sec || !match.second) && (c & ~modifierMask) == keyCode) {
      toSecond = !sec && a->keyCode2 != 0;

      c &= modifierMask;
      b = c == modifiers;
      if (toSecond) {
        if (b) {
          second[z] = true;
          match.consumeKeyEvent = true;
        }
      } else {
        unmatchedModifiers = modifiers & ~c;
        flagCmd = 0;
        if (!b) {
          b = (c | category->selectModifiers) == modifiers;
          flagCmd = category->selectFlag;
        }
        if (!b) {
          b = (c | category->mSelectModifiers) == modifiers;
          flagCmd = category->mSelectFlag;
        }
        flagCmd |= command->cmdNum;
        if (b && (unmatchedModifiers & ~match.unmatchedModifiers) == 0) {
          match.unmatchedModifiers = unmatchedModifiers;
          match.cmdNum = flagCmd;
          match.command = this;
          match.category = categoryActivator;
          match.consumeKeyEvent = true;
          match.second = sec;
        }
      }
    }

  }
}

void KGuiCmdActivator::trigger() {
  emit activated(command->cmdNum);
  emit categoryActivator->activated(command->cmdNum);
}

KGuiCmdCategoryActivator::KGuiCmdCategoryActivator(KGuiCmdCategory *category)
  : category(category) {

  commandList.setAutoDelete(true);
  category->getCommands(this);
}

void KGuiCmdCategoryActivator::matchKey(int keyCode, int modifiers, KGuiCmdMatch &match) {
  int z;

  for (z = 0; z < (int) commandList.count(); z++) {
    commandList.at(z)->matchKey(keyCode, modifiers, match);
  }
}

KGuiCmdActivator *KGuiCmdCategoryActivator::getCommand(int cmdNum) {
  int z;
  KGuiCmdActivator *command;

  for (z = 0; z < (int) commandList.count(); z++) {
    command = commandList.at(z);
    if (command->command->cmdNum == cmdNum) return command;
  }
  return 0L;
}

KGuiCmdDispatcher::KGuiCmdDispatcher(QObject *host, KGuiCmdManager *m)
  : commandManager(m), enabled(true) {

  host->installEventFilter(this);
  connect(host, SIGNAL(destroyed()), this, SLOT(destroy()));
  categoryList.setAutoDelete(true);
  commandManager->getCategories(this);
}

void KGuiCmdDispatcher::connectCommand(int catNum, int cmdNum,
  const QObject *receiver, const char *member) {

  KGuiCmdCategoryActivator *category;
  KGuiCmdActivator *command;

  category = getCategory(catNum);
  if (category != 0L) {
    command = category->getCommand(cmdNum);
    if (command != 0L) connect(command, SIGNAL(activated(int)), receiver,
      member);
  }
}

void KGuiCmdDispatcher::connectText(int catNum, int cmdNum,
  const QObject *receiver, const char *member) {

  KGuiCmd *command;

  command = commandManager->getCommand(catNum, cmdNum);
  if (command != 0L) connect(command, SIGNAL(changed(const QString &)), receiver,
    member);
  command->emitAccelString();
}

void KGuiCmdDispatcher::connectCategory(int catNum, const QObject *receiver,
  const char *member) {

  KGuiCmdCategoryActivator *category;

  category = getCategory(catNum);
  if (category != 0L) connect(category, SIGNAL(activated(int)), receiver,
    member);
}

KGuiCmdActivator *KGuiCmdDispatcher::getCommand(int catNum, int cmdNum) {
  KGuiCmdCategoryActivator *category;

  category = getCategory(catNum);
  if (category != 0L) return category->getCommand(cmdNum);
  return 0L;
}

void KGuiCmdDispatcher::destroy() {
  //printf("KGuiCmdDispatcher destroyed\n");
  delete this;
}

KGuiCmdCategoryActivator *KGuiCmdDispatcher::getCategory(int catNum) {
  int z;
  KGuiCmdCategoryActivator *category;

  for (z = 0; z < (int) categoryList.count(); z++) {
    category = categoryList.at(z);
    if (category->category->catNum == catNum) return category;
  }
  return 0L;
}
/*
KGuiCmdActivator *KGuiCmdDispatcher::getCommand(int catNum, int cmdNum) {

  KGuiCmdCategoryActivator *category;

  category = getCategory(catNum);
  if (category != 0L) return category->getCommand(cmdNum);
  return 0L;
}
*/

bool KGuiCmdDispatcher::eventFilter(QObject *, QEvent *e) {

  if (enabled && e->type() == QEvent::Accel) {    // key press
    int keyCode, modifiers, z;
    KGuiCmdMatch match;

    keyCode = ((QKeyEvent *) e)->key();
    if (isSpecialKey(keyCode) || modifier2Flag(keyCode)) return false;
    modifiers = globalModifiers;//((KGuiCmdApp *) kapp)->getModifiers();
    match.unmatchedModifiers = -1;
    match.consumeKeyEvent = false;
    match.second = false;
//  printf("Event Filter %d %d\n", keyCode, modifiers);
    for (z = 0; z < (int) categoryList.count(); z++) {
      categoryList.at(z)->matchKey(keyCode, modifiers, match);
    }
    if (match.consumeKeyEvent) {
      if (match.unmatchedModifiers != -1) {
        emit match.command->activated(match.cmdNum);
        emit match.category->activated(match.cmdNum);
      }
      ((QKeyEvent *) e)->accept();
      return true;
    }
  }
  return false;
}


KAccelInput::KAccelInput(QWidget *parent) : QLineEdit(parent) {
  pos = 0;
  keyCodes[0] = keyCodes[1] = 0;
}

int KAccelInput::getKeyCode1() {
  return keyCodes[0];
}

int KAccelInput::getKeyCode2() {
  return keyCodes[1];
}

void KAccelInput::clear() {
  //clear input line
  pos = 0;
  keyCodes[0] = keyCodes[1] = 0;
  setText("");
  emit valid(false);
  emit changed();
}
/*
void KAccelInput::removeModifiers() {

  keyCodes[0] &= ~modifierMask;
  keyCodes[1] &= ~modifierMask;
  setContents();
  emit changed();
}
*/
void KAccelInput::setContents() {
  QString s = keyToLanguage(keyCodes[0]);
  if (keyCodes[1] != 0) {
    s += ", ";
    s += keyToLanguage(keyCodes[1]);
  }
  setText(s);
}

void KAccelInput::keyPressEvent(QKeyEvent *e) {
  int keyCode, modifiers, m;

  keyCode = e->key();
  if (isSpecialKey(keyCode)) return;
  if ((pos != 0 && keyCode == Key_Backspace) || pos >= 2) {
    clear();
    if (keyCode == Key_Backspace) return;
  }

  modifiers = globalModifiers;//((KGuiCmdApp *) kapp)->getModifiers();
  m = modifier2Flag(keyCode);
  if (m) {
    //modifier pressed
    keyCodes[pos] = modifiers |= m;
  } else {
    //key pressed
    keyCodes[pos] = keyCode | modifiers;
    if (pos == 0) emit valid(true);
    emit changed();
    pos++;
  }

  //set contents of input line
  setContents();
}

void KAccelInput::keyReleaseEvent(QKeyEvent *e) {
  int m;

  //remove modifier flags on release
  m = modifier2Flag(e->key());
  if (m && pos < 2) {
    keyCodes[pos] &= ~m;
    setContents();
  }
}


KGuiCmdConfigTab::KGuiCmdConfigTab(QWidget *parent, KGuiCmdManager *manager,
  const char *name) : QWidget(parent, name), commandManager(manager) {
  int z;
  QRect r;
  QComboBox *combo;
  QLabel *label;
  QButton *button;

  commandManager->saveAccels();

  combo = new QComboBox(false, this); //read only combo
  label = new QLabel(combo, i18n("Category:"), this);
  connect(combo, SIGNAL(activated(int)), SLOT(categoryChanged(int)));
  r.setRect(10, 5, 150, 25);
  label->setGeometry(r);
  r.moveBy(0, 25);
  combo->setGeometry(r);

  for (z = 0; z < (int) commandManager->categoryCount(); z++) {
    combo->insertItem(commandManager->getCategory(z)->getName());
  }

  commandList = new QListBox(this);
  label = new QLabel(commandList, i18n("Commands:"), this);
  connect(commandList, SIGNAL(highlighted(int)), SLOT(commandChanged(int)));
  r.moveBy(0, 25);
  label->setGeometry(r);
  r.moveBy(0, 25);
  r.setHeight(185);
  commandList->setGeometry(r);

  keyList = new QListBox(this);
  label = new QLabel(keyList, i18n("Current Keys:"), this);
  connect(keyList, SIGNAL(highlighted(int)), SLOT(keyChanged(int)));
  r.setRect(r.right() +10, 5, 180, 25);
  label->setGeometry(r);
  r.moveBy(0, 25);
  r.setHeight(85);
  keyList->setGeometry(r);

  accelInput = new KAccelInput(this);
  label = new QLabel(accelInput, i18n("Press New Shortcut Key:"), this);
  r.setRect(r.x(), r.bottom() +5, r.width(), 25);
  label->setGeometry(r);
  r.moveBy(0, 25);
  accelInput->setGeometry(r);

  currentBinding = new QLabel(this);
  r.setRect(r.x(), r.bottom() +5, r.width(), 60);
  currentBinding->setGeometry(r);
/*
  button = new QPushButton(i18n("OK"), this);
  connect(button, SIGNAL(clicked()), this, SLOT(assign()));
  connect(button, SIGNAL(clicked()), this, SLOT(accept()));
  r.setRect(r.x(), r.bottom() + 10, 80, 25);
  button->setGeometry(r);

  button = new QPushButton(i18n("Cancel"), this);
  connect(button, SIGNAL(clicked()), this, SLOT(reject()));
  r.moveBy(100, 0);
  button->setGeometry(r);
*/
  assignButton = new QPushButton(i18n("Assign"), this);
  assignButton->setEnabled(false);
  connect(accelInput, SIGNAL(valid(bool)), assignButton, SLOT(setEnabled(bool)));
  connect(accelInput, SIGNAL(changed()), this, SLOT(accelChanged()));
  connect(assignButton, SIGNAL(clicked()), this, SLOT(assign()));
  r.setRect(r.right() +10, 30, 80, 25);
  assignButton->setGeometry(r);

  removeButton = new QPushButton(i18n("Remove"), this);
  removeButton->setEnabled(false);
  connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
  r.moveBy(0, 30);
  removeButton->setGeometry(r);

  button = new QPushButton(i18n("Defaults"), this);
  connect(button, SIGNAL(clicked()), this, SLOT(defaults()));
  r.moveBy(0, 30);
  button->setGeometry(r);

  categoryChanged(0);
}

void KGuiCmdConfigTab::updateKeyList() {
  int z, count, current;

  current = keyList->currentItem();
  keyList->clear();
  count = command->accelCount();
  for (z = 0; z < count; z++) {
    keyList->insertItem(command->getAccelString(z));
  }

  count = keyList->count();
  if (current >= count) current = count -1;
  if (current >= 0) keyList->setCurrentItem(current);
    else removeButton->setEnabled(false);
}

void KGuiCmdConfigTab::categoryChanged(int n) {
  int z;

  category = commandManager->getCategory(n);
  commandList->clear();
  for (z = 0; z < (int) category->commandCount(); z++) {
    commandList->insertItem(removeAnd(category->getCommand(z)->getName()));
  }
  commandChanged(0);
}

void KGuiCmdConfigTab::commandChanged(int n) {
  command = category->getCommand(n);
  updateKeyList();
}

void KGuiCmdConfigTab::keyChanged(int) {
  removeButton->setEnabled(true);
}

void KGuiCmdConfigTab::accelChanged() {
  KGuiCmd *command;

  command = commandManager->containsAccel(accelInput->getKeyCode1(),
    accelInput->getKeyCode2());
  if (command != 0L) {
    QString s;

//    s.sprintf(i18n("Key Binding used in:\n%s\n%s"),
//      command->getCategoryName(), removeAnd(command->getName()).data());
//    currentBinding->setText(s);
    currentBinding->setText(i18n("Key Binding used in:\n%1\n%2")
      .arg(command->getCategoryName()).arg(removeAnd(command->getName())));
  } else {
    if (accelInput->getKeyCode1() == Key_Escape+META+ALT+CTRL+SHIFT)
      currentBinding->setText(i18n("Hi Emacs Fan! ;-)"));
      else currentBinding->setText("");
  }
}

void KGuiCmdConfigTab::assign() {

  commandManager->removeAccel(accelInput->getKeyCode1(),
    accelInput->getKeyCode2());
  command->addAccel(accelInput->getKeyCode1(), accelInput->getKeyCode2());
  accelInput->clear();
  updateKeyList();
}

void KGuiCmdConfigTab::remove() {
  int n;

  n = keyList->currentItem();
  if (n >= 0) command->removeAccel(n);
  updateKeyList();
}

void KGuiCmdConfigTab::defaults() {
  commandManager->restoreDefault();
  updateKeyList();
}

KGuiCmdPopupItem::KGuiCmdPopupItem(KGuiCmdPopup *p) : popup(p) {

}

void KGuiCmdPopupItem::setAccel(const QString &accelString) {
  popup->setAccel(menuItem, accelString);
}

KGuiCmdPopup::KGuiCmdPopup(KGuiCmdDispatcher *disp) : dispatcher(disp) {
  itemList.setAutoDelete(true);
}

int KGuiCmdPopup::addCommand(int catNum, int cmdNum, int id, int index) {
  KGuiCmd *command;
  KGuiCmdActivator *commandActivator;
  KGuiCmdPopupItem *item;

  command = dispatcher->getManager()->getCommand(catNum, cmdNum);
  if (command == 0L) return 0;
  commandActivator = dispatcher->getCommand(catNum, cmdNum);

  item = new KGuiCmdPopupItem(this);
  id = insertItem(command->getName(), commandActivator, SLOT(trigger()),
    0, id, index);

  item->menuItem = findItem(id);
  itemList.append(item);
  connect(command, SIGNAL(changed(const QString &)), item, SLOT(setAccel(const QString &)));
  item->setAccel(command->getAccelString());
//  command->emitAccelString();
  return id;
}

int KGuiCmdPopup::addCommand(int catNum, int cmdNum, QPixmap &pixmap,
  int id, int index) {

  KGuiCmd *command;
  KGuiCmdActivator *commandActivator;
  KGuiCmdPopupItem *item;
  QIconSet icon(pixmap);

  command = dispatcher->getManager()->getCommand(catNum, cmdNum);
  if (command == 0L) return 0;
  commandActivator = dispatcher->getCommand(catNum, cmdNum);

  item = new KGuiCmdPopupItem(this);
  id = insertItem(icon, command->getName(), commandActivator, SLOT(trigger()),
    0, id, index);

  item->menuItem = findItem(id);
  itemList.append(item);
  connect(command, SIGNAL(changed(const QString &)), item, SLOT(setAccel(const QString &)));
  item->setAccel(command->getAccelString());
//  command->emitAccelString();
  return id;
}

int KGuiCmdPopup::addCommand(int catNum, int cmdNum,
  const QObject *receiver, const char *member, int id, int index) {

  KGuiCmd *command;
  KGuiCmdActivator *commandActivator;
  KGuiCmdPopupItem *item;

  command = dispatcher->getManager()->getCommand(catNum, cmdNum);
  if (command == 0L) return 0;
  commandActivator = dispatcher->getCommand(catNum, cmdNum);

  item = new KGuiCmdPopupItem(this);
  connect(commandActivator, SIGNAL(activated(int)), receiver, member);
  id = insertItem(command->getName(), commandActivator, SLOT(trigger()),
    0, id, index);

  item->menuItem = findItem(id);
  itemList.append(item);
  connect(command, SIGNAL(changed(const QString &)), item, SLOT(setAccel(const QString &)));
  item->setAccel(command->getAccelString());
//  command->emitAccelString();
  return id;
}

int KGuiCmdPopup::addCommand(int catNum, int cmdNum, QPixmap &pixmap,
  const QObject *receiver, const char *member, int id, int index) {

  KGuiCmd *command;
  KGuiCmdActivator *commandActivator;
  KGuiCmdPopupItem *item;
  QIconSet icon(pixmap);

  command = dispatcher->getManager()->getCommand(catNum, cmdNum);
  if (command == 0L) return 0;
  commandActivator = dispatcher->getCommand(catNum, cmdNum);

  item = new KGuiCmdPopupItem(this);
  connect(commandActivator, SIGNAL(activated(int)), receiver, member);
  id = insertItem(icon, command->getName(), commandActivator, SLOT(trigger()),
    0, id, index);

  item->menuItem = findItem(id);
  itemList.append(item);
  connect(command, SIGNAL(changed(const QString &)), item, SLOT(setAccel(const QString &)));
  item->setAccel(command->getAccelString());
//  command->emitAccelString();
  return id;
}

//void KGuiCmdPopup::insertItem(KGuiCmdPopupItem *item, int id) {
//}


void KGuiCmdPopup::setText(const QString &text, int id) {
  QString s;
  int i;

  s = this->text(id);
  i = s.find('\t');
  if (i >= 0) {
    s.replace(0, i, text);
  } else {
    s = text;
  }
  changeItem(s, id);
}

void KGuiCmdPopup::setAccel(const QString &accelString, int id) {
  QString s;
  int i;

  s = text(id);
  i = s.find('\t');
  if (i >= 0) {
    s.replace(i + 1, s.length() - i, accelString);
  } else {
    s += '\t';
    s += accelString;
  }
  changeItem(s, id);
}

void KGuiCmdPopup::clear() {
  itemList.clear();
  QPopupMenu::clear();
}

void KGuiCmdPopup::setAccel(QMenuItem *item, const QString &accelString) {
  int index;

  index = ((QList<void> *) mitems)->find(item);
  if (index >= 0) setAccel(accelString, idAt(index));
}


void setGlobalModifiers(XEvent *e) {
  if (e->type == KeyPress) {
    int state;

    globalModifiers = 0;
    state = e->xkey.state;
    //man XKeyEvent
    if (state & ShiftMask  ) globalModifiers = SHIFT;
    if (state & ControlMask) globalModifiers |= CTRL;
    if (state & Mod1Mask   ) globalModifiers |= ALT;
//    if (state & Mod2Mask   ) globalModifiers |= MOD2;
//    if (state & Mod3Mask   ) globalModifiers |= MOD3;
    if (state & Mod4Mask   ) globalModifiers |= META;
//    if (state & Mod5Mask   ) globalModifiers |= MOD5;
  }
}

bool KGuiCmdApp::x11EventFilter(XEvent *e) {
  setGlobalModifiers(e);
  return KApplication::x11EventFilter(e);
}

bool KGuiCmdKWMModuleApp::x11EventFilter(XEvent *e) {
  setGlobalModifiers(e);
  return KWMModuleApplication::x11EventFilter(e);
}
