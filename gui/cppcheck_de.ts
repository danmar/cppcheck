<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
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
    <message>
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-2018 Cppcheck team.</source>
        <translation>Copyright © 2007-2018 Cppcheck-Team.</translation>
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
        <location filename="librarydialog.cpp" line="154"/>
        <source>Library files (*.cfg)</source>
        <translation>Bibliotheksdateien (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="84"/>
        <source>Open library file</source>
        <translation>Bibliothek öffnen</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="95"/>
        <location filename="librarydialog.cpp" line="107"/>
        <location filename="librarydialog.cpp" line="144"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="96"/>
        <source>Can not open file %1.</source>
        <translation>Datei %1 kann nicht geöffnet werden.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="108"/>
        <source>Failed to load %1. %2.</source>
        <translation>%1 kann nicht geladen werden. %2.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="145"/>
        <source>Can not save file %1.</source>
        <translation>Datei %1 kann nicht gespeichert werden.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="157"/>
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
        <location filename="libraryeditargdialog.ui" line="203"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="214"/>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <source>argvalue</source>
        <translation>Argumentwert</translation>
    </message>
    <message>
        <source>constant</source>
        <translation type="vanished">Konstant</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <source>mul</source>
        <translation>Multiplikation</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="124"/>
        <location filename="libraryeditargdialog.ui" line="229"/>
        <source>strlen</source>
        <translation>strlen</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="132"/>
        <location filename="libraryeditargdialog.ui" line="237"/>
        <source>Arg</source>
        <translation>Argument 1</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="159"/>
        <location filename="libraryeditargdialog.ui" line="264"/>
        <source>Arg2</source>
        <translation>Argument 2</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="194"/>
        <source>and</source>
        <translation>und</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="310"/>
        <source>Valid values</source>
        <translation>Zulässige Werte</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.ui" line="589"/>
        <location filename="mainwindow.cpp" line="327"/>
        <location filename="mainwindow.cpp" line="490"/>
        <location filename="mainwindow.cpp" line="554"/>
        <location filename="mainwindow.cpp" line="650"/>
        <location filename="mainwindow.cpp" line="672"/>
        <location filename="mainwindow.cpp" line="1076"/>
        <location filename="mainwindow.cpp" line="1191"/>
        <location filename="mainwindow.cpp" line="1312"/>
        <location filename="mainwindow.cpp" line="1449"/>
        <location filename="mainwindow.cpp" line="1466"/>
        <location filename="mainwindow.cpp" line="1546"/>
        <location filename="mainwindow.cpp" line="1612"/>
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
        <location filename="mainwindow.ui" line="131"/>
        <source>A&amp;nalyze</source>
        <translation>A&amp;nalysieren</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="135"/>
        <source>C++ standard</source>
        <translation>C++-Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="143"/>
        <source>&amp;C standard</source>
        <translation>&amp;C-Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="167"/>
        <source>&amp;Edit</source>
        <translation>&amp;Bearbeiten</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="228"/>
        <source>&amp;License...</source>
        <translation>&amp;Lizenz...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="233"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Autoren...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="242"/>
        <source>&amp;About...</source>
        <translation>Ü&amp;ber...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="247"/>
        <source>&amp;Files...</source>
        <translation>&amp;Dateien...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="250"/>
        <location filename="mainwindow.ui" line="253"/>
        <source>Analyze files</source>
        <translation>Analysiere Dateien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="256"/>
        <source>Ctrl+F</source>
        <translation>Strg+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="265"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Verzeichnis...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="268"/>
        <location filename="mainwindow.ui" line="271"/>
        <source>Analyze directory</source>
        <translation>Analysiere Verzeichnis</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="274"/>
        <source>Ctrl+D</source>
        <translation>Strg+D</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="286"/>
        <source>Ctrl+R</source>
        <translation>Strg+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="304"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stoppen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="307"/>
        <location filename="mainwindow.ui" line="310"/>
        <source>Stop analysis</source>
        <translation>Analyse abbrechen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="313"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="322"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Ergebnisse in Datei speichern...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="325"/>
        <source>Ctrl+S</source>
        <translation>Strg+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="330"/>
        <source>&amp;Quit</source>
        <translation>&amp;Beenden</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="339"/>
        <source>&amp;Clear results</source>
        <translation>Ergebnisse &amp;löschen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="348"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Einstellungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="381"/>
        <location filename="mainwindow.ui" line="384"/>
        <source>Show errors</source>
        <translation>Zeige Fehler</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="518"/>
        <location filename="mainwindow.ui" line="521"/>
        <source>Show warnings</source>
        <translation>Zeige Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="536"/>
        <location filename="mainwindow.ui" line="539"/>
        <source>Show performance warnings</source>
        <translation>Zeige Performance-Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="547"/>
        <source>Show &amp;hidden</source>
        <translation>Zeige &amp;versteckte</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="761"/>
        <location filename="mainwindow.cpp" line="799"/>
        <source>Information</source>
        <translation>Information</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="562"/>
        <source>Show information messages</source>
        <translation>Zeige Informationsmeldungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="577"/>
        <source>Show portability warnings</source>
        <translation>Zeige Portabilitätswarnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="592"/>
        <source>Show Cppcheck results</source>
        <translation>Zeige Cppcheck-Ergebnisse</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="604"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="607"/>
        <source>Show Clang results</source>
        <translation>Zeige Clang-Ergebnisse</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="615"/>
        <source>&amp;Filter</source>
        <translation>&amp;Filter</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="618"/>
        <source>Filter results</source>
        <translation>Gefilterte Ergebnisse</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="634"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit, ANSI</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="642"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit, Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="650"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="658"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="666"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="736"/>
        <source>&amp;Print...</source>
        <translation>Drucken...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="739"/>
        <source>Print the Current Report</source>
        <translation>Aktuellen Bericht ausdrucken</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="744"/>
        <source>Print Pre&amp;view...</source>
        <translation>Druckvorschau</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="747"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>Druckvorschaudialog für aktuelle Ergebnisse öffnen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="755"/>
        <source>Open library editor</source>
        <translation>Bibliothekseditor öffnen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="389"/>
        <source>&amp;Check all</source>
        <translation>Alle &amp;auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="217"/>
        <source>Filter</source>
        <translation>Filter</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="283"/>
        <source>&amp;Reanalyze modified files</source>
        <translation>Veränderte Dateien neu analysieren</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="295"/>
        <source>Reanal&amp;yze all files</source>
        <translation>Alle Dateien erneut anal&amp;yseieren</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="360"/>
        <source>Style war&amp;nings</source>
        <translation>Stilwar&amp;nungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="378"/>
        <source>E&amp;rrors</source>
        <translation>F&amp;ehler</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="394"/>
        <source>&amp;Uncheck all</source>
        <translation>Alle a&amp;bwählen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="399"/>
        <source>Collapse &amp;all</source>
        <translation>Alle &amp;reduzieren</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="404"/>
        <source>&amp;Expand all</source>
        <translation>Alle &amp;erweitern</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="412"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="415"/>
        <source>Standard items</source>
        <translation>Standardeinträge</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="431"/>
        <source>Toolbar</source>
        <translation>Symbolleiste</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="439"/>
        <source>&amp;Categories</source>
        <translation>&amp;Kategorien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="442"/>
        <source>Error categories</source>
        <translation>Fehler-Kategorien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="447"/>
        <source>&amp;Open XML...</source>
        <translation>Öffne &amp;XML...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="456"/>
        <source>Open P&amp;roject File...</source>
        <translation>Pr&amp;ojektdatei öffnen...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="465"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation>&amp;Zeige Schmierzettel...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="470"/>
        <source>&amp;New Project File...</source>
        <translation>&amp;Neue Projektdatei...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="475"/>
        <source>&amp;Log View</source>
        <translation>&amp;Loganzeige</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="478"/>
        <source>Log View</source>
        <translation>Loganzeige</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="486"/>
        <source>C&amp;lose Project File</source>
        <translation>Projektdatei &amp;schließen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="494"/>
        <source>&amp;Edit Project File...</source>
        <translation>Projektdatei &amp;bearbeiten...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="503"/>
        <source>&amp;Statistics</source>
        <translation>&amp;Statistik</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="515"/>
        <source>&amp;Warnings</source>
        <translation>&amp;Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="533"/>
        <source>Per&amp;formance warnings</source>
        <translation>Per&amp;formance-Warnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="559"/>
        <source>&amp;Information</source>
        <translation>&amp;Information</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="574"/>
        <source>&amp;Portability</source>
        <translation>&amp;Portabilität</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="674"/>
        <source>P&amp;latforms</source>
        <translation>P&amp;lattformen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="688"/>
        <source>C++&amp;11</source>
        <translation>C++&amp;11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="699"/>
        <source>C&amp;99</source>
        <translation>C&amp;99</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="707"/>
        <source>&amp;Posix</source>
        <translation>Posix</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="715"/>
        <source>C&amp;11</source>
        <translation>C&amp;11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="723"/>
        <source>&amp;C89</source>
        <translation>&amp;C89</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="731"/>
        <source>&amp;C++03</source>
        <translation>&amp;C++03</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="752"/>
        <source>&amp;Library Editor...</source>
        <translation>&amp;Bibliothekseditor</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="763"/>
        <source>&amp;Auto-detect language</source>
        <translation>Sprache &amp;automatisch erkennen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="771"/>
        <source>&amp;Enforce C++</source>
        <translation>C++ &amp;erzwingen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="779"/>
        <source>E&amp;nforce C</source>
        <translation>C e&amp;rzwingen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="790"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="420"/>
        <source>&amp;Contents</source>
        <translation>&amp;Inhalte</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="198"/>
        <source>Categories</source>
        <translation>Kategorien</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="363"/>
        <location filename="mainwindow.ui" line="366"/>
        <source>Show style warnings</source>
        <translation>Zeige Stilwarnungen</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="423"/>
        <source>Open the help contents</source>
        <translation>Öffnet die Hilfe-Inhalte</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="426"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="121"/>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="85"/>
        <source>Quick Filter:</source>
        <translation>Schnellfilter:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="572"/>
        <source>C/C++ Source, Compile database, Visual Studio (%1 %2 *.sln *.vcxproj)</source>
        <translation>C/C++-Sourcecode, Compile-Datenbank, Visual Studio (%1 %2 *.sln *.vcxproj)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="621"/>
        <source>Select configuration</source>
        <translation>Konfiguration wählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="651"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>Gefundene Projektdatei: %1

Möchten Sie stattdessen diese öffnen?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="770"/>
        <source>File not found</source>
        <translation>Datei nicht gefunden</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="773"/>
        <source>Bad XML</source>
        <translation>Fehlerhaftes XML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="776"/>
        <source>Missing attribute</source>
        <translation>Fehlendes Attribut</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="779"/>
        <source>Bad attribute value</source>
        <translation>Falscher Attributwert</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="785"/>
        <source>Duplicate platform type</source>
        <translation>Plattformtyp doppelt</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="788"/>
        <source>Platform type redefined</source>
        <translation>Plattformtyp neu definiert</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="799"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>Laden der ausgewählten Bibliothek &apos;%1&apos; schlug fehl.
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1233"/>
        <source>License</source>
        <translation>Lizenz</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1240"/>
        <source>Authors</source>
        <translation>Autoren</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1255"/>
        <source>Save the report file</source>
        <translation>Speichert die Berichtdatei</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1092"/>
        <location filename="mainwindow.cpp" line="1262"/>
        <source>XML files (*.xml)</source>
        <translation>XML-Dateien (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="322"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>Beim Laden der Editor-Anwendungseinstellungen trat ein Problem auf.

Dies wurde vermutlich durch einen Wechsel der Cppcheck-Version hervorgerufen. Bitte prüfen (und korrigieren) Sie die Einstellungen, andernfalls könnte die Editor-Anwendung nicht korrekt starten.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="555"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Sie müssen die Projektdatei schließen, bevor Sie neue Dateien oder Verzeichnisse auswählen!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="761"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>Die Bibliothek &apos;%1&apos; enthält unbekannte Elemente:
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="782"/>
        <source>Unsupported format</source>
        <translation>Nicht unterstütztes Format</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Unknown element</source>
        <translation>Unbekanntes Element</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="794"/>
        <source>Unknown issue</source>
        <translation>Unbekannter Fehler</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="895"/>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="895"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation>Laden von %1 fehlgeschlagen. Ihre Cppcheck-Installation ist defekt. Sie können --data-dir=&lt;Verzeichnis&gt; als Kommandozeilenparameter verwenden, um anzugeben, wo die Datei sich befindet. Bitte beachten Sie, dass --data-dir in Installationsroutinen genutzt werden soll, und die GUI bei dessen Nutzung nicht startet, sondern die Einstellungen konfiguriert.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1077"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation>Aktuelle Ergebnisse werden gelöscht.

          Das Einlesen einer XML-Datei löscht die aktuellen Ergebnisse. Fortfahren?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1094"/>
        <source>Open the report file</source>
        <translation>Berichtdatei öffnen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1266"/>
        <source>Text files (*.txt)</source>
        <translation>Textdateien (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1270"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-Dateien (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1314"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1361"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Projektdateien (*.cppcheck);;Alle Dateien(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1363"/>
        <source>Select Project File</source>
        <translation>Projektdatei auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="154"/>
        <location filename="mainwindow.cpp" line="1391"/>
        <location filename="mainwindow.cpp" line="1515"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="491"/>
        <source>No suitable files found to analyze!</source>
        <translation>Keine passenden Dateien für Analyse gefunden!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="570"/>
        <source>Select files to analyze</source>
        <translation>Dateien für Analyse auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="584"/>
        <source>Select directory to analyze</source>
        <translation>Verzeichnis für Analyse auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="621"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>Zu analysierende Konfiguration auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="673"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation>Projektdateien im Verzeichnis gefunden.
        
Wollen sie fortfahren, ohne diese Projektdateien zu nutzen?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1187"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>Analyse läuft.
        
Wollen sie die Analyse abbrechen und Cppcheck beenden?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1253"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML-Dateien (*.xml);;Textdateien (*.txt);;CSV-Dateien (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1450"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>Erstellungsverzeichnis &apos;%1&apos; existiert nicht. Erstellen?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1467"/>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation>Import von &apos;%1&apos; fehlgeschlagen; Analyse wurde abgebrochen.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1500"/>
        <source>Project files (*.cppcheck)</source>
        <translation>Projektdateien (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1502"/>
        <source>Select Project Filename</source>
        <translation>Projektnamen auswählen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1547"/>
        <source>No project file loaded</source>
        <translation>Keine Projektdatei geladen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1607"/>
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
        <location filename="main.cpp" line="105"/>
        <source>Cppcheck GUI.

Syntax:
    cppcheck-gui [OPTIONS] [files or paths]

Options:
    -h, --help              Print this help
    -p &lt;file&gt;               Open given project file and start checking it
    -l &lt;file&gt;               Open given results xml file
    -d &lt;directory&gt;          Specify the directory that was checked to generate the results xml specified with -l
    -v, --version           Show program version
    --data-dir=&lt;directory&gt;  This option is for installation scripts so they can configure the directory where
                            datafiles are located (translations, cfg). The GUI is not started when this option
                            is used.</source>
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
          --data-dir=&lt;directory&gt;  Gibt das Verzeichnis an, unter dem sich die Konfigurationsdateien für die GUI (Übersetzungen, Cfg) befinden. Die GUI startet bei Nutzung dieser Option nicht.
        </translation>
    </message>
    <message>
        <location filename="main.cpp" line="120"/>
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
    <name>ProjectFile</name>
    <message>
        <location filename="projectfiledialog.ui" line="14"/>
        <source>Project File</source>
        <translation>Projektdatei</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="273"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation>Pfade und Definitionen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="30"/>
        <source>Import Project (Visual studio / compile database)</source>
        <translation>Importiere Projekt (Visual Studio / Compile-Datenbank)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="163"/>
        <source>Defines must be separated by a semicolon &apos;;&apos;</source>
        <translation>Definitionen müssen mit einem Semikolon getrennt werden.</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="331"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>Hinweis: Legen Sie eigene .cfg-Dateien in den Ordner der Projektdatei. Dann sollten sie oben sichtbar werden.</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="60"/>
        <location filename="projectfiledialog.ui" line="312"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="69"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Sie haben die Auswahl:&lt;/p&gt;&lt;p&gt; * Alle Debug- und Release-Konfigurationen analysieren&lt;/p&gt;&lt;p&gt; * Nur die erste passende Debug-Konfiguration analysieren&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="72"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation>Alle Visual-Studio-Konfigurationen analysieren</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="86"/>
        <location filename="projectfiledialog.ui" line="364"/>
        <source>Paths:</source>
        <translation>Pfade:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="113"/>
        <location filename="projectfiledialog.ui" line="207"/>
        <location filename="projectfiledialog.ui" line="378"/>
        <source>Add...</source>
        <translation>Hinzufügen...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="120"/>
        <location filename="projectfiledialog.ui" line="214"/>
        <location filename="projectfiledialog.ui" line="385"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="127"/>
        <location filename="projectfiledialog.ui" line="221"/>
        <location filename="projectfiledialog.ui" line="392"/>
        <location filename="projectfiledialog.ui" line="455"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="176"/>
        <source>Include Paths:</source>
        <translation>Includepfade:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="241"/>
        <source>Up</source>
        <translation>Auf</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="248"/>
        <source>Down</source>
        <translation>Ab</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="279"/>
        <source>Root path:</source>
        <translation>Wurzelverzeichnis:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="291"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>Warnungs-Tags (Semikolon-getrennt)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="303"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation>Cppcheck-Arbeitsverzeichnis (Vollständige Programmanalyse, inkrementelle Analyse, Statistiken, etc.)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="322"/>
        <source>Libraries</source>
        <translation>Bibliotheken</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="358"/>
        <source>Exclude</source>
        <translation>Ausschließen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="417"/>
        <source>Suppressions</source>
        <translation>Fehlerunterdrückungen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="423"/>
        <source>Suppression list:</source>
        <translation>Fehlerunterdrückungsliste:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="448"/>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="465"/>
        <source>Addons</source>
        <translation>Add-Ons</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="471"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="478"/>
        <source>Thread safety</source>
        <translation>Threadsicherheit</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="485"/>
        <source>Coding standards</source>
        <translation>Programmierstandards</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="492"/>
        <source>Cert</source>
        <translation>Cert</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="513"/>
        <source>Extra Tools</source>
        <translation>Zusatzwerkzeuge</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="519"/>
        <source>It is common best practice to use several tools.</source>
        <translation>Es wird empfohlen, mehrere Werkzeuge einzusetzen.</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="526"/>
        <source>Clang analyzer</source>
        <translation>Clang-Analyzer</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="533"/>
        <source>Clang-tidy</source>
        <translation>Clang-Tidy</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="153"/>
        <source>Defines:</source>
        <translation>Definitionen:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="47"/>
        <source>Project file: %1</source>
        <translation>Projektdatei: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="247"/>
        <source>Select Cppcheck build dir</source>
        <translation>Wähle Cppcheck-Erstellungsverzeichnis</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="283"/>
        <source>Visual Studio (*.sln *.vcxproj);;Compile database (compile_commands.json)</source>
        <translation>Visual Studio (*.sln *.vcxproj);;Compile-Datenbank (compile_commands.json)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="497"/>
        <source>Select include directory</source>
        <translation>Wähle Include-Verzeichnisse</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="477"/>
        <source>Select a directory to check</source>
        <translation>Wähle zu prüfendes Verzeichnis</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="172"/>
        <source>Clang-tidy (not found)</source>
        <translation>Clang-tidy (nicht gefunden)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="281"/>
        <source>Import Project</source>
        <translation>Projekt importieren</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="517"/>
        <source>Select directory to ignore</source>
        <translation>Wähle zu ignorierendes Verzeichnis</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="571"/>
        <source>Add Suppression</source>
        <translation>Fehlerunterdrückung hinzufügen</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="572"/>
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
        <location filename="translationhandler.cpp" line="132"/>
        <source>Language file %1 not found!</source>
        <translation>Sprachdatei %1 nicht gefunden!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="138"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>Die Übersetzungen der Sprache %1 konnten nicht aus der Datei %2 geladen werden</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="33"/>
        <source>line %1: Unhandled element %2</source>
        <translation>Zeile %1: Nicht behandeltes Element %2</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="144"/>
        <source> (Not found)</source>
        <translation> (nicht gefunden)</translation>
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
        <location filename="resultstree.cpp" line="1341"/>
        <source>File</source>
        <translation>Datei</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Severity</source>
        <translation>Schweregrad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Line</source>
        <translation>Zeile</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Summary</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="137"/>
        <source>Undefined file</source>
        <translation>Undefinierte Datei</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="841"/>
        <source>[Inconclusive]</source>
        <translation>[unklar]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="308"/>
        <source>debug</source>
        <translation>Debug</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="245"/>
        <source>note</source>
        <translation>Anmerkung</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="608"/>
        <source>Recheck</source>
        <translation>Erneut prüfen</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="609"/>
        <source>Copy filename</source>
        <translation>Dateiname kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="610"/>
        <source>Copy full path</source>
        <translation>Vollständigen Pfad kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="611"/>
        <source>Copy message</source>
        <translation>Meldung kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="612"/>
        <source>Copy message id</source>
        <translation>Meldungs-Id kopieren</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="613"/>
        <source>Hide</source>
        <translation>Verstecken</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="614"/>
        <source>Hide all with id</source>
        <translation>Verstecke alle mit gleicher ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="615"/>
        <source>Suppress selected id(s)</source>
        <translation>Ausgewählte ID(s) unterdrücken</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="616"/>
        <source>Open containing folder</source>
        <translation>Übergeordneten Ordner öffnen</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="656"/>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Tag</source>
        <translation>Tag</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="658"/>
        <source>No tag</source>
        <translation>Kein Tag</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="701"/>
        <location filename="resultstree.cpp" line="715"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="702"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <translation>Keine Editor-Anwendung eingestellt.

Konfigurieren Sie diese unter Einstellungen/Anwendungen.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="716"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>Keine Standard-Editor-Anwendung eingestellt.

 Bitte wählen Sie eine Standardanwendung unter Einstellungen/Anwendungen.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="745"/>
        <source>Could not find the file!</source>
        <translation>Datei konnte nicht gefunden werden!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="791"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>%1 konnte nicht gestartet werden.

Bitte überprüfen Sie ob der Pfad und die Parameter der Anwendung richtig eingestellt sind.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="805"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>Datei konnte nicht gefunden werden:
%1
Bitte wählen Sie das Verzeichnis, in dem sich die Datei befindet.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="812"/>
        <source>Select Directory</source>
        <translation>Wähle Verzeichnis</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Inconclusive</source>
        <translation>Unklar</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Since date</source>
        <translation>Seit Datum</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="290"/>
        <source>style</source>
        <translation>Stil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="293"/>
        <source>error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="296"/>
        <source>warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="299"/>
        <source>performance</source>
        <translation>Performance</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="302"/>
        <source>portability</source>
        <translation>Portabilität</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="305"/>
        <source>information</source>
        <translation>Information</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="198"/>
        <source>Print Report</source>
        <translation>Bericht drucken</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="217"/>
        <source>No errors found, nothing to print.</source>
        <translation>Keine Funde, nichts zu drucken.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="255"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 von %2 Dateien geprüft)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="268"/>
        <location filename="resultsview.cpp" line="279"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="269"/>
        <source>No errors found.</source>
        <translation>Keine Fehler gefunden.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="276"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Es wurden Fehler gefunden, aber sie sind so konfiguriert, ausgeblendet zu werden.
Legen Sie unter dem Menü Ansicht fest, welche Arten von Fehlern angezeigt werden sollen.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="322"/>
        <location filename="resultsview.cpp" line="343"/>
        <location filename="resultsview.cpp" line="351"/>
        <source>Failed to read the report.</source>
        <translation>Lesen des Berichts fehlgeschlagen.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="329"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>XML-Format-Version 1 wird nicht länger unterstützt.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="388"/>
        <source>Summary</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="389"/>
        <source>Message</source>
        <translation>Meldung</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="393"/>
        <source>First included by</source>
        <translation>Zuerst inkludiert von</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="396"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="443"/>
        <source>Clear Log</source>
        <translation>Protokoll leeren</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="444"/>
        <source>Copy this Log entry</source>
        <translation>Diesen Protokolleintrag kopieren</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="445"/>
        <source>Copy complete Log</source>
        <translation>Gesamtes Protokoll kopieren</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="156"/>
        <source>No errors found, nothing to save.</source>
        <translation>Keine Fehler gefunden, nichts zu speichern.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="180"/>
        <location filename="resultsview.cpp" line="188"/>
        <source>Failed to save the report.</source>
        <translation>Der Bericht konnte nicht speichern werden.</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>Berichte</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="82"/>
        <source>Analysis Log</source>
        <translation>Analyseprotokoll</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="104"/>
        <source>Warning Details</source>
        <translation>Warnungs-Details</translation>
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
        <location filename="settings.ui" line="195"/>
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
        <location filename="settings.ui" line="287"/>
        <source>Addons</source>
        <translation>Add-Ons</translation>
    </message>
    <message>
        <location filename="settings.ui" line="293"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation>Python-Binärdatei (Python aus PATH wird genutzt, wenn leer)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="302"/>
        <location filename="settings.ui" line="345"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="326"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="settings.ui" line="332"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation>Clang-Verzeichnis (PATH wird genutzt, wenn leer)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="355"/>
        <source>Visual Studio headers</source>
        <translation>Visual-Studio-Header</translation>
    </message>
    <message>
        <location filename="settings.ui" line="361"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Pfade zu Visual-Studio-Headern, Semikolon-getrennt.&lt;/p&gt;&lt;p&gt;Sie können eine Visual-Studio-Kommandozeile öffnen, &amp;quot;SET INCLUDE&amp;quot; eingeben und dann die Pfade hier reinkopieren.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="209"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="184"/>
        <source>Applications</source>
        <translation>Anwendungen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="202"/>
        <source>Edit...</source>
        <translation>Bearbeiten...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="216"/>
        <source>Set as default</source>
        <translation>Als Standard festlegen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="239"/>
        <source>Reports</source>
        <translation>Berichte</translation>
    </message>
    <message>
        <location filename="settings.ui" line="245"/>
        <source>Save all errors when creating report</source>
        <translation>Alle Fehler beim Erstellen von Berichten speichern</translation>
    </message>
    <message>
        <location filename="settings.ui" line="252"/>
        <source>Save full path to files in reports</source>
        <translation>Vollständigen Dateipfad in Berichten speichern</translation>
    </message>
    <message>
        <location filename="settings.ui" line="273"/>
        <source>Language</source>
        <translation>Sprache</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="88"/>
        <source>N/A</source>
        <translation>kA</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="186"/>
        <source>Add a new application</source>
        <translation>Neue Anwendung hinzufügen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="219"/>
        <source>Modify an application</source>
        <translation>Anwendung ändern</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="224"/>
        <source> [Default]</source>
        <translation> [Standard]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="249"/>
        <source>[Default]</source>
        <translation>[Standard]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="304"/>
        <source>Select python binary</source>
        <translation>Python-Binärdatei auswählen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="312"/>
        <source>Select clang path</source>
        <translation>Clang-Verzeichnis auswählen</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="228"/>
        <location filename="statsdialog.cpp" line="135"/>
        <location filename="statsdialog.cpp" line="181"/>
        <source>Statistics</source>
        <translation>Statistik</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="173"/>
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
        <location filename="statsdialog.cpp" line="177"/>
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
        <location filename="stats.ui" line="343"/>
        <source>History</source>
        <translation>Verlauf</translation>
    </message>
    <message>
        <location filename="stats.ui" line="349"/>
        <source>File:</source>
        <translation>Datei:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="387"/>
        <source>Copy to Clipboard</source>
        <translation>In die Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="stats.ui" line="394"/>
        <source>Pdf Export</source>
        <translation>PDF-Export</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>1 day</source>
        <translation>1 Tag</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>%1 days</source>
        <translation>%1 Tage</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>1 hour</source>
        <translation>1 Stunde</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>%1 hours</source>
        <translation>%1 Stunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>1 minute</source>
        <translation>1 Minute</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>%1 minutes</source>
        <translation>%1 Minuten</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>1 second</source>
        <translation>1 Sekunde</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>%1 seconds</source>
        <translation>%1 Sekunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="121"/>
        <source>0.%1 seconds</source>
        <translation>0,%1 Sekunden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="123"/>
        <source> and </source>
        <translation> und </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="150"/>
        <source>Export PDF</source>
        <translation>Exportiere PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="172"/>
        <source>Project Settings</source>
        <translation>Projekteinstellungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="174"/>
        <source>Paths</source>
        <translation>Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="175"/>
        <source>Include paths</source>
        <translation>Include-Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="176"/>
        <source>Defines</source>
        <translation>Definitionen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="178"/>
        <source>Path selected</source>
        <translation>Gewählte Pfade</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="179"/>
        <source>Number of files scanned</source>
        <translation>Anzahl geprüfter Dateien</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="180"/>
        <source>Scan duration</source>
        <translation>Prüfungsdauer</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="137"/>
        <location filename="statsdialog.cpp" line="182"/>
        <source>Errors</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="64"/>
        <source>File: </source>
        <translation>Datei: </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="64"/>
        <source>No cppcheck build dir</source>
        <translation>Kein Cppcheck-Analyseverzeichnis</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="139"/>
        <location filename="statsdialog.cpp" line="183"/>
        <source>Warnings</source>
        <translation>Warnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="141"/>
        <location filename="statsdialog.cpp" line="184"/>
        <source>Style warnings</source>
        <translation>Stilwarnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="143"/>
        <location filename="statsdialog.cpp" line="185"/>
        <source>Portability warnings</source>
        <translation>Portabilitätswarnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="145"/>
        <location filename="statsdialog.cpp" line="186"/>
        <source>Performance warnings</source>
        <translation>Performance-Warnungen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="147"/>
        <location filename="statsdialog.cpp" line="187"/>
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
        <location filename="translationhandler.cpp" line="144"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>Wechsel der Sprache der Benutzeroberfläche fehlgeschlagen:

%1

Die Sprache wurde auf Englisch zurückgesetzt. Öffnen Sie den Einstellungen-Dialog um eine verfügbare Sprache auszuwählen.</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="150"/>
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
