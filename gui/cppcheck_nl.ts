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
        <source>Copyright © 2007-2015 Daniel Marjamäki and cppcheck team.</source>
        <oldsource>Copyright © 2007-2013 Daniel Marjamäki and cppcheck team.</oldsource>
        <translation type="unfinished">Copyright (C) 2007-2009 Daniel Marjamäki en het cppcheck team.</translation>
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
        <location filename="application.ui" line="23"/>
        <source>Add an application</source>
        <translation>Voeg een nieuwe applicatie toe</translation>
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
        <translation>Hier kunt u toepassingen toevoegen die de foute bestanden kan openen. Geef naam op van de toepassing, het uitvoerbare bestand en command lijn parameters voor de toepassing

De volgende tekst in de parameters word vervangen door de juiste waarden wanneer de toepassing wordt uitgevoerd:
(bestand) - Bestandsnaam waarin de fout zit
(lijn) - Lijnnummer waar de fout zit
(bericht) Foutmelding
(ernst) Ernst van foutmelding

Voorbeeld een bestand openen met KonOs2 en laat KonOs2 scrollen naar de juiste lijn:
Uitvoerbaar: KonOs2
Parameters: -l(lijn) (bestand)</translation>
    </message>
    <message>
        <location filename="application.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>&amp;Naam:</translation>
    </message>
    <message>
        <location filename="application.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>&amp;Uitvoerbaar:</translation>
    </message>
    <message>
        <location filename="application.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation></translation>
    </message>
    <message>
        <location filename="application.ui" line="138"/>
        <source>Browse</source>
        <translation>Bladeren</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Uitvoerbare bestanden (*.exe);;Alle bestanden(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="62"/>
        <source>Select viewer application</source>
        <translation>Selecteer applicatie</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="77"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="78"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>Geef een naam op, een pad en eventueel parameters voor de toepassing!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="42"/>
        <source>Could not find the file: %1</source>
        <oldsource>Could not find the file:
</oldsource>
        <translation>Kon het bestand niet vinden: %1</translation>
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
        <translation>Kon het bestand niet lezen: %1</translation>
    </message>
</context>
<context>
    <name>LogView</name>
    <message>
        <location filename="logview.ui" line="17"/>
        <source>Checking Log</source>
        <translation>Controleer log</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="34"/>
        <source>Clear</source>
        <translation>Wis</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="68"/>
        <source>Save Log</source>
        <translation>Opslaan log</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="69"/>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation>Tekst bestanden (*.txt *.log);;Alle bestanden(*.*)</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="73"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="74"/>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation>Kan bestand: &quot;%1&quot; niet openen om te schrijven</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="main.ui" line="26"/>
        <location filename="mainwindow.cpp" line="278"/>
        <location filename="mainwindow.cpp" line="339"/>
        <location filename="mainwindow.cpp" line="395"/>
        <location filename="mainwindow.cpp" line="458"/>
        <location filename="mainwindow.cpp" line="480"/>
        <location filename="mainwindow.cpp" line="777"/>
        <location filename="mainwindow.cpp" line="882"/>
        <location filename="mainwindow.cpp" line="1002"/>
        <location filename="mainwindow.cpp" line="1170"/>
        <location filename="mainwindow.cpp" line="1251"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="170"/>
        <source>Standard</source>
        <translation>Standaard</translation>
    </message>
    <message>
        <location filename="main.ui" line="70"/>
        <source>&amp;File</source>
        <translation>&amp;Bestand</translation>
    </message>
    <message>
        <location filename="main.ui" line="85"/>
        <source>&amp;View</source>
        <translation>&amp;Weergave</translation>
    </message>
    <message>
        <location filename="main.ui" line="89"/>
        <source>&amp;Toolbars</source>
        <translation>&amp;Werkbalken</translation>
    </message>
    <message>
        <location filename="main.ui" line="127"/>
        <source>&amp;Check</source>
        <translation>&amp;Controleer</translation>
    </message>
    <message>
        <location filename="main.ui" line="131"/>
        <source>C++ standard</source>
        <translation>C++standaard</translation>
    </message>
    <message>
        <location filename="main.ui" line="138"/>
        <source>C standard</source>
        <translation>C standaard</translation>
    </message>
    <message>
        <location filename="main.ui" line="157"/>
        <source>&amp;Edit</source>
        <translation>Be&amp;werken</translation>
    </message>
    <message>
        <location filename="main.ui" line="218"/>
        <source>&amp;License...</source>
        <translation>&amp;Licentie...</translation>
    </message>
    <message>
        <location filename="main.ui" line="223"/>
        <source>A&amp;uthors...</source>
        <translation>A&amp;uteurs...</translation>
    </message>
    <message>
        <location filename="main.ui" line="232"/>
        <source>&amp;About...</source>
        <translation>&amp;Over...</translation>
    </message>
    <message>
        <location filename="main.ui" line="237"/>
        <source>&amp;Files...</source>
        <translation>&amp;Bestanden...</translation>
    </message>
    <message>
        <location filename="main.ui" line="240"/>
        <location filename="main.ui" line="243"/>
        <source>Check files</source>
        <translation>Controleer bestanden</translation>
    </message>
    <message>
        <location filename="main.ui" line="246"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="255"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Mappen...</translation>
    </message>
    <message>
        <location filename="main.ui" line="258"/>
        <location filename="main.ui" line="261"/>
        <source>Check directory</source>
        <translation>Controleer Map</translation>
    </message>
    <message>
        <location filename="main.ui" line="264"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="273"/>
        <source>&amp;Recheck files</source>
        <translation>&amp;Opnieuw controleren</translation>
    </message>
    <message>
        <location filename="main.ui" line="276"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="285"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stop</translation>
    </message>
    <message>
        <location filename="main.ui" line="288"/>
        <location filename="main.ui" line="291"/>
        <source>Stop checking</source>
        <translation>Stop controle</translation>
    </message>
    <message>
        <location filename="main.ui" line="294"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="303"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Resultaten opslaan...</translation>
    </message>
    <message>
        <location filename="main.ui" line="306"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="311"/>
        <source>&amp;Quit</source>
        <translation>&amp;Afsluiten</translation>
    </message>
    <message>
        <location filename="main.ui" line="320"/>
        <source>&amp;Clear results</source>
        <translation>&amp;Resultaten wissen</translation>
    </message>
    <message>
        <location filename="main.ui" line="329"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Voorkeuren</translation>
    </message>
    <message>
        <location filename="main.ui" line="359"/>
        <source>Errors</source>
        <translation>Fouten</translation>
    </message>
    <message>
        <location filename="main.ui" line="362"/>
        <location filename="main.ui" line="365"/>
        <source>Show errors</source>
        <translation>Toon fouten</translation>
    </message>
    <message>
        <location filename="main.ui" line="446"/>
        <source>Show S&amp;cratchpad...</source>
        <translation>Toon S&amp;cratchpad...</translation>
    </message>
    <message>
        <location filename="main.ui" line="496"/>
        <source>Warnings</source>
        <translation>Waarschuwingen</translation>
    </message>
    <message>
        <location filename="main.ui" line="499"/>
        <location filename="main.ui" line="502"/>
        <source>Show warnings</source>
        <translation>Toon waarschuwingen</translation>
    </message>
    <message>
        <location filename="main.ui" line="514"/>
        <source>Performance warnings</source>
        <translation>Presentatie waarschuwingen</translation>
    </message>
    <message>
        <location filename="main.ui" line="517"/>
        <location filename="main.ui" line="520"/>
        <source>Show performance warnings</source>
        <translation>Toon presentatie waarschuwingen</translation>
    </message>
    <message>
        <location filename="main.ui" line="528"/>
        <source>Show &amp;hidden</source>
        <translation>Toon &amp;verborgen</translation>
    </message>
    <message>
        <location filename="main.ui" line="540"/>
        <location filename="mainwindow.cpp" line="556"/>
        <location filename="mainwindow.cpp" line="588"/>
        <source>Information</source>
        <translation>Informatie</translation>
    </message>
    <message>
        <location filename="main.ui" line="543"/>
        <source>Show information messages</source>
        <translation>Toon informatie bericht</translation>
    </message>
    <message>
        <location filename="main.ui" line="555"/>
        <source>Portability</source>
        <translation>Portabiliteit</translation>
    </message>
    <message>
        <location filename="main.ui" line="558"/>
        <source>Show portability warnings</source>
        <translation>Toon portabiliteit waarschuwingen</translation>
    </message>
    <message>
        <location filename="main.ui" line="566"/>
        <source>&amp;Filter</source>
        <translation>&amp;Filter</translation>
    </message>
    <message>
        <location filename="main.ui" line="569"/>
        <source>Filter results</source>
        <translation>Filter resultaten</translation>
    </message>
    <message>
        <location filename="main.ui" line="585"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSI</translation>
    </message>
    <message>
        <location filename="main.ui" line="593"/>
        <source>Windows 32-bit Unicode</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="601"/>
        <source>Unix 32-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="609"/>
        <source>Unix 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="617"/>
        <source>Windows 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="625"/>
        <source>Platforms</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="639"/>
        <source>C++11</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="650"/>
        <source>C99</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="658"/>
        <source>Posix</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="666"/>
        <source>C11</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="674"/>
        <source>C89</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="682"/>
        <source>C++03</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="370"/>
        <source>&amp;Check all</source>
        <translation>&amp;Controleer alles</translation>
    </message>
    <message>
        <location filename="main.ui" line="207"/>
        <source>Filter</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="375"/>
        <source>&amp;Uncheck all</source>
        <translation>Selecteer &amp;niets</translation>
    </message>
    <message>
        <location filename="main.ui" line="380"/>
        <source>Collapse &amp;all</source>
        <translation>Alles Inkl&amp;appen</translation>
    </message>
    <message>
        <location filename="main.ui" line="385"/>
        <source>&amp;Expand all</source>
        <translation>Alles &amp;Uitklappen</translation>
    </message>
    <message>
        <location filename="main.ui" line="393"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standaard</translation>
    </message>
    <message>
        <location filename="main.ui" line="396"/>
        <source>Standard items</source>
        <translation>Standaard items</translation>
    </message>
    <message>
        <location filename="main.ui" line="412"/>
        <source>Toolbar</source>
        <translation>Werkbalk</translation>
    </message>
    <message>
        <location filename="main.ui" line="420"/>
        <source>&amp;Categories</source>
        <translation>&amp;Categorieën</translation>
    </message>
    <message>
        <location filename="main.ui" line="423"/>
        <source>Error categories</source>
        <translation>Foute Categorieën</translation>
    </message>
    <message>
        <location filename="main.ui" line="428"/>
        <source>&amp;Open XML...</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="437"/>
        <source>Open P&amp;roject File...</source>
        <translation>Open P&amp;oject bestand...</translation>
    </message>
    <message>
        <location filename="main.ui" line="451"/>
        <source>&amp;New Project File...</source>
        <translation>&amp;Nieuw Project Bestand...</translation>
    </message>
    <message>
        <location filename="main.ui" line="456"/>
        <source>&amp;Log View</source>
        <translation>&amp;Log weergave</translation>
    </message>
    <message>
        <location filename="main.ui" line="459"/>
        <source>Log View</source>
        <translation>Log weergave</translation>
    </message>
    <message>
        <location filename="main.ui" line="467"/>
        <source>C&amp;lose Project File</source>
        <translation>&amp;Sluit Project Bestand</translation>
    </message>
    <message>
        <location filename="main.ui" line="475"/>
        <source>&amp;Edit Project File...</source>
        <translation>&amp;Bewerk Project Bestand...</translation>
    </message>
    <message>
        <location filename="main.ui" line="484"/>
        <source>&amp;Statistics</source>
        <translation>&amp;Statistieken</translation>
    </message>
    <message>
        <location filename="main.ui" line="401"/>
        <source>&amp;Contents</source>
        <translation>&amp;Inhoud</translation>
    </message>
    <message>
        <location filename="main.ui" line="190"/>
        <source>Categories</source>
        <translation>Categorieën</translation>
    </message>
    <message>
        <location filename="main.ui" line="341"/>
        <source>Style warnings</source>
        <translation>Stijl waarschuwingen</translation>
    </message>
    <message>
        <location filename="main.ui" line="344"/>
        <location filename="main.ui" line="347"/>
        <source>Show style warnings</source>
        <translation>Toon stijl waarschuwingen</translation>
    </message>
    <message>
        <location filename="main.ui" line="404"/>
        <source>Open the help contents</source>
        <translation>Open de help inhoud</translation>
    </message>
    <message>
        <location filename="main.ui" line="407"/>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="main.ui" line="117"/>
        <source>&amp;Help</source>
        <translation>&amp;Help</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="421"/>
        <source>Select directory to check</source>
        <translation>Selecteer een map om te controleren</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="340"/>
        <source>No suitable files found to check!</source>
        <translation>Geen geschikte bestanden gevonden om te controleren!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="75"/>
        <source>Quick Filter:</source>
        <translation>Snel Filter:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="459"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>Project bestand gevonden: %1
Wilt u dit project laden in plaats van?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="481"/>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation>Project bestanden gevonden in de map.
Wil je verder wilt gaan zonder controle van deze project bestanden?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="565"/>
        <source>File not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="568"/>
        <source>Bad XML</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="571"/>
        <source>Missing attribute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="574"/>
        <source>Bad attribute value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="588"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="666"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="924"/>
        <source>License</source>
        <translation>Licentie</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="931"/>
        <source>Authors</source>
        <translation>Auteurs</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="939"/>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation>XML bestanden (*.xml);;Tekst bestanden (*.txt);;CSV bestanden (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="941"/>
        <source>Save the report file</source>
        <translation>Rapport opslaan </translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="793"/>
        <source>XML files (*.xml)</source>
        <translation>XML bestanden (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="273"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>Er was een probleem met het laden van de bewerker instellingen.

Dit is waarschijnlijk omdat de instellingen zijn gewijzigd tussen de versies van cppcheck. Controleer (en maak) de bewerker instellingen, anders zal de bewerker niet correct starten.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="396"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Je moet project bestanden sluiten voordat je nieuwe bestanden of mappen selekteerd!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="410"/>
        <source>Select files to check</source>
        <translation>Selecteer bestanden om te controleren</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="556"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="577"/>
        <source>Unsupported format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="580"/>
        <source>Duplicate platform type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="583"/>
        <source>Platform type redefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="666"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="778"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation>Huidige resultaten zullen worden gewist

Een nieuw XML-bestand openen zal de huidige resultaten wissen Wilt u verder gaan?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="795"/>
        <source>Open the report file</source>
        <translation>Open het rapport bestand</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="878"/>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?</source>
        <translation>Het controleren loopt.

Wil je het controleren stoppen en Cppcheck sluiten?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="948"/>
        <source>XML files version 1 (*.xml)</source>
        <translation>XML files version 1 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="952"/>
        <source>XML files version 2 (*.xml)</source>
        <translation>XML files version 2 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="956"/>
        <source>Text files (*.txt)</source>
        <translation>Tekst bestanden (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="960"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV bestanden (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1004"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1049"/>
        <location filename="mainwindow.cpp" line="1132"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Project bestanden (*.cppcheck);;Alle bestanden(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1051"/>
        <source>Select Project File</source>
        <translation>Selecteer project bestand</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1079"/>
        <location filename="mainwindow.cpp" line="1146"/>
        <source>Project:</source>
        <translation>Project:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1134"/>
        <source>Select Project Filename</source>
        <translation>Selecteer project bestandsnaam</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1171"/>
        <source>No project file loaded</source>
        <translation>Geen project bestand geladen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1246"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>Het project bestand

%1

Kan niet worden gevonden!
Wilt u het bestand van de onlangs gebruikte project verwijderen -lijst?</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="47"/>
        <source>Finnish</source>
        <translation>Fins</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="46"/>
        <source>English</source>
        <translation>Engels</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="44"/>
        <source>Chinese (Simplified)</source>
        <translation>Chinees (Vereenvoudigd)</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="45"/>
        <source>Dutch</source>
        <translation>Nederlands</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="48"/>
        <source>French</source>
        <translation>Frans</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="50"/>
        <source>Italian</source>
        <translation>Italiaans</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="52"/>
        <source>Korean</source>
        <translation>Koreaans</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="55"/>
        <source>Spanish</source>
        <translation>Spaans</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="56"/>
        <source>Swedish</source>
        <translation>Zweeds</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="49"/>
        <source>German</source>
        <translation>Duits</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="53"/>
        <source>Russian</source>
        <translation>Russisch</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="51"/>
        <source>Japanese</source>
        <oldsource>Japanease</oldsource>
        <translation>Japans</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="54"/>
        <source>Serbian</source>
        <translation>Servisch</translation>
    </message>
    <message>
        <source>Cppcheck GUI.

Syntax:
    cppcheck-gui [OPTIONS] [files or paths]

Options:
    -h, --help     Print this help
    -p &lt;file&gt;      Open given project file and start checking it
    -l &lt;file&gt;      Open given results xml file
    -d &lt;directory&gt; Specify the directory that was checked to generate the results xml specified with -l
    -v, --version  Show program version</source>
        <translation type="obsolete">Cppcheck GUI.
Syntax:
....cppcheck-gui [Opies] [bestanden of paden]

Opties:
.....-h, --help     Print deze help
.....-p &lt;bestand&gt;......Open project bestand en start de controle
.....-l &lt;bestand&gt;......Open gegeven resultaten xml bestand
.....-d &lt;map&gt; Geef de map aan wat gecontroleerd werd om de xml resultaten te genereren met gespecificeerde -l
.....-v,.--versie  Toon versie van programma</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.cpp" line="113"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation>Cppcheck GUI - Command lijn parameters</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <location filename="platforms.cpp" line="37"/>
        <source>Built-in</source>
        <translation>Gemaakt in</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Unix 32-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="39"/>
        <source>Unix 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="40"/>
        <source>Windows 32-bit ANSI</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="41"/>
        <source>Windows 32-bit Unicode</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="42"/>
        <source>Windows 64-bit</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Project</name>
    <message>
        <location filename="project.cpp" line="71"/>
        <location filename="project.cpp" line="123"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="project.cpp" line="72"/>
        <source>Could not read the project file.</source>
        <translation>Kon project bestand niet lezen.</translation>
    </message>
    <message>
        <location filename="project.cpp" line="124"/>
        <source>Could not write the project file.</source>
        <translation>Kon niet naar project bestand schrijven.</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation>Project Bestand</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <source>Project</source>
        <translation>Project</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="49"/>
        <source>Root:</source>
        <translation>Hoofdmap:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="66"/>
        <source>Libraries:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="75"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="89"/>
        <location filename="projectfile.ui" line="238"/>
        <source>Paths:</source>
        <translation>Paden:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="116"/>
        <location filename="projectfile.ui" line="179"/>
        <location filename="projectfile.ui" line="252"/>
        <source>Add...</source>
        <translation>Toevoegen...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="123"/>
        <location filename="projectfile.ui" line="186"/>
        <location filename="projectfile.ui" line="259"/>
        <source>Edit</source>
        <translation>Bewerk</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="130"/>
        <location filename="projectfile.ui" line="193"/>
        <location filename="projectfile.ui" line="266"/>
        <location filename="projectfile.ui" line="329"/>
        <source>Remove</source>
        <translation>Verwijder</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="155"/>
        <source>Includes</source>
        <translation>Inclusief</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="161"/>
        <source>Include directories:</source>
        <translation>Include mappen:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="213"/>
        <source>Up</source>
        <translation>Omhoog</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="220"/>
        <source>Down</source>
        <translation>Omlaag</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="232"/>
        <source>Exclude</source>
        <translation>Exclusief</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="291"/>
        <source>Suppressions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="297"/>
        <source>Suppression list:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="322"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="32"/>
        <source>Defines:</source>
        <translation>Omschrijft:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="42"/>
        <source>Project file: %1</source>
        <translation>Project Bestand %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="290"/>
        <source>Select include directory</source>
        <translation>Selecteer include map</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="313"/>
        <source>Select a directory to check</source>
        <translation>Selecteer een map om te controleren</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="353"/>
        <source>Select directory to ignore</source>
        <translation>Selecteer een map om te negeren</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="412"/>
        <source>Add Suppression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="413"/>
        <source>Select error id suppress:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QDialogButtonBox</name>
    <message>
        <location filename="translationhandler.cpp" line="31"/>
        <source>OK</source>
        <translation></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="32"/>
        <source>Cancel</source>
        <translation>Annuleer</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="33"/>
        <source>Close</source>
        <translation>Sluit</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Save</source>
        <translation>Opslaan</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="93"/>
        <source>Unknown language specified!</source>
        <translation>Onbekende taal gekozen!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="123"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation>Kon het taalbestand niet vinden: %1!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="129"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Kon de vertaling voor taal %1 in bestand %2 niet laden</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="1045"/>
        <source>File</source>
        <translation>Bestand</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1045"/>
        <source>Severity</source>
        <translation>Ernst</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1045"/>
        <source>Line</source>
        <translation>Regel</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1045"/>
        <source>Summary</source>
        <translation>Overzicht</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="102"/>
        <source>Undefined file</source>
        <translation>Niet gedefinieerd bestand</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="206"/>
        <location filename="resultstree.cpp" line="727"/>
        <source>[Inconclusive]</source>
        <translation>[Onduidelijk]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="264"/>
        <source>debug</source>
        <translation></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="533"/>
        <source>Copy filename</source>
        <translation>Kopier bestandsnaam</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="534"/>
        <source>Copy full path</source>
        <translation>Kopieer volledig pad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="535"/>
        <source>Copy message</source>
        <translation>Kopieer bericht</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="536"/>
        <source>Copy message id</source>
        <translation>Kopieer bericht id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="537"/>
        <source>Hide</source>
        <translation>Verberg</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="538"/>
        <source>Hide all with id</source>
        <translation>Verberg alles met id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="587"/>
        <location filename="resultstree.cpp" line="601"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="588"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>Er is geen bewerker toepassing geconfigureerd.

Configureer de bewerker toepassing voor cppcheck in voorkeuren/Applicaties.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="602"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>Geen standaard bewerker geselecteerd.
Selecteer de standaard bewerker in voorkeuren/Applicaties.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="631"/>
        <source>Could not find the file!</source>
        <translation>Kon het bestand niet vinden!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="677"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>Kon applicatie %1 niet starten

Gelieve te controleren of de het pad en de parameters correct zijn.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="691"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>%1
Selecteer de map waarin het bestand zich bevindt.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="698"/>
        <source>Select Directory</source>
        <translation>Selecteer map</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1045"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="246"/>
        <source>style</source>
        <translation>Stijlfouten</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="249"/>
        <source>error</source>
        <translation>Fouten</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="252"/>
        <source>warning</source>
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="255"/>
        <source>performance</source>
        <translation>Presentatie</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="258"/>
        <source>portability</source>
        <translation>Portabiliteit</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="261"/>
        <source>information</source>
        <translation>Informatie</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="201"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 van %2 bestanden gecontroleerd)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="214"/>
        <location filename="resultsview.cpp" line="225"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="215"/>
        <source>No errors found.</source>
        <translation>Geen fouten gevonden.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="222"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Fouten werden gevonden, maar volgens de configuratie zijn deze verborgen.
Gebruik het uitzicht menu om te selecteren welke fouten getoond worden.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="268"/>
        <location filename="resultsview.cpp" line="286"/>
        <location filename="resultsview.cpp" line="294"/>
        <source>Failed to read the report.</source>
        <translation>Kon rapport niet lezen.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="331"/>
        <source>Summary</source>
        <translation>Overzicht</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="332"/>
        <source>Message</source>
        <translation>Bericht</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="334"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="140"/>
        <source>No errors found, nothing to save.</source>
        <translation>Geen fouten gevonden; geen data om op te slaan.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="167"/>
        <location filename="resultsview.cpp" line="175"/>
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
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <source>Scratchpad</source>
        <translation>Scratchpad</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="48"/>
        <source>filename</source>
        <translation>bestandsnaam</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="55"/>
        <source>Check</source>
        <translation>Controleer</translation>
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
        <location filename="settings.ui" line="169"/>
        <source>Include paths:</source>
        <translation>Include paden:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="187"/>
        <location filename="settings.ui" line="237"/>
        <source>Add...</source>
        <translation>Toevoegen...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>Aantal threads: </translation>
    </message>
    <message>
        <location filename="settings.ui" line="85"/>
        <source>Ideal count:</source>
        <translation>Ideale telling:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="114"/>
        <source>Force checking all #ifdef configurations</source>
        <oldsource>Check all #ifdef configurations</oldsource>
        <translation>Controleer alle #ifdef combinaties</translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Show full path of files</source>
        <translation>Toon het volledige pad van bestanden</translation>
    </message>
    <message>
        <location filename="settings.ui" line="128"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Toon &quot;Geen fouten gevonden&quot; indien geen fouten gevonden werden</translation>
    </message>
    <message>
        <location filename="settings.ui" line="135"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>Toon fout ld in kolom &quot;Id&quot;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="142"/>
        <source>Enable inline suppressions</source>
        <translation>Schakel inline suppressies in</translation>
    </message>
    <message>
        <location filename="settings.ui" line="163"/>
        <source>Paths</source>
        <translation>Paden</translation>
    </message>
    <message>
        <location filename="settings.ui" line="194"/>
        <source>Edit</source>
        <translation>Bewerk</translation>
    </message>
    <message>
        <location filename="settings.ui" line="201"/>
        <location filename="settings.ui" line="251"/>
        <source>Remove</source>
        <translation>Verwijder</translation>
    </message>
    <message>
        <location filename="settings.ui" line="226"/>
        <source>Applications</source>
        <translation>Applicaties</translation>
    </message>
    <message>
        <location filename="settings.ui" line="244"/>
        <source>Edit...</source>
        <translation>Bewerk...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="258"/>
        <source>Set as default</source>
        <translation>Instellen als standaard</translation>
    </message>
    <message>
        <location filename="settings.ui" line="281"/>
        <source>Reports</source>
        <translation>Rapporten</translation>
    </message>
    <message>
        <location filename="settings.ui" line="287"/>
        <source>Save all errors when creating report</source>
        <translation>Alle fouten opslaan</translation>
    </message>
    <message>
        <location filename="settings.ui" line="294"/>
        <source>Save full path to files in reports</source>
        <translation>Volledig pad opslaan</translation>
    </message>
    <message>
        <location filename="settings.ui" line="315"/>
        <source>Language</source>
        <translation>Taal</translation>
    </message>
    <message>
        <location filename="settings.ui" line="329"/>
        <source>Advanced</source>
        <translation>Geavanceerd</translation>
    </message>
    <message>
        <location filename="settings.ui" line="335"/>
        <source>&amp;Show inconclusive errors</source>
        <translation>&amp;Toon onduidelijke fouten</translation>
    </message>
    <message>
        <location filename="settings.ui" line="342"/>
        <source>S&amp;how internal warnings in log</source>
        <translation>T&amp;oon interne waarschuwingen in log</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="82"/>
        <source>N/A</source>
        <translation></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="203"/>
        <source>Add a new application</source>
        <translation>Nieuwe applicatie toevoegen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="236"/>
        <source>Modify an application</source>
        <translation>Applicatie wijzigen</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="263"/>
        <source>[Default]</source>
        <translation>[Standaard]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="314"/>
        <source>Select include directory</source>
        <translation>Selecteer include map</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="228"/>
        <location filename="statsdialog.cpp" line="105"/>
        <source>Statistics</source>
        <translation>Statistieken</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="97"/>
        <source>Project</source>
        <translation>Project</translation>
    </message>
    <message>
        <location filename="stats.ui" line="33"/>
        <source>Project:</source>
        <translation>Project:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="53"/>
        <source>Paths:</source>
        <translation>Paden:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="85"/>
        <source>Include paths:</source>
        <translation>Bevat paden:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation>Omschrijft:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="145"/>
        <location filename="statsdialog.cpp" line="101"/>
        <source>Previous Scan</source>
        <translation>Vorige scan</translation>
    </message>
    <message>
        <location filename="stats.ui" line="151"/>
        <source>Path Selected:</source>
        <translation>Pad Geselekteerd:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="161"/>
        <source>Number of Files Scanned:</source>
        <translation>Aantal bestanden gescanned:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Scan Duration:</source>
        <translation>Scan tijd:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="236"/>
        <source>Errors:</source>
        <translation>Fouten:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="254"/>
        <source>Warnings:</source>
        <translation>Waarschuwingen:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="272"/>
        <source>Stylistic warnings:</source>
        <translation>Stilistisch waarschuwingen:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="290"/>
        <source>Portability warnings:</source>
        <translation>Portabiliteit waarschuwingen:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="308"/>
        <source>Performance issues:</source>
        <translation>Presentatie problemen:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="326"/>
        <source>Information messages:</source>
        <translation>Informatie bericht:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="364"/>
        <source>Copy to Clipboard</source>
        <translation>Kopieer naar Clipbord</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="77"/>
        <source>1 day</source>
        <translation>1 dag</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="77"/>
        <source>%1 days</source>
        <translation>%1 dagen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="79"/>
        <source>1 hour</source>
        <translation>1 uur</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="79"/>
        <source>%1 hours</source>
        <translation>%1 uren</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>1 minute</source>
        <translation>1 minuut</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>%1 minutes</source>
        <translation>%1 minuten</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>1 second</source>
        <translation>1 seconde</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>%1 seconds</source>
        <translation>%1 secondes</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="87"/>
        <source>0.%1 seconds</source>
        <translation>0.%1 secondes</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="89"/>
        <source> and </source>
        <translation> en </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="96"/>
        <source>Project Settings</source>
        <translation>Project instellingen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="98"/>
        <source>Paths</source>
        <translation>Paden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="99"/>
        <source>Include paths</source>
        <translation>Bevat paden</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="100"/>
        <source>Defines</source>
        <translation>Omschrijft</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="102"/>
        <source>Path selected</source>
        <translation>Pad Geselekteerd</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="103"/>
        <source>Number of files scanned</source>
        <translation>Aantal bestanden gescanned</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="104"/>
        <source>Scan duration</source>
        <translation>Scan tijd</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="106"/>
        <source>Errors</source>
        <translation>Fouten</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="107"/>
        <source>Warnings</source>
        <translation>Waarschuwingen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="108"/>
        <source>Style warnings</source>
        <translation>Stijl waarschuwingen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="109"/>
        <source>Portability warnings</source>
        <translation>Portabiliteit waarschuwingen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="110"/>
        <source>Performance warnings</source>
        <translation>Presentatie waarschuwingen</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>Information messages</source>
        <translation>Informatie bericht</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="54"/>
        <source>%1 of %2 files checked</source>
        <translation>%1 van %2 bestanden gecontroleerd</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="135"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>Mislukt om de gebruikers taal te wijzigen:

%1

De gebruikerstaal is gereset naar Engels. Open het dialoogvenster om een van de beschikbare talen te selecteren.</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="141"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="73"/>
        <source>inconclusive</source>
        <translation>Onduidelijk</translation>
    </message>
</context>
</TS>
