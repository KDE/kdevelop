/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2001 by Richard Dale                                    *
 *   Richard_Dale@tipitina.demon.co.uk                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import org.kde.qt.*;
import org.kde.koala.*;
import java.util.ArrayList;
import java.util.Iterator;

public class JavaSupportPart extends KDevLanguageSupport
{

	private JavaClassParser m_parser;

	public JavaSupportPart(KDevApi api, QObject parent, String name)
	{
		super(api, parent, name);
		setInstance(JavaSupportFactory.instance());

	    connect( (QObject) core(), SIGNAL("projectOpened()"), this, SLOT("projectOpened()") );
	    connect( (QObject) core(), SIGNAL("projectClosed()"), this, SLOT("projectClosed()") );
	    connect( (QObject) core(), SIGNAL("savedFile(String)"),
	             this, SLOT("savedFile(String)") );
	    connect( (QObject) core(), SIGNAL("contextMenu(QPopupMenu, org.kde.koala.Context)"),
	             this, SLOT("contextMenu(QPopupMenu, org.kde.koala.Context)") );

		m_parser = null;
		setFeatures(Classes | AddMethod | AddAttribute);
		setFileFilters( new String[] { "*.java" } );
	    connect( this, SIGNAL("updatedJavaSourceInfo()"), this, SLOT("updatedJavaSourceInfo()") );
	}

	public void projectOpened()
	{
//	    kdDebug(9007) << "JavaSupportPart::projectSpaceOpened()" << endl;

	    connect( project(), SIGNAL("addedFileToProject(String)"),
	             this, SLOT("addedFileToProject(String)") );
	    connect( project(), SIGNAL("removedFileFromProject(String)"),
	             this, SLOT("removedFileFromProject(String)") );

	    // We want to parse only after all components have been
	    // properly initialized
	    m_parser = new JavaClassParser(classStore());
	    QTimer.singleShot(0, this, SLOT("initialParse()"));
	}

	public void projectClosed()
	{
	    m_parser = null;
	}

	public void contextMenu(QPopupMenu popup, Context context)
	{
	    if (context.hasType("editor")) {
//	        String str = context.linestr();
	    }
	}

	public void maybeParse(String fileName)
	{

		QFileInfo fi = new QFileInfo(fileName);
	    String path = fi.filePath();
	    String ext = fi.extension();
	    if (ext.equals("java")) {
	        m_parser.parse(fileName);
	    }
	}

	public void initialParse()
	{
//	    kdDebug(9013) << "JavaSupportPart::initialParse()" << endl;
		System.out.println("JavaSupportPart.initialParse()");

	    if (project() != null) {
	        KApplication.kApplication().setOverrideCursor(KCursor.waitCursor());
        	ArrayList files = project().allSourceFiles();
			Iterator it = files.iterator();

			while (it.hasNext()) {
				maybeParse((String) it.next());
    		}

        	emit("updatedJavaSourceInfo");
        	KApplication.kApplication().restoreOverrideCursor();
    	} else {
//        	kdDebug(9013) << "No project" << endl;
    	}
	}

	public void addedFileToProject(String fileName)
	{
//	    kdDebug(9013) << "JavaSupportPart::addedFileToProject()" << endl;
	    maybeParse(fileName);
	    emit("updatedJavaSourceInfo");
	}

	public void removedFileFromProject(String fileName)
	{
//	    kdDebug(9013) << "JavaSupportPart::removedFileFromProject()" << endl;
	    m_parser.removeWithReferences(fileName);
	    emit("updatedJavaSourceInfo");
	}


	public void savedFile(String fileName)
	{
//	    kdDebug(9013) << "JavaSupportPart::savedFile()" << endl;

	    if (project().allSourceFiles().contains(fileName)) {
	        maybeParse(fileName);
	        emit("updatedJavaSourceInfo");
	    }
	}

	public void newClass()
	{
	}

public void addMethod(String className)
{
    JavaAddClassMethodDialog dlg = new JavaAddClassMethodDialog(null, "methodDlg");
    if (dlg.exec() == QDialog.Rejected)
        return;

    ParsedMethod pm = dlg.asSystemObj();
    pm.setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass pc = classStore().getClassByName(className);

	Iterator methodIterator = pc.methodList().iterator();

	while (methodIterator.hasNext()) {
    	ParsedMethod meth = (ParsedMethod) methodIterator.next();
     	if (meth.access() == pm.access() &&
      		atLine < meth.declarationEndsOnLine())
                atLine = meth.declarationEndsOnLine();
	}

    String javaCode = asJavaCode(pm);

    core().gotoSourceFile(new KURL(pc.definedInFile()), atLine);
    System.out.println("####################" + "Adding at line " + atLine
                  + " " + javaCode
                  + "####################");

    pm = null;

}


public void addAttribute(String className)
{
    JavaAddClassAttributeDialog dlg = new JavaAddClassAttributeDialog(null, "attrDlg");
    if( dlg.exec() == QDialog.Rejected )
      return;

    ParsedAttribute pa = dlg.asSystemObj();
    pa.setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass pc = classStore().getClassByName(className);

	Iterator attributeIterator = pc.attributeList().iterator();
	while (attributeIterator.hasNext()) {
	   	ParsedAttribute attr = (ParsedAttribute) attributeIterator.next();
        if (attr.access() == pa.access() &&
            atLine < attr.declarationEndsOnLine())
            atLine = attr.declarationEndsOnLine();
	}

    core().gotoSourceFile(new KURL(pc.declaredInFile()), atLine);

    pa = null;
}


public String asJavaCode(ParsedMethod pm)
{
    if (pm.isPure() || pm.isSignal())
        return "";

    String str = pm.comment();
    str += "\n";

    // Take the path and replace all . with ::
    String path = pm.path().replace( ':', '.' );

    str += pm.type();
    str += " ";
    str += pm.path();

    if (pm.isConst())
        str += " final";

    str += "{\n}\n";

    return str;
}


public String asJavaCode(ParsedAttribute pa)
{
    String str = "  ";
    str += pa.comment();
    str += "\n  ";

    if (pa.isStatic())
        str += "static ";

    str += pa.asString();
    str += ";\n";

    return str;
}

}

