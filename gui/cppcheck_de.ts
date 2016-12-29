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
        <source>Copyright © 2007-2016 Cppcheck team.</source>
        <translation>Copyright © 2007-2016 Cppcheck-Team.</translation>
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

Die folgenden Texte in Parametern werden durch die passenden Werte ersetzt, wenn die Anwendung ausgeführt wird:
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
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Ausführbare Dateien (*.exe);;Alle Dateien(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="62"/>
        <source>Select viewer application</source>
        <translation>Anzeigeanwendung auswählen</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="77"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="78"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>Sie müssen einen Namen, einen Pfad und ggf. Parameter für die Anwendung angeben!</translation>
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
    <name>LibraryAddFunctionDialog</name>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="23"/>
        <source>Add function</source>
        <translation>Funktion hinzufügen</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <source>Function name(s)</source>
        <translation>Funktionsname(n)</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <source>Number of arguments</source>
        <translation>Anzahl Argumente</translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <source>Library Editor</source>
        <translation>Bibliothekseditor</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <source>Open</source>
        <translation>Öffnen</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <source>Save as</source>
        <translation>Speichern unter</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <source>Functions</source>
        <translation>Funktionen</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <source>Sort</source>
        <translation>Sortiere</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <source>Filter:</source>
        <translation>Filter:</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <source>Comments</source>
        <translation>Kommentare</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="204"/>
        <source>noreturn</source>
        <translation>Zurückkehrend</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="212"/>
        <source>False</source>
        <translation>Ja</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="217"/>
        <source>True</source>
        <translation>Nein</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="222"/>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="232"/>
        <source>return value must be used</source>
        <translation>Rückgabewert muss genutzt werden</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <source>ignore function in leaks checking</source>
        <translation>Ignoriere Funktion in Speicherleck-Prüfung</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <source>Arguments</source>
        <translation>Argumente</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="82"/>
        <location filename="librarydialog.cpp" line="140"/>
        <source>Library files (*.cfg)</source>
        <translation>Bibliotheksdateien (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="84"/>
        <source>Open library file</source>
        <translation>Bibliothek öffnen</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="110"/>
        <location filename="librarydialog.cpp" line="130"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="111"/>
        <source>Can not open file %1.</source>
        <translation>Datei %1 kann nicht geöffnet werden.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="131"/>
        <source>Can not save file %1.</source>
        <translation>Datei %1 kann nicht gespeichert werden.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="143"/>
        <source>Save the library as</source>
        <translation>Speichere Bibliothek unter</translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <source>Edit argument</source>
        <translation>Argument bearbeiten</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="20"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is bool value allowed? For instance result from comparison or from &apos;!&apos; operator.&lt;/p&gt;
&lt;p&gt;Typically, set this if the argument is a pointer, size, etc.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // last argument should not have a bool value&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Ist ein boolscher Wert, beispielsweise das Ergebnis eines Vergleichsoperators, oder von &apos;!&apos; zulässig?&lt;/p&gt;
&lt;p&gt;Diese Option wird typischerweise gesetzt, wenn das Argument ein Zeiger, eine Größe, etc. ist.&lt;/p&gt;
&lt;p&gt;Beispiel:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // Das letzte Argument sollte kein boolscher Wert sein.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="28"/>
        <source>Not bool</source>
        <translation>Nicht boolsch</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="35"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is a null parameter value allowed?&lt;/p&gt;
&lt;p&gt;Typically this should be used on any pointer parameter that does not allow null.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // neither x or y is allowed to be null.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Ist die Übergabe von Null zulässig?&lt;/p&gt;
&lt;p&gt;Dies wird typischerweise für Funktionen mit Zeigern als Parameter genutzt, die nicht Null sein dürfen.&lt;/p&gt;
&lt;p&gt;Beispiel:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // Weder x noch y dürfen ein Nullzeiger sein.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="43"/>
        <source>Not null</source>
        <translation>Nicht Null</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="50"/>
        <source>Not uninit</source>
        <translation>Nicht uninitialisiert</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="57"/>
        <source>String</source>
        <translation>String</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="70"/>
        <source>Format string</source>
        <translation>Formatstring</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="92"/>
        <source>Min size of buffer</source>
        <translation>Minimale Puffergröße</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="101"/>
        <location filename="libraryeditargdialog.ui" line="208"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <source>argvalue</source>
        <translation>Argumentwert</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="229"/>
        <source>constant</source>
        <translation>Konstant</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="124"/>
        <location filename="libraryeditargdialog.ui" line="234"/>
        <source>mul</source>
        <translation>Multiplikation</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="129"/>
        <location filename="libraryeditargdialog.ui" line="239"/>
        <source>strlen</source>
        <translation>strlen</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="137"/>
        <location filename="libraryeditargdialog.ui" line="247"/>
        <source>Arg</source>
        <translation>Argument 1</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="164"/>
        <location filename="libraryeditargdialog.ui" line="274"/>
        <source>Arg2</source>
        <translation>Argument 2</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="199"/>
        <source>and</source>
        <translation>und</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="320"/>
        <source>Valid values</source>
        <translation>Zulässige Werte</translation>
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
        <location filename="logview.cpp" line="34"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="68"/>
        <source>Save Log</source>
        <translation>Speichere Log</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="69"/>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation>Textdateien (*.txt *.log);;Alle Dateien(*.*)</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="73"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="74"/>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation>Datei kann nicht zum Schreiben geöffnet werden: &quot;%1&quot;</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.cpp" line="308"/>
        <location filename="mainwindow.cpp" line="423"/>
        <location filename="mainwindow.cpp" line="487"/>
        <location filename="mainwindow.cpp" line="581"/>
        <location filename="mainwindow.cpp" line="603"/>
        <location filename="mainwindow.cpp" line="974"/>
        <location filename="mainwindow.cpp" line="1086"/>
        <location filename="mainwindow.cpp" line="1214"/>
        <location filename="mainwindow.cpp" line="1393"/>
        <location filename="mainwindow.cpp" line="1481"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="180"/>
        <source>Standard</source>
        <translation>Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="70"/>
        <source>&amp;File</source>
        <translation>&amp;Datei</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="89"/>
        <source>&amp;View</source>
        <translation>&amp;Ansicht</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="93"/>
        <source>&amp;Toolbars</source>
        <translation>&amp;Symbolleisten</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="132"/>
        <source>&amp;Check</source>
        <translation>&amp;Prüfen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="136"/>
        <source>C++ standard</source>
        <translation>C++-Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="143"/>
        <source>C standard</source>
        <translation>C-Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="167"/>
        <source>&amp;Edit</source>
        <translation>&amp;Bearbeiten</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="229"/>
        <source>&amp;License...</source>
        <translation>&amp;Lizenz...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="234"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Autoren...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="243"/>
        <source>&amp;About...</source>
        <translation>Ü&amp;ber...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="248"/>
        <source>&amp;Files...</source>
        <translation>&amp;Dateien...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="251"/>
        <location filename="mainwindow.ui" line="254"/>
        <source>Check files</source>
        <translation>Prüfe Dateien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="257"/>
        <source>Ctrl+F</source>
        <translation>Strg+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="266"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Verzeichnis...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="269"/>
        <location filename="mainwindow.ui" line="272"/>
        <source>Check directory</source>
        <translation>Prüfe Verzeichnis</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="275"/>
        <source>Ctrl+D</source>
        <translation>Strg+D</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="287"/>
        <source>Ctrl+R</source>
        <translation>Strg+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="296"/>
        <source>&amp;Recheck all files</source>
        <translation>Alle Dateien neu prüfen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="305"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stoppen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="308"/>
        <location filename="mainwindow.ui" line="311"/>
        <source>Stop checking</source>
        <translation>Prüfung abbrechen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="314"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="323"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Ergebnisse in Datei speichern...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="326"/>
        <source>Ctrl+S</source>
        <translation>Strg+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="331"/>
        <source>&amp;Quit</source>
        <translation>&amp;Beenden</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="340"/>
        <source>&amp;Clear results</source>
        <translation>Ergebnisse &amp;löschen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="349"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Einstellungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="379"/>
        <source>Errors</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="382"/>
        <location filename="mainwindow.ui" line="385"/>
        <source>Show errors</source>
        <translation>Zeige Fehler</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="466"/>
        <source>Show S&amp;cratchpad...</source>
        <translation>Zeige Schmierzettel</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="516"/>
        <source>Warnings</source>
        <translation>Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="519"/>
        <location filename="mainwindow.ui" line="522"/>
        <source>Show warnings</source>
        <translation>Zeige Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="534"/>
        <source>Performance warnings</source>
        <translation>Performance-Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="537"/>
        <location filename="mainwindow.ui" line="540"/>
        <source>Show performance warnings</source>
        <translation>Zeige Performance-Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="548"/>
        <source>Show &amp;hidden</source>
        <translation>Zeige &amp;versteckte</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="560"/>
        <location filename="mainwindow.cpp" line="679"/>
        <location filename="mainwindow.cpp" line="717"/>
        <source>Information</source>
        <translation>Information</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="563"/>
        <source>Show information messages</source>
        <translation>Zeige Informationsmeldungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="575"/>
        <source>Portability</source>
        <translation>Portabilität</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="578"/>
        <source>Show portability warnings</source>
        <translation>Zeige Portabilitätswarnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="586"/>
        <source>&amp;Filter</source>
        <translation>&amp;Filter</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="589"/>
        <source>Filter results</source>
        <translation>Gefilterte Ergebnisse</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="605"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit, ANSI</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="613"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit, Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="621"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="629"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="637"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="645"/>
        <source>Platforms</source>
        <translation>Plattformen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="659"/>
        <source>C++11</source>
        <translation>C++11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="670"/>
        <source>C99</source>
        <translation>C99</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="678"/>
        <source>Posix</source>
        <translation>Posix</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="686"/>
        <source>C11</source>
        <translation>C11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="694"/>
        <source>C89</source>
        <translation>C89</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="702"/>
        <source>C++03</source>
        <translation>C++03</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="707"/>
        <source>&amp;Print...</source>
        <translation>Drucken...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="710"/>
        <source>Print the Current Report</source>
        <translation>Aktuellen Bericht ausdrucken</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="715"/>
        <source>Print Pre&amp;view...</source>
        <translation>Druckvorschau</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="718"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>Druckvorschaudialog für aktuelle Ergebnisse öffnen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="723"/>
        <source>Library Editor...</source>
        <translation>Bibliothekseditor...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="726"/>
        <source>Open library editor</source>
        <translation>Bibliothekseditor öffnen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="734"/>
        <source>Auto-detect language</source>
        <translation>Sprache automatisch erkennen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="742"/>
        <source>Enforce C++</source>
        <translation>Erzwinge C++</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="750"/>
        <source>Enforce C</source>
        <translation>Erzwinge C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="390"/>
        <source>&amp;Check all</source>
        <translation>Alle &amp;auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="218"/>
        <source>Filter</source>
        <translation>Filter</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="284"/>
        <source>&amp;Recheck modified files</source>
        <translation>Veränderte Dateien neu prüfen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="395"/>
        <source>&amp;Uncheck all</source>
        <translation>Alle a&amp;bwählen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="400"/>
        <source>Collapse &amp;all</source>
        <translation>Alle &amp;reduzieren</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="405"/>
        <source>&amp;Expand all</source>
        <translation>Alle &amp;erweitern</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="413"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="416"/>
        <source>Standard items</source>
        <translation>Standardeinträge</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="432"/>
        <source>Toolbar</source>
        <translation>Symbolleiste</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="440"/>
        <source>&amp;Categories</source>
        <translation>&amp;Kategorien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="443"/>
        <source>Error categories</source>
        <translation>Fehler-Kategorien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="448"/>
        <source>&amp;Open XML...</source>
        <translation>Öffne &amp;XML...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="457"/>
        <source>Open P&amp;roject File...</source>
        <translation>Pr&amp;ojektdatei öffnen...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="471"/>
        <source>&amp;New Project File...</source>
        <translation>&amp;Neue Projektdatei...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="476"/>
        <source>&amp;Log View</source>
        <translation>&amp;Loganzeige</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="479"/>
        <source>Log View</source>
        <translation>Loganzeige</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="487"/>
        <source>C&amp;lose Project File</source>
        <translation>Projektdatei &amp;schließen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="495"/>
        <source>&amp;Edit Project File...</source>
        <translation>Projektdatei &amp;bearbeiten...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="504"/>
        <source>&amp;Statistics</source>
        <translation>&amp;Statistik</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="421"/>
        <source>&amp;Contents</source>
        <translation>&amp;Inhalte</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="201"/>
        <source>Categories</source>
        <translation>Kategorien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="361"/>
        <source>Style warnings</source>
        <translation>Stilwarnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="364"/>
        <location filename="mainwindow.ui" line="367"/>
        <source>Show style warnings</source>
        <translation>Zeige Stilwarnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="424"/>
        <source>Open the help contents</source>
        <translation>Öffnet die Hilfe-Inhalte</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="427"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="122"/>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="517"/>
        <source>Select directory to check</source>
        <translation>Verzeichnis zum Überprüfen auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="424"/>
        <source>No suitable files found to check!</source>
        <translation>Keine passenden Dateien zum Überprüfen gefunden!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="79"/>
        <source>Quick Filter:</source>
        <translation>Schnellfilter:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="505"/>
        <source>C/C++ Source, Compile database, Visual Studio (%1 %2 *.sln *.vcxproj)</source>
        <translation>C/C++-Sourcecode, Compile-Datenbank, Visual Studio (%1 %2 *.sln *.vcxproj)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="552"/>
        <source>Select configuration</source>
        <translation>Konfiguration wählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="552"/>
        <source>Select the configuration that will be checked</source>
        <translation>Wählen Sie die Konfiguration, die geprüft werden soll</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="582"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>Gefundene Projektdatei: %1

Möchten Sie stattdessen diese öffnen?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="604"/>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation>Projektdatei im Verzeichnis gefunden.

Möchten Sie die Prüfung wirklich durchführen, ohne eine Projektdatei zu verwenden?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="688"/>
        <source>File not found</source>
        <translation>Datei nicht gefunden</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="691"/>
        <source>Bad XML</source>
        <translation>Fehlerhaftes XML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="694"/>
        <source>Missing attribute</source>
        <translation>Fehlendes Attribut</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="697"/>
        <source>Bad attribute value</source>
        <translation>Falscher Attributwert</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="703"/>
        <source>Duplicate platform type</source>
        <translation>Plattformtyp doppelt</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="706"/>
        <source>Platform type redefined</source>
        <translation>Plattformtyp neu definiert</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="717"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>Laden der ausgewählten Bibliothek &apos;%1&apos; schlug fehl. %2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="806"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located.</source>
        <translation>Laden von %1 fehlgeschlagen. Ihre Cppcheck-Installation ist defekt. Sie können --data-dir=&lt;Verzeichnis&gt; als Kommandozeilenparameter verwenden, um anzugeben, wo die Datei sich befindet.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1128"/>
        <source>License</source>
        <translation>Lizenz</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1135"/>
        <source>Authors</source>
        <translation>Autoren</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1148"/>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML-Dateien Version 2 (*.xml);;XML-Dateien Version 1 (*.xml);;Textdateien (*.txt);;CSV-Dateien (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1150"/>
        <source>Save the report file</source>
        <translation>Speichert die Berichtdatei</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="990"/>
        <source>XML files (*.xml)</source>
        <translation>XML-Dateien (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="303"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>Beim Laden der Editor-Anwendungseinstellungen trat ein Problem auf.

Dies wurde vermutlich durch einen Wechsel der Cppcheck-Version hervorgerufen. Bitte prüfen (und korrigieren) Sie die Einstellungen, andernfalls könnte die Editor-Anwendung nicht korrekt starten.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="488"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Sie müssen die Projektdatei schließen, bevor Sie neue Dateien oder Verzeichnisse auswählen!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="503"/>
        <source>Select files to check</source>
        <translation>Dateien zum Überprüfen auswählen</translation>
    </message>
    <message>
        <source>C/C++ Source (%1)</source>
        <translation type="obsolete">C/C++-Code (%1)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="679"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>Die Bibliothek &apos;%1&apos; enthält unbekannte Elemente:
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="700"/>
        <source>Unsupported format</source>
        <translation>Nicht unterstütztes Format</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="709"/>
        <source>Unknown element</source>
        <translation>Unbekanntes Element</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="712"/>
        <source>Unknown issue</source>
        <translation>Unbekannter Fehler</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="806"/>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="975"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation>Aktuelle Ergebnisse werden gelöscht.

          Das Einlesen einer XML-Datei löscht die aktuellen Ergebnisse. Fortfahren?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="992"/>
        <source>Open the report file</source>
        <translation>Berichtdatei öffnen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1082"/>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?</source>
        <translation>Prüfung läuft.

Möchten Sie die Prüfung abbrechen und Cppcheck beenden?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1157"/>
        <source>XML files version 1 (*.xml)</source>
        <translation>XML-Dateien Version 1 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1158"/>
        <source>Deprecated XML format</source>
        <translation>Veraltetes XML-Format</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1158"/>
        <source>XML format 1 is deprecated and will be removed in cppcheck 1.81.</source>
        <translation>XML-Format 1 ist veraltet und wird in cppcheck 1.81 entfernt.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1164"/>
        <source>XML files version 2 (*.xml)</source>
        <translation>XML-Dateien Version 2 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1168"/>
        <source>Text files (*.txt)</source>
        <translation>Textdateien (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1172"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-Dateien (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1216"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1261"/>
        <location filename="mainwindow.cpp" line="1355"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Projektdateien (*.cppcheck);;Alle Dateien(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1263"/>
        <source>Select Project File</source>
        <translation>Projektdatei auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1291"/>
        <location filename="mainwindow.cpp" line="1369"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1357"/>
        <source>Select Project Filename</source>
        <translation>Projektnamen auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1394"/>
        <source>No project file loaded</source>
        <translation>Keine Projektdatei geladen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1476"/>
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
        <location filename="main.cpp" line="100"/>
        <source>Cppcheck GUI.

Syntax:
    cppcheck-gui [OPTIONS] [files or paths]

Options:
    -h, --help              Print this help
    -p &lt;file&gt;               Open given project file and start checking it
    -l &lt;file&gt;               Open given results xml file
    -d &lt;directory&gt;          Specify the directory that was checked to generate the results xml specified with -l
    -v, --version           Show program version
    --data-dir=&lt;directory&gt;  Specify directory where GUI datafiles are located (translations, cfg)</source>
        <translation>
          Cppcheck GUI.

          Syntax:
          cppcheck-gui [OPTIONEN] [Dateien oder Pfade]

          Options:
          -h, --help              Gibt diese Hilfeinformationen aus
          -p &lt;file&gt;               Öffnet das angegebene Projekt und beginnt die Prüfung
          -l &lt;file&gt;               Öffnet die angegebene XML-Ergebnisdatei
          -d &lt;directory&gt;          Gibt das Verzeichnis an, das geprüft wurde, um das unter -l angegebene XML-Ergebnis zu erzeugen
          -v, --version           Zeigt Programmversion an
          --data-dir=&lt;directory&gt;  Gibt das Verzeichnis an, unter dem sich die Konfigurationsdateien für die GUI (Übersetzungen, Cfg) befinden
        </translation>
    </message>
    <message>
        <location filename="main.cpp" line="113"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation>Cppcheck GUI - Kommandozeilenparameter</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <location filename="platforms.cpp" line="37"/>
        <source>Native</source>
        <translation>Nativ</translation>
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
        <location filename="project.cpp" line="73"/>
        <location filename="project.cpp" line="97"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="project.cpp" line="74"/>
        <source>Could not read the project file.</source>
        <translation>Projektdatei konnte nicht gelesen werden.</translation>
    </message>
    <message>
        <location filename="project.cpp" line="98"/>
        <source>Could not write the project file.</source>
        <translation>Projektdatei konnte nicht geschrieben werden.</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfiledialog.ui" line="14"/>
        <source>Project File</source>
        <translation>Projektdatei</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="24"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="297"/>
        <source>Defines must be separated by a semicolon &apos;;&apos;</source>
        <translation>Definitionen müssen mit einem Semikolon getrennt werden.</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="32"/>
        <source>Root:</source>
        <translation>Wurzelverzeichnis:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="83"/>
        <source>Libraries:</source>
        <translation>Bibliotheken:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="92"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>Hinweis: Legen Sie eigene .cfg-Dateien in den Ordner der Projektdatei. Dann sollten sie oben sichtbar werden.</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="116"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="122"/>
        <source>Visual Studio

Cppcheck can import visual studio solutions and projects.

Files to check, include paths, configurations, defines, platform settings are imported.

Library settings are not imported.</source>
        <translation>Visual Studio

Cppcheck kann Visual-Studio-Projektmappen und -Projekte öffnen.

Zu prüfende Dateien, Includepfade, Konfigurationen, Definitionen und Plattformeinstellungen werden importiert.

Bibliothekseinstellungen werden nicht importiert.</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="59"/>
        <location filename="projectfiledialog.ui" line="140"/>
        <location filename="projectfiledialog.ui" line="187"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="49"/>
        <source>Cppcheck build dir (optional)</source>
        <translation>Cppcheck-Erstellungsverzeichnis (optional)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="163"/>
        <source>CMake</source>
        <translation>CMake</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="169"/>
        <source>Compile database

Cppcheck can import files to analyse, include paths, defines from the compile database.

Platform settings are not provided in compile database and must be configured.

Library settings are not provided in compile database, be careful about this configuration also.</source>
        <translation>Compile-Datenbank

Cppcheck kann zu prüfende Dateien, Includepfade und Definitionen von der Compile-Datenbank importieren.

Plattformeinstellungen werden nicht bereitgestellt und müssen konfiguriert werden.

Bibliothekseinstellungen werden ebenfalls nicht bereitgestellt, geben Sie also auf deren Konfiguration acht.</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="210"/>
        <source>Other</source>
        <translation>Andere</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="220"/>
        <location filename="projectfiledialog.ui" line="413"/>
        <source>Paths:</source>
        <translation>Pfade:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="247"/>
        <location filename="projectfiledialog.ui" line="341"/>
        <location filename="projectfiledialog.ui" line="427"/>
        <source>Add...</source>
        <translation>Hinzufügen...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="254"/>
        <location filename="projectfiledialog.ui" line="348"/>
        <location filename="projectfiledialog.ui" line="434"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="261"/>
        <location filename="projectfiledialog.ui" line="355"/>
        <location filename="projectfiledialog.ui" line="441"/>
        <location filename="projectfiledialog.ui" line="504"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="310"/>
        <source>Include Paths:</source>
        <translation>Includepfade:</translation>
    </message>
    <message>
        <source>Includes</source>
        <translation type="obsolete">Includes</translation>
    </message>
    <message>
        <source>Include directories:</source>
        <translation type="obsolete">Include-Verzeichnisse:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="375"/>
        <source>Up</source>
        <translation>Auf</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="382"/>
        <source>Down</source>
        <translation>Ab</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="407"/>
        <source>Exclude</source>
        <translation>Ausschließen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="466"/>
        <source>Suppressions</source>
        <translation>Fehlerunterdrückungen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="472"/>
        <source>Suppression list:</source>
        <translation>Fehlerunterdrückungsliste:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="497"/>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="287"/>
        <source>Defines:</source>
        <translation>Definitionen:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="43"/>
        <source>Project file: %1</source>
        <translation>Projektdatei: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="177"/>
        <source>Select Cppcheck build dir</source>
        <translation>Wähle Cppcheck-Erstellungsverzeichnis</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="186"/>
        <source>Compile Database</source>
        <translation>Compile-Datenbank</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="188"/>
        <source>Compile database (compile_database.json)</source>
        <translation>Compile-Datenbank (compile_database.json)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="196"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="198"/>
        <source>Visual Studio Solution/Project (*.sln *.vcxproj)</source>
        <translation>Visual-Studio-Projektmappe/-Projekt (*.sln *.vcxproj)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="413"/>
        <source>Select include directory</source>
        <translation>Wähle Include-Verzeichnisse</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="393"/>
        <source>Select a directory to check</source>
        <translation>Wähle zu prüfendes Verzeichnis</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="433"/>
        <source>Select directory to ignore</source>
        <translation>Wähle zu ignorierendes Verzeichnis</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="487"/>
        <source>Add Suppression</source>
        <translation>Fehlerunterdrückung hinzufügen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="488"/>
        <source>Select error id suppress:</source>
        <translation>Wähle Fehler-ID zum Unterdrücken:</translation>
    </message>
</context>
<context>
    <name>QDialogButtonBox</name>
    <message>
        <location filename="translationhandler.cpp" line="33"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="103"/>
        <source>Unknown language specified!</source>
        <translation>Unbekannte Sprache angegeben!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="133"/>
        <source>Language file %1 not found!</source>
        <translation>Sprachdatei %1 nicht gefunden!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="139"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>Die Übersetzungen der Sprache %1 konnten nicht aus der Datei %2 geladen werden</translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="1162"/>
        <source>File</source>
        <translation>Datei</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1162"/>
        <source>Severity</source>
        <translation>Schweregrad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1162"/>
        <source>Line</source>
        <translation>Zeile</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1162"/>
        <source>Summary</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="124"/>
        <source>Undefined file</source>
        <translation>Undefinierte Datei</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="783"/>
        <source>[Inconclusive]</source>
        <translation>[unklar]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="296"/>
        <source>debug</source>
        <translation>Debug</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="576"/>
        <source>Recheck</source>
        <translation>Erneut prüfen</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="577"/>
        <source>Copy filename</source>
        <translation>Dateiname kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="578"/>
        <source>Copy full path</source>
        <translation>Vollständigen Pfad kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="579"/>
        <source>Copy message</source>
        <translation>Meldung kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="580"/>
        <source>Copy message id</source>
        <translation>Meldungs-Id kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="581"/>
        <source>Hide</source>
        <translation>Verstecken</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="582"/>
        <source>Hide all with id</source>
        <translation>Verstecke alle mit gleicher ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="583"/>
        <source>Open containing folder</source>
        <translation>Übergeordneten Ordner öffnen</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="643"/>
        <location filename="resultstree.cpp" line="657"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="644"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <translation>Keine Editor-Anwendung eingestellt.

Konfigurieren Sie diese unter Einstellungen/Anwendungen.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="658"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>Keine Standard-Editor-Anwendung eingestellt.

 Bitte wählen Sie eine Standardanwendung unter Einstellungen/Anwendungen.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="687"/>
        <source>Could not find the file!</source>
        <translation>Datei konnte nicht gefunden werden!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="733"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>%1 konnte nicht gestartet werden.

Bitte überprüfen Sie ob der Pfad und die Parameter der Anwendung richtig eingestellt sind.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="747"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>Datei konnte nicht gefunden werden:
%1
Bitte wählen Sie das Verzeichnis, in dem sich die Datei befindet.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="754"/>
        <source>Select Directory</source>
        <translation>Wähle Verzeichnis</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1162"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1162"/>
        <source>Inconclusive</source>
        <translation>Unklar</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="278"/>
        <source>style</source>
        <translation>Stil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="281"/>
        <source>error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="284"/>
        <source>warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="287"/>
        <source>performance</source>
        <translation>Performance</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="290"/>
        <source>portability</source>
        <translation>Portabilität</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="293"/>
        <source>information</source>
        <translation>Information</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="187"/>
        <source>Print Report</source>
        <translation>Bericht drucken</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="206"/>
        <source>No errors found, nothing to print.</source>
        <translation>Keine Funde, nichts zu drucken.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="244"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 von %2 Dateien geprüft)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="257"/>
        <location filename="resultsview.cpp" line="268"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="258"/>
        <source>No errors found.</source>
        <translation>Keine Fehler gefunden.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="265"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Es wurden Fehler gefunden, aber sie sind so konfiguriert, ausgeblendet zu werden.
Legen Sie unter dem Menü Ansicht fest, welche Arten von Fehlern angezeigt werden sollen.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="311"/>
        <location filename="resultsview.cpp" line="329"/>
        <location filename="resultsview.cpp" line="337"/>
        <source>Failed to read the report.</source>
        <translation>Lesen des Berichts fehlgeschlagen.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="374"/>
        <source>Summary</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="375"/>
        <source>Message</source>
        <translation>Meldung</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="379"/>
        <source>First included by</source>
        <translation>Zuerst inkludiert von</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="382"/>
        <source>Id</source>
        <translation>Id</translation>
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
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <source>Scratchpad</source>
        <translation>Schmierzettel</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="48"/>
        <source>filename</source>
        <translation>Dateiname</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="55"/>
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
        <location filename="settings.ui" line="190"/>
        <source>Include paths:</source>
        <translation>Include-Pfade:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="208"/>
        <location filename="settings.ui" line="258"/>
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
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>Zeige Meldungs-Id in Spalte &quot;Id&quot;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="142"/>
        <source>Enable inline suppressions</source>
        <translation>Inline-Fehlerunterdrückung aktivieren</translation>
    </message>
    <message>
        <location filename="settings.ui" line="149"/>
        <source>Check for inconclusive errors also</source>
        <translation>Auch nach unklaren Fehlern suchen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="156"/>
        <source>Show statistics on check completion</source>
        <translation>Zeige Statistiken nach Prüfungsabschluss.</translation>
    </message>
    <message>
        <location filename="settings.ui" line="176"/>
        <source>Show internal warnings in log</source>
        <translation>Interne Warnungen im Log anzeigen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="184"/>
        <source>Paths</source>
        <translation>Pfade</translation>
    </message>
    <message>
        <location filename="settings.ui" line="215"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="settings.ui" line="222"/>
        <location filename="settings.ui" line="272"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="247"/>
        <source>Applications</source>
        <translation>Anwendungen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="265"/>
        <source>Edit...</source>
        <translation>Bearbeiten...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="279"/>
        <source>Set as default</source>
        <translation>Als Standard festlegen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="302"/>
        <source>Reports</source>
        <translation>Berichte</translation>
    </message>
    <message>
        <location filename="settings.ui" line="308"/>
        <source>Save all errors when creating report</source>
        <translation>Alle Fehler beim Erstellen von Berichten speichern</translation>
    </message>
    <message>
        <location filename="settings.ui" line="315"/>
        <source>Save full path to files in reports</source>
        <translation>Vollständigen Dateipfad in Berichten speichern</translation>
    </message>
    <message>
        <location filename="settings.ui" line="336"/>
        <source>Language</source>
        <translation>Sprache</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="83"/>
        <source>N/A</source>
        <translation>kA</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="205"/>
        <source>Add a new application</source>
        <translation>Neue Anwendung hinzufügen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="238"/>
        <source>Modify an application</source>
        <translation>Anwendung ändern</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="243"/>
        <source> [Default]</source>
        <translation> [Standard]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="268"/>
        <source>[Default]</source>
        <translation>[Standard]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="324"/>
        <source>Select include directory</source>
        <translation>Wähle Include-Verzeichnis</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="228"/>
        <location filename="statsdialog.cpp" line="105"/>
        <source>Statistics</source>
        <translation>Statistik</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="97"/>
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
        <location filename="statsdialog.cpp" line="101"/>
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
        <location filename="stats.ui" line="364"/>
        <source>Copy to Clipboard</source>
        <translation>In die Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="77"/>
        <source>1 day</source>
        <translation>1 Tag</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="77"/>
        <source>%1 days</source>
        <translation>%1 Tage</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="79"/>
        <source>1 hour</source>
        <translation>1 Stunde</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="79"/>
        <source>%1 hours</source>
        <translation>%1 Stunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>1 minute</source>
        <translation>1 Minute</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>%1 minutes</source>
        <translation>%1 Minuten</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>1 second</source>
        <translation>1 Sekunde</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>%1 seconds</source>
        <translation>%1 Sekunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="87"/>
        <source>0.%1 seconds</source>
        <translation>0,%1 Sekunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="89"/>
        <source> and </source>
        <translation> und </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="96"/>
        <source>Project Settings</source>
        <translation>Projekteinstellungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="98"/>
        <source>Paths</source>
        <translation>Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="99"/>
        <source>Include paths</source>
        <translation>Include-Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="100"/>
        <source>Defines</source>
        <translation>Definitionen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="102"/>
        <source>Path selected</source>
        <translation>Gewählte Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="103"/>
        <source>Number of files scanned</source>
        <translation>Anzahl geprüfter Dateien</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="104"/>
        <source>Scan duration</source>
        <translation>Prüfungsdauer</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="106"/>
        <source>Errors</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="107"/>
        <source>Warnings</source>
        <translation>Warnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="108"/>
        <source>Style warnings</source>
        <translation>Stilwarnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="109"/>
        <source>Portability warnings</source>
        <translation>Portabilitätswarnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="110"/>
        <source>Performance warnings</source>
        <translation>Performance-Warnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
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
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="145"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>Wechsel der Sprache der Benutzeroberfläche fehlgeschlagen:

%1

Die Sprache wurde auf Englisch zurückgesetzt. Öffnen Sie den Einstellungen-Dialog um eine verfügbare Sprache auszuwählen.</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="151"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="73"/>
        <source>inconclusive</source>
        <translation>unklar</translation>
    </message>
</context>
</TS>
