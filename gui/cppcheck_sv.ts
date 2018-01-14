<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="sv_SE">
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
    <message>
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-2018 Cppcheck team.</source>
        <translation>Copyright © 2007-2018 Cppcheck team.</translation>
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
        <location filename="application.ui" line="23"/>
        <source>Add an application</source>
        <translation>Lägg till program</translation>
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
        <translation>Här kan du ange en applikation som kan användas för att visa fel. Ange applikationens namn, körbara fil samt kommandorads parametrar.

Följande texter i parametrarna ersätts med motsvarande värden när applikationen körs:
(file) - filnamn för källkodsfil
(line) - radnummer
(message) - felmeddelande
(severity) - typ / svårighetsgrad

Exempel för att öppna en fil med Kate och ange att Kate skall skrolla till rätt rad:
Körbar fil: kate
Parametrar: -l(line) (file)</translation>
    </message>
    <message>
        <location filename="application.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>Namn:</translation>
    </message>
    <message>
        <location filename="application.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>Körbar fil:</translation>
    </message>
    <message>
        <location filename="application.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>Parametrar:</translation>
    </message>
    <message>
        <location filename="application.ui" line="138"/>
        <source>Browse</source>
        <translation>Bläddra</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Exekverbara filer (*.exe);;Alla filer(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="62"/>
        <source>Select viewer application</source>
        <translation>Välj program</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="77"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="78"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>Du måste ange namn, sökväg samt eventuellt parametrar för applikationen!</translation>
    </message>
    <message>
        <source>You must specify a name, a path and parameters for the application!</source>
        <translation type="obsolete">Du måste ange ett namn, en sökväg samt parametrar för programmet!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="42"/>
        <source>Could not find the file: %1</source>
        <oldsource>Could not find the file:
</oldsource>
        <translation>Kunde inte hitta filen: %1</translation>
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
        <translation>Kunde inte läsa filen: %1</translation>
    </message>
</context>
<context>
    <name>LibraryAddFunctionDialog</name>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="23"/>
        <source>Add function</source>
        <translation>Lägg till funktion</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <source>Function name(s)</source>
        <translation>Funktion namn</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <source>Number of arguments</source>
        <translation>Antal argument</translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <source>Library Editor</source>
        <translation>Library Editor</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <source>Open</source>
        <translation>Öppna</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <source>Save</source>
        <translation>Spara</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <source>Save as</source>
        <translation>Spara som</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <source>Functions</source>
        <translation>Funktioner</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <source>Sort</source>
        <translation>Sortera</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <source>Add</source>
        <translation>Lägg till</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <source>Filter:</source>
        <translation>Filter:</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <source>Comments</source>
        <translation>Kommentar</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="204"/>
        <source>noreturn</source>
        <translation>noreturn</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="212"/>
        <source>False</source>
        <translation>False</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="217"/>
        <source>True</source>
        <translation>True</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="222"/>
        <source>Unknown</source>
        <translation>Vet ej</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="232"/>
        <source>return value must be used</source>
        <translation>retur värde måste användas</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <source>ignore function in leaks checking</source>
        <translation>Ignorera funktionen när cppcheck letar efter läckor</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <source>Arguments</source>
        <translation>Argument</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <source>Edit</source>
        <translation>Redigera</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="82"/>
        <location filename="librarydialog.cpp" line="154"/>
        <source>Library files (*.cfg)</source>
        <translation>Library fil (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="84"/>
        <source>Open library file</source>
        <translation>Öppna Library fil</translation>
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
        <translation>Kunde ej öppna filen %1.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="108"/>
        <source>Failed to load %1. %2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="145"/>
        <source>Can not save file %1.</source>
        <translation>Kunde ej spara filen %1.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="157"/>
        <source>Save the library as</source>
        <translation>Spara library som</translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <source>Edit argument</source>
        <translation>Konfigurera argument</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="20"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is bool value allowed? For instance result from comparison or from &apos;!&apos; operator.&lt;/p&gt;
&lt;p&gt;Typically, set this if the argument is a pointer, size, etc.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // last argument should not have a bool value&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation>Är bool värde tillåtet? Exempelvis resultatet från jämförelse eller från ! operatorn.
Normalt bör inte bool värde användas om argumentet är en pekare eller en storlek etc.
Exempel:
    memcmp(x, y, i == 123);   // sista argumentet bör inte vara ett bool värde</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="28"/>
        <source>Not bool</source>
        <translation>Ej bool</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="35"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is a null parameter value allowed?&lt;/p&gt;
&lt;p&gt;Typically this should be used on any pointer parameter that does not allow null.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // neither x or y is allowed to be null.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation>Är null värde tillåtet?
Klicka i denna om argumentet är en pointer parameter som ej tillåter null.
Exempel:
    strcpy(x,y); // varken x eller y får vara null.</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="43"/>
        <source>Not null</source>
        <translation>Ej null</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="50"/>
        <source>Not uninit</source>
        <translation>Ej uninit</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="57"/>
        <source>String</source>
        <translation>Sträng</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="70"/>
        <source>Format string</source>
        <translation>Format sträng</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="92"/>
        <source>Min size of buffer</source>
        <translation>Minsta storlek för buffer</translation>
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
        <translation>Ingen</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <source>argvalue</source>
        <translation>argvalue</translation>
    </message>
    <message>
        <source>constant</source>
        <translation type="vanished">constant</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <source>mul</source>
        <translation>mul</translation>
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
        <translation>Arg</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="159"/>
        <location filename="libraryeditargdialog.ui" line="264"/>
        <source>Arg2</source>
        <translation>Arg2</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="194"/>
        <source>and</source>
        <translation>och</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="310"/>
        <source>Valid values</source>
        <translation>Tillåtna värden</translation>
    </message>
</context>
<context>
    <name>LogView</name>
    <message>
        <source>Checking Log</source>
        <translation type="vanished">Analys logg</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation type="obsolete">&amp;Spara</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">Töm</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="obsolete">Stäng</translation>
    </message>
    <message>
        <source>Save Log</source>
        <translation type="vanished">Spara logg</translation>
    </message>
    <message>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation type="vanished">Text filer (*.txt *.log);;Alla filer (*.*)</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation type="vanished">Kunde ej öppna fil för skrivning: &quot;%1&quot;</translation>
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
        <location filename="mainwindow.ui" line="131"/>
        <source>A&amp;nalyze</source>
        <translation>Analysera</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="180"/>
        <source>Standard</source>
        <translation>Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="70"/>
        <source>&amp;File</source>
        <translation>&amp;Arkiv</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="89"/>
        <source>&amp;View</source>
        <translation>&amp;Visa</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="93"/>
        <source>&amp;Toolbars</source>
        <translation>Verktygsfält</translation>
    </message>
    <message>
        <source>&amp;Check</source>
        <translation type="obsolete">&amp;Check</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="135"/>
        <source>C++ standard</source>
        <translation>C++ standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="143"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation>C standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="167"/>
        <source>&amp;Edit</source>
        <translation>&amp;Redigera</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="228"/>
        <source>&amp;License...</source>
        <translation>&amp;Licens...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="233"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Utvecklat av...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="242"/>
        <source>&amp;About...</source>
        <translation>&amp;Om...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="247"/>
        <source>&amp;Files...</source>
        <translation>&amp;Filer...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="250"/>
        <location filename="mainwindow.ui" line="253"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation>Analysera filer</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="256"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="265"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Katalog...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="268"/>
        <location filename="mainwindow.ui" line="271"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation>Analysera mapp</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="274"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>&amp;Recheck files</source>
        <translation type="obsolete">Starta &amp;om check</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="286"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <source>&amp;Reanalyze all files</source>
        <oldsource>&amp;Recheck all files</oldsource>
        <translation type="obsolete">Analysera om alla filer</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="304"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stoppa</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="307"/>
        <location filename="mainwindow.ui" line="310"/>
        <source>Stop analysis</source>
        <oldsource>Stop checking</oldsource>
        <translation>Stoppa analys</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="313"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="322"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Spara resultat till fil...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="325"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="330"/>
        <source>&amp;Quit</source>
        <translation>&amp;Avsluta</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="339"/>
        <source>&amp;Clear results</source>
        <translation>&amp;Töm resultat</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="348"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Inställningar</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation type="vanished">Fel</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="381"/>
        <location filename="mainwindow.ui" line="384"/>
        <source>Show errors</source>
        <translation>Visa fel</translation>
    </message>
    <message>
        <source>Show S&amp;cratchpad...</source>
        <translation type="vanished">Visa s&amp;cratchpad...</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation type="vanished">Varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="518"/>
        <location filename="mainwindow.ui" line="521"/>
        <source>Show warnings</source>
        <translation>Visa varningar</translation>
    </message>
    <message>
        <source>Performance warnings</source>
        <translation type="vanished">Prestanda varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="536"/>
        <location filename="mainwindow.ui" line="539"/>
        <source>Show performance warnings</source>
        <translation>Visa prestanda varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="547"/>
        <source>Show &amp;hidden</source>
        <translation>Visa dolda</translation>
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
        <translation>Visa informations meddelanden</translation>
    </message>
    <message>
        <source>Portability</source>
        <translation type="vanished">Portabilitet</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="577"/>
        <source>Show portability warnings</source>
        <translation>Visa portabilitets varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="592"/>
        <source>Show Cppcheck results</source>
        <translation>Visa Cppcheck resultat</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="604"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="607"/>
        <source>Show Clang results</source>
        <translation>Visa Clang resultat</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="615"/>
        <source>&amp;Filter</source>
        <translation>&amp;Filter</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="618"/>
        <source>Filter results</source>
        <translation>Filtrera resultat</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="634"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSI</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="642"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
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
        <source>Platforms</source>
        <translation type="vanished">Plattformar</translation>
    </message>
    <message>
        <source>C++11</source>
        <translation type="vanished">C++11</translation>
    </message>
    <message>
        <source>C99</source>
        <translation type="vanished">C99</translation>
    </message>
    <message>
        <source>Posix</source>
        <translation type="vanished">Posix</translation>
    </message>
    <message>
        <source>C11</source>
        <translation type="vanished">C11</translation>
    </message>
    <message>
        <source>C89</source>
        <translation type="vanished">C89</translation>
    </message>
    <message>
        <source>C++03</source>
        <translation type="vanished">C++03</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="736"/>
        <source>&amp;Print...</source>
        <translation>Skriv ut...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="739"/>
        <source>Print the Current Report</source>
        <translation>Skriv ut aktuell rapport</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="744"/>
        <source>Print Pre&amp;view...</source>
        <translation>Förhandsgranska utskrift...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="747"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>Öppnar förhandsgranskning för nuvarande resultat</translation>
    </message>
    <message>
        <source>Library Editor...</source>
        <translation type="vanished">Library Editor...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="755"/>
        <source>Open library editor</source>
        <translation>Öppna library editor</translation>
    </message>
    <message>
        <source>Auto-detect language</source>
        <translation type="vanished">Välj språk automatiskt</translation>
    </message>
    <message>
        <source>Enforce C++</source>
        <translation type="vanished">C++</translation>
    </message>
    <message>
        <source>Enforce C</source>
        <translation type="vanished">C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="389"/>
        <source>&amp;Check all</source>
        <translation>&amp;Kryssa alla</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="217"/>
        <source>Filter</source>
        <translation>Filter</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="283"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation>Analysera om ändrade filer</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="295"/>
        <source>Reanal&amp;yze all files</source>
        <translation>Analysera om alla filer</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="360"/>
        <source>Style war&amp;nings</source>
        <translation>Style varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="378"/>
        <source>E&amp;rrors</source>
        <translation>Fel</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="394"/>
        <source>&amp;Uncheck all</source>
        <translation>Kryssa &amp;ur alla</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="399"/>
        <source>Collapse &amp;all</source>
        <translatorcomment>Ingen bra översättning!</translatorcomment>
        <translation>&amp;Fäll ihop alla</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="404"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Expandera alla</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="412"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="415"/>
        <source>Standard items</source>
        <translation>Standard poster</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="431"/>
        <source>Toolbar</source>
        <translation>Verktygsfält</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="439"/>
        <source>&amp;Categories</source>
        <translation>&amp;Kategorier</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="442"/>
        <source>Error categories</source>
        <translation>Fel kategorier</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="447"/>
        <source>&amp;Open XML...</source>
        <translation>&amp;Öppna XML...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="456"/>
        <source>Open P&amp;roject File...</source>
        <translation>Öppna Projektfil...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="465"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation>Visa Scratchpad...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="470"/>
        <source>&amp;New Project File...</source>
        <translation>Ny projektfil...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="475"/>
        <source>&amp;Log View</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="478"/>
        <source>Log View</source>
        <translation>Logg vy</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="486"/>
        <source>C&amp;lose Project File</source>
        <translation>Stäng projektfil</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="494"/>
        <source>&amp;Edit Project File...</source>
        <translation>Redigera projektfil...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="503"/>
        <source>&amp;Statistics</source>
        <translation>Statistik</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="515"/>
        <source>&amp;Warnings</source>
        <translation>Varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="533"/>
        <source>Per&amp;formance warnings</source>
        <translation>Optimerings varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="559"/>
        <source>&amp;Information</source>
        <translation>Information</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="574"/>
        <source>&amp;Portability</source>
        <translation>Portabilitet</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="674"/>
        <source>P&amp;latforms</source>
        <translation>Plattformar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="688"/>
        <source>C++&amp;11</source>
        <translation>C++11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="699"/>
        <source>C&amp;99</source>
        <translation>C99</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="707"/>
        <source>&amp;Posix</source>
        <translation>Posix</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="715"/>
        <source>C&amp;11</source>
        <translation>C11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="723"/>
        <source>&amp;C89</source>
        <translation>C89</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="731"/>
        <source>&amp;C++03</source>
        <translation>C++03</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="752"/>
        <source>&amp;Library Editor...</source>
        <translation>Library Editor...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="763"/>
        <source>&amp;Auto-detect language</source>
        <translation>Detektera språk automatiskt</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="771"/>
        <source>&amp;Enforce C++</source>
        <translation>Tvinga C++</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="779"/>
        <source>E&amp;nforce C</source>
        <translation>Tvinga C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="790"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="420"/>
        <source>&amp;Contents</source>
        <translation>&amp;Innehåll</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="198"/>
        <source>Categories</source>
        <translation>Kategorier</translation>
    </message>
    <message>
        <source>Style warnings</source>
        <translation type="vanished">Stil varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="363"/>
        <location filename="mainwindow.ui" line="366"/>
        <source>Show style warnings</source>
        <translation>Visa stil varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="423"/>
        <source>Open the help contents</source>
        <translation>Öppna hjälp</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="426"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="121"/>
        <source>&amp;Help</source>
        <translation>&amp;Hjälp</translation>
    </message>
    <message>
        <source>Select directory to check</source>
        <translation type="obsolete">Välj katalog som skall kontrolleras</translation>
    </message>
    <message>
        <source>No suitable files found to check!</source>
        <translation type="obsolete">Inga lämpliga filer hittades!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="85"/>
        <source>Quick Filter:</source>
        <translation>Snabbfilter:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="572"/>
        <source>C/C++ Source, Compile database, Visual Studio (%1 %2 *.sln *.vcxproj)</source>
        <translation>C/C++ källkod, Compile database, Visual Studio (%1 %2 *.sln *.vcxproj)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="621"/>
        <source>Select configuration</source>
        <translation>Välj konfiguration</translation>
    </message>
    <message>
        <source>Select the configuration that will be checked</source>
        <translation type="obsolete">Välj konfiguration som kommer analyseras</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="651"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>Hittade projektfil: %1

Vill du ladda denna projektfil istället?</translation>
    </message>
    <message>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation type="obsolete">Hittade projektfil(er) i mappen.

Vill du fortsätta analysen utan att använda någon av dessa projektfiler?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="770"/>
        <source>File not found</source>
        <translation>Filen hittades ej</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="773"/>
        <source>Bad XML</source>
        <translation>Ogiltig XML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="776"/>
        <source>Missing attribute</source>
        <translation>Attribut finns ej</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="779"/>
        <source>Bad attribute value</source>
        <translation>Ogiltigt attribut värde</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="782"/>
        <source>Unsupported format</source>
        <translation>Format stöds ej</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="799"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>Misslyckades att ladda valda library &apos;%1&apos;.
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1233"/>
        <source>License</source>
        <translation>Licens</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1240"/>
        <source>Authors</source>
        <translation>Utvecklare</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation type="obsolete">XML filer version 2 (*.xml);;XML filer version 1 (*.xml);;Text filer (*.txt);;CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1255"/>
        <source>Save the report file</source>
        <translation>Spara rapport</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1092"/>
        <location filename="mainwindow.cpp" line="1262"/>
        <source>XML files (*.xml)</source>
        <translation>XML filer (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="322"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>Det uppstod ett problem när programinställningarna skulle laddas.

En trolig orsak är att inställningarna ändrats för olika Cppcheck versioner. Kontrollera programinställningarna.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="555"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Du måste stänga projektfilen innan nya filer eller sökvägar kan väljas!</translation>
    </message>
    <message>
        <source>Select files to check</source>
        <translation type="obsolete">Välj filer att kontrollera</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="761"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>Library filen &apos;%1&apos; har element som ej hanteras:
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="785"/>
        <source>Duplicate platform type</source>
        <translation>Dubbel plattformstyp</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="788"/>
        <source>Platform type redefined</source>
        <translation>Plattformstyp definieras igen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Unknown element</source>
        <translation>Element hanteras ej</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="794"/>
        <source>Unknown issue</source>
        <translation>Något problem</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="895"/>
        <source>Error</source>
        <translation>Fel</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="895"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation>Misslyckades att ladda %1. Din Cppcheck installation är ej komplett. Du kan använda --data-dir&lt;directory&gt; på kommandoraden för att specificera var denna fil finns. Det är meningen att --data-dir kommandot skall köras under installationen,så GUIt kommer ej visas när --data-dir används allt som händer är att en inställning görs.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1077"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation>Nuvarande resultat kommer rensas bort.

När en ny XML fil öppnas så tas alla nuvarande resultat bort. Vill du fortsätta?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1094"/>
        <source>Open the report file</source>
        <translation>Öppna rapportfilen</translation>
    </message>
    <message>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?</source>
        <translation type="obsolete">Cppcheck kör.

Vill du stoppa analysen och avsluta Cppcheck?</translation>
    </message>
    <message>
        <source>XML files version 1 (*.xml)</source>
        <translation type="obsolete">XML filer version 1 (*.xml)</translation>
    </message>
    <message>
        <source>Deprecated XML format</source>
        <translation type="obsolete">Gammalt XML format</translation>
    </message>
    <message>
        <source>XML format 1 is deprecated and will be removed in cppcheck 1.81.</source>
        <translation type="obsolete">XML format 1 är gammalt och stödet kommer tas bort i Cppcheck 1.81</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml)</source>
        <translation type="obsolete">XML filer version 2 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1266"/>
        <source>Text files (*.txt)</source>
        <translation>Text filer (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1270"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1314"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation type="obsolete">Misslyckades att ändra språk:

%1

Språket har nollställts till Engelska. Öppna Preferences och välj något av de tillgängliga språken.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1361"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Projektfiler (*.cppcheck);;Alla filer(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1363"/>
        <source>Select Project File</source>
        <translation>Välj projektfil</translation>
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
        <translation>Inga filer hittades att analysera!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="570"/>
        <source>Select files to analyze</source>
        <translation>Välj filer att analysera</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="584"/>
        <source>Select directory to analyze</source>
        <translation>Välj mapp att analysera</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="621"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>Välj konfiguration som kommer analyseras</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="673"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation>Hittade projekt filer i mappen.

Vill du fortsätta analysen utan att använda någon av dessa projekt filer?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1187"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>Analys körs.

Vill du stoppa analysen och avsluta Cppcheck?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1253"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML filer (*.xml);;Text filer (*.txt);;CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1450"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>Build dir &apos;%1&apos; existerar ej, skapa den?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1467"/>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation>Misslyckades att importera &apos;%1&apos;, analysen stoppas</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1500"/>
        <source>Project files (*.cppcheck)</source>
        <translation>Projekt filer (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1502"/>
        <source>Select Project Filename</source>
        <translation>Välj Projektfil</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1547"/>
        <source>No project file loaded</source>
        <translation>Inget projekt laddat</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1607"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>Projektfilen

%1

 kunde inte hittas!

Vill du ta bort filen från &apos;senast använda projekt&apos;-listan?</translation>
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
    cppcheck-gui [OPTIONS] [files or paths]

Options:
    -h, --help     Print this help
    -p &lt;file&gt;      Open given project file and start checking it
    -l &lt;file&gt;      Open given results xml file
    -d &lt;directory&gt; Specify the directory that was checked to generate the results xml specified with -l
    -v, --version  Show program version</translation>
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
        <oldsource>Cppcheck GUI.

Syntax:
    cppcheck-gui [OPTIONS] [files or paths]

Options:
    -h, --help              Print this help
    -p &lt;file&gt;               Open given project file and start checking it
    -l &lt;file&gt;               Open given results xml file
    -d &lt;directory&gt;          Specify the directory that was checked to generate the results xml specified with -l
    -v, --version           Show program version
    --data-dir=&lt;directory&gt;  Specify directory where GUI datafiles are located (translations, cfg)</oldsource>
        <translation>Cppcheck GUI.

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
                            is used.</translation>
    </message>
    <message>
        <location filename="main.cpp" line="120"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation>Cppcheck GUI - Command line parameters</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <source>Built-in</source>
        <translation type="obsolete">Generell</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="37"/>
        <source>Native</source>
        <translation>Native</translation>
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
        <translation>Windows 32-bit ANSI</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="41"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
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
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <source>Could not read the project file.</source>
        <translation type="vanished">Kunde ej läsa projektfilen.</translation>
    </message>
    <message>
        <source>Could not write the project file.</source>
        <translation type="vanished">Kunde ej skriva projektfilen</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfiledialog.ui" line="14"/>
        <source>Project File</source>
        <translation>Projektfil</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="273"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;In the build dir, cppcheck stores data about each translation unit.&lt;/p&gt;&lt;p&gt;With a build dir you get whole program analysis.&lt;/p&gt;&lt;p&gt;Unchanged files will be analyzed much faster; Cppcheck skip the analysis of these files and reuse their old data.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="vanished">I build dir sparar Cppcheck information för varje translation unit.
Med build dir får du whole program analys.
Omodifierade filer analyseras mycket fortare, Cppcheck hoppar över analysen och återanvänder den gamla informationen</translation>
    </message>
    <message>
        <source>Cppcheck build dir (whole program analysis, faster analysis for unchanged files)</source>
        <translation type="vanished">Cppcheck build dir (whole program analys, snabbare analys för omodifierade filer)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation>Sökvägar och defines</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Cppcheck can import Visual studio solutions (*.sln), Visual studio projects (*.vcxproj) or compile databases.&lt;/p&gt;&lt;p&gt;Files to check, defines, include paths are imported.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="vanished">Cppcheck kan importera Visual studio solutions (*.sln), Visual studio projekt (*.vcxproj) eller compile databases.
Sökvägar och defines importeras.</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="30"/>
        <source>Import Project (Visual studio / compile database)</source>
        <translation>Importera Projekt (Visual Studio / compile database)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="163"/>
        <source>Defines must be separated by a semicolon &apos;;&apos;</source>
        <translation>Defines separeras med semicolon &apos;;&apos;</translation>
    </message>
    <message>
        <source>&amp;Root:</source>
        <oldsource>Root:</oldsource>
        <translation type="vanished">Rot:</translation>
    </message>
    <message>
        <source>Libraries:</source>
        <translation type="vanished">Libraries:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="331"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>Obs: Lägg dina egna .cfg filer i samma folder som projekt filen. De skall isåfall visas ovan.</translation>
    </message>
    <message>
        <source>Visual Studio</source>
        <translation type="obsolete">Visual Studio</translation>
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
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Du har ett val:&lt;/p&gt;&lt;p&gt; * Analysera alla Debug och Release konfigurationer&lt;/p&gt;&lt;p&gt; * Analysera bara den första matchande Debug konfigurationen&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="72"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation>Analysera alla Visual Studio konfigurationer</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="86"/>
        <location filename="projectfiledialog.ui" line="364"/>
        <source>Paths:</source>
        <translation>Sökvägar:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="113"/>
        <location filename="projectfiledialog.ui" line="207"/>
        <location filename="projectfiledialog.ui" line="378"/>
        <source>Add...</source>
        <translation>Lägg till...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="120"/>
        <location filename="projectfiledialog.ui" line="214"/>
        <location filename="projectfiledialog.ui" line="385"/>
        <source>Edit</source>
        <translation>Redigera</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="127"/>
        <location filename="projectfiledialog.ui" line="221"/>
        <location filename="projectfiledialog.ui" line="392"/>
        <location filename="projectfiledialog.ui" line="455"/>
        <source>Remove</source>
        <translation>Ta bort</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="176"/>
        <source>Include Paths:</source>
        <translation>Include sökvägar:</translation>
    </message>
    <message>
        <source>Includes</source>
        <translation type="obsolete">Include</translation>
    </message>
    <message>
        <source>Include directories:</source>
        <translation type="obsolete">Include sökvägar</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="241"/>
        <source>Up</source>
        <translation>Upp</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="248"/>
        <source>Down</source>
        <translation>Ned</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="279"/>
        <source>Root path:</source>
        <translation>Bas sökväg:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="291"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>Varnings taggar (separerade med semikolon)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="303"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation>Cppcheck build dir (whole program analys, incremental analys, statistik, etc)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="322"/>
        <source>Libraries</source>
        <translation>Libraries</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="358"/>
        <source>Exclude</source>
        <translation>Exkludera</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="417"/>
        <source>Suppressions</source>
        <translation>Suppressions</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="423"/>
        <source>Suppression list:</source>
        <translation>Suppression-list:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="448"/>
        <source>Add</source>
        <translation>Lägg till</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="465"/>
        <source>Addons</source>
        <translation>Addons</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="471"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="478"/>
        <source>Thread safety</source>
        <translation>Tråd säkerhet</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="485"/>
        <source>Coding standards</source>
        <translation>Kodstandarder</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="492"/>
        <source>Cert</source>
        <translation>Cert</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="513"/>
        <source>Extra Tools</source>
        <translation>Extra verktyg</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="519"/>
        <source>It is common best practice to use several tools.</source>
        <translation>Best practice är att använda flera verktyg</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="526"/>
        <source>Clang analyzer</source>
        <translation>Clang analyzer</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="533"/>
        <source>Clang-tidy</source>
        <translation>Clang-tidy</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="153"/>
        <source>Defines:</source>
        <translation>Defines:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="47"/>
        <source>Project file: %1</source>
        <translation>Projektfil: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="172"/>
        <source>Clang-tidy (not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="247"/>
        <source>Select Cppcheck build dir</source>
        <translation>Välj Cppcheck build dir</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="283"/>
        <source>Visual Studio (*.sln *.vcxproj);;Compile database (compile_commands.json)</source>
        <translation>Visual Studio (*.sln *.vcxproj);;Compile database (compile_commands.json)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="497"/>
        <source>Select include directory</source>
        <translation>Välj include sökväg</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="477"/>
        <source>Select a directory to check</source>
        <translation>Välj mapp att analysera</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="281"/>
        <source>Import Project</source>
        <translation>Importera Projekt</translation>
    </message>
    <message>
        <source>Visual Studio (*.sln *.vcxproj);;Compile database (compile_database.json)</source>
        <translation type="vanished">Visual Studio (*.sln *.vcxproj);;Compile database (compile_database.json)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="517"/>
        <source>Select directory to ignore</source>
        <translation>Välj sökväg att ignorera</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="571"/>
        <source>Add Suppression</source>
        <translation>Lägg till Suppression</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="572"/>
        <source>Select error id suppress:</source>
        <translation>Välj error Id suppress:</translation>
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
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>Save</source>
        <translation>Spara</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="103"/>
        <source>Unknown language specified!</source>
        <translation>Okänt språk valt!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="132"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation>Språk filen %1 hittades ej!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="138"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Misslyckades med att ladda översättningen för %1 från filen %2</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="33"/>
        <source>line %1: Unhandled element %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="144"/>
        <source> (Not found)</source>
        <translation type="unfinished"></translation>
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
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>Save</source>
        <translation>Spara</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>File</source>
        <translation>Fil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Severity</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Line</source>
        <translation>Rad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Summary</source>
        <translation>Sammanfattning</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="137"/>
        <source>Undefined file</source>
        <translation>Odefinierad fil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="841"/>
        <source>[Inconclusive]</source>
        <translation>[Inconclusive]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="308"/>
        <source>debug</source>
        <translation>debug</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="245"/>
        <source>note</source>
        <translation>note</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="608"/>
        <source>Recheck</source>
        <translation>Analysera om</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="609"/>
        <source>Copy filename</source>
        <translation>Kopiera filnamn</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="610"/>
        <source>Copy full path</source>
        <translation>Kopiera full sökväg</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="611"/>
        <source>Copy message</source>
        <translation>Kopiera meddelande</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="612"/>
        <source>Copy message id</source>
        <translation>Kopiera meddelande id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="613"/>
        <source>Hide</source>
        <translation>Dölj</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="614"/>
        <source>Hide all with id</source>
        <translation>Dölj alla med id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="615"/>
        <source>Suppress selected id(s)</source>
        <translation>Stäng av valda id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="616"/>
        <source>Open containing folder</source>
        <translation>Öppna mapp</translation>
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
        <translation>Ingen tag</translation>
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
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>Ingen editor konfigurerad.

Konfigurera program i inställningar/program.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="716"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>Ingen standard editor vald.

Vänligen välj standard editor i inställningar/Program.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="745"/>
        <source>Could not find the file!</source>
        <translation>Kunde inte hitta filen!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="791"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>Kunde inte starta %1

Kontrollera att sökvägen och parametrarna är korrekta.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="805"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>Kunde inte hitta filen:
%1
Välj mappen där filen finns.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="812"/>
        <source>Select Directory</source>
        <translation>Välj mapp</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Inconclusive</source>
        <translation>Inconclusive</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Since date</source>
        <translation>Sedan datum</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="290"/>
        <source>style</source>
        <translation>stil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="293"/>
        <source>error</source>
        <translation>fel</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="296"/>
        <source>warning</source>
        <translation>varning</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="299"/>
        <source>performance</source>
        <translation>prestanda</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="302"/>
        <source>portability</source>
        <translation>portabilitet</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="305"/>
        <source>information</source>
        <translation>information</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="198"/>
        <source>Print Report</source>
        <translation>Skriv ut rapport</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="217"/>
        <source>No errors found, nothing to print.</source>
        <translation>Inga fel hittades, inget att skriva ut.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="255"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 av %2 filer analyserade)</translation>
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
        <translation>Inga fel hittades.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="276"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Fel hittades, men de visas ej.
För att ställa in vilka fel som skall visas använd visa menyn.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="322"/>
        <location filename="resultsview.cpp" line="343"/>
        <location filename="resultsview.cpp" line="351"/>
        <source>Failed to read the report.</source>
        <translation>Misslyckades att läsa rapporten.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="329"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>XML format version 1 stöds ej längre.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="388"/>
        <source>Summary</source>
        <translation>Sammanfattning</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="389"/>
        <source>Message</source>
        <translation>Meddelande</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="393"/>
        <source>First included by</source>
        <translation>Först inkluderad av</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="396"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="443"/>
        <source>Clear Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="444"/>
        <source>Copy this Log entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="445"/>
        <source>Copy complete Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="156"/>
        <source>No errors found, nothing to save.</source>
        <translation>Inga fel hittades, ingenting att spara.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="180"/>
        <location filename="resultsview.cpp" line="188"/>
        <source>Failed to save the report.</source>
        <translation>Misslyckades med att spara rapporten.</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>Resultat</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="82"/>
        <source>Analysis Log</source>
        <translation>Analys Log</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="104"/>
        <source>Warning Details</source>
        <translation>Varningsdetaljer</translation>
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
        <translation>Filnamn</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="55"/>
        <source>Check</source>
        <translation>Analysera</translation>
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
        <source>Include paths:</source>
        <translation type="vanished">Include sökvägar:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="195"/>
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
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>Visa meddelande id i kolumn &quot;Id&quot;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="142"/>
        <source>Enable inline suppressions</source>
        <translation>Använd inline suppressions</translation>
    </message>
    <message>
        <location filename="settings.ui" line="149"/>
        <source>Check for inconclusive errors also</source>
        <translation>Kör inconclusive analys</translation>
    </message>
    <message>
        <location filename="settings.ui" line="156"/>
        <source>Show statistics on check completion</source>
        <translation>Visa statistik när analys är klar</translation>
    </message>
    <message>
        <location filename="settings.ui" line="176"/>
        <source>Show internal warnings in log</source>
        <translation>Visa interna fel i loggen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="287"/>
        <source>Addons</source>
        <translation>Addons</translation>
    </message>
    <message>
        <location filename="settings.ui" line="293"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation>Python binär fil (lämna tom för att använda python i PATH)</translation>
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
        <translation>Clang sökväg (lämna tom för att använda PATH)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="355"/>
        <source>Visual Studio headers</source>
        <translation>Visual Studio headers</translation>
    </message>
    <message>
        <location filename="settings.ui" line="361"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Sökvägar till Visual Studio headers, separerade med semikolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;Du kan öppna en Visual Studio command prompt, och skriva &amp;quot;SET INCLUDE&amp;quot;. Sedan kopiera och klistra in sökvägarna.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Paths</source>
        <translation type="vanished">Sökvägar</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="vanished">Redigera</translation>
    </message>
    <message>
        <location filename="settings.ui" line="209"/>
        <source>Remove</source>
        <translation>Ta bort</translation>
    </message>
    <message>
        <location filename="settings.ui" line="184"/>
        <source>Applications</source>
        <translation>Program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="202"/>
        <source>Edit...</source>
        <translation>Redigera...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="216"/>
        <source>Set as default</source>
        <translation>Sätt förvald</translation>
    </message>
    <message>
        <location filename="settings.ui" line="239"/>
        <source>Reports</source>
        <translation>Rapporter</translation>
    </message>
    <message>
        <location filename="settings.ui" line="245"/>
        <source>Save all errors when creating report</source>
        <translation>Spara alla fel</translation>
    </message>
    <message>
        <location filename="settings.ui" line="252"/>
        <source>Save full path to files in reports</source>
        <translation>Spara fulla sökvägar</translation>
    </message>
    <message>
        <location filename="settings.ui" line="273"/>
        <source>Language</source>
        <translation>Språk</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="obsolete">Avancerade</translation>
    </message>
    <message>
        <source>&amp;Show inconclusive errors</source>
        <translation type="obsolete">Visa inconclusive meddelanden</translation>
    </message>
    <message>
        <source>S&amp;how internal warnings in log</source>
        <translation type="obsolete">Visa interna fel i loggen</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="88"/>
        <source>N/A</source>
        <translation>Ej tillgängligt</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="186"/>
        <source>Add a new application</source>
        <translation>Lägg till program</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="219"/>
        <source>Modify an application</source>
        <translation>Ändra program</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="224"/>
        <source> [Default]</source>
        <translation> [Vald]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="249"/>
        <source>[Default]</source>
        <translation>[Förvald]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="304"/>
        <source>Select python binary</source>
        <translation>Välj python binär</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="312"/>
        <source>Select clang path</source>
        <translation>Välj Clang sökväg</translation>
    </message>
    <message>
        <source>Select include directory</source>
        <translation type="vanished">Välj include mapp</translation>
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
        <location filename="statsdialog.cpp" line="177"/>
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
        <location filename="stats.ui" line="343"/>
        <source>History</source>
        <translation>Historik</translation>
    </message>
    <message>
        <location filename="stats.ui" line="349"/>
        <source>File:</source>
        <translation>Fil:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="387"/>
        <source>Copy to Clipboard</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="stats.ui" line="394"/>
        <source>Pdf Export</source>
        <translation>Pdf Export</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>1 day</source>
        <translation>1 dag</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>%1 days</source>
        <translation>%1 dagar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>1 hour</source>
        <translation>1 timme</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>%1 hours</source>
        <translation>%1 timmar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>1 minute</source>
        <translation>1 minut</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>%1 minutes</source>
        <translation>%1 minuter</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>1 second</source>
        <translation>1 sekund</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>%1 seconds</source>
        <translation>%1 sekunder</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="121"/>
        <source>0.%1 seconds</source>
        <translation>0.%1 sekunder</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="123"/>
        <source> and </source>
        <translation> och </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="150"/>
        <source>Export PDF</source>
        <translation>Exportera PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="172"/>
        <source>Project Settings</source>
        <translation>Projekt inställningar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="174"/>
        <source>Paths</source>
        <translation>Sökvägar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="175"/>
        <source>Include paths</source>
        <translation>Include sökvägar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="176"/>
        <source>Defines</source>
        <translation>Definitioner</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="178"/>
        <source>Path selected</source>
        <translation>Vald sökväg</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="179"/>
        <source>Number of files scanned</source>
        <translation>Antal analyserade filer</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="180"/>
        <source>Scan duration</source>
        <translation>Tid</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="137"/>
        <location filename="statsdialog.cpp" line="182"/>
        <source>Errors</source>
        <translation>Fel</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="64"/>
        <source>File: </source>
        <translation>Fil:</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="64"/>
        <source>No cppcheck build dir</source>
        <translation>Ingen Cppcheck build dir</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="139"/>
        <location filename="statsdialog.cpp" line="183"/>
        <source>Warnings</source>
        <translation>Varningar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="141"/>
        <location filename="statsdialog.cpp" line="184"/>
        <source>Style warnings</source>
        <translation>Stil varningar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="143"/>
        <location filename="statsdialog.cpp" line="185"/>
        <source>Portability warnings</source>
        <translation>Portabilitetsvarningar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="145"/>
        <location filename="statsdialog.cpp" line="186"/>
        <source>Performance warnings</source>
        <translation>Prestanda varningar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="147"/>
        <location filename="statsdialog.cpp" line="187"/>
        <source>Information messages</source>
        <translation>Informationsmeddelanden</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="54"/>
        <source>%1 of %2 files checked</source>
        <translation>%1 av %2 filer analyserade</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="144"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>Misslyckades att ändra språk:

%1

Språket har nollställts till Engelska. Öppna Preferences och välj något av de tillgängliga språken.</translation>
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
        <translation>inconclusive</translation>
    </message>
</context>
</TS>
