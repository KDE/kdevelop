SET(kdevpropertyeditor_SRCS
childproperty.cpp
pcombobox.cpp   
pdummywidget.cpp  
ppointedit.cpp     
propertymachinefactory.cpp  
pstringlistedit.cpp
multiproperty.cpp  
pcursoredit.cpp      
pfontbutton.cpp   
prectedit.cpp       
propertywidget.cpp          
psymbolcombo.cpp
pcheckbox.cpp     
pdateedit.cpp        
pfontcombo.cpp    
property.cpp        
psizeedit.cpp               
purledit.cpp
pcolorbutton.cpp   
pdatetimeedit.cpp    
plineedit.cpp     
propertyeditor.cpp  
psizepolicyedit.cpp         
pyesnobutton.cpp
pcolorcombo.cpp    
pdoublenuminput.cpp  
ppixmapedit.cpp   
propertylist.cpp    
pspinbox.cpp 
propertywidgetproxy.cpp 
plinestyleedit.cpp 
qeditlistbox.cpp
)

KDE_AUTOMOC(${kdevpropertyeditor_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(kdevpropertyeditor SHARED ${kdevpropertyeditor_SRCS})

