<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="de_DE">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>Über Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>Version %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - Ein Werkzeug zur statischen C/C++-Code-Analyse.</translation>
    </message>
    <message utf8="true">
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-2010 Daniel Marjamäki and cppcheck team.</source>
        <oldsource>Copyright (C) 2007-2010 Daniel Marjamäki and cppcheck team.</oldsource>
        <translation type="unfinished">Copyright (C) 2007-2010 Daniel Marjamäki und das Cppcheck-Team.</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>Dieses Programm ist unter den Bedingungen
der GNU General Public License Version 3 lizenziert</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Besuchen Sie die Cppcheck-Homepage unter %1</translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="application.ui" line="17"/>
        <source>Add an application</source>
        <translation>Anwendung hinzufügen</translation>
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
        <translation>Hier können Sie Anwendungen hinzufügen, die Fehler-Dateien öffnen können.
Geben Sie einen Namen und die ausführbare Anwendung an.

Die folgenden Texte werden mit den entsprechenden Werten bei der Ausführung ersetzt:
(file) - Dateiname die den Fehler enthält
(line) - Zeilennummer mit dem Fehler
(message) - Fehlermeldung
(severity) - Schweregrad des Fehlers

Das Beispiel öffnet eine Datei mit dem Editor Kate und blättert zur richtigen Zeile:
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
        <translation>Suchen</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="56"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Ausführbare Dateien (*.exe);;Alle Dateien(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Select viewer application</source>
        <translation>Anzeigeanwendung auswählen</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="96"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="97"/>
        <source>You must specify a name and a path for the application!</source>
        <translation>Sie müssen einen Namen und einen Pfad für die Anweundung angeben!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="43"/>
        <source>Could not find the file: %1</source>
        <translation>Konnte die Datei nicht finden:%1</translation>
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
        <translation>Konnte die Datei nicht lesen:%1</translation>
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
        <source>Clear</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="logview.ui" line="55"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="main.ui" line="26"/>
        <location filename="mainwindow.cpp" line="231"/>
        <location filename="mainwindow.cpp" line="261"/>
        <location filename="mainwindow.cpp" line="535"/>
        <location filename="mainwindow.cpp" line="649"/>
        <location filename="mainwindow.cpp" line="667"/>
        <location filename="mainwindow.cpp" line="831"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="154"/>
        <source>Standard</source>
        <translation>Standard</translation>
    </message>
    <message>
        <location filename="main.ui" line="74"/>
        <source>&amp;File</source>
        <translation>&amp;Datei</translation>
    </message>
    <message>
        <location filename="main.ui" line="88"/>
        <source>&amp;View</source>
        <translation>&amp;Ansicht</translation>
    </message>
    <message>
        <location filename="main.ui" line="92"/>
        <source>&amp;Toolbars</source>
        <translation>&amp;Symbolleisten</translation>
    </message>
    <message>
        <location filename="main.ui" line="131"/>
        <source>&amp;Check</source>
        <translation>&amp;Prüfen</translation>
    </message>
    <message>
        <location filename="main.ui" line="140"/>
        <source>&amp;Edit</source>
        <translation>&amp;Bearbeiten</translation>
    </message>
    <message>
        <location filename="main.ui" line="188"/>
        <source>&amp;License...</source>
        <translation>&amp;Lizenz...</translation>
    </message>
    <message>
        <location filename="main.ui" line="193"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Autoren...</translation>
    </message>
    <message>
        <location filename="main.ui" line="202"/>
        <source>&amp;About...</source>
        <translation>Ü&amp;ber...</translation>
    </message>
    <message>
        <location filename="main.ui" line="207"/>
        <source>&amp;Files...</source>
        <translation>&amp;Dateien...</translation>
    </message>
    <message>
        <location filename="main.ui" line="210"/>
        <source>Ctrl+F</source>
        <translation>Strg+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="219"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Verzeichnis...</translation>
    </message>
    <message>
        <location filename="main.ui" line="222"/>
        <source>Ctrl+D</source>
        <translation>Strg+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="231"/>
        <source>&amp;Recheck files</source>
        <translation>Dateien &amp;neu prüfen</translation>
    </message>
    <message>
        <location filename="main.ui" line="234"/>
        <source>Ctrl+R</source>
        <translation>Strg+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="243"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stoppen</translation>
    </message>
    <message>
        <location filename="main.ui" line="246"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="255"/>
        <source>&amp;Save results to file...</source>
        <translation>Ergebnisse in Datei &amp;speichern...</translation>
    </message>
    <message>
        <location filename="main.ui" line="258"/>
        <source>Ctrl+S</source>
        <translation>Strg+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="263"/>
        <source>&amp;Quit</source>
        <translation>&amp;Beenden</translation>
    </message>
    <message>
        <location filename="main.ui" line="272"/>
        <source>&amp;Clear results</source>
        <translation>Ergebnisse &amp;leeren</translation>
    </message>
    <message>
        <location filename="main.ui" line="281"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Einstellungen</translation>
    </message>
    <message>
        <location filename="main.ui" line="439"/>
        <source>Warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="442"/>
        <location filename="main.ui" line="445"/>
        <source>Show warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="457"/>
        <source>Performance warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="460"/>
        <location filename="main.ui" line="463"/>
        <source>Show performance warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="471"/>
        <source>Show &amp;hidden</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="314"/>
        <location filename="main.ui" line="317"/>
        <source>Show common errors</source>
        <translation>Allgemeine Fehler anzeigen</translation>
    </message>
    <message>
        <location filename="main.ui" line="322"/>
        <source>&amp;Check all</source>
        <translation>Alle &amp;auswählen</translation>
    </message>
    <message>
        <location filename="main.ui" line="327"/>
        <source>&amp;Uncheck all</source>
        <translation>Alle a&amp;bwählen</translation>
    </message>
    <message>
        <location filename="main.ui" line="332"/>
        <source>Collapse &amp;all</source>
        <translation>Alle &amp;reduzieren</translation>
    </message>
    <message>
        <location filename="main.ui" line="337"/>
        <source>&amp;Expand all</source>
        <translation>Alle &amp;erweitern</translation>
    </message>
    <message>
        <location filename="main.ui" line="345"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standard</translation>
    </message>
    <message>
        <location filename="main.ui" line="348"/>
        <source>Standard items</source>
        <translation>Standardeinträge</translation>
    </message>
    <message>
        <location filename="main.ui" line="364"/>
        <source>Toolbar</source>
        <translation>Symbolleiste</translation>
    </message>
    <message>
        <location filename="main.ui" line="372"/>
        <source>&amp;Categories</source>
        <translation>&amp;Kategorien</translation>
    </message>
    <message>
        <location filename="main.ui" line="375"/>
        <source>Error categories</source>
        <translation>Fehler-Kategorien</translation>
    </message>
    <message>
        <location filename="main.ui" line="380"/>
        <source>&amp;Open XML...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="389"/>
        <source>Open P&amp;roject File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="394"/>
        <source>&amp;New Project File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="399"/>
        <source>&amp;Log View</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="402"/>
        <source>Log View</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="410"/>
        <source>C&amp;lose Project File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="418"/>
        <source>&amp;Edit Project File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="427"/>
        <source>&amp;Statistics</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="353"/>
        <source>&amp;Contents</source>
        <translation>&amp;Inhalte</translation>
    </message>
    <message>
        <location filename="main.ui" line="173"/>
        <source>Categories</source>
        <translation>Kategorien</translation>
    </message>
    <message>
        <location filename="main.ui" line="293"/>
        <source>Style warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="296"/>
        <location filename="main.ui" line="299"/>
        <source>Show style warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="311"/>
        <source>Common errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="356"/>
        <source>Open the help contents</source>
        <translation>Öffnet die Hilfe-Inhalte</translation>
    </message>
    <message>
        <location filename="main.ui" line="359"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="main.ui" line="116"/>
        <source>&amp;Language</source>
        <translation>&amp;Sprache</translation>
    </message>
    <message>
        <location filename="main.ui" line="121"/>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="277"/>
        <source>Select files to check</source>
        <translation>Dateien zum Überprüfen auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="291"/>
        <source>Select directory to check</source>
        <translation>Verzeichnis zum Überprüfen auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="232"/>
        <source>No suitable files found to check!</source>
        <translation>Kein passenden Dateien zum Überprüfen gefunden!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="574"/>
        <source>License</source>
        <translation>Lizenz</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="581"/>
        <source>Authors</source>
        <translation>Autoren</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="589"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML-Dateien (*.xml);;Textdateien (*.txt);;CSV-Dateien (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="591"/>
        <source>Save the report file</source>
        <translation>Speichert die Berichtdatei</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="464"/>
        <location filename="mainwindow.cpp" line="599"/>
        <source>XML files (*.xml)</source>
        <translation>XML-Dateien (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="262"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="334"/>
        <location filename="mainwindow.cpp" line="762"/>
        <location filename="mainwindow.cpp" line="808"/>
        <source>Project: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="466"/>
        <source>Open the report file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="531"/>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="605"/>
        <source>Text files (*.txt)</source>
        <translation>Textdateien (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="611"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-Dateien (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="651"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="668"/>
        <source>Failed to change the language:

%1

</source>
        <oldsource>Failed to change language:

%1</oldsource>
        <translation>Fehler beim Ändern der Sprache:

%1

</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="729"/>
        <location filename="mainwindow.cpp" line="738"/>
        <source>Cppcheck Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="729"/>
        <source>Failed to load help file (not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="738"/>
        <source>Failed to load help file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="752"/>
        <location filename="mainwindow.cpp" line="797"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="754"/>
        <source>Select Project File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="799"/>
        <source>Select Project Filename</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="832"/>
        <source>No project file loaded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Finnish</source>
        <translation>Finnisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="32"/>
        <source>English</source>
        <translation>Englisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="33"/>
        <source>Dutch</source>
        <translation>Niederländisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Swedish</source>
        <translation>Schwedisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>German</source>
        <translation>Deutsch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Russian</source>
        <translation>Russisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Polish</source>
        <translation>Polnisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>Japanease</source>
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
        <translation type="unfinished">Verzeichnis zum Überprüfen auswählen</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="97"/>
        <source>Incorrect language specified!</source>
        <translation>Falsche Sprache angegeben!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="107"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation>Sprachdatei %1 nicht gefunden!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="113"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Die Übersetzungen der Sprache %1 konnten nicht aus der Datei %2 geladen werden</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="962"/>
        <source>File</source>
        <translation>Datei</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="962"/>
        <source>Severity</source>
        <translation>Schweregrad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="962"/>
        <source>Line</source>
        <translation>Zeile</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="962"/>
        <source>Summary</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="108"/>
        <source>Undefined file</source>
        <translation>Undefinierte Datei</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="504"/>
        <source>Copy filename</source>
        <translation>Dateiname kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="505"/>
        <source>Copy full path</source>
        <translation>Vollständigen Pfad kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="506"/>
        <source>Copy message</source>
        <translation>Meldung kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="507"/>
        <source>Hide</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="554"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="555"/>
        <source>Configure the text file viewer program in Cppcheck preferences/Applications.</source>
        <oldsource>You can open this error by specifying applications in program&apos;s settings.</oldsource>
        <translation>Konfigurieren Sie das Text-Dateibetrachter-Programm unter Einstellungen/Anwendungen.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="582"/>
        <source>Could not find the file!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="620"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>%1 konnte nicht gestartet werden.

Bitte überprüfen Sie ob der Pfad und die Parameter der Anwendung richtig eingestellt sind.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="634"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="641"/>
        <source>Select Directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="819"/>
        <source>style</source>
        <translation>Stil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="823"/>
        <source>error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="827"/>
        <source>warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="831"/>
        <source>performance</source>
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
        <translation>Keine Fehler gefunden.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="199"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Es wurden Fehler gefunden, aber sie sind so konfiguriert, ausgeblendet zu werden.
Legen Sie unter dem Menü Ansicht fest, welche Art von Fehlern angezeigt werden sollen.</translation>
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
        <translation>Keine Fehler gefunden, nichts zu speichern.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="144"/>
        <location filename="resultsview.cpp" line="154"/>
        <source>Failed to save the report.</source>
        <translation>Der Bericht konnte nicht speichern werden.</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>Berichte</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>Allgemein</translation>
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
        <location filename="settings.ui" line="56"/>
        <source>Number of threads: </source>
        <translation>Anzahl der Threads: </translation>
    </message>
    <message>
        <location filename="settings.ui" line="78"/>
        <source>Check all #ifdef configurations</source>
        <translation>Alle #ifdef-Konfigurationen überprüfen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="85"/>
        <source>Show full path of files</source>
        <translation>Vollständigen Dateipfad anzeigen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="92"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>&quot;Keine Fehler gefunden&quot;-Meldung anzeigen, wenn keine Fehler gefunden werden</translation>
    </message>
    <message>
        <location filename="settings.ui" line="99"/>
        <source>Show internal warnings in log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="120"/>
        <source>Applications</source>
        <translation>Anwendungen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="129"/>
        <source>Add application</source>
        <translation>Anwendung hinzufügen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="136"/>
        <source>Delete application</source>
        <translation>Anwendung löschen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="143"/>
        <source>Modify application</source>
        <translation>Anwendung ändern</translation>
    </message>
    <message>
        <location filename="settings.ui" line="150"/>
        <source>Set as default application</source>
        <translation>Als Standard-Anwendung verwenden</translation>
    </message>
    <message>
        <location filename="settings.ui" line="158"/>
        <source>Reports</source>
        <translation>Berichte</translation>
    </message>
    <message>
        <location filename="settings.ui" line="164"/>
        <source>Save all errors when creating report</source>
        <translation>Alle Fehler beim Erstellen von Berichten speichern</translation>
    </message>
    <message>
        <location filename="settings.ui" line="171"/>
        <source>Save full path to files in reports</source>
        <translation>Vollständigen Dateipfad in Berichten speichern</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="136"/>
        <source>Add a new application</source>
        <translation>Neue Anwendung hinzufügen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="169"/>
        <source>Modify an application</source>
        <translation>Anwendung ändern</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="234"/>
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
        <source>Performance issues:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="326"/>
        <source>Copy to Clipboard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="69"/>
        <source>%1 secs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="78"/>
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
	Performance warnings:	%11
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="107"/>
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
 &lt;tr&gt;&lt;th&gt;Performance warnings:&lt;/th&gt;&lt;td&gt;%11&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;
</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
