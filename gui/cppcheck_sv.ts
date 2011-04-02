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
        <source>Copyright © 2007-2011 Daniel Marjamäki and cppcheck team.</source>
        <oldsource>Copyright © 2007-2010 Daniel Marjamäki and cppcheck team.</oldsource>
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
        <source>Here you can add an application that can open error files. Specify a name for the application, the application executable and command line parameters for the application.

The following texts in parameters are replaced with appropriate values when application is executed:
(file) - Filename containing the error
(line) - Line number containing the error
(message) - Error message
(severity) - Error severity

Example opening a file with Kate and make Kate scroll to the correct line:
executable: kate
parameters: -l(line) (file)</source>
        <oldsource>Here you can add applications that can open error files.
Specify a name for the application and the application to execute.

The following texts are replaced with appropriate values when application is executed:
(file) - Filename containing the error
(line) - Line number containing the error
(message) - Error message
(severity) - Error severity

Example opening a file with Kate and make Kate scroll to the correct line:
kate -l(line) (file)</oldsource>
        <translation type="unfinished">Här anger du det program som visar fel i filer.
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
        <location filename="application.ui" line="50"/>
        <source>&amp;Application&apos;s name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="application.ui" line="60"/>
        <source>&amp;Executable:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="application.ui" line="70"/>
        <source>&amp;Parameters:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="application.ui" line="112"/>
        <source>Browse</source>
        <translation>Bläddra</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="58"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Exekverbara filer (*.exe);;Alla filer(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="61"/>
        <source>Select viewer application</source>
        <translation>Välj program</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="94"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="95"/>
        <source>You must specify a name, a path and parameters for the application!</source>
        <translation type="unfinished"></translation>
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
    <name>LogView</name>
    <message>
        <location filename="logview.ui" line="17"/>
        <source>Checking Log</source>
        <translation>Analys logg</translation>
    </message>
    <message>
        <location filename="logview.ui" line="48"/>
        <source>&amp;Save</source>
        <translation>&amp;Spara</translation>
    </message>
    <message>
        <location filename="logview.ui" line="55"/>
        <source>Clear</source>
        <translation>Töm</translation>
    </message>
    <message>
        <location filename="logview.ui" line="62"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="65"/>
        <source>Save Log</source>
        <translation>Spara logg</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="66"/>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation>Text filer (*.txt *.log);;Alla filer (*.*)</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="72"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="73"/>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation>Kunde ej öppna fil för skrivning: &quot;%1&quot;</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="main.ui" line="26"/>
        <location filename="mainwindow.cpp" line="166"/>
        <location filename="mainwindow.cpp" line="220"/>
        <location filename="mainwindow.cpp" line="247"/>
        <location filename="mainwindow.cpp" line="542"/>
        <location filename="mainwindow.cpp" line="666"/>
        <location filename="mainwindow.cpp" line="682"/>
        <location filename="mainwindow.cpp" line="801"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="150"/>
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
        <location filename="main.ui" line="128"/>
        <source>&amp;Check</source>
        <translation>&amp;Check</translation>
    </message>
    <message>
        <location filename="main.ui" line="137"/>
        <source>&amp;Edit</source>
        <translation>&amp;Redigera</translation>
    </message>
    <message>
        <location filename="main.ui" line="186"/>
        <source>&amp;License...</source>
        <translation>&amp;Licens...</translation>
    </message>
    <message>
        <location filename="main.ui" line="191"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Utvecklat av...</translation>
    </message>
    <message>
        <location filename="main.ui" line="200"/>
        <source>&amp;About...</source>
        <translation>&amp;Om...</translation>
    </message>
    <message>
        <location filename="main.ui" line="205"/>
        <source>&amp;Files...</source>
        <translation>&amp;Filer...</translation>
    </message>
    <message>
        <location filename="main.ui" line="208"/>
        <location filename="main.ui" line="211"/>
        <source>Check files</source>
        <translation>Analysera filer</translation>
    </message>
    <message>
        <location filename="main.ui" line="214"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="223"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Katalog...</translation>
    </message>
    <message>
        <location filename="main.ui" line="226"/>
        <location filename="main.ui" line="229"/>
        <source>Check directory</source>
        <translation>Analysera mapp</translation>
    </message>
    <message>
        <location filename="main.ui" line="232"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="241"/>
        <source>&amp;Recheck files</source>
        <translation>Starta &amp;om check</translation>
    </message>
    <message>
        <location filename="main.ui" line="244"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="253"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stoppa</translation>
    </message>
    <message>
        <location filename="main.ui" line="256"/>
        <location filename="main.ui" line="259"/>
        <source>Stop checking</source>
        <translation>Stoppa analys</translation>
    </message>
    <message>
        <location filename="main.ui" line="262"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="271"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Spara resultat till fil...</translation>
    </message>
    <message>
        <location filename="main.ui" line="274"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="279"/>
        <source>&amp;Quit</source>
        <translation>&amp;Avsluta</translation>
    </message>
    <message>
        <location filename="main.ui" line="288"/>
        <source>&amp;Clear results</source>
        <translation>&amp;Töm resultat</translation>
    </message>
    <message>
        <location filename="main.ui" line="297"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Inställningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="327"/>
        <source>Errors</source>
        <translation>Fel</translation>
    </message>
    <message>
        <location filename="main.ui" line="330"/>
        <location filename="main.ui" line="333"/>
        <source>Show errors</source>
        <translation>Visa fel</translation>
    </message>
    <message>
        <location filename="main.ui" line="455"/>
        <source>Warnings</source>
        <translation>Varningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="458"/>
        <location filename="main.ui" line="461"/>
        <source>Show warnings</source>
        <translation>Visa varningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="473"/>
        <source>Performance warnings</source>
        <translation>Prestanda varningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="476"/>
        <location filename="main.ui" line="479"/>
        <source>Show performance warnings</source>
        <translation>Visa prestanda varningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="487"/>
        <source>Show &amp;hidden</source>
        <translation>Visa dolda</translation>
    </message>
    <message>
        <location filename="main.ui" line="499"/>
        <source>Information</source>
        <translation>Information</translation>
    </message>
    <message>
        <location filename="main.ui" line="502"/>
        <source>Show information messages</source>
        <translation>Visa informations meddelanden</translation>
    </message>
    <message>
        <location filename="main.ui" line="514"/>
        <source>Portability</source>
        <translation>Portabilitet</translation>
    </message>
    <message>
        <location filename="main.ui" line="517"/>
        <source>Show portability warnings</source>
        <translation>Visa portabilitets varningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="338"/>
        <source>&amp;Check all</source>
        <translation>&amp;Kryssa alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="343"/>
        <source>&amp;Uncheck all</source>
        <translation>Kryssa &amp;ur alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="348"/>
        <source>Collapse &amp;all</source>
        <translatorcomment>Ingen bra översättning!</translatorcomment>
        <translation>&amp;Fäll ihop alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="353"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Expandera alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="361"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standard</translation>
    </message>
    <message>
        <location filename="main.ui" line="364"/>
        <source>Standard items</source>
        <translation>Standard poster</translation>
    </message>
    <message>
        <location filename="main.ui" line="380"/>
        <source>Toolbar</source>
        <translation>Verktygsfält</translation>
    </message>
    <message>
        <location filename="main.ui" line="388"/>
        <source>&amp;Categories</source>
        <translation>&amp;Kategorier</translation>
    </message>
    <message>
        <location filename="main.ui" line="391"/>
        <source>Error categories</source>
        <translation>Fel kategorier</translation>
    </message>
    <message>
        <location filename="main.ui" line="396"/>
        <source>&amp;Open XML...</source>
        <translation>&amp;Öppna XML...</translation>
    </message>
    <message>
        <location filename="main.ui" line="405"/>
        <source>Open P&amp;roject File...</source>
        <translation>Öppna Projektfil...</translation>
    </message>
    <message>
        <location filename="main.ui" line="410"/>
        <source>&amp;New Project File...</source>
        <translation>Ny projektfil...</translation>
    </message>
    <message>
        <location filename="main.ui" line="415"/>
        <source>&amp;Log View</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="418"/>
        <source>Log View</source>
        <translation>Logg vy</translation>
    </message>
    <message>
        <location filename="main.ui" line="426"/>
        <source>C&amp;lose Project File</source>
        <translation>Stäng projektfil</translation>
    </message>
    <message>
        <location filename="main.ui" line="434"/>
        <source>&amp;Edit Project File...</source>
        <translation>Redigera projektfil...</translation>
    </message>
    <message>
        <location filename="main.ui" line="443"/>
        <source>&amp;Statistics</source>
        <translation>Statistik</translation>
    </message>
    <message>
        <location filename="main.ui" line="369"/>
        <source>&amp;Contents</source>
        <translation>&amp;Innehåll</translation>
    </message>
    <message>
        <location filename="main.ui" line="169"/>
        <source>Categories</source>
        <translation>Kategorier</translation>
    </message>
    <message>
        <location filename="main.ui" line="309"/>
        <source>Style warnings</source>
        <translation>Stil varningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="312"/>
        <location filename="main.ui" line="315"/>
        <source>Show style warnings</source>
        <translation>Visa stil varningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="372"/>
        <source>Open the help contents</source>
        <translation>Öppna hjälp</translation>
    </message>
    <message>
        <location filename="main.ui" line="375"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="main.ui" line="118"/>
        <source>&amp;Help</source>
        <translation>&amp;Hjälp</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="263"/>
        <source>Select files to check</source>
        <translation>Välj filer att kontrollera</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="277"/>
        <source>Select directory to check</source>
        <translation>Välj katalog som skall kontrolleras</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="221"/>
        <source>No suitable files found to check!</source>
        <translation>Inga lämpliga filer hittades!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="585"/>
        <source>License</source>
        <translation>Licens</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="592"/>
        <source>Authors</source>
        <translation>Utvecklare</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="600"/>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation type="unfinished">XML filer (*.xml);;Text filer (*.txt);;CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="602"/>
        <source>Save the report file</source>
        <translation>Spara rapport</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="464"/>
        <source>XML files (*.xml)</source>
        <translation>XML filer (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="161"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="248"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Du måste stänga projektfilen innan nya filer eller sökvägar kan väljas!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="320"/>
        <location filename="mainwindow.cpp" line="731"/>
        <location filename="mainwindow.cpp" line="778"/>
        <source>Project: </source>
        <translation>Projekt.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="466"/>
        <source>Open the report file</source>
        <translation>Öppna rapportfilen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="538"/>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?.</source>
        <translation>Cppcheck kör.

Vill du stoppa analysen och avsluta Cppcheck?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="610"/>
        <source>XML files version 1 (*.xml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="616"/>
        <source>XML files version 2 (*.xml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="622"/>
        <source>Text files (*.txt)</source>
        <translation>Text filer (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="628"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="668"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="683"/>
        <source>Failed to change the language:

%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="721"/>
        <location filename="mainwindow.cpp" line="767"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Projektfiler (*.cppcheck);;Alla filer(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="723"/>
        <source>Select Project File</source>
        <translation>Välj projektfil</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="769"/>
        <source>Select Project Filename</source>
        <translation>Välj Projektfil</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="802"/>
        <source>No project file loaded</source>
        <translation>Inget projekt laddat</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Finnish</source>
        <translation>Finska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="33"/>
        <source>English</source>
        <translation>Engelska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="32"/>
        <source>Dutch</source>
        <translation>Nederländska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>French</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>Swedish</source>
        <translation>Svenska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>German</source>
        <translation>Tyska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>Russian</source>
        <translation>Ryska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Polish</source>
        <translation>Polska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Japanese</source>
        <oldsource>Japanease</oldsource>
        <translation>Japanska</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Serbian</source>
        <translation>Serbiska</translation>
    </message>
</context>
<context>
    <name>Project</name>
    <message>
        <location filename="project.cpp" line="63"/>
        <location filename="project.cpp" line="109"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="project.cpp" line="64"/>
        <source>Could not read the project file.</source>
        <translation>Kunde ej läsa projektfilen.</translation>
    </message>
    <message>
        <location filename="project.cpp" line="110"/>
        <source>Could not write the project file.</source>
        <translation>Kunde ej skriva projektfilen</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation>Projektfil</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <source>Project</source>
        <translation type="unfinished">Projekt</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="34"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="70"/>
        <location filename="projectfile.ui" line="207"/>
        <source>Paths:</source>
        <translation>Sökvägar:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="97"/>
        <location filename="projectfile.ui" line="162"/>
        <location filename="projectfile.ui" line="221"/>
        <source>Add...</source>
        <translation type="unfinished">Lägg till...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="104"/>
        <location filename="projectfile.ui" line="169"/>
        <location filename="projectfile.ui" line="228"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="111"/>
        <location filename="projectfile.ui" line="176"/>
        <location filename="projectfile.ui" line="235"/>
        <source>Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="138"/>
        <source>Includes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="144"/>
        <source>Include directories:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="201"/>
        <source>Ignore</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="51"/>
        <source>Defines:</source>
        <translation>Defines:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="36"/>
        <source>Project file: %1</source>
        <translation>Projektfil: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="191"/>
        <source>Select include directory</source>
        <translation>Välj include sökväg</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="203"/>
        <source>Select directory to check</source>
        <translation>Välj katalog som skall kontrolleras</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="241"/>
        <source>Select directory to ignore</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="89"/>
        <source>Unknown language specified!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="99"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation>Språk filen %1 hittades ej!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="105"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Misslyckades med att ladda översättningen för %1 från filen %2</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="1049"/>
        <source>File</source>
        <translation>Fil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="1049"/>
        <source>Severity</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="1049"/>
        <source>Line</source>
        <translation>Rad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="1049"/>
        <source>Summary</source>
        <translation>Sammanfattning</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="109"/>
        <source>Undefined file</source>
        <translation>Odefinierad fil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="345"/>
        <source>debug</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="593"/>
        <source>Copy filename</source>
        <translation>Kopiera filnamn</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="594"/>
        <source>Copy full path</source>
        <translation>Kopiera full sökväg</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="595"/>
        <source>Copy message</source>
        <translation>Kopiera meddelande</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="596"/>
        <source>Hide</source>
        <translation>Dölj</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="643"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="644"/>
        <source>Configure the text file viewer program in Cppcheck preferences/Applications.</source>
        <oldsource>You can open this error by specifying applications in program&apos;s settings.</oldsource>
        <translation>Konfigurera program i inställningar/program.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="674"/>
        <source>Could not find the file!</source>
        <translation>Kunde inte hitta filen!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="727"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>Kunde inte starta %1

Kontrollera att sökvägen och parametrarna är korrekta.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="741"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>Kunde inte hitta filen:
%1
Välj mappen där filen finns.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="748"/>
        <source>Select Directory</source>
        <translation>Välj mapp</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="321"/>
        <source>style</source>
        <translation>stil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="325"/>
        <source>error</source>
        <translation>fel</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="329"/>
        <source>warning</source>
        <translation>varning</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="333"/>
        <source>performance</source>
        <translation>prestanda</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="337"/>
        <source>portability</source>
        <translation>portabilitet</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="341"/>
        <source>information</source>
        <translation>information</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="195"/>
        <location filename="resultsview.cpp" line="207"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="196"/>
        <source>No errors found.</source>
        <translation>Inga fel hittades.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="204"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Fel hittades, men de visas ej.
För att ställa in vilka fel som skall visas använd visa menyn.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="251"/>
        <location filename="resultsview.cpp" line="271"/>
        <location filename="resultsview.cpp" line="281"/>
        <source>Failed to read the report.</source>
        <translation>Misslyckades att läsa rapporten.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="321"/>
        <source>Summary</source>
        <translation>Sammanfattning</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="322"/>
        <source>Message</source>
        <translation>Meddelande</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="119"/>
        <source>No errors found, nothing to save.</source>
        <translation>Inga fel hittades, ingenting att spara.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="149"/>
        <location filename="resultsview.cpp" line="159"/>
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
        <location filename="settings.ui" line="169"/>
        <source>Include paths:</source>
        <translation>Include sökvägar:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="187"/>
        <location filename="settings.ui" line="239"/>
        <source>Add...</source>
        <translation>Lägg till...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>Antal trådar:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="85"/>
        <source>Ideal count:</source>
        <translation>Optimalt värde:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="92"/>
        <source>TextLabel</source>
        <translation>TextLabel</translation>
    </message>
    <message>
        <location filename="settings.ui" line="114"/>
        <source>Force checking all #ifdef configurations</source>
        <oldsource>Check all #ifdef configurations</oldsource>
        <translation>Kontrollera alla #ifdef konfigurationer</translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Show full path of files</source>
        <translation>Visa den fulla sökvägen för filer</translation>
    </message>
    <message>
        <location filename="settings.ui" line="128"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Visa &quot;Inga fel hittades&quot; meddelande när inga fel hittas</translation>
    </message>
    <message>
        <location filename="settings.ui" line="135"/>
        <source>Show internal warnings in log</source>
        <translation>Visa interna fel i loggen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="142"/>
        <source>Enable inline suppressions</source>
        <translation>Använd inline suppressions</translation>
    </message>
    <message>
        <location filename="settings.ui" line="163"/>
        <source>Paths</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="194"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="201"/>
        <location filename="settings.ui" line="253"/>
        <source>Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="226"/>
        <source>Applications</source>
        <translation>Program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="246"/>
        <source>Edit...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="260"/>
        <source>Set as default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="285"/>
        <source>Reports</source>
        <translation>Rapporter</translation>
    </message>
    <message>
        <location filename="settings.ui" line="291"/>
        <source>Save all errors when creating report</source>
        <translation>Spara alla fel</translation>
    </message>
    <message>
        <location filename="settings.ui" line="298"/>
        <source>Save full path to files in reports</source>
        <translation>Spara fulla sökvägar</translation>
    </message>
    <message>
        <location filename="settings.ui" line="319"/>
        <source>Language</source>
        <translation>Språk</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="84"/>
        <source>N/A</source>
        <translation>Ej tillgängligt</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="201"/>
        <source>Add a new application</source>
        <translation>Lägg till program</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="241"/>
        <source>Modify an application</source>
        <translation>Ändra program</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="274"/>
        <source>[Default]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="321"/>
        <source>Select include directory</source>
        <translation>Välj include mapp</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="228"/>
        <source>Statistics</source>
        <translation>Statistik</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="stats.ui" line="33"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="53"/>
        <source>Paths:</source>
        <translation>Sökvägar:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="85"/>
        <source>Include paths:</source>
        <translation>Include sökvägar:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation>Defines:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="145"/>
        <source>Previous Scan</source>
        <translation>Föregående analys</translation>
    </message>
    <message>
        <location filename="stats.ui" line="151"/>
        <source>Path Selected:</source>
        <translation>Vald sökväg:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="161"/>
        <source>Number of Files Scanned:</source>
        <translation>Antal analyserade filer:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Scan Duration:</source>
        <translation>Analys tid:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="236"/>
        <source>Errors:</source>
        <translation>Fel:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="254"/>
        <source>Warnings:</source>
        <translation>Varningar:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="272"/>
        <source>Stylistic warnings:</source>
        <translation>Stil varningar:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="290"/>
        <source>Portability warnings:</source>
        <translation>Portabilitets varningar:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="297"/>
        <location filename="stats.ui" line="333"/>
        <source>TextLabel</source>
        <translation>TextLabel</translation>
    </message>
    <message>
        <location filename="stats.ui" line="308"/>
        <source>Performance issues:</source>
        <translation>Prestanda varningar:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="326"/>
        <source>Information messages:</source>
        <translation>Informations meddelanden:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="362"/>
        <source>Copy to Clipboard</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>1 day</source>
        <translation>1 dag</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>%1 days</source>
        <translation>%1 dagar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>1 hour</source>
        <translation>1 timme</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>%1 hours</source>
        <translation>%1 timmar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="85"/>
        <source>1 minute</source>
        <translation>1 minut</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="85"/>
        <source>%1 minutes</source>
        <translation>%1 minuter</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="87"/>
        <source>1 second</source>
        <translation>1 sekund</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="87"/>
        <source>%1 seconds</source>
        <translation>%1 sekunder</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="91"/>
        <source>0.%1 seconds</source>
        <translation>0.%1 sekunder</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="93"/>
        <source> and </source>
        <translation> och </translation>
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
        <translation>Projectinställningar
	Projekt:	%1
	Sökvägar:	%2
	Include sökvägar:	%3
	Defines:	%4
Föregående analys
	Sökväg:	%5
	Antal analyserade filer:	%6
	Tid:	%7
Statistik
	Fel:	%8
	Varningar:	%9
	Stil varningar:	%10
	Portability warnings:	%11
	Prestanda varningar:	%12
	Information meddelanden:	%13</translation>
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
