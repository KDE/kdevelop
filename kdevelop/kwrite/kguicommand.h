// $Id$

#ifndef _KGUICOMMAND_H_
#define _KGUICOMMAND_H_

#include <qobject.h>
#include <qlist.h>
#include <qlineedit.h>
#include <qmenudata.h>
#include <qdialog.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>

#include <kapp.h>
#include <kwmmapp.h>
#include <klocale.h>
#include <kconfig.h>

#include <X11/Xlib.h> //used to have the XEvent type

//key functions
bool isSpecialKey(int keyCode);
int modifier2Flag(int keyCode);
const QString keyToLanguage(int keyCode);


class KGuiCmd;
class KGuiCmdCategory;
class KGuiCmdManager;
class KGuiCmdActivator;
class KGuiCmdCategoryActivator;
class KGuiCmdDispatcher;

struct KGuiCmdAccel {
  bool equals(int kc1, int kc2);
  int keyCode1;
  int keyCode2;
};

struct KGuiCmdMatch {
  int unmatchedModifiers;
  int cmdNum;
  KGuiCmdActivator *command;
  KGuiCmdCategoryActivator *category;
  bool consumeKeyEvent;
  bool second;
};

class KGuiCmd : public QObject {
    Q_OBJECT
    friend KGuiCmdCategory;
    friend KGuiCmdManager;
    friend KGuiCmdActivator;
    friend KGuiCmdCategoryActivator;
  public:
    KGuiCmd(int cmdNum, const QString &name, KGuiCmdCategory *);
//    int getCommand() {return cmdNum;}
    int accelCount();
    void addAccel(int keyCode1, int keyCode2);
    QString getAccelString(int pos = 0);
    int containsAccel(int keyCode1, int keyCode2);
    void removeAccel(int pos);
    void removeAccel(int keyCode1, int keyCode2);
    QString getName() {return i18n(name);}
    QString getCategoryName();

    void makeDefault();
    void restoreDefault();
    void saveAccels();
    void restoreAccels();
    void changeAccels();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);

    void emitAccelString();
  signals:
    void changed(const QString &accelString);
  protected:
    static const int nAccels = 4;

    int cmdNum;
    QString name;
    KGuiCmdCategory *category;

    KGuiCmdAccel accels[nAccels];
    KGuiCmdAccel oldAccels[nAccels]; //for "cancel" in the dialog
    KGuiCmdAccel defaultAccels[nAccels];
};

class KGuiCmdCategory {
    friend KGuiCmd;
    friend KGuiCmdManager;
    friend KGuiCmdActivator;
    friend KGuiCmdCategoryActivator;
    friend KGuiCmdDispatcher;
  public:
    KGuiCmdCategory(int catNum, const QString &name);
    ~KGuiCmdCategory() {}
    void setSelectModifiers(int selectModifiers, int selectFlag,
      int mSelectModifiers, int mSelectFlag);
    KGuiCmd *containsAccel(int keyCode1, int keyCode2);
    void removeAccel(int keyCode1, int keyCode2);
    void addCommand(KGuiCmd *command);
    int commandCount() {return (int) commandList.count();}
    KGuiCmd *getCommand(int n) {return commandList.at(n);}
    QString getName() {return i18n(name);}

    KGuiCmd *setCurrentCommand(int cmdNum);
    void addAccel(int keyCode1, int keyCode2);
//    void connectCommand(const QObject *receiver, const char *member);
//    void disconnectCommand(const QObject *receiver, const char *member);

    void makeDefault();
    void restoreDefault();
    void saveAccels();
    void restoreAccels();
    void changeAccels();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);
  protected:
    void getCommands(KGuiCmdCategoryActivator *);

    int catNum;
    QString name;
    int selectModifiers, selectFlag;
    int mSelectModifiers, mSelectFlag;
    QList<KGuiCmd> commandList;
};

/** This is the manager for all gui commands of the application. It should
    exist only once
*/
class KGuiCmdManager {
    friend KGuiCmdDispatcher;
  public:
    KGuiCmdManager();
    ~KGuiCmdManager();
    /** Inserts a new command category and returns a category number (id).
        The name must not be translated (i18n()-ed)
    */
    int addCategory(const QString &name);
    /** Inserts a new command category with given category number.
        The name must not be translated (i18n()-ed)
    */
    int addCategory(int catNum, const QString &name);
    KGuiCmdCategory *setCurrentCategory(int catNum);
    void setSelectModifiers(int selectModifiers, int selectFlag,
      int mSelectModifiers, int mSelectFlag);
    int categoryCount() {return (int) categoryList.count();}
    KGuiCmdCategory *getCategory(int n) {return categoryList.at(n);}

    /** Inserts a new Command into the current category and returns a command
        number (id). It takes up to two default accelerators with one key. To
        insert a default accelerator with two keys use addAccel() after this call.
        The name must not be translated (i18n()-ed)
    */
    int addCommand(const QString &name, int keyCode01 = 0,
      int keyCode11 = 0, int keyCode21 = 0);
    /** Inserts a new Command with the given command number into the current
        category. The name must not be translated (i18n()-ed)
    */
    int addCommand(int cmdNum, const QString &name, int keyCode01 = 0,
      int keyCode11 = 0, int keyCode21 = 0);
    /** Adds a default accelerator to the most recently added command
    */
    void addAccel(int keyCode1, int keyCode2 = 0);

    KGuiCmd *getCommand(int catNum, int cmdNum);
    KGuiCmd *containsAccel(int keyCode1, int keyCode2);
    void removeAccel(int keyCode1, int keyCode2);

    void makeDefault();
    void restoreDefault();
    void saveAccels();
    void restoreAccels();
    void changeAccels();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);
  protected:
    void getCategories(KGuiCmdDispatcher *);

    QList<KGuiCmdCategory> categoryList;
    int nextId;
};


class KGuiCmdActivator : public QObject {
    Q_OBJECT
//    friend KGuiCmdManager;
    friend KGuiCmdCategoryActivator;
    friend KGuiCmdDispatcher;
  public:
    KGuiCmdActivator(KGuiCmd *, KGuiCmdCategoryActivator *);
    ~KGuiCmdActivator() {}
    void matchKey(int keyCode, int modifiers, KGuiCmdMatch &match);
  public slots:
    void trigger();
  signals:
    void activated(int cmdNum);
  protected:
    KGuiCmd *command;
    KGuiCmdCategoryActivator *categoryActivator;
    bool second[KGuiCmd::nAccels];
};

class KGuiCmdCategoryActivator : public QObject {
    Q_OBJECT
    friend KGuiCmdCategory;
    friend KGuiCmdActivator;
    friend KGuiCmdDispatcher;
  public:
    KGuiCmdCategoryActivator(KGuiCmdCategory *);
    ~KGuiCmdCategoryActivator() {}
    void matchKey(int keyCode, int modifiers, KGuiCmdMatch &match);
  signals:
    void activated(int cmdNum);
  protected:
    void addCommand(KGuiCmdActivator *a) {commandList.append(a);}
    KGuiCmdActivator *getCommand(int cmdNum);

    QList<KGuiCmdActivator> commandList;
    KGuiCmdCategory *category;
};


/** This is the gui command dispatcher. It installs an event filter to the
    host to get all key events. This should exist once per instance (in
    other words once per main window)
*/
class KGuiCmdDispatcher : public QObject {
    Q_OBJECT

    friend KGuiCmdManager;
  public:
    KGuiCmdDispatcher(QObject *host, KGuiCmdManager *);
    ~KGuiCmdDispatcher() {}
    /** This connects the receiver to a command. The command number or'ed
        with the optional modifier flags is sent as integer.
    */
    void connectCommand(int catNum, int cmdNum, const QObject *receiver,
      const char *member);
    /** This makes the receiver get notice about accelerator changes. A const
        QString & is sent, containing the accelerator as translated text.
    */
    void connectText(int catNum, int cmdNum, const QObject *receiver,
      const char *member);
    /** This connects a whole category of commands to the receiver. It
        is convenient for example for edit commands.
    */
    void connectCategory(int catNum, const QObject *receiver,
      const char *member);
    KGuiCmdActivator *getCommand(int catNum, int cmdNum);
    KGuiCmdManager *getManager() {return commandManager;}
    void setEnabled(bool e) {enabled = e;}
  protected slots:
    void destroy();
  protected:
    void addCategory(KGuiCmdCategoryActivator *a) {categoryList.append(a);}
    KGuiCmdCategoryActivator *getCategory(int catNum);
    bool eventFilter(QObject *, QEvent *);

    KGuiCmdManager *commandManager;
    QList<KGuiCmdCategoryActivator> categoryList;
    bool enabled;
};


/** A class for accelerator input
*/
class KAccelInput : public QLineEdit {
    Q_OBJECT
  public:
    KAccelInput(QWidget *parent);
    int getKeyCode1();
    int getKeyCode2();
    void clear();
  signals:
    void valid(bool);
    void changed();
//  public slots:
//    void removeModifiers();
  protected:
    void setContents();
    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);

    int pos;
    int keyCodes[2];
};

class KGuiCmdConfigTab : public QWidget {
    Q_OBJECT
  public:
    KGuiCmdConfigTab(QWidget *parent, KGuiCmdManager *, const char *name = 0L);
    void updateKeyList();
  protected slots:
    void categoryChanged(int);
    void commandChanged(int);
    void keyChanged(int);
    void accelChanged();
    void assign();
    void remove();
    void defaults();
  protected:
//    virtual void done(int);

    QListBox *commandList;
    QListBox *keyList;
    KAccelInput *accelInput;
    QLabel *currentBinding;
    QPushButton *assignButton;
    QPushButton *removeButton;
    KGuiCmdManager *commandManager;
    KGuiCmdCategory *category;
    KGuiCmd *command;
};

class KGuiCmdPopup;

class KGuiCmdPopupItem : public QObject {
     Q_OBJECT
     friend KGuiCmdPopup;
   public:
     KGuiCmdPopupItem(KGuiCmdPopup *);
   public slots:
     void setAccel(const QString &);
   protected:
     KGuiCmdPopup *popup;
     QMenuItem *menuItem;
};

/** A popup menu which works together with the gui command system
*/
class KGuiCmdPopup : public QPopupMenu {
    Q_OBJECT
    friend KGuiCmdPopupItem;
  public:
    /** The popup gets some needed information from the given dispatcher
    */
    KGuiCmdPopup(KGuiCmdDispatcher *);
    ~KGuiCmdPopup() {}
    /** Inserts a command into the popup menu. When the accelerator is
        changed (for example with the dialog) the menu item is updated
        automatically
    */
    int addCommand(int catNum, int cmdNum, int id = -1, int index = -1);
    int addCommand(int catNum, int cmdNum, QPixmap &pixmap,
      int id = -1, int index = -1);
    int addCommand(int catNum, int cmdNum, const QObject *receiver,
      const char *member, int id = -1, int index = -1);
    int addCommand(int catNum, int cmdNum, QPixmap &pixmap,
      const QObject *receiver, const char *member, int id = -1, int index = -1);

    /** Sets the text. changeItem() removes the accelerator string, so don't
        use it
    */
    void setText(const QString &text, int id);
    /** Sets the accelerator string
    */
    void setAccel(const QString &accelString, int id);
    /** Clears the whole popup.
    */
    void clear();
  protected:
    void setAccel(QMenuItem *, const QString &);
 //   void insertItem(KGuiCmdPopupItem *, int id);

    KGuiCmdDispatcher *dispatcher;
    QList<KGuiCmdPopupItem> itemList;
};


class KGuiCmdApp : public KApplication {
  public:
    KGuiCmdApp(int &argc, char *argv[]) : KApplication(argc, argv) {}
    virtual bool x11EventFilter(XEvent *e);
//    int getModifiers() {return modifiers;}
//  protected:
//    int modifiers;
};

class KGuiCmdKWMModuleApp : public KWMModuleApplication {
  public:
    KGuiCmdKWMModuleApp(int &argc, char *argv[])
      : KWMModuleApplication(argc, argv) {}
    KGuiCmdKWMModuleApp(int &argc, char *argv[], const QString &rAppname)
      : KWMModuleApplication(argc, argv, rAppname) {}
    virtual bool x11EventFilter(XEvent *e);
};


#endif
