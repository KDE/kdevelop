/*
 * Copyright (C) %{YEAR} %{AUTHOR} <%{EMAIL}>
 */

import java.util.*;
import org.kde.qt.*;
import org.kde.koala.*;

public class %{APPNAME}Preferences extends KDialogBase
{
private %{APPNAME}PrefPageOne m_pageOne;
private %{APPNAME}PrefPageTwo m_pageTwo;

public %{APPNAME}Preferences()
{
    super(TreeList, "%{APPNAME} Preferences",
                  Help|Default|Ok|Apply|Cancel, Ok);

    // this is the base class for your preferences dialog.  it is now
    // a Treelist dialog.. but there are a number of other
    // possibilities (including Tab, Swallow, and just Plain)
    QFrame frame;
    frame = addPage(tr("First Page"), tr("Page One Options"));
    m_pageOne = new %{APPNAME}PrefPageOne(frame);

    frame = addPage(tr("Second Page"), tr("Page Two Options"));
    m_pageTwo = new %{APPNAME}PrefPageTwo(frame);
}

public class %{APPNAME}PrefPageOne extends QFrame {
public %{APPNAME}PrefPageOne(QWidget parent)
{
    super(parent);
    QHBoxLayout layout = new QHBoxLayout(this);
    layout.setAutoAdd(true);

    new QLabel("Add something here", this);
}
}

public class %{APPNAME}PrefPageTwo extends QFrame {
public %{APPNAME}PrefPageTwo(QWidget parent)
{
    super(parent);
    QHBoxLayout layout = new QHBoxLayout(this);
    layout.setAutoAdd(true);

    new QLabel("Add something here", this);
}
}

}
