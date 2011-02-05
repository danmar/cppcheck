<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="nl_BE">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>Over Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>Versie %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - Een tool voor statische C/C++ code analyse.</translation>
    </message>
    <message utf8="true">
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-2010 Daniel Marjamäki and cppcheck team.</source>
        <oldsource>Copyright (C) 2007-2010 Daniel Marjamäki and cppcheck team.</oldsource>
        <translation type="unfinished">Copyright (C) 2007-2009 Daniel Marjamäki en het cppcheck.</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>Dit programma is beschikbaar onder te termen
van de GNU General Public License versie 3</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Bezoek de Cppcheck homepage op %1</translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="application.ui" line="17"/>
        <source>Add an application</source>
        <translation>Voeg een nieuwe applicatie toe</translation>
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
        <translation type="unfinished">Hier kan men applicaties toevoegen om bestanden met fouten te openen.
Specifieer een naam voor de applicatie en de applicatie om uit te voeren.

De volgende tekst wordt vervangen door geschikte waarden wanneer de applicatie uitgevoerd wordt:
(file) - Bestandsnaam van het bestand dat de fout bevat
(line) - Regelnummer van de fout
(message) - Foutboodschap
(severity) - Ernst van de fout

Bijvoorbeeld: om het bestand met Kate te openen en naar de correcte regel te gaan:
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
        <translation>Bladeren</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="56"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Uitvoerbare bestanden (*.exe);;Alle bestanden(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Select viewer application</source>
        <translation>Selecteer applicatie</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="96"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="97"/>
        <source>You must specify a name and a path for the application!</source>
        <translation>Een naam en pad voor de applicatie zijn vereist!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="43"/>
        <source>Could not find the file: %1</source>
        <oldsource>Could not find the file:
</oldsource>
        <translation>Kon het bestand niet vinden: %1</translation>
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
        <translation>Kon het bestand niet lezen: %1</translation>
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
        <location filename="mainwindow.cpp" line="203"/>
        <location filename="mainwindow.cpp" line="233"/>
        <location filename="mainwindow.cpp" line="524"/>
        <location filename="mainwindow.cpp" line="648"/>
        <location filename="mainwindow.cpp" line="666"/>
        <location filename="mainwindow.cpp" line="817"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="150"/>
        <source>Standard</source>
        <translation>Standaard</translation>
    </message>
    <message>
        <location filename="main.ui" line="74"/>
        <source>&amp;File</source>
        <translation>&amp;Bestand</translation>
    </message>
    <message>
        <location filename="main.ui" line="88"/>
        <source>&amp;View</source>
        <translation>&amp;Weergave</translation>
    </message>
    <message>
        <location filename="main.ui" line="92"/>
        <source>&amp;Toolbars</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="128"/>
        <source>&amp;Check</source>
        <translation>&amp;Controleer</translation>
    </message>
    <message>
        <location filename="main.ui" line="137"/>
        <source>&amp;Edit</source>
        <translation>Be&amp;werken</translation>
    </message>
    <message>
        <location filename="main.ui" line="186"/>
        <source>&amp;License...</source>
        <translation>&amp;Licentie...</translation>
    </message>
    <message>
        <location filename="main.ui" line="191"/>
        <source>A&amp;uthors...</source>
        <translation>A&amp;uteurs...</translation>
    </message>
    <message>
        <location filename="main.ui" line="200"/>
        <source>&amp;About...</source>
        <translation>&amp;Over...</translation>
    </message>
    <message>
        <location filename="main.ui" line="205"/>
        <source>&amp;Files...</source>
        <translation>&amp;Bestanden...</translation>
    </message>
    <message>
        <location filename="main.ui" line="208"/>
        <location filename="main.ui" line="211"/>
        <source>Check files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="214"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="223"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Mappen...</translation>
    </message>
    <message>
        <location filename="main.ui" line="226"/>
        <location filename="main.ui" line="229"/>
        <source>Check directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="232"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="241"/>
        <source>&amp;Recheck files</source>
        <translation>&amp;Opnieuw controleren</translation>
    </message>
    <message>
        <location filename="main.ui" line="244"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="253"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stop</translation>
    </message>
    <message>
        <location filename="main.ui" line="256"/>
        <location filename="main.ui" line="259"/>
        <source>Stop checking</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="262"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="271"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Resultaten opslaan...</translation>
    </message>
    <message>
        <location filename="main.ui" line="274"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="279"/>
        <source>&amp;Quit</source>
        <translation>&amp;Afsluiten</translation>
    </message>
    <message>
        <location filename="main.ui" line="288"/>
        <source>&amp;Clear results</source>
        <translation>&amp;Resultaten wissen</translation>
    </message>
    <message>
        <location filename="main.ui" line="297"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Voorkeuren</translation>
    </message>
    <message>
        <location filename="main.ui" line="327"/>
        <source>Errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="330"/>
        <location filename="main.ui" line="333"/>
        <source>Show errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="455"/>
        <source>Warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="458"/>
        <location filename="main.ui" line="461"/>
        <source>Show warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="473"/>
        <source>Performance warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="476"/>
        <location filename="main.ui" line="479"/>
        <source>Show performance warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="487"/>
        <source>Show &amp;hidden</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="499"/>
        <source>Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="502"/>
        <source>Show information messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="514"/>
        <source>Portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="517"/>
        <source>Show portability warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="338"/>
        <source>&amp;Check all</source>
        <translation>&amp;Selecteer alles</translation>
    </message>
    <message>
        <location filename="main.ui" line="343"/>
        <source>&amp;Uncheck all</source>
        <translation>Selecteer &amp;niets</translation>
    </message>
    <message>
        <location filename="main.ui" line="348"/>
        <source>Collapse &amp;all</source>
        <translation>Alles Inkl&amp;appen</translation>
    </message>
    <message>
        <location filename="main.ui" line="353"/>
        <source>&amp;Expand all</source>
        <translation>Alles &amp;Uitklappen</translation>
    </message>
    <message>
        <location filename="main.ui" line="361"/>
        <source>&amp;Standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="364"/>
        <source>Standard items</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="380"/>
        <source>Toolbar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="388"/>
        <source>&amp;Categories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="391"/>
        <source>Error categories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="396"/>
        <source>&amp;Open XML...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="405"/>
        <source>Open P&amp;roject File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="410"/>
        <source>&amp;New Project File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="415"/>
        <source>&amp;Log View</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="418"/>
        <source>Log View</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="426"/>
        <source>C&amp;lose Project File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="434"/>
        <source>&amp;Edit Project File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="443"/>
        <source>&amp;Statistics</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="369"/>
        <source>&amp;Contents</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="169"/>
        <source>Categories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="309"/>
        <source>Style warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="312"/>
        <location filename="main.ui" line="315"/>
        <source>Show style warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="372"/>
        <source>Open the help contents</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="375"/>
        <source>F1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="118"/>
        <source>&amp;Help</source>
        <translation>&amp;Help</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="249"/>
        <source>Select files to check</source>
        <translation>Selecteer bestanden om te controleren</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="263"/>
        <source>Select directory to check</source>
        <translation>Selecteer een map om te controleren</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="204"/>
        <source>No suitable files found to check!</source>
        <translation>Geen geschikte bestanden gevonden om te controleren!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="567"/>
        <source>License</source>
        <translation>Licentie</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="574"/>
        <source>Authors</source>
        <translation>Auteurs</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="582"/>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation type="unfinished">XML bestanden (*.xml);;Tekst bestanden (*.txt);;CSV bestanden (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="584"/>
        <source>Save the report file</source>
        <translation>Rapport opslaan </translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="443"/>
        <source>XML files (*.xml)</source>
        <translation>XML bestanden (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="234"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="306"/>
        <location filename="mainwindow.cpp" line="748"/>
        <location filename="mainwindow.cpp" line="794"/>
        <source>Project: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="445"/>
        <source>Open the report file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="520"/>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="604"/>
        <source>Text files (*.txt)</source>
        <translation>Tekst bestanden (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="610"/>
        <source>CSV files (*.csv)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="650"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="667"/>
        <source>Failed to change the language:

%1

</source>
        <oldsource>Failed to change language:

%1</oldsource>
        <translation>Kon de taal niet wisselen:

%1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="715"/>
        <location filename="mainwindow.cpp" line="724"/>
        <source>Cppcheck Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="715"/>
        <source>Failed to load help file (not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="724"/>
        <source>Failed to load help file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="738"/>
        <location filename="mainwindow.cpp" line="783"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="740"/>
        <source>Select Project File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="785"/>
        <source>Select Project Filename</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="818"/>
        <source>No project file loaded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="33"/>
        <source>Finnish</source>
        <translation>Fins</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="31"/>
        <source>English</source>
        <translation>Engels</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="32"/>
        <source>Dutch</source>
        <translation>Nederlands</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Swedish</source>
        <translation>Zweeds</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>German</source>
        <translation>Duits</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>Russian</source>
        <translation>Russisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Polish</source>
        <translation>Pools</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Japanese</source>
        <oldsource>Japanease</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
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
        <translation type="unfinished">Selecteer een map om te controleren</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="96"/>
        <source>Incorrect language specified!</source>
        <translation>Ongeldige taal gespecifieerd!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="106"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation>Kon het taalbestand niet vinden: %1!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="112"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Kon de vertaling voor taal %1 in bestand %2 niet laden</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="978"/>
        <source>File</source>
        <translation>Bestand</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="978"/>
        <source>Severity</source>
        <translation>Ernst</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="978"/>
        <source>Line</source>
        <translation>Regel</translation>
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
        <translation>Niet gedefinieerd bestand</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="508"/>
        <source>Copy filename</source>
        <translation>Kopier bestandsnaam</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="509"/>
        <source>Copy full path</source>
        <translation>Kopieer volledig pad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="510"/>
        <source>Copy message</source>
        <translation type="unfinished"></translation>
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
        <translation>U dient een applicatie te configureren in de instellingen om deze fout in te openen.</translation>
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
        <translation>Kon applicatie %1 niet starten

Gelieve te controleren of de het pad en de parameters correct zijn.</translation>
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
        <translation>Stijlfouten</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="831"/>
        <source>error</source>
        <translation>Fouten</translation>
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
        <location filename="resultsview.cpp" line="195"/>
        <location filename="resultsview.cpp" line="207"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="196"/>
        <source>No errors found.</source>
        <translation>Geen fouten gevonden.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="204"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Fouten werden gevonden, maar volgens de configuratie zijn deze verborgen.
Gebruik het uitzicht menu om te selecteren welke fouten getoond worden.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="251"/>
        <location filename="resultsview.cpp" line="271"/>
        <location filename="resultsview.cpp" line="281"/>
        <source>Failed to read the report.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="321"/>
        <source>Summary</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="322"/>
        <source>Message</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="119"/>
        <source>No errors found, nothing to save.</source>
        <translation>Geen fouten gevonden; geen data om op te slaan.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="149"/>
        <location filename="resultsview.cpp" line="159"/>
        <source>Failed to save the report.</source>
        <translation>Kon het rapport niet opslaan.</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>Resultaten</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>Instellingen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>Algemeen</translation>
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
        <translation>Aantal threads: </translation>
    </message>
    <message>
        <location filename="settings.ui" line="109"/>
        <source>Ideal count:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="116"/>
        <source>TextLabel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="138"/>
        <source>Force checking all #ifdef configurations</source>
        <oldsource>Check all #ifdef configurations</oldsource>
        <translation type="unfinished">Controleer alle #ifdef combinaties</translation>
    </message>
    <message>
        <location filename="settings.ui" line="145"/>
        <source>Show full path of files</source>
        <translation>Toon het volledige pad van bestanden</translation>
    </message>
    <message>
        <location filename="settings.ui" line="152"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Toon &quot;Geen fouten gevonden&quot; indien geen fouten gevonden werden</translation>
    </message>
    <message>
        <location filename="settings.ui" line="159"/>
        <source>Show internal warnings in log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="166"/>
        <source>Enable inline suppressions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="187"/>
        <source>Applications</source>
        <translation>Applicaties</translation>
    </message>
    <message>
        <location filename="settings.ui" line="196"/>
        <source>Add application</source>
        <translation>Applicatie toevoegen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="203"/>
        <source>Delete application</source>
        <translation>Applicatie verwijderen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="210"/>
        <source>Modify application</source>
        <translation>Applicatie wijzigen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="217"/>
        <source>Set as default application</source>
        <translation>Configureer als standaard applicatie</translation>
    </message>
    <message>
        <location filename="settings.ui" line="225"/>
        <source>Reports</source>
        <translation>Rapporten</translation>
    </message>
    <message>
        <location filename="settings.ui" line="231"/>
        <source>Save all errors when creating report</source>
        <translation>Alle fouten opslaan</translation>
    </message>
    <message>
        <location filename="settings.ui" line="238"/>
        <source>Save full path to files in reports</source>
        <translation>Volledig pad opslaan</translation>
    </message>
    <message>
        <location filename="settings.ui" line="259"/>
        <source>Language</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="81"/>
        <source>N/A</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="161"/>
        <source>Add a new application</source>
        <translation>Nieuwe applicatie toevoegen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="194"/>
        <source>Modify an application</source>
        <translation>Applicatie wijzigen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="259"/>
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
