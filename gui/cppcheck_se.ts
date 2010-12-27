<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="sv_SE">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>Om Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>Version %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - Ett verktyg för statisk analys av C/C++ kod.</translation>
    </message>
    <message utf8="true">
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-2010 Daniel Marjamäki and cppcheck team.</source>
        <oldsource>Copyright (C) 2007-2010 Daniel Marjamäki and cppcheck team.</oldsource>
        <translation type="unfinished">Copyright (C) 2007-2010 Daniel Marjamäki and cppcheck team.</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>This program is licensed under the terms
of the GNU General Public License version 3</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Hemsida: %1</translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="application.ui" line="17"/>
        <source>Add an application</source>
        <translation>Lägg till program</translation>
    </message>
    <message>
        <location filename="application.ui" line="23"/>
        <source>Here you can add applications that can open error files.
Specify a name for the application and the application to execute.

The following texts are replaced with appropriate values when application is executed:
(file) - Filename containing the error
(line) - Line number containing the error
(message) - Error message
(severity) - Error severity

Example opening a file with Kate and make Kate scroll to the correct line:
kate -l(line) (file)</source>
        <oldsource>Here you can add applications that can open error files.
Specify a name for the application and the application to execute.

The following texts are replaced with appriproate values when application is executed:
(file) - Filename containing the error
(line) - Line number containing the error
(message) - Error message
(severity) - Error severity

Example opening a file with Kate and make Kate scroll to the correct line:
kate -l(line) (file)</oldsource>
        <translation>Här anger du det program som visar fel i filer.
Ange programmets namn och kommandorad.

Följande parametrar kan användas i kommandoraden:
(file) - Filnamnet
(line) - Radnummer
(message) - Felmeddelande
(severity) - Svårighetsgrad

Exempel för att öppna filen och scrolla till rätt rad med programmet Kate:
kate -l(line) (file)</translation>
    </message>
    <message>
        <location filename="application.ui" line="47"/>
        <source>Application&apos;s name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="application.ui" line="57"/>
        <source>Command to execute:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="application.ui" line="96"/>
        <source>Browse</source>
        <translation>Bläddra</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="56"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Exekverbara filer (*.exe);;Alla filer(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Select viewer application</source>
        <translation>Välj program</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="96"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="97"/>
        <source>You must specify a name and a path for the application!</source>
        <translation>Du måste ange programmets namn och kommandorad!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="43"/>
        <source>Could not find the file: %1</source>
        <oldsource>Could not find the file:
</oldsource>
        <translation>Kunde inte hitta filen: %1</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="47"/>
        <location filename="fileviewdialog.cpp" line="62"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="58"/>
        <source>Could not read the file: %1</source>
        <translation>Kunde inte läsa filen: %1</translation>
    </message>
</context>
<context>
    <name>HelpWindow</name>
    <message>
        <location filename="helpwindow.ui" line="14"/>
        <source>Cppcheck Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="22"/>
        <source>Go back</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="25"/>
        <source>Back</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="42"/>
        <source>Go forward</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="45"/>
        <source>Forward</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="62"/>
        <source>Start</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="65"/>
        <source>Home</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LogView</name>
    <message>
        <location filename="logview.ui" line="17"/>
        <source>Checking Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="logview.ui" line="48"/>
        <source>&amp;Save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="logview.ui" line="55"/>
        <source>Clear</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="logview.ui" line="62"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="logview.cpp" line="65"/>
        <source>Save Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="logview.cpp" line="66"/>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="logview.cpp" line="72"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="73"/>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="main.ui" line="26"/>
        <location filename="mainwindow.cpp" line="237"/>
        <location filename="mainwindow.cpp" line="267"/>
        <location filename="mainwindow.cpp" line="552"/>
        <location filename="mainwindow.cpp" line="670"/>
        <location filename="mainwindow.cpp" line="688"/>
        <location filename="mainwindow.cpp" line="852"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="156"/>
        <source>Standard</source>
        <translation>Standard</translation>
    </message>
    <message>
        <location filename="main.ui" line="74"/>
        <source>&amp;File</source>
        <translation>&amp;Arkiv</translation>
    </message>
    <message>
        <location filename="main.ui" line="88"/>
        <source>&amp;View</source>
        <translation>&amp;Visa</translation>
    </message>
    <message>
        <location filename="main.ui" line="92"/>
        <source>&amp;Toolbars</source>
        <translation>Verktygsfält</translation>
    </message>
    <message>
        <location filename="main.ui" line="133"/>
        <source>&amp;Check</source>
        <translation>&amp;Check</translation>
    </message>
    <message>
        <location filename="main.ui" line="142"/>
        <source>&amp;Edit</source>
        <translation>&amp;Redigera</translation>
    </message>
    <message>
        <location filename="main.ui" line="192"/>
        <source>&amp;License...</source>
        <translation>&amp;Licens...</translation>
    </message>
    <message>
        <location filename="main.ui" line="197"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Utvecklat av...</translation>
    </message>
    <message>
        <location filename="main.ui" line="206"/>
        <source>&amp;About...</source>
        <translation>&amp;Om...</translation>
    </message>
    <message>
        <location filename="main.ui" line="211"/>
        <source>&amp;Files...</source>
        <translation>&amp;Filer...</translation>
    </message>
    <message>
        <location filename="main.ui" line="214"/>
        <location filename="main.ui" line="217"/>
        <source>Check files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="220"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="229"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Katalog...</translation>
    </message>
    <message>
        <location filename="main.ui" line="232"/>
        <location filename="main.ui" line="235"/>
        <source>Check directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="238"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="247"/>
        <source>&amp;Recheck files</source>
        <translation>Starta &amp;om check</translation>
    </message>
    <message>
        <location filename="main.ui" line="250"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="259"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stoppa</translation>
    </message>
    <message>
        <location filename="main.ui" line="262"/>
        <location filename="main.ui" line="265"/>
        <source>Stop checking</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="268"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="277"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Spara resultat till fil...</translation>
    </message>
    <message>
        <location filename="main.ui" line="280"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="285"/>
        <source>&amp;Quit</source>
        <translation>&amp;Avsluta</translation>
    </message>
    <message>
        <location filename="main.ui" line="294"/>
        <source>&amp;Clear results</source>
        <translation>&amp;Töm resultat</translation>
    </message>
    <message>
        <location filename="main.ui" line="303"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Inställningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="333"/>
        <source>Errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="336"/>
        <location filename="main.ui" line="339"/>
        <source>Show errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="461"/>
        <source>Warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="464"/>
        <location filename="main.ui" line="467"/>
        <source>Show warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="479"/>
        <source>Performance warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="482"/>
        <location filename="main.ui" line="485"/>
        <source>Show performance warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="493"/>
        <source>Show &amp;hidden</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="505"/>
        <source>Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="508"/>
        <source>Show information messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="520"/>
        <source>Portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="523"/>
        <source>Show portability warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="344"/>
        <source>&amp;Check all</source>
        <translation>&amp;Kryssa alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="349"/>
        <source>&amp;Uncheck all</source>
        <translation>Kryssa &amp;ur alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="354"/>
        <source>Collapse &amp;all</source>
        <translatorcomment>Ingen bra översättning!</translatorcomment>
        <translation>&amp;Fäll ihop alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="359"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Expandera alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="367"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standard</translation>
    </message>
    <message>
        <location filename="main.ui" line="370"/>
        <source>Standard items</source>
        <translation>Standard poster</translation>
    </message>
    <message>
        <location filename="main.ui" line="386"/>
        <source>Toolbar</source>
        <translation>Verktygsfält</translation>
    </message>
    <message>
        <location filename="main.ui" line="394"/>
        <source>&amp;Categories</source>
        <translation>&amp;Kategorier</translation>
    </message>
    <message>
        <location filename="main.ui" line="397"/>
        <source>Error categories</source>
        <translation>Fel kategorier</translation>
    </message>
    <message>
        <location filename="main.ui" line="402"/>
        <source>&amp;Open XML...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="411"/>
        <source>Open P&amp;roject File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="416"/>
        <source>&amp;New Project File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="421"/>
        <source>&amp;Log View</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="424"/>
        <source>Log View</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="432"/>
        <source>C&amp;lose Project File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="440"/>
        <source>&amp;Edit Project File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="449"/>
        <source>&amp;Statistics</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="375"/>
        <source>&amp;Contents</source>
        <translation>&amp;Innehåll</translation>
    </message>
    <message>
        <location filename="main.ui" line="175"/>
        <source>Categories</source>
        <translation>Kategorier</translation>
    </message>
    <message>
        <location filename="main.ui" line="315"/>
        <source>Style warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="318"/>
        <location filename="main.ui" line="321"/>
        <source>Show style warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="378"/>
        <source>Open the help contents</source>
        <translation>Öppna hjälp</translation>
    </message>
    <message>
        <location filename="main.ui" line="381"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="main.ui" line="118"/>
        <source>&amp;Language</source>
        <translation>&amp;Språk</translation>
    </message>
    <message>
        <location filename="main.ui" line="123"/>
        <source>&amp;Help</source>
        <translation>&amp;Hjälp</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="283"/>
        <source>Select files to check</source>
        <translation>Välj filer att kontrollera</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="297"/>
        <source>Select directory to check</source>
        <translation>Välj katalog som skall kontrolleras</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="238"/>
        <source>No suitable files found to check!</source>
        <translation>Inga lämpliga filer hittades!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="595"/>
        <source>License</source>
        <translation>Licens</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="602"/>
        <source>Authors</source>
        <translation>Utvecklare</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="610"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML filer (*.xml);;Text filer (*.txt);;CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="612"/>
        <source>Save the report file</source>
        <translation>Spara rapport</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="471"/>
        <location filename="mainwindow.cpp" line="620"/>
        <source>XML files (*.xml)</source>
        <translation>XML filer (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="268"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="340"/>
        <location filename="mainwindow.cpp" line="783"/>
        <location filename="mainwindow.cpp" line="829"/>
        <source>Project: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="473"/>
        <source>Open the report file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="548"/>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="626"/>
        <source>Text files (*.txt)</source>
        <translation>Text filer (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="632"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="672"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="689"/>
        <source>Failed to change the language:

%1

</source>
        <oldsource>Failed to change language:

%1</oldsource>
        <translation>Misslyckades med att byta språk:

%1

</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="750"/>
        <location filename="mainwindow.cpp" line="759"/>
        <source>Cppcheck Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="750"/>
        <source>Failed to load help file (not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="759"/>
        <source>Failed to load help file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="773"/>
        <location filename="mainwindow.cpp" line="818"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="775"/>
        <source>Select Project File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="820"/>
        <source>Select Project Filename</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="853"/>
        <source>No project file loaded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Finnish</source>
        <translation>Finska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="32"/>
        <source>English</source>
        <translation>Engelska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="33"/>
        <source>Dutch</source>
        <translation>Nederländska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Swedish</source>
        <translation>Svenska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>German</source>
        <translation>Tyska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Russian</source>
        <translation>Ryska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Polish</source>
        <translation>Polska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>Japanese</source>
        <oldsource>Japanease</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Serbian</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Project</name>
    <message>
        <location filename="project.cpp" line="63"/>
        <location filename="project.cpp" line="103"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
    <message>
        <location filename="project.cpp" line="64"/>
        <source>Could not read the project file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="project.cpp" line="104"/>
        <source>Could not write the project file.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="22"/>
        <source>Project:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="39"/>
        <source>Paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="52"/>
        <location filename="projectfile.ui" line="76"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="63"/>
        <source>Include paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="87"/>
        <source>Defines:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="36"/>
        <source>Project file: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="153"/>
        <source>Select include directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="165"/>
        <source>Select directory to check</source>
        <translation type="unfinished">Välj katalog som skall kontrolleras</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="97"/>
        <source>Incorrect language specified!</source>
        <translation>valt språk är ej korrekt!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="107"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation>Språk filen %1 hittades ej!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="113"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Misslyckades med att ladda översättningen för %1 från filen %2</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="978"/>
        <source>File</source>
        <translation>Fil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="978"/>
        <source>Severity</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="978"/>
        <source>Line</source>
        <translation>Rad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="978"/>
        <source>Summary</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="108"/>
        <source>Undefined file</source>
        <translation>Odefinierad fil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="508"/>
        <source>Copy filename</source>
        <translation>Kopiera filnamn</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="509"/>
        <source>Copy full path</source>
        <translation>Kopiera full sökväg</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="510"/>
        <source>Copy message</source>
        <translation>Kopiera meddelande</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="511"/>
        <source>Hide</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="558"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="559"/>
        <source>Configure the text file viewer program in Cppcheck preferences/Applications.</source>
        <oldsource>You can open this error by specifying applications in program&apos;s settings.</oldsource>
        <translation>Konfigurera program i inställningar/program.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="586"/>
        <source>Could not find the file!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="624"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>Kunde inte starta %1

Kontrollera att sökvägen och parametrarna är korrekta.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="638"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="645"/>
        <source>Select Directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="827"/>
        <source>style</source>
        <translation>stil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="831"/>
        <source>error</source>
        <translation>fel</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="835"/>
        <source>warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="839"/>
        <source>performance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="843"/>
        <source>portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="847"/>
        <source>information</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="190"/>
        <location filename="resultsview.cpp" line="202"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="191"/>
        <source>No errors found.</source>
        <translation>Inga fel hittades.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="199"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Fel hittades, men de visas ej.
För att ställa in vilka fel som skall visas använd visa menyn.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="251"/>
        <location filename="resultsview.cpp" line="261"/>
        <source>Failed to read the report.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="302"/>
        <source>Summary</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="303"/>
        <source>Message</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="117"/>
        <source>No errors found, nothing to save.</source>
        <translation>Inga fel hittades, ingenting att spara.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="144"/>
        <location filename="resultsview.cpp" line="154"/>
        <source>Failed to save the report.</source>
        <translation>Misslyckades med att spara rapporten.</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>Resultat</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>Inställningar</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>Allmänt</translation>
    </message>
    <message>
        <location filename="settings.ui" line="32"/>
        <source>Include paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="45"/>
        <source>Add...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="65"/>
        <source>Number of threads: </source>
        <translation>Antal trådar:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="124"/>
        <source>Check all #ifdef configurations</source>
        <translation>Kontrollera alla #ifdef konfigurationer</translation>
    </message>
    <message>
        <location filename="settings.ui" line="131"/>
        <source>Show full path of files</source>
        <translation>Visa den fulla sökvägen för filer</translation>
    </message>
    <message>
        <location filename="settings.ui" line="138"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Visa &quot;Inga fel hittades&quot; meddelande när inga fel hittas</translation>
    </message>
    <message>
        <location filename="settings.ui" line="145"/>
        <source>Show internal warnings in log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="152"/>
        <source>Enable inline suppressions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="173"/>
        <source>Applications</source>
        <translation>Program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="182"/>
        <source>Add application</source>
        <translation>Lägg till program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="189"/>
        <source>Delete application</source>
        <translation>Ta bort program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="196"/>
        <source>Modify application</source>
        <translation>Ändra program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="203"/>
        <source>Set as default application</source>
        <translation>Ange som standard program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="211"/>
        <source>Reports</source>
        <translation>Rapporter</translation>
    </message>
    <message>
        <location filename="settings.ui" line="217"/>
        <source>Save all errors when creating report</source>
        <translation>Spara alla fel</translation>
    </message>
    <message>
        <location filename="settings.ui" line="224"/>
        <source>Save full path to files in reports</source>
        <translation>Spara fulla sökvägar</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="138"/>
        <source>Add a new application</source>
        <translation>Lägg till program</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="171"/>
        <source>Modify an application</source>
        <translation>Ändra program</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="236"/>
        <source>Select include directory</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="228"/>
        <source>Statistics</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <source>Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="33"/>
        <source>Project:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="53"/>
        <source>Paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="85"/>
        <source>Include paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="145"/>
        <source>Previous Scan</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="151"/>
        <source>Path Selected:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="161"/>
        <source>Number of Files Scanned:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Scan Duration:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="236"/>
        <source>Errors:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="254"/>
        <source>Warnings:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="272"/>
        <source>Stylistic warnings:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="290"/>
        <source>Portability warnings:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="297"/>
        <location filename="stats.ui" line="333"/>
        <source>TextLabel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="308"/>
        <source>Performance issues:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="326"/>
        <source>Information messages:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="362"/>
        <source>Copy to Clipboard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>1 day</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>%1 days</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>1 hour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>%1 hours</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="85"/>
        <source>1 minute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="85"/>
        <source>%1 minutes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="87"/>
        <source>1 second</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="87"/>
        <source>%1 seconds</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="91"/>
        <source>0.%1 seconds</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="93"/>
        <source> and </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="102"/>
        <source>Project Settings
	Project:	%1
	Paths:	%2
	Include paths:	%3
	Defines:	%4
Previous Scan
	Path selected:	%5
	Number of files scanned:	%6
	Scan duration:	%7
Statistics
	Errors:	%8
	Warnings:	%9
	Style warnings:	%10
	Portability warnings:	%11
	Performance warnings:	%12
	Information messages:	%13
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="135"/>
        <source>&lt;h3&gt;Project Settings&lt;h3&gt;
&lt;table&gt;
 &lt;tr&gt;&lt;th&gt;Project:&lt;/th&gt;&lt;td&gt;%1&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Paths:&lt;/th&gt;&lt;td&gt;%2&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Include paths:&lt;/th&gt;&lt;td&gt;%3&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Defines:&lt;/th&gt;&lt;td&gt;%4&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;
&lt;h3&gt;Previous Scan&lt;/h3&gt;
&lt;table&gt;
 &lt;tr&gt;&lt;th&gt;Path selected:&lt;/th&gt;&lt;td&gt;%5&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Number of files scanned:&lt;/th&gt;&lt;td&gt;%6&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Scan duration:&lt;/th&gt;&lt;td&gt;%7&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;
&lt;h3&gt;Statistics&lt;/h3&gt;
 &lt;tr&gt;&lt;th&gt;Errors:&lt;/th&gt;&lt;td&gt;%8&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Warnings:&lt;/th&gt;&lt;td&gt;%9&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Style warnings:&lt;/th&gt;&lt;td&gt;%10&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Portability warnings:&lt;/th&gt;&lt;td&gt;%11&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Performance warnings:&lt;/th&gt;&lt;td&gt;%12&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Information messages:&lt;/th&gt;&lt;td&gt;%13&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;
</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
