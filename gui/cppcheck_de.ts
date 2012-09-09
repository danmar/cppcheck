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
        <source>Copyright © 2007-2012 Daniel Marjamäki and cppcheck team.</source>
        <translation>Copyright © 2007-2012 Daniel Marjamäki und das Cppcheck-Team.</translation>
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
        <location filename="application.ui" line="23"/>
        <source>Add an application</source>
        <translation>Anwendung hinzufügen</translation>
    </message>
    <message>
        <location filename="application.ui" line="41"/>
        <source>Here you can add an application that can open error files. Specify a name for the application, the application executable and command line parameters for the application.

The following texts in parameters are replaced with appropriate values when application is executed:
(file) - Filename containing the error
(line) - Line number containing the error
(message) - Error message
(severity) - Error severity

Example opening a file with Kate and make Kate scroll to the correct line:
Executable: kate
Parameters: -l(line) (file)</source>
        <translation>Hier können Sie Anwendungen hinzufügen, die Codedateien öffnen können. Geben Sie den Namen der Anwendung, deren ausführbare Datei und Kommandozeilenparameter für die Ausführung an.

Die folgenden Texte in Paramtern werden durch die passenden Werte ersetzt, wenn die Anwendung ausgeführt wird:
(file) - Name der Datei, die den Fehler enthält
(line) - Zeile, die den Fehler enthält
(message) - Fehlermeldung
(severity) - Schweregrad des Fehlers

Beispiel: Öffnen einer Datei mit Kate, automatisch zur korrekten Zeile scrollen:
Ausführbare Datei: kate
Parameter: -l(line) (file)
</translation>
    </message>
    <message>
        <location filename="application.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>&amp;Name:</translation>
    </message>
    <message>
        <location filename="application.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>&amp;Ausführbare Datei:</translation>
    </message>
    <message>
        <location filename="application.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>&amp;Parameter:</translation>
    </message>
    <message>
        <location filename="application.ui" line="138"/>
        <source>Browse</source>
        <translation>Suchen</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="58"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Ausführbare Dateien (*.exe);;Alle Dateien(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="61"/>
        <source>Select viewer application</source>
        <translation>Anzeigeanwendung auswählen</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="85"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="86"/>
        <source>You must specify a name, a path and parameters for the application!</source>
        <translation>Sie müssen einen Namen, einen Pfad und Parameter für die Anwendung angeben!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="42"/>
        <source>Could not find the file: %1</source>
        <translation>Konnte die Datei nicht finden: %1</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="46"/>
        <location filename="fileviewdialog.cpp" line="60"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="56"/>
        <source>Could not read the file: %1</source>
        <translation>Konnte die Datei nicht lesen: %1</translation>
    </message>
</context>
<context>
    <name>LogView</name>
    <message>
        <location filename="logview.ui" line="17"/>
        <source>Checking Log</source>
        <translation>Untersuchungs-Log</translation>
    </message>
    <message>
        <location filename="logview.ui" line="48"/>
        <source>&amp;Save</source>
        <translation>&amp;Speichern</translation>
    </message>
    <message>
        <location filename="logview.ui" line="55"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="logview.ui" line="62"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="66"/>
        <source>Save Log</source>
        <translation>Speichere Log</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="67"/>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation>Textdateien (*.txt *.log);;Alle Dateien(*.*)</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="71"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="72"/>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation>Datei kann nicht zum Schreiben geöffnet werden: &quot;%1&quot;</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="main.ui" line="26"/>
        <location filename="mainwindow.cpp" line="248"/>
        <location filename="mainwindow.cpp" line="309"/>
        <location filename="mainwindow.cpp" line="339"/>
        <location filename="mainwindow.cpp" line="406"/>
        <location filename="mainwindow.cpp" line="428"/>
        <location filename="mainwindow.cpp" line="616"/>
        <location filename="mainwindow.cpp" line="707"/>
        <location filename="mainwindow.cpp" line="826"/>
        <location filename="mainwindow.cpp" line="846"/>
        <location filename="mainwindow.cpp" line="987"/>
        <location filename="mainwindow.cpp" line="1068"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="158"/>
        <source>Standard</source>
        <translation>Standard</translation>
    </message>
    <message>
        <location filename="main.ui" line="74"/>
        <source>&amp;File</source>
        <translation>&amp;Datei</translation>
    </message>
    <message>
        <location filename="main.ui" line="89"/>
        <source>&amp;View</source>
        <translation>&amp;Ansicht</translation>
    </message>
    <message>
        <location filename="main.ui" line="93"/>
        <source>&amp;Toolbars</source>
        <translation>&amp;Symbolleisten</translation>
    </message>
    <message>
        <location filename="main.ui" line="130"/>
        <source>&amp;Check</source>
        <translation>&amp;Prüfen</translation>
    </message>
    <message>
        <location filename="main.ui" line="145"/>
        <source>&amp;Edit</source>
        <translation>&amp;Bearbeiten</translation>
    </message>
    <message>
        <location filename="main.ui" line="205"/>
        <source>&amp;License...</source>
        <translation>&amp;Lizenz...</translation>
    </message>
    <message>
        <location filename="main.ui" line="210"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Autoren...</translation>
    </message>
    <message>
        <location filename="main.ui" line="219"/>
        <source>&amp;About...</source>
        <translation>Ü&amp;ber...</translation>
    </message>
    <message>
        <location filename="main.ui" line="224"/>
        <source>&amp;Files...</source>
        <translation>&amp;Dateien...</translation>
    </message>
    <message>
        <location filename="main.ui" line="227"/>
        <location filename="main.ui" line="230"/>
        <source>Check files</source>
        <translation>Prüfe Dateien</translation>
    </message>
    <message>
        <location filename="main.ui" line="233"/>
        <source>Ctrl+F</source>
        <translation>Strg+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="242"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Verzeichnis...</translation>
    </message>
    <message>
        <location filename="main.ui" line="245"/>
        <location filename="main.ui" line="248"/>
        <source>Check directory</source>
        <translation>Prüfe Verzeichnis</translation>
    </message>
    <message>
        <location filename="main.ui" line="251"/>
        <source>Ctrl+D</source>
        <translation>Strg+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="260"/>
        <source>&amp;Recheck files</source>
        <translation>Dateien &amp;neu prüfen</translation>
    </message>
    <message>
        <location filename="main.ui" line="263"/>
        <source>Ctrl+R</source>
        <translation>Strg+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="272"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stoppen</translation>
    </message>
    <message>
        <location filename="main.ui" line="275"/>
        <location filename="main.ui" line="278"/>
        <source>Stop checking</source>
        <translation>Prüfung abbrechen</translation>
    </message>
    <message>
        <location filename="main.ui" line="281"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="290"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Ergebnisse in Datei speichern...</translation>
    </message>
    <message>
        <location filename="main.ui" line="293"/>
        <source>Ctrl+S</source>
        <translation>Strg+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="298"/>
        <source>&amp;Quit</source>
        <translation>&amp;Beenden</translation>
    </message>
    <message>
        <location filename="main.ui" line="307"/>
        <source>&amp;Clear results</source>
        <translation>Ergebnisse &amp;löschen</translation>
    </message>
    <message>
        <location filename="main.ui" line="316"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Einstellungen</translation>
    </message>
    <message>
        <location filename="main.ui" line="346"/>
        <source>Errors</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="main.ui" line="349"/>
        <location filename="main.ui" line="352"/>
        <source>Show errors</source>
        <translation>Zeige Fehler</translation>
    </message>
    <message>
        <location filename="main.ui" line="474"/>
        <source>Warnings</source>
        <translation>Warnungen</translation>
    </message>
    <message>
        <location filename="main.ui" line="477"/>
        <location filename="main.ui" line="480"/>
        <source>Show warnings</source>
        <translation>Zeige Warnungen</translation>
    </message>
    <message>
        <location filename="main.ui" line="492"/>
        <source>Performance warnings</source>
        <translation>Performance-Warnungen</translation>
    </message>
    <message>
        <location filename="main.ui" line="495"/>
        <location filename="main.ui" line="498"/>
        <source>Show performance warnings</source>
        <translation>Zeige Performance-Warnungen</translation>
    </message>
    <message>
        <location filename="main.ui" line="506"/>
        <source>Show &amp;hidden</source>
        <translation>Zeige &amp;versteckte</translation>
    </message>
    <message>
        <location filename="main.ui" line="518"/>
        <source>Information</source>
        <translation>Information</translation>
    </message>
    <message>
        <location filename="main.ui" line="521"/>
        <source>Show information messages</source>
        <translation>Zeige Informationsmeldungen</translation>
    </message>
    <message>
        <location filename="main.ui" line="533"/>
        <source>Portability</source>
        <translation>Portabilität</translation>
    </message>
    <message>
        <location filename="main.ui" line="536"/>
        <source>Show portability warnings</source>
        <translation>Zeige Portabilitätswarnungen</translation>
    </message>
    <message>
        <location filename="main.ui" line="544"/>
        <source>&amp;Filter</source>
        <translation>&amp;Filter</translation>
    </message>
    <message>
        <location filename="main.ui" line="547"/>
        <source>Filter results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="552"/>
        <source>Project MRU placeholder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="563"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit, ANSI</translation>
    </message>
    <message>
        <location filename="main.ui" line="571"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit, Unicode</translation>
    </message>
    <message>
        <location filename="main.ui" line="579"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="main.ui" line="587"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="main.ui" line="595"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <location filename="main.ui" line="603"/>
        <source>Platforms</source>
        <translation>Platformen</translation>
    </message>
    <message>
        <location filename="main.ui" line="614"/>
        <source>C++11</source>
        <translation>C++11</translation>
    </message>
    <message>
        <location filename="main.ui" line="622"/>
        <source>C99</source>
        <translation>C99</translation>
    </message>
    <message>
        <location filename="main.ui" line="630"/>
        <source>Posix</source>
        <translation>Posix</translation>
    </message>
    <message>
        <location filename="main.ui" line="357"/>
        <source>&amp;Check all</source>
        <translation>Alle &amp;auswählen</translation>
    </message>
    <message>
        <location filename="main.ui" line="194"/>
        <source>Filter</source>
        <translation>Filter</translation>
    </message>
    <message>
        <location filename="main.ui" line="362"/>
        <source>&amp;Uncheck all</source>
        <translation>Alle a&amp;bwählen</translation>
    </message>
    <message>
        <location filename="main.ui" line="367"/>
        <source>Collapse &amp;all</source>
        <translation>Alle &amp;reduzieren</translation>
    </message>
    <message>
        <location filename="main.ui" line="372"/>
        <source>&amp;Expand all</source>
        <translation>Alle &amp;erweitern</translation>
    </message>
    <message>
        <location filename="main.ui" line="380"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standard</translation>
    </message>
    <message>
        <location filename="main.ui" line="383"/>
        <source>Standard items</source>
        <translation>Standardeinträge</translation>
    </message>
    <message>
        <location filename="main.ui" line="399"/>
        <source>Toolbar</source>
        <translation>Symbolleiste</translation>
    </message>
    <message>
        <location filename="main.ui" line="407"/>
        <source>&amp;Categories</source>
        <translation>&amp;Kategorien</translation>
    </message>
    <message>
        <location filename="main.ui" line="410"/>
        <source>Error categories</source>
        <translation>Fehler-Kategorien</translation>
    </message>
    <message>
        <location filename="main.ui" line="415"/>
        <source>&amp;Open XML...</source>
        <translation>Öffne &amp;XML...</translation>
    </message>
    <message>
        <location filename="main.ui" line="424"/>
        <source>Open P&amp;roject File...</source>
        <translation>Pr&amp;ojektdatei öffnen...</translation>
    </message>
    <message>
        <location filename="main.ui" line="429"/>
        <source>&amp;New Project File...</source>
        <translation>&amp;Neue Projektdatei...</translation>
    </message>
    <message>
        <location filename="main.ui" line="434"/>
        <source>&amp;Log View</source>
        <translation>&amp;Loganzeige</translation>
    </message>
    <message>
        <location filename="main.ui" line="437"/>
        <source>Log View</source>
        <translation>Loganzeige</translation>
    </message>
    <message>
        <location filename="main.ui" line="445"/>
        <source>C&amp;lose Project File</source>
        <translation>Projektdatei &amp;schließen</translation>
    </message>
    <message>
        <location filename="main.ui" line="453"/>
        <source>&amp;Edit Project File...</source>
        <translation>Projektdatei &amp;bearbeiten...</translation>
    </message>
    <message>
        <location filename="main.ui" line="462"/>
        <source>&amp;Statistics</source>
        <translation>&amp;Statistik</translation>
    </message>
    <message>
        <location filename="main.ui" line="388"/>
        <source>&amp;Contents</source>
        <translation>&amp;Inhalte</translation>
    </message>
    <message>
        <location filename="main.ui" line="177"/>
        <source>Categories</source>
        <translation>Kategorien</translation>
    </message>
    <message>
        <location filename="main.ui" line="328"/>
        <source>Style warnings</source>
        <translation>Stilwarnungen</translation>
    </message>
    <message>
        <location filename="main.ui" line="331"/>
        <location filename="main.ui" line="334"/>
        <source>Show style warnings</source>
        <translation>Zeige Stilwarnungen</translation>
    </message>
    <message>
        <location filename="main.ui" line="391"/>
        <source>Open the help contents</source>
        <translation>Öffnet die Hilfe-Inhalte</translation>
    </message>
    <message>
        <location filename="main.ui" line="394"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="main.ui" line="120"/>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="371"/>
        <source>Select directory to check</source>
        <translation>Verzeichnis zum Überprüfen auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="310"/>
        <source>No suitable files found to check!</source>
        <translation>Keine passenden Dateien zum Überprüfen gefunden!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="71"/>
        <source>Quick Filter:</source>
        <translation>Schnellfilter:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="407"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>Gefundene Projektdatei: %1

Möchten Sie stattdessen diese öffnen?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="429"/>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation>Projektdatei im Verzeichnis gefunden.

Möchten Sie die Prüfung wirklich durchführen, ohne eine Projektdatei zu verwenden?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="749"/>
        <source>License</source>
        <translation>Lizenz</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="756"/>
        <source>Authors</source>
        <translation>Autoren</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="764"/>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML-Dateien Version 2 (*.xml);;XML-Dateien Version 1 (*.xml);;Textdateien (*.txt);;CSV-Dateien (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="766"/>
        <source>Save the report file</source>
        <translation>Speichert die Berichtdatei</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="632"/>
        <source>XML files (*.xml)</source>
        <translation>XML-Dateien (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="243"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>Beim Laden der Editor-Anwendungseinstellungen trat ein Problem auf.

Dies wurde vermutlich durch einen Wechsel der Cppcheck-Version hervorgerufen. Bitte prüfen (und korrigieren) Sie die Einstellungen, andernfalls könnte die Editor-Anwendung nicht korrekt starten.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="340"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Sie müssen die Projektdatei schließen, bevor Sie neue Dateien oder Verzeichnisse auswählen!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="617"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation>Aktuelle Ergebnisse werden gelöscht.

          Das Einlesen einer XML Datei löscht die aktuellen Ergebnisse. Fortfahren?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="634"/>
        <source>Open the report file</source>
        <translation>Berichtdatei öffnen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="703"/>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?.</source>
        <translation>Prüfung läuft.

Möchten Sie die Prüfung abbrechen und Cppcheck beenden?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="773"/>
        <source>XML files version 1 (*.xml)</source>
        <translation>XML-Dateien Version 1 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="777"/>
        <source>XML files version 2 (*.xml)</source>
        <translation>XML-Dateien Version 2 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="781"/>
        <source>Text files (*.txt)</source>
        <translation>Textdateien (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="785"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-Dateien (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="828"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="840"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>Wechsel der Sprache der Benutzeroberfläche fehlgeschlagen:

%1

Die Sprache wurde auf Englisch zurückgesetzt. Öffnen Sie den Einstellungen-Dialog um eine verfügbare Sprache auszuwählen.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="884"/>
        <location filename="mainwindow.cpp" line="951"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Projektdateien (*.cppcheck);;Alle Dateien(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="886"/>
        <source>Select Project File</source>
        <translation>Projektdatei auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="903"/>
        <location filename="mainwindow.cpp" line="963"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="953"/>
        <source>Select Project Filename</source>
        <translation>Projektnamen auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="988"/>
        <source>No project file loaded</source>
        <translation>Keine Projektdatei geladen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1063"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>Die Projektdatei

%1

 konnte nicht gefunden werden!

Möchten Sie die Datei von der Liste der zuletzt benutzten Projekte entfernen?</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Finnish</source>
        <translation>Finnisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="33"/>
        <source>English</source>
        <translation>Englisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="32"/>
        <source>Dutch</source>
        <translation>Niederländisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>French</source>
        <translation>Französisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Korean</source>
        <translation>Koreanisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>Spanish</source>
        <translation>Spanisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="42"/>
        <source>Swedish</source>
        <translation>Schwedisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>German</source>
        <translation>Deutsch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>Russian</source>
        <translation>Russisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Japanese</source>
        <translation>Japanisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Serbian</source>
        <translation>Serbisch</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <location filename="platforms.cpp" line="37"/>
        <source>Build-in</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="39"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="40"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit, ANSI</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="41"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit, Unicode</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="42"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
</context>
<context>
    <name>Project</name>
    <message>
        <location filename="project.cpp" line="71"/>
        <location filename="project.cpp" line="115"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="project.cpp" line="72"/>
        <source>Could not read the project file.</source>
        <translation>Projektdatei konnte nicht gelesen werden.</translation>
    </message>
    <message>
        <location filename="project.cpp" line="116"/>
        <source>Could not write the project file.</source>
        <translation>Projektdatei konnte nicht geschrieben werden.</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation>Projektdatei</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="51"/>
        <source>Root:</source>
        <translation>Wurzel:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="70"/>
        <location filename="projectfile.ui" line="221"/>
        <source>Paths:</source>
        <translation>Pfade:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="97"/>
        <location filename="projectfile.ui" line="162"/>
        <location filename="projectfile.ui" line="235"/>
        <source>Add...</source>
        <translation>Hinzufügen...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="104"/>
        <location filename="projectfile.ui" line="169"/>
        <location filename="projectfile.ui" line="242"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="111"/>
        <location filename="projectfile.ui" line="176"/>
        <location filename="projectfile.ui" line="249"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="138"/>
        <source>Includes</source>
        <translation>Includes</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="144"/>
        <source>Include directories:</source>
        <translation>Include-Verzeichnisse:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="196"/>
        <source>Up</source>
        <translation>Auf</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="203"/>
        <source>Down</source>
        <translation>Ab</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="215"/>
        <source>Exclude</source>
        <translation>Ausschließen</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="34"/>
        <source>Defines:</source>
        <translation>Definitionen:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="39"/>
        <source>Project file: %1</source>
        <translation>Projektdatei: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="210"/>
        <source>Select include directory</source>
        <translation>Wähle Include-Verzeichnisse</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="233"/>
        <source>Select a directory to check</source>
        <translation>Wähle zu prüfendes Verzeichnis</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="273"/>
        <source>Select directory to ignore</source>
        <translation>Wähle zu ingorierendes Verzeichnis</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="76"/>
        <source>Unknown language specified!</source>
        <translation>Unbekannte Sprache angegeben!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="88"/>
        <source>Language file %1 not found!</source>
        <translation>Sprachdatei %1 nicht gefunden!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="94"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>Die Übersetzungen der Sprache %1 konnten nicht aus der Datei %2 geladen werden</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="58"/>
        <location filename="resultstree.cpp" line="990"/>
        <source>File</source>
        <translation>Datei</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="58"/>
        <location filename="resultstree.cpp" line="990"/>
        <source>Severity</source>
        <translation>Schweregrad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="58"/>
        <location filename="resultstree.cpp" line="990"/>
        <source>Line</source>
        <translation>Zeile</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="58"/>
        <location filename="resultstree.cpp" line="990"/>
        <source>Summary</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="106"/>
        <source>Undefined file</source>
        <translation>Undefinierte Datei</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="208"/>
        <location filename="resultstree.cpp" line="731"/>
        <source>[Inconclusive]</source>
        <translation>[unklar]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="282"/>
        <source>debug</source>
        <translation>Debug</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="546"/>
        <source>Copy filename</source>
        <translation>Dateiname kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="547"/>
        <source>Copy full path</source>
        <translation>Vollständigen Pfad kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="548"/>
        <source>Copy message</source>
        <translation>Meldung kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="549"/>
        <source>Hide</source>
        <translation>Verstecken</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="592"/>
        <location filename="resultstree.cpp" line="606"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="593"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <translation>Keine Editor-Anwendung eingestellt.

Konfigurieren Sie diese unter Einstellungen/Anwendungen.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="607"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>Keine Standard-Editor-Anwendung eingestellt.

 Bitte wählen Sie eine Standardanwendung unter Einstellungen/Anwendungen.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="636"/>
        <source>Could not find the file!</source>
        <translation>Datei konnte nicht gefunden werden!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="682"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>%1 konnte nicht gestartet werden.

Bitte überprüfen Sie ob der Pfad und die Parameter der Anwendung richtig eingestellt sind.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="696"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>Datei konnte nicht gefunden werden:
%1
Bitte wählen Sie das Verzeichnis, in dem sich die Datei befindet.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="703"/>
        <source>Select Directory</source>
        <translation>Wähle Verzeichnis</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="258"/>
        <source>style</source>
        <translation>Stil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="262"/>
        <source>error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="266"/>
        <source>warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="270"/>
        <source>performance</source>
        <translation>Performance</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="274"/>
        <source>portability</source>
        <translation>Portabilität</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="278"/>
        <source>information</source>
        <translation>Information</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="202"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 von %2 Dateien geprüft)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="215"/>
        <location filename="resultsview.cpp" line="226"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="216"/>
        <source>No errors found.</source>
        <translation>Keine Fehler gefunden.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="223"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Es wurden Fehler gefunden, aber sie sind so konfiguriert, ausgeblendet zu werden.
Legen Sie unter dem Menü Ansicht fest, welche Art von Fehlern angezeigt werden sollen.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="269"/>
        <location filename="resultsview.cpp" line="287"/>
        <location filename="resultsview.cpp" line="295"/>
        <source>Failed to read the report.</source>
        <translation>Lesen des Berichts fehlgeschlagen.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="332"/>
        <source>Summary</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="333"/>
        <source>Message</source>
        <translation>Meldung</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="142"/>
        <source>No errors found, nothing to save.</source>
        <translation>Keine Fehler gefunden, nichts zu speichern.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="169"/>
        <location filename="resultsview.cpp" line="177"/>
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
    <name>SelectFilesDialog</name>
    <message>
        <location filename="selectfilesdialog.ui" line="14"/>
        <source>Select files to check</source>
        <translation>Dateien zum Überprüfen auswählen</translation>
    </message>
    <message>
        <location filename="selectfilesdialog.cpp" line="232"/>
        <source>Check</source>
        <translation>Prüfe</translation>
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
        <location filename="settings.ui" line="162"/>
        <source>Include paths:</source>
        <translation>Include-Pfade:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="180"/>
        <location filename="settings.ui" line="232"/>
        <source>Add...</source>
        <translation>Hinzufügen...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>Anzahl der Threads: </translation>
    </message>
    <message>
        <location filename="settings.ui" line="85"/>
        <source>Ideal count:</source>
        <translation>Ideale Anzahl:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="92"/>
        <source>TextLabel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="114"/>
        <source>Force checking all #ifdef configurations</source>
        <translation>Erzwinge Prüfung aller #ifdef-Konfigurationen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Show full path of files</source>
        <translation>Vollständigen Dateipfad anzeigen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="128"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>&quot;Keine Fehler gefunden&quot;-Meldung anzeigen, wenn keine Fehler gefunden werden</translation>
    </message>
    <message>
        <location filename="settings.ui" line="135"/>
        <source>Enable inline suppressions</source>
        <translation>Inline-Fehlerunterdrückung aktivieren</translation>
    </message>
    <message>
        <location filename="settings.ui" line="156"/>
        <source>Paths</source>
        <translation>Pfade</translation>
    </message>
    <message>
        <location filename="settings.ui" line="187"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="settings.ui" line="194"/>
        <location filename="settings.ui" line="246"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="219"/>
        <source>Applications</source>
        <translation>Anwendungen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="239"/>
        <source>Edit...</source>
        <translation>Bearbeiten...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="253"/>
        <source>Set as default</source>
        <translation>Als Standard festlegen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="278"/>
        <source>Reports</source>
        <translation>Berichte</translation>
    </message>
    <message>
        <location filename="settings.ui" line="284"/>
        <source>Save all errors when creating report</source>
        <translation>Alle Fehler beim Erstellen von Berichten speichern</translation>
    </message>
    <message>
        <location filename="settings.ui" line="291"/>
        <source>Save full path to files in reports</source>
        <translation>Vollständigen Dateipfad in Berichten speichern</translation>
    </message>
    <message>
        <location filename="settings.ui" line="312"/>
        <source>Language</source>
        <translation>Sprache</translation>
    </message>
    <message>
        <location filename="settings.ui" line="326"/>
        <source>Advanced</source>
        <translation>Erweitert</translation>
    </message>
    <message>
        <location filename="settings.ui" line="332"/>
        <source>&amp;Show inconclusive errors</source>
        <translation>&amp;Unklare Fehler anzeigen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="339"/>
        <source>S&amp;how internal warnings in log</source>
        <translation>&amp;Interne Warnungen im Log anzeigen</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="84"/>
        <source>N/A</source>
        <translation>kA</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="202"/>
        <source>Add a new application</source>
        <translation>Neue Anwendung hinzufügen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="236"/>
        <source>Modify an application</source>
        <translation>Anwendung ändern</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="265"/>
        <source>[Default]</source>
        <translation>[Standard]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="311"/>
        <source>Select include directory</source>
        <translation>Wähle Include-Verzeichnis</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="228"/>
        <location filename="statsdialog.cpp" line="106"/>
        <source>Statistics</source>
        <translation>Statistik</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="98"/>
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
        <translation>Pfade:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="85"/>
        <source>Include paths:</source>
        <translation>Include-Pfade:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation>Definitionen:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="145"/>
        <location filename="statsdialog.cpp" line="102"/>
        <source>Previous Scan</source>
        <translation>Vorherige Prüfung</translation>
    </message>
    <message>
        <location filename="stats.ui" line="151"/>
        <source>Path Selected:</source>
        <translation>Ausgewählte Pfade:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="161"/>
        <source>Number of Files Scanned:</source>
        <translation>Anzahl der geprüften Dateien:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Scan Duration:</source>
        <translation>Prüfungsdauer:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="236"/>
        <source>Errors:</source>
        <translation>Fehler:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="254"/>
        <source>Warnings:</source>
        <translation>Warnungen:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="272"/>
        <source>Stylistic warnings:</source>
        <translation>Stilwarnungen:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="290"/>
        <source>Portability warnings:</source>
        <translation>Portabilitätswarnungen:</translation>
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
        <translation>Performance-Probleme:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="326"/>
        <source>Information messages:</source>
        <translation>Informationsmeldungen:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="362"/>
        <source>Copy to Clipboard</source>
        <translation>In die Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="78"/>
        <source>1 day</source>
        <translation>1 Tag</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="78"/>
        <source>%1 days</source>
        <translation>%1 Tage</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="80"/>
        <source>1 hour</source>
        <translation>1 Stunde</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="80"/>
        <source>%1 hours</source>
        <translation>%1 Stunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="82"/>
        <source>1 minute</source>
        <translation>1 Minute</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="82"/>
        <source>%1 minutes</source>
        <translation>%1 Minuten</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="84"/>
        <source>1 second</source>
        <translation>1 Sekunde</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="84"/>
        <source>%1 seconds</source>
        <translation>%1 Sekunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="88"/>
        <source>0.%1 seconds</source>
        <translation>0,%1 Sekunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="90"/>
        <source> and </source>
        <translation> und </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="97"/>
        <source>Project Settings</source>
        <translation>Projekteinstellungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="99"/>
        <source>Paths</source>
        <translation>Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="100"/>
        <source>Include paths</source>
        <translation>Include-Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="101"/>
        <source>Defines</source>
        <translation>Definitionen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="103"/>
        <source>Path selected</source>
        <translation>Gewählte Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="104"/>
        <source>Number of files scanned</source>
        <translation>Anzahl geprüfter Dateien</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="105"/>
        <source>Scan duration</source>
        <translation>Prüfungsdauer</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="107"/>
        <source>Errors</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="108"/>
        <source>Warnings</source>
        <translation>Warnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="109"/>
        <source>Style warnings</source>
        <translation>Stilwarnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="110"/>
        <source>Portability warnings</source>
        <translation>Portabilitätswarnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>Performance warnings</source>
        <translation>Performance-Warnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>Information messages</source>
        <translation>Informationsmeldungen</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="54"/>
        <source>%1 of %2 files checked</source>
        <translation>%1 von %2 Dateien geprüft</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="76"/>
        <source>inconclusive</source>
        <translation>unklar</translation>
    </message>
</context>
</TS>
