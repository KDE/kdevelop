/****************************************************************************
 * cclasspropertiesdlgimpl.h                                                *
 * Safe Implementation class from cclassproperties.ui                       *
 *--------------------------------------------------------------------------*
 * Author :                                                                 *
 *            Serge Lussier                                                 *
 * email: serge.lussier@videotron.ca                                        *
 * Started on January 15, 2001                                              *
 *--------------------------------------------------------------------------*
 * NOTE:                                                                    *
 * Before re-implement the ui file (with -impl option), make a safe copy of *
 * this file then re-put                                                    *
 * added stuff in this file to the new file.                                *
 ****************************************************************************/


#ifndef CCLASSPROPERTIESDLGIMPL_H
#define CCLASSPROPERTIESDLGIMPL_H
#include "wzconnectdlg.h"
#include <kcompletion.h>

//#include "./classparser/ClassStore.h"
//#include "./classparser/ClassParser.h"

class CClassParser;
class ClassStore;
class ParsedAttribute;
class ParsedArgument;
class ParsedClass;
class ParsedMethod;

/** For what action the dialog was called and current action : */
enum CTPACTION {CTPVIEW=0, CTPADDATTR, CTPADDMETH, CTPADDSIGNAL, CTPADDSLOT, CTPCONNECTSIG };
struct ClsHeader {
   QString ClsName;
   QString Filename;
};

extern ClsHeader QtKde[];

// Index of tabs
#define CTPCLASSVIEW    (int)0
#define CTPATTRIBUTE    (int)1
#define CTPMETHOD       (int)2
#define CTPSIGNAL       (int)3
#define CTPSLOT         (int)4
#define CTPIMPL         (int)5
//-------------------------------

#include <kconfig.h>

class CClassToolDlg;
class CClassView;
class CClassPropertiesDlgImpl : public CClassPropertiesDlg
{
    Q_OBJECT

public:
  CClassPropertiesDlgImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  CClassPropertiesDlgImpl( CTPACTION action, CClassToolDlg* ctdlg, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  CClassPropertiesDlgImpl( CClassView* cv, CTPACTION action, CClassToolDlg* ctdlg, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  ~CClassPropertiesDlgImpl();
  /**  */
  void setCurrentClassName ( const QString& );
  /** Fill'in data into the widgets depending on which TAB ' vtab ' is
       invoqued and the current action.*/
  void updateData( int vtab );
  /** Check if data onscreen is the same as its CTabData.
    @return true if one item is different.
 */
  bool isModified( int tabw );
  /**  */
  void setImplTabMethList ( QValueList<ParsedMethod*> MethList, bool bclear=true);
  /**  */
  void setSlotTabSlotList ( QValueList<ParsedMethod*> MethList , bool bmatchsig=false);
  /**  */
  void setSigTabAttrList ( QValueList<ParsedAttribute*> AttrList );
  /**  */
  void setClass ( ParsedClass* aClass );
  void setStore ( ClassStore* s) { store = s; }
  void init();
  /** This function tries guess if aName is a QT or a KDE class then set filename of the include file according to the classname.
 */
  ParsedClass* unParsedClass( const QString& );
  /**  */
  void viewParents();
  /**  */
  void viewChildren();
  /**  */
  void setClassToolDlg( CClassToolDlg* ct);
  /** This function will try to fill up the combo with
      availble Signals/Slots methods from the class itself and, if possible,
      from its parent classes. */
  bool fillSignalCombo(ParsedClass* aClass, bool bClear = true);

  /** This build a list of ParsedAttribute items from the current class and its parents classes */
  QValueList <ParsedAttribute*> getAllParentAttr(ParsedClass*, bool initList=true);

  /** static member to hold completion */
  static KCompletion* typeCompletion;
public slots:
    void slotClone();
    void slotBtnApply();
    void slotBtnUndo();
    void slotSigClassNameEditEnter();
  /**  */
  void slotClassViewChanged( ParsedClass* );

protected slots:
    void slotMethVirtualState(int);
    void slotSlotModifierChanged(int );
    void slotAddSlotState(int);
    void slotImplMethodSelected(const QString&);
    void slotMethAccessChanged(int);
    void slotMethModifierChanged(int);
    void slotMethNameChanged( const QString & );
    void slotMethTypeChanged( const QString & );
    void slotSigAccessChanged(int);
    void slotSigAddSignalState(int);
    void slotSigMemberSelected(const QString&);
    void slotSigNameChanged( const QString& );
    void slotSigSignalSelected(const QString&);
    void slotSlotAccessChanged(int);
    void slotSlotMemberSelected(const QString&);
    void slotSlotNameChanged( const QString& );
    void slotVarNameChanged( const QString& );
    void slotVarTypeChanged( const QString& );
    void slotTabChanged( QWidget* );
  /**  */
  //void slotSetClass( ParsedClass* );
    protected:
    virtual void resizeEvent( QResizeEvent* e) ;
  /**  */
  void getClassNameFromString( const  QString & aName, QString & newName);
  /**  */
  void setSignalsMemberList( ParsedClass*, bool bClear = true );
    class CTabData {
        public:
            QString editFields[2];
            QString combos[2];
            bool Access[4];
            bool Modifier[3];
            bool bModified;
            bool bApplied;
    };
public: // Public methods
    CTabData tbdata[6];
    CTPACTION ctpAction;
    /** Table of pointer to tabWidgets to use for iteration. internal use.*/
    QWidget* WidgetTable[6];
    ParsedClass *currentClass;
    /**  */
    ParsedArgument* Arg;
    /**  */
    ParsedMethod* slotMethod;
    /** current signal member */
    ParsedMethod* signalMethod;
    /**  */
    ParsedMethod* implMethod;
    ClassStore* store;

private: // Private attributes
  /**  */
  ParsedClass* classOfSig;
  KConfig* config;
  /**  */
  CClassParser* theParser;
  /** Holds the class store of the selected Working class variable member */
  ParsedClass* selectedSigAttrClass;
  /** used to build the connect instruction.
        If value is 0 ( NULL ) then the default class member [ this ]  is used*/
  ParsedAttribute* attrMember;
  /**  */
  QValueList <ParsedAttribute*> workClassAttrList;
  CClassToolDlg* CTDlg;
  /**  */
  bool bMemberIsPointer;
  void applyAddAttribute();
  void applyAddMethod();
  void applySignalSlotMapImplementation();
  /**  */
  void getMemberFromString ( const QString& str, QString& newName);
signals: // Signals
  /**  */
  void sigAddMethod( const char *, ParsedMethod*);
  void sigAddAttribute( const char*, ParsedAttribute*);
  void sigSigSlotMapImplement ( ParsedClass*, const QString&, ParsedMethod* );
public: // Public attributes
  /**  */
  CClassView* class_tree;
  /**  */
  static QString CppCodeExtra;
protected: // Protected attributes
  /**  */
  QString strSlotMethod;
  /**  */
  QString strSignalMethod;
  /**  */
  QStrList argList;
  /**  */
  QValueList <ParsedClass*> sigClassList;
  QString Member;
};

#endif // CCLASSPROPERTIESDLGIMPL_H
