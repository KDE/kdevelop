/***************************************************************
 * cclasspropertiesdlgimpl.h                                                                            *
 * Safe Implementation class from cclassproperties.ui                                          *
 *-------------------------------------------------------------------------------*
 * Author :                                                                                                  *
 *            Serge Lussier                                                                                 *
 * email: serge.lussier@videotron.ca                                                                 *
 * Started on January 15, 2001
 *-------------------------------------------------------------------------------*
 * NOTE:                                                                                                     *
 * Before re-implement the ui file (with -impl option), make a safe copy of this file then re-put             *
 * added stuff in this file to the new file.                                                           *
 ***************************************************************/

#include "wzconnectdlgimpl.h"
#include <qtabwidget.h>
#include <iostream>
#include <qlabel.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <kglobal.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qmultilineedit.h>
#include <qcheckbox.h>
#include "cclasstooldlg.h"
#include <kmessagebox.h>
#include "cclonefunctiondlg.h"
#include <kiconloader.h>
/* 
 *  Constructs a CClassPropertiesDlgImpl which is a child of 'CClassPropertiesDlg', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CClassPropertiesDlgImpl::CClassPropertiesDlgImpl( QWidget* parent,  const char* name, bool, WFlags fl )
    : CClassPropertiesDlg( parent, name, true, fl ), ctpAction ( CTPVIEW )
{
    init();
    classOfSig = 0;
    signalMethod = 0;
    slotMethod = 0;
    theParser = 0;
    implMethod  = 0;
    attrMember = 0;
    slotMethod = 0;
    signalMethod = 0;
}

CClassPropertiesDlgImpl::CClassPropertiesDlgImpl( CTPACTION action, CClassToolDlg* ctdlg, QWidget* parent, const char* name, bool, WFlags fl )
: CClassPropertiesDlg( parent, name, true, fl ), ctpAction(action), CTDlg(ctdlg)
{
    // ToDo: init stuff here...

    classOfSig = 0;
    signalMethod = 0;
    slotMethod = 0;
    theParser = 0;
    implMethod  = 0;
    attrMember = 0;
    slotMethod = 0;
    signalMethod = 0;
    init();

}

CClassPropertiesDlgImpl::CClassPropertiesDlgImpl( CClassView* cv, CTPACTION action, CClassToolDlg* ctdlg, QWidget* parent, const char* name, bool, WFlags fl )
: CClassPropertiesDlg( parent, name, true, fl ), ctpAction(action), CTDlg(ctdlg), class_tree(cv)
{
    // ToDo: init stuff here...

    classOfSig = 0;
    signalMethod = 0;
    slotMethod = 0;
    theParser = 0;
    implMethod  = 0;
    attrMember = 0;
    slotMethod = 0;
    signalMethod = 0;
    init();

}

void CClassPropertiesDlgImpl::resizeEvent( QResizeEvent* e)
{
    QDialog::resizeEvent(e);
    // For a reason I don't know yet, the CClassToolDlg::resizeEvent is not called,
    // even though it is a child widget ...
    //tpgClassView -> resize(tabWidget -> width(), tabWidget -> height());
    //cerr << "new tabWidget Size: W:" << tabWidget -> width() << "H:" << tabWidget -> height() << endl;
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CClassPropertiesDlgImpl::~CClassPropertiesDlgImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot
 */

void CClassPropertiesDlgImpl::applyAddAttribute()
{
  CParsedAttribute *aAttr = new CParsedAttribute();
  QString comment;

  aAttr->setType( leVarType_2 -> text() );
  aAttr->setName( leVarName_2 -> text() );
  aAttr->setDeclaredInScope( currentClass -> path() );
  // Set export
  if( rbVarPublic_2 -> isChecked() )
    aAttr->setExport( PIE_PUBLIC );
  else if( rbVarProtected_2 -> isChecked() )
    aAttr->setExport( PIE_PROTECTED );
  else if( rbVarPrivate_2 -> isChecked() )
    aAttr->setExport( PIE_PRIVATE );

  // Set modifiers
  aAttr->setIsStatic( chVarStatic_2 -> isChecked() );
  aAttr->setIsConst( chVarConst_2 -> isChecked() );

  // Set comment
  comment = "/** " + meVarDoc_2 -> text() + " */";
  aAttr->setComment( comment );

  // update rollback data
  tbdata[CTPATTRIBUTE].editFields[0] = leVarType_2 -> text();
  tbdata[CTPATTRIBUTE].editFields[1] = leVarName_2 -> text();
  tbdata[CTPATTRIBUTE].bModified = false;
  tbdata[CTPATTRIBUTE].bApplied = true;
  emit sigAddAttribute( currentClass -> name.data(), aAttr );
  buttonApply -> setEnabled ( false );
  buttonUndo -> setEnabled ( false );
  //currentClass -> addAttribute ( aAttr ); oops! already done in CKdevelop...
  // causes a crash if applied twice!

}

void CClassPropertiesDlgImpl::applyAddMethod()
{
    int lpPos;
    QString decl;
    CParsedMethod *aMethod = new CParsedMethod();
    QString comment;
    QLineEdit* eType=0, * eName=0;
    QRadioButton * rbPub=0, * rbPro=0, * rbPri=0;
    QCheckBox* chVirtual=0, * chPure=0, * chStatic=0, *chConst=0;
    QMultiLineEdit* meDoc;
    int pg = tabWidget -> currentPageIndex();
    cerr << "applyAddMethod: in page #" << pg << endl;
    switch ( pg )
    {
        case CTPSIGNAL:
            rbPub = rbSigPublic_2;
            rbPro = rbSigProtected_2;
            rbPri   = 0;
            chPure = chVirtual = chStatic = chConst = 0;
            meDoc = meSigDoc_2;
            eType = 0;
            eName = leSigDeclare_2;
            break;
        case CTPSLOT:
            rbPub = rbSlotPublic_2;
            rbPro = rbSlotProtected_3;
            rbPri  = rbSlotProtected_2_2;
            chVirtual = chSlotVirtual_2;
            chConst = chStatic = chPure = 0;
            meDoc = meSlotDoc_2;
            eType = 0;
            eName = leSlotDeclare_2;
            break;
        case CTPMETHOD:
            rbPub = rbMethPublic_2;
            rbPro = rbMethProtected_2;
            rbPri = rbMethPrivate_2;
            chVirtual = chMethVirtual_2;
            chPure = chMethPure_2;
            chStatic = chMethStatic_2;
            chConst = chMethConst_2;
            meDoc = meMethDoc_2;
            eType = leMethType_2;
            eName = leMethDeclare_2;
            break;
        default:
            rbPub = rbPro = rbPri = 0;
            meDoc = 0;
            chVirtual = chPure = chStatic = chConst = 0;
            break;
    }
    aMethod->setType( eType ? eType -> text() : QString("void") );
    aMethod->setDeclaredInScope( currentClass -> path() );
    decl = eName -> text();
    lpPos = decl.find( "(" );
    // If no arguments we add ().
    if( lpPos == -1 )
        aMethod->setName( decl + "()" );
    else // Else just set the whole thing as the name
        aMethod->setName( decl );
    // Set the type.

    if ( pg == CTPSIGNAL )
        aMethod -> setIsSignal( true );
    else if ( pg == CTPSLOT )
        aMethod -> setIsSlot ( true );
    // Set the export.

  if( rbPub && rbPub -> isChecked() )
    aMethod->setExport( PIE_PUBLIC );
  else if( rbPro && rbPro -> isChecked() )
    aMethod->setExport( PIE_PROTECTED );
  else if( rbPri && rbPri -> isChecked() )
    aMethod->setExport( PIE_PRIVATE );

  // Set the modifiers if they are enabled.
  if( chPure && chPure -> isEnabled() )
    aMethod->setIsPure( chPure -> isChecked() );
  if( chStatic && chStatic -> isEnabled() )
    aMethod->setIsStatic( chStatic -> isChecked() );
  if( chConst && chConst -> isEnabled() )
    aMethod->setIsConst( chConst -> isChecked() );
  if( chVirtual && chVirtual -> isEnabled())
    aMethod->setIsVirtual( chVirtual -> isChecked() );

  // Set comment
  if ( meDoc && (meDoc -> text() != "") ) comment = "/** " + meDoc -> text() + " */";
  else comment = QString("/** ") + /*i18n(*/QString("No descriptions")/*)*/ + " */";
  aMethod->setComment( comment );
  if (! currentClass )
  {
    cerr << "ERROR currentClass != NULL!!!!!!!" << endl;
    return;
  }
  emit sigAddMethod( currentClass -> name.data(), aMethod);


  //cerr << "Definition of method ends on line: " <<aMethod -> definitionEndsOnLine << endl;
  //delete aMethod;
  tbdata[pg].editFields[0] = eType ? eType -> text() : QString("");
  tbdata[pg].editFields[1] = eName -> text();
  tbdata[pg].bModified = false;
  tbdata[pg].bApplied = true;
  buttonApply -> setEnabled ( false );
  buttonUndo -> setEnabled ( false );

    if ( pg == CTPMETHOD)
    {
        //QList <CParsedMethod> * m = currentClass -> getSortedMethodList();
        //cerr << "class " << currentClass -> name.data() << " : updating methods list..." << endl;
        currentClass -> addMethod( aMethod );
        cbImplMethodList_2 -> insertItem( aMethod -> name) ;
    }
    else
    if ( pg == CTPSLOT )
    {
        //QList <CParsedMethod> * m = currentClass -> getSortedSlotList();
        cerr << "adding new slot to currentClass..." << endl;
        currentClass -> addSlot( aMethod );
        cbSlotMemberList_2 -> insertItem( aMethod -> name );
    }
    if ( pg == CTPSIGNAL )
    {
        cerr << "adding new method to currentClass ..." << endl;
        currentClass -> addSignal ( aMethod );
    }
    // don't touch to signals list...

}


void CClassPropertiesDlgImpl::applySignalSlotMapImplementation()
{
    QString strSigMethod= "";
    QString strSlotMethod = "";
    QString strImplMethod = "";
    QString strAttrMember="";
    QString toAdd;
    cerr  << "applying signal-clot connection ..." << endl;
    if ( !signalMethod || !slotMethod || !implMethod )
    {
        cerr << "Error: caugth NULL POINTER! can't be! uh ?" << endl;
        return;
    }

    signalMethod -> asString ( strSigMethod );
    cerr << "signalMethod: " << strSigMethod.data() << endl;
    slotMethod -> asString ( strSlotMethod );
    cerr << "slotMethod: " << strSlotMethod.data() << endl;
    implMethod -> asString ( strImplMethod );
    cerr << "implMethod: " << strImplMethod.data() << endl;

    if ( attrMember ) strAttrMember = attrMember -> name;
    else strAttrMember = "this";

    toAdd = "connect( " + strAttrMember + ", ";
    toAdd = toAdd + "SIGNAL( " + strSigMethod + " ), ";
    toAdd = toAdd + "SLOT( " + strSlotMethod + " ) );";

    cerr << " toAdd = " << toAdd.data() << endl;

    QString Message;

    if( implMethod -> definitionEndsOnLine < 1 )
    {
        Message = i18n("Unable to locate definition of method") + "\n" +
                       implMethod -> name + "\n" +
                       i18n("Sorry...");
        KMessageBox::error(this, Message, "Implementation method error", true);
        return;
    }
    Message = i18n("You are about to commit connection with this settings:") + "\n" +
                   i18n("Implement in class: ") + currentClass -> name + "\n" +
                   i18n("Attribute member: ") + (attrMember ? attrMember -> name : QString("this")) + "\n" +
                   i18n("Class of attribute member: ") + classOfSig -> name + "\n" +
                   i18n("Signal name: ") + signalMethod -> name + "\n" +
                   i18n("Slot method: ") + slotMethod -> name + "\n" +
                   i18n("Into method: ") + implMethod -> name + " "  + i18n("as") + "\n" +
                   toAdd+ "\n" + i18n("Do you want to continue ?");

    int r = KMessageBox::warningContinueCancel (this, Message, QString( "Signal-Slot " + i18n("connection:")) , i18n("&Continue"));// i18n("&Continue"), i18n("&Cancel"), true);
    if ( r != KMessageBox::Continue ) return;

    emit sigSigSlotMapImplement ( currentClass, toAdd, implMethod );

    buttonApply -> setEnabled ( false );
    buttonUndo -> setEnabled ( false );
    accept();

}



void CClassPropertiesDlgImpl::slotBtnApply()
{
    int pg = tabWidget -> currentPageIndex();
    switch ( pg )
    {
        case CTPATTRIBUTE:
            cerr << "Applying new Attribute..." << endl;
            applyAddAttribute();
            accept();
            return;
        case CTPMETHOD:
        case CTPSIGNAL:
        case CTPSLOT:
            cerr << "Applying new method..." << endl;
            applyAddMethod();
            accept();
            return;
        case CTPIMPL:
            applySignalSlotMapImplementation();
            accept();
            break;
        default:
            buttonApply -> setEnabled( false );
            buttonUndo -> setEnabled ( false );
            break;
    }
}

/* 
 * public slot
 */
void CClassPropertiesDlgImpl::slotBtnUndo()
{
    qWarning( "CClassPropertiesDlgImpl::slotBtnUndo() not yet implemented!" ); 
}

/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSlotModifierChanged(int )
{
    qWarning( "CClassPropertiesDlgImpl::slotSlotModifierChanged(int ) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotAddSlotState(int)
{
    if (chSlotAdd -> isChecked())
    {
        gbAddNewSlot -> setEnabled(true);
        gbSlotMembers -> setEnabled(false);
        QString strDeclare = "new_slot(";
        QString strArgs = "";
        if ( signalMethod )
        {
            QList <CParsedArgument>  argList = signalMethod -> arguments;
            cerr << "build args from signal " << signalMethod -> name.data() << endl;
            CParsedArgument * arg;
            if ( argList. count() > 0 )
            {
                for ( arg = argList.first(); arg; arg = argList.next())
                {
                    if ( strArgs != "" ) strArgs = strArgs + ", ";
                    strArgs = strArgs + arg -> type;
                }
                cerr << "args are " << strArgs.data() << endl;
            }
        }
        strDeclare = strDeclare + strArgs + ")";
        leSlotDeclare_2 -> setText ( strDeclare );
    }
    else
    {
        gbAddNewSlot -> setEnabled(false);
        gbSlotMembers -> setEnabled(true);
        buttonUndo -> setEnabled (false);
        buttonApply -> setEnabled(false);
    }
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotImplMethodSelected(const QString& strMethod)
{
    bool btrue;
    QList < CParsedMethod> * l =  currentClass -> getSortedMethodList();
    QString am;
    cerr << "in slotImplMethodSelected:" << endl;
    cerr << "getting instance of " << strMethod.data() << endl;
    for ( implMethod = l -> first() ; implMethod; implMethod = l -> next())
    {
        implMethod -> asString ( am );
        cerr << "mathing " << am.data() << endl;
        if ( am == strMethod ) break;
    }
    //implMethod = currentClass -> getMethodByNameAndArg( strMethod.data() );
    if ( signalMethod && slotMethod && implMethod ) btrue = true;
    else btrue = false;
    buttonApply -> setEnabled(btrue);
    buttonUndo -> setEnabled(btrue);

    if ( signalMethod == NULL ) cerr << " signal method is NULL! " << endl;
    if ( slotMethod == NULL ) cerr << " slot method is NULL! " << endl;
    if ( implMethod == NULL ) cerr << " impl. methodis NULL! " << endl;
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotMethAccessChanged(int)
{
    qWarning( "CClassPropertiesDlgImpl::slotMethAccessChanged(int) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotMethModifierChanged(int)
{
    qWarning( "CClassPropertiesDlgImpl::slotMethModifierChanged(int) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotMethNameChanged( const QString & )
{
    if( tbdata[CTPMETHOD].bModified == true) return;
    tbdata[CTPMETHOD].bModified = true;
    tbdata[CTPMETHOD].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotMethTypeChanged( const QString & )
{
    if( tbdata[CTPMETHOD].bModified == true) return;
    tbdata[CTPMETHOD].bModified = true;
    tbdata[CTPMETHOD].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSigAccessChanged(int)
{
    qWarning( "CClassPropertiesDlgImpl::slotSigAccessChanged(int) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSigAddSignalState(int)
{
    if (chSigAdd_2 -> isChecked())
    {
        gbAddNewSig_2 -> setEnabled(true);
        GroupBox9_2 -> setEnabled(false);
        if ( leSigDeclare_2 -> text() == "" )
            leSigDeclare_2 -> setText ( "new_signal()" );
    }
    else
    {
        gbAddNewSig_2 -> setEnabled(false);
        GroupBox9_2 -> setEnabled(true);
        buttonUndo -> setEnabled (false);
        buttonApply -> setEnabled(false);
    }


}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSigMemberSelected(const QString& aMember)
{
    QString aName;
    QString a;
    getClassNameFromString( aMember, aName );
    getMemberFromString( aMember, a);

    CParsedClass * aClass;
    cbSigSignalList_2 -> clear();
    if ( theParser != NULL )
    {// Clean up old parser....
        delete theParser;
        theParser = NULL;
    }
    //cerr <<  "class to get signal list '" << aName.data() <<"'" << "curerntClass:'"<< currentClass -> name << "'" << (int) (aName == currentClass -> name) <<  endl;
    leSigClassName -> setText ( aName );
    if (aName == currentClass -> name)
    {
        classOfSig = currentClass;
        setSignalsMemberList( currentClass );
        selectedSigAttrClass = currentClass;
        return;
    }

    aClass = store -> getClassByName ( aName );
    if ( aClass == NULL ){
        aClass = unParsedClass( aName );
        selectedSigAttrClass = aClass;
        if( aClass == NULL)
        {
            cerr << " CClassPropertiesDlgImpl::slotSigMemberSelected: failed to parse..." << endl;
            return;
        }
    }
    classOfSig = aClass;

    setSignalsMemberList ( aClass );
//    qWarning( "CClassPropertiesDlgImpl::slotSigMemberSelected(const QString&) not completely implemented!" );
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSigNameChanged( const QString& )
{
    if( tbdata[CTPSIGNAL].bModified == true) return;
    tbdata[CTPSIGNAL].bModified = true;
    tbdata[CTPSIGNAL].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSigSignalSelected(const QString& aName)
{
    CParsedMethod* meth;
    QList <CParsedMethod>* methList;
    if (classOfSig == 0) return;
    meth  = classOfSig -> getSignalByNameAndArg( aName.data() );
    signalMethod = meth;
    if ( meth == 0 ) return;
    methList = currentClass -> getSortedSlotList();
    setSlotTabSlotList ( methList, true);
    cerr << "CClassPropertiesDlgImpl::slotSigSignalSelected() - signal '" << signalMethod -> name << "' selected.." << endl;
    delete methList;
    // Ok now that since we can give a arbitrary class name which could have signal member different from
    // our working class, we need to recheck if our member attribute does have the same signal member than
    // the one from the edited class name.


    //qWarning( "CClassPropertiesDlgImpl::slotSigSignalSelected(const QString&) not yet implemented!" );
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSlotAccessChanged(int)
{
    if( tbdata[CTPSLOT].bModified || tbdata[CTPSLOT].bApplied ) return;

    qWarning( "CClassPropertiesDlgImpl::slotSlotAccessChanged(int) not yet implemented!" );
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSlotMemberSelected(const QString&)
{
    slotMethod = currentClass -> getSlotByNameAndArg ( cbSlotMemberList_2 -> currentText().data() );
    cerr << "slot member " << (slotMethod ? slotMethod -> name.data() : "oops - NULL") << "selected..." << endl;
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSlotNameChanged( const QString& )
{
    if( tbdata[CTPSLOT].bModified == true) return;
    tbdata[CTPSLOT].bModified = true;
    tbdata[CTPSLOT].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotVarNameChanged( const QString& )
{
    if( tbdata[CTPATTRIBUTE].bModified == true) return;
    tbdata[CTPATTRIBUTE].bModified = true;
    tbdata[CTPATTRIBUTE].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}
/* 
 * protected slot
 */
void CClassPropertiesDlgImpl::slotVarTypeChanged( const QString& )
{
    if( tbdata[CTPATTRIBUTE].bModified == true) return;
    tbdata[CTPATTRIBUTE].bModified = true;
    tbdata[CTPATTRIBUTE].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}


/**  */
void CClassPropertiesDlgImpl::setCurrentClassName ( const QString& aClassName )
{
    lbWorkingClass -> setText (QString (i18n("Working on class: ")) + aClassName);
}
/**  */
void CClassPropertiesDlgImpl::slotClassViewChanged( CParsedClass* aClass)
{
    setClass ( aClass );
}

void CClassPropertiesDlgImpl::slotTabChanged( QWidget* )
{// don;t need QWidget* arg

    int i;
    bool btrue;
    i = tabWidget -> currentPageIndex();
    switch ( i )
    {
        case 0: // ClassView
            buttonUndo -> setEnabled(false);
            buttonApply -> setEnabled(false);
            break;
        case 1: // Attributes
        case 2: // Methods
        case 3: // Signals
        case 4: // Slots
            btrue = tbdata[i].bModified;
            buttonApply -> setEnabled(btrue);
            buttonUndo -> setEnabled(btrue);
            break;
        case 5:
            if ( signalMethod && slotMethod && implMethod ) btrue = true;
            else btrue = false;
            buttonApply -> setEnabled(btrue);
            buttonUndo -> setEnabled(btrue);
    }
}
/** Check if data onscreen is the same as its rollback data.
@return true if diff.
 */
bool CClassPropertiesDlgImpl::isModified( int  )
{
    return false;
}
/** Fill'in data into the widgets depending on which TAB ' vtab ' is invoqued */
void CClassPropertiesDlgImpl::updateData( int  )
{

}
/**  */
void CClassPropertiesDlgImpl::init()
{
    int x;
    // Do I need this ?
    WidgetTable [0] = tpgClassView;
    WidgetTable [1] = tpgVars;
    WidgetTable [2] = tpgMeth;
    WidgetTable [3] = tpgSignals;
    WidgetTable [4] = tpgSlots;
    WidgetTable [5] = tpgImpl;

    tabWidget -> changeTab (tpgClassView, SmallIcon("CVclass"), "Class Viewer");
    tabWidget -> changeTab (tpgVars, SmallIcon("CVglobal_var"), "Attributes");
    tabWidget -> changeTab (tpgMeth, SmallIcon("CVpublic_meth"), "Methods");
    tabWidget -> changeTab (tpgSignals, SmallIcon("CVpublic_signal"), "Signals");
    tabWidget -> changeTab (tpgSlots, SmallIcon("CVpublic_slot"), "Slots");
    tabWidget -> changeTab (tpgImpl, SmallIcon("CVstruct"), "Connection Implementation");


    // -------------------

    for ( x = 0; x < 6; x++ )
    {
        tbdata[x].editFields[0] = "";
        tbdata[x].editFields[1] = "";
        tbdata[x].combos[0] = "";
        tbdata[x].combos[1] = "";
        tbdata[x].bModified = false;
        tbdata[x].bApplied = false;
    }
    switch ( ctpAction )
    {
        case CTPVIEW:
            break;
        case CTPADDATTR:
            tabWidget -> setCurrentPage( CTPATTRIBUTE );
            break;
        case CTPADDMETH:
            tabWidget -> setCurrentPage( CTPMETHOD );
            break;
        case CTPADDSIGNAL:
            tabWidget -> setCurrentPage( CTPSIGNAL );
            break;
        case CTPADDSLOT:
            tabWidget -> setCurrentPage( CTPSLOT );
            break;
        default:
            break;
    }
    setMinimumSize( 540, 500 );
}
/**  */
void CClassPropertiesDlgImpl::setClass ( CParsedClass* aClass )
{
    currentClass = aClass;
    setCurrentClassName( aClass -> name );
    // Propagate change to the rest of the dialog data
    // Get attributes list
    QList<CParsedAttribute> *AttrList = aClass->getSortedAttributeList();
    // update Signals Tab data:
    setSigTabAttrList ( AttrList );
    // Get slot list:
    QList<CParsedMethod> *MethList = aClass ->getSortedSlotList();
    // update Slots Tab data:
    setSlotTabSlotList ( MethList );
    // Hide slot methods for now ....
    //setImplTabMethList ( MethList );
    // Get Methods:
    MethList = aClass -> getSortedMethodList();
    // update Connect Implementation Tab data:
    setImplTabMethList ( MethList, false );
    delete MethList;
    delete AttrList;
}
/**  */
void CClassPropertiesDlgImpl::setSigTabAttrList ( QList <CParsedAttribute>* AttrList )
{
    CParsedAttribute * Attr;
    int apos,pos;
    QString strType, strItem;
    if (AttrList == NULL ) return;
    cbSigMemberList_2 -> clear();
    strItem = currentClass -> name + "* this";
    cbSigMemberList_2 -> insertItem ( strItem );
    if ( AttrList -> count() == 0 )
    {
        cerr << "no attributes in class "<< currentClass -> name.data () << endl;
        return;
    }
    for ( Attr = AttrList -> first() ; Attr; Attr = AttrList -> next())
    {
        strType = Attr -> type;
        //cerr << "type of " << Attr -> name .data() << " is " << strType.data() << endl;
        apos = strType.find('*'); // remove pointer modifier if any
        if(apos != -1) strType.remove(apos,1);
        // remove all spaces from type name!
        // remove modifiers 'const', 'static' from name: we don;t need it...
        if( (pos = strType.find("const")) != -1) strType.remove(pos, 5);
        if( (pos = strType.find("static")) != -1) strType.remove(pos, 6);
        if( (pos = strType.find("[]")) != -1) strType.remove(pos, 2);
        while ( (pos = strType.find(" ")) != -1) strType.remove(pos,1);
        pos = -1;
        if ( !store -> hasClass( strType.data() ) )
        {
            //cerr << strType.data () << " not parsed ..." << endl;
            // here I am screwed for the moment...
            if ( (pos = strType.find('Q')) == 0)
            {
                // Hope this is a QT class....
                //if(apos != -1 ) strType = strType + "*";
                // pix = SmallIcon("CV_QT");
            }
            else
            if ( (pos = strType.find('K')) == 0)
            {
                // Hope this is a KDE class....
                //if(apos != -1 ) strType = strType + "*";
                // pix = SmallIcon("CV_KDE");
            }
            else continue;
        }
        if(apos != -1) strType = strType + "*";
        strItem = strType + " " + Attr -> name;
        cbSigMemberList_2 -> insertItem ( strItem );
    }
     leSigClassName -> setText( currentClass -> name );
}
/** Fill in current class's slots member in ComboBox :*/
void CClassPropertiesDlgImpl::setSlotTabSlotList ( QList<CParsedMethod> * list, bool bmatchsig)
{
    CParsedMethod* meth;
    cbSlotMemberList_2 -> clear();
    QString str;
    QList <CParsedArgument> sigArgs;
    QList <CParsedArgument> slotArgs;
    CParsedArgument *sigArgIt;
    CParsedArgument *slotArgIt;
    bool match = false;
    if(!list) return;
    if ( list -> isEmpty())
    {
        cerr << "no slot members ...." << endl;
        //signalMethod = NULL;
        slotMethod = 0;
        return;
    }
    if ( signalMethod == NULL && bmatchsig )
    {
        cerr << "can't match signal arguments : signal is null" << endl;
        lbSlotClassOfSig -> setText ( i18n("no matching slots") );
        slotMethod  =0;
        cerr << "slotMethod set to NULL " << endl;

        return;
    }
    if( bmatchsig && signalMethod != NULL ) sigArgs = signalMethod -> arguments;
    for (meth = list -> first() ; meth; meth = list -> next())
    {
        if( bmatchsig )
        {
            match = false;
            slotArgs = meth -> arguments;
            //cerr << meth -> name.data()  << "numargs: " << slotArgs . count() << endl;
            if( sigArgs . count() ==0 && slotArgs . count() ==0 ) goto ARGMATH;
            if ( sigArgs . count() != slotArgs . count()) continue;
            for ( sigArgIt = sigArgs . first(),
                   slotArgIt = slotArgs . first();
                   sigArgIt;
                   sigArgIt = sigArgs . next(),
                   slotArgIt = slotArgs . next())
            {
                //cerr << "mathing " << sigArgIt -> type << " with " << slotArgIt -> type << endl;
                if ( sigArgIt -> type == slotArgIt -> type)
                {
                    match = true;
                    break;
                }
            }
            if (!match) continue;
        }
        ARGMATH:
        meth -> asString( str );
        cbSlotMemberList_2 -> insertItem ( str );
    }
    if( !cbSlotMemberList_2 -> count() )
    {
        cerr << "No matching slot members found..." << endl;
        lbSlotClassOfSig -> setText (i18n("no matching slots"));
        slotMethod = 0;
        cerr << "slotMethod set to NULL " << endl;
        return;
    }
    else lbSlotClassOfSig -> setText ( classOfSig ? classOfSig -> name + "'s signals": QString(i18n("no signal selected")) );
    cerr << "getting slot method instacne..." << endl;
    slotMethod = currentClass -> getSlotByNameAndArg( cbSlotMemberList_2 -> currentText().data() );
    cerr << "slotMethod set to '" << slotMethod -> name.data() << endl;
}
/**  */
void CClassPropertiesDlgImpl::setImplTabMethList ( QList<CParsedMethod>* mlist, bool bclear)
{
    CParsedMethod* meth;
    if(bclear) cbImplMethodList_2 -> clear();
    QString strMeth;
    for ( meth = mlist -> first(); meth; meth = mlist -> next())
    {
        meth -> asString( strMeth );
        //cerr << "method " << strMeth.data() << " ends on line#" << meth -> definitionEndsOnLine << endl;
        cbImplMethodList_2 -> insertItem ( strMeth );
    }
    implMethod = currentClass -> getMethodByNameAndArg ( cbImplMethodList_2 -> currentText().data() );
    cerr << "implMethod set to " << (implMethod ? implMethod -> name.data() : "NULL") << endl;
}
/** EditEnter:
      This method tries to get the header filename from the line edit text
      then parse the file. If the file is parsed successfully, then
      it gets its signal member methods if any, then build
      the signals comboxbox with that list.
      The signal method selected from that list will be taken as
      if this is a member of the selected Working classe's attribute
      member.

      This erratic behaviour is set until KDE/QT classes will be actually part of
      store data.
*/
void CClassPropertiesDlgImpl::slotSigClassNameEditEnter()
{
    QString aClassName = leSigClassName -> text();
    CParsedClass* fixClass = unParsedClass( aClassName );
/*
    CParsedClass* guestClass;
    CParsedClass* parentClass;
*/
    if (! fixClass )
    { // abort...
        cerr << " no class or can't get header file of class " << aClassName.data() << endl;
        delete theParser;
        theParser = NULL;
        return;
    }
    /*
    guestClass = theParser -> store . getClassByName (  classOfSig -> name );
    if ( ! guestClass )
    {
        cerr << " No class named '" << classOfSig -> name.data() << "' in parsed file!" << endl;
        cerr << "trying to get parent class of  " << classOfSif -> name.data() << endl;
        if ( fixClass -> hasParent ( aClassName ) )
        {
            parentClass
        delete theParser;
        theParser = NULL;
        return;
    }
    */
    classOfSig = fixClass;
    leSigClassName -> setText ( classOfSig -> name );
    setSignalsMemberList( classOfSig );
}
/**  */
void CClassPropertiesDlgImpl::getClassNameFromString( const QString & aName, QString& newName)
{
    int pos;
    if( aName.find(" ") != -1) newName = aName.left( aName.find(" ") + 1);
    else
    {
        newName = aName;
        return;
    }
    if ( (pos = newName.find("*")) != -1) newName.remove(pos,1);
    while ( (pos = newName.find(" ")) != -1) newName.remove(pos, 1);
    if ((pos = newName.find("[]")) != -1) newName.remove(pos, 2);
}
/**  */
void CClassPropertiesDlgImpl::setSignalsMemberList( CParsedClass* aClass)
{
    QList<CParsedMethod> *siglist = aClass -> getSortedSignalList();
    QString item;
    cbSigSignalList_2 -> clear();
    if ( (siglist == (QList<CParsedMethod> *)0) || ( siglist -> isEmpty()) )
    {
        cerr << "CClassPropertiesDlgImpl::setSignalsMemberList() : class " << aClass -> name.data() << " has no signal members." << endl;
        // ToDo: KMessageBox::warning(...)...
        signalMethod = 0;
        cerr << "signalMethod set to NULL" << endl;
        return;
    }
    CParsedMethod* sig;
    for ( sig = siglist -> first() ; sig; sig = siglist -> next())
    {
        sig -> asString( item );
        cbSigSignalList_2 -> insertItem( item );
    }
    signalMethod = aClass -> getSignalByNameAndArg ( cbSigSignalList_2 -> currentText().data() );
    cerr << " signalMethod set to '" << signalMethod -> name.data() << "'" << endl;
}
/** This function tries guess if aName is a QT or a KDE class then set filename
      of the include file according to the classname.
 */
CParsedClass* CClassPropertiesDlgImpl::unParsedClass( const QString& aName)
{
    CClassParser *parser =  new CClassParser;
    config = KGlobal::config();
    config -> setGroup("QT2");
    CParsedClass* theClass=0l;
    QString qt2dir = config -> readEntry("qt2dir");
    QString kde2dir  = config -> readEntry("kde2dir");
    QString filePath;
    if ( aName.find("Q") == 0 )
        filePath = qt2dir + "/include/" + aName.lower() + ".h";
    else
    if ( aName.find("K") == 0 )
        filePath = kde2dir + "/include/" + aName.lower() + ".h";

    cerr << "about to parse file " << filePath.data() << endl;
    parser -> parse( filePath.data() );
    if (! parser -> store.hasClass( aName ))
    {
        cerr << "Parse failed for " << aName.data() << endl;
        delete parser;
        return NULL;
    }
    // Woohoo! we've got it...
    theClass = parser -> store.getClassByName( aName );
    theParser = parser;
    return theClass;
}

void CClassPropertiesDlgImpl::slotMethVirtualState(int)
{
    if ( chMethVirtual_2 -> isChecked() )
    {
        chMethPure_2 -> setEnabled ( true );
        return;
    }
    chMethPure_2 -> setChecked( false );
    chMethPure_2 -> setEnabled( false );
}

/**  */
void CClassPropertiesDlgImpl::getMemberFromString ( const QString& str, QString& newName)
{
    int spos;
    CParsedAttribute* aAttr;
    QList < CParsedAttribute>  *l;
    l = currentClass -> getSortedAttributeList();
    spos = str.length() - str.find(" ");
    //if( spos == -1) spos = 0;
    newName = str.right(spos-1);
    cerr << newName.data() << endl;
    for ( aAttr = l -> first(); aAttr; aAttr = l  -> next())
    {
        if (aAttr -> name == newName)
        {
            attrMember = aAttr;
            cerr << "Atttribute member match: " << attrMember -> name.data() << endl;
            return;
        }
    }
    attrMember = NULL;
}
/**  */
void CClassPropertiesDlgImpl::viewChildren()
{
    CTDlg -> viewChildren();
}
/**  */
void CClassPropertiesDlgImpl::viewParents()
{
    CTDlg -> viewParents();
}
/**  */
void CClassPropertiesDlgImpl::setClassToolDlg( CClassToolDlg* ct)
{
    CTDlg = ct;
}
void CClassPropertiesDlgImpl::slotClone()
{
CCloneFunctionDlg volDlg(class_tree, this, "volnameDlg");

  if (volDlg.exec()) {
    CParsedMethod* res = volDlg.getMethod();
    if (! res)
    	return;
   	
   // copy type and declaration
   QString str;
   leMethType_2 -> setText(res->type);;
   leMethDeclare_2 -> setText(res->asString(str));
      // the comment needs some adjustment
   str = res->comment;
   // remove /** and */
   str.replace( QRegExp("^/\\** *"), "" );
   str.replace( QRegExp(" *\\**/$"), "" );
   // clean up line breaks
   str.replace( QRegExp("\n *\\** *"), "\n" );
	meMethDoc_2 -> setText(str);

    	// all the buttons
   	rbMethPrivate_2  -> setChecked( res->isPrivate() );
    	rbMethProtected_2 -> setChecked( res->isProtected() );
    	rbMethPublic_2 -> setChecked( res->isPublic() );
     	
   	//methodRb.setChecked( true ); // ??
    	//slotRb.setChecked( res->isSlot );
    	//signalRb.setChecked( res->isSignal );

    	chMethConst_2 -> setChecked( res->isConst);
    	chMethStatic_2 -> setChecked( res->isStatic );
    	chMethPure_2 -> setChecked( res->isPure );
    	chMethVirtual_2 -> setChecked( res->isVirtual );
  }
}	


//#include "wzconnectdlgimpl.moc"

