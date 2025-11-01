<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="sv_SE">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_about.h" line="130"/>
        <source>About Cppcheck</source>
        <translation>Om Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_about.h" line="132"/>
        <source>Version %1</source>
        <translation>Version %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_about.h" line="133"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - Ett verktyg för statisk analys av C/C++ kod.</translation>
    </message>
    <message>
        <location filename="about.ui" line="81"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_about.h" line="134"/>
        <source>Copyright © 2007-%1 Cppcheck team.</source>
        <oldsource>Copyright © 2007-2021 Cppcheck team.</oldsource>
        <translation type="unfinished">Copyright © 2007-2021 Cppcheck team.</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_about.h" line="135"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>This program is licensed under the terms
of the GNU General Public License version 3</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_about.h" line="137"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Hemsida: %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="115"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_about.h" line="138"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PCRE&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PicoJSON&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;TinyXML2&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Boost&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <oldsource>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;pcre&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;picojson&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;tinyxml2&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</oldsource>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="applicationdialog.ui" line="23"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_applicationdialog.h" line="161"/>
        <source>Add an application</source>
        <translation>Lägg till program</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="41"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_applicationdialog.h" line="162"/>
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
        <location filename="applicationdialog.ui" line="76"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_applicationdialog.h" line="173"/>
        <source>&amp;Name:</source>
        <translation>Namn:</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="86"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_applicationdialog.h" line="174"/>
        <source>&amp;Executable:</source>
        <translation>Körbar fil:</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="96"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_applicationdialog.h" line="175"/>
        <source>&amp;Parameters:</source>
        <translation>Parametrar:</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="138"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_applicationdialog.h" line="176"/>
        <source>Browse</source>
        <translation>Bläddra</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="65"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Exekverbara filer (*.exe);;Alla filer(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="68"/>
        <source>Select viewer application</source>
        <translation>Välj program</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="83"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="84"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>Du måste ange namn, sökväg samt eventuellt parametrar för applikationen!</translation>
    </message>
</context>
<context>
    <name>ComplianceReportDialog</name>
    <message>
        <location filename="compliancereportdialog.ui" line="14"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_compliancereportdialog.h" line="112"/>
        <source>Compliance Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="29"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_compliancereportdialog.h" line="114"/>
        <source>Project name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="22"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_compliancereportdialog.h" line="113"/>
        <source>Project version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="42"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_compliancereportdialog.h" line="115"/>
        <source>Coding Standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="50"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_compliancereportdialog.h" line="116"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="55"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_compliancereportdialog.h" line="117"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="60"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_compliancereportdialog.h" line="118"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="70"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_compliancereportdialog.h" line="120"/>
        <source>List of files with md5 checksums</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="133"/>
        <source>Compliance report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="135"/>
        <source>HTML files (*.html)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="176"/>
        <location filename="compliancereportdialog.cpp" line="239"/>
        <source>Save compliance report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="177"/>
        <source>Failed to import &apos;%1&apos; (%2), can not show files in compliance report</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="53"/>
        <source>Could not find the file: %1</source>
        <oldsource>Could not find the file:
</oldsource>
        <translation>Kunde inte hitta filen: %1</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="57"/>
        <location filename="fileviewdialog.cpp" line="71"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="67"/>
        <source>Could not read the file: %1</source>
        <translation>Kunde inte läsa filen: %1</translation>
    </message>
</context>
<context>
    <name>HelpDialog</name>
    <message>
        <location filename="helpdialog.ui" line="14"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_helpdialog.h" line="88"/>
        <source>Cppcheck GUI help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="34"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_helpdialog.h" line="89"/>
        <source>Contents</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="44"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_helpdialog.h" line="90"/>
        <source>Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="84"/>
        <source>Helpfile &apos;%1&apos; was not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="86"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
</context>
<context>
    <name>LibraryAddFunctionDialog</name>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="23"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryaddfunctiondialog.h" line="105"/>
        <source>Add function</source>
        <translation>Lägg till funktion</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryaddfunctiondialog.h" line="106"/>
        <source>Function name(s)</source>
        <translation>Funktion namn</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryaddfunctiondialog.h" line="107"/>
        <source>Number of arguments</source>
        <translation>Antal argument</translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="275"/>
        <source>Library Editor</source>
        <translation>Library Editor</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="276"/>
        <source>Open</source>
        <translation>Öppna</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="277"/>
        <source>Save</source>
        <translation>Spara</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="278"/>
        <source>Save as</source>
        <translation>Spara som</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="279"/>
        <source>Functions</source>
        <translation>Funktioner</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="280"/>
        <source>Sort</source>
        <translation>Sortera</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="281"/>
        <source>Add</source>
        <translation>Lägg till</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="282"/>
        <source>Filter:</source>
        <translation>Filter:</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="283"/>
        <source>Comments</source>
        <translation>Kommentar</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="204"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="284"/>
        <source>noreturn</source>
        <translation>noreturn</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="212"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="285"/>
        <source>False</source>
        <translation>False</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="217"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="286"/>
        <source>True</source>
        <translation>True</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="222"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="287"/>
        <source>Unknown</source>
        <translation>Vet ej</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="232"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="289"/>
        <source>return value must be used</source>
        <translation>retur värde måste användas</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="290"/>
        <source>ignore function in leaks checking</source>
        <translation>Ignorera funktionen när cppcheck letar efter läckor</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="291"/>
        <source>Arguments</source>
        <translation>Argument</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_librarydialog.h" line="292"/>
        <source>Edit</source>
        <translation>Redigera</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="98"/>
        <location filename="librarydialog.cpp" line="170"/>
        <source>Library files (*.cfg)</source>
        <translation>Library fil (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="100"/>
        <source>Open library file</source>
        <translation>Öppna Library fil</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="111"/>
        <location filename="librarydialog.cpp" line="123"/>
        <location filename="librarydialog.cpp" line="160"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="112"/>
        <source>Cannot open file %1.</source>
        <oldsource>Can not open file %1.</oldsource>
        <translation type="unfinished">Kunde ej öppna filen %1.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="124"/>
        <source>Failed to load %1. %2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="161"/>
        <source>Cannot save file %1.</source>
        <oldsource>Can not save file %1.</oldsource>
        <translation type="unfinished">Kunde ej spara filen %1.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="173"/>
        <source>Save the library as</source>
        <translation>Spara library som</translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="284"/>
        <source>Edit argument</source>
        <translation>Konfigurera argument</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="20"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="286"/>
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
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="293"/>
        <source>Not bool</source>
        <translation>Ej bool</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="35"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="295"/>
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
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="302"/>
        <source>Not null</source>
        <translation>Ej null</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="50"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="303"/>
        <source>Not uninit</source>
        <translation>Ej uninit</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="57"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="304"/>
        <source>String</source>
        <translation>Sträng</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="70"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="305"/>
        <source>Format string</source>
        <translation>Format sträng</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="92"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="306"/>
        <source>Min size of buffer</source>
        <translation>Minsta storlek för buffer</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="101"/>
        <location filename="libraryeditargdialog.ui" line="203"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="307"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="316"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="214"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="308"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="317"/>
        <source>None</source>
        <translation>Ingen</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="309"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="318"/>
        <source>argvalue</source>
        <translation>argvalue</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="310"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="319"/>
        <source>mul</source>
        <translation>mul</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="124"/>
        <location filename="libraryeditargdialog.ui" line="229"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="311"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="320"/>
        <source>strlen</source>
        <translation>strlen</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="132"/>
        <location filename="libraryeditargdialog.ui" line="237"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="313"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="322"/>
        <source>Arg</source>
        <translation>Arg</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="159"/>
        <location filename="libraryeditargdialog.ui" line="264"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="314"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="323"/>
        <source>Arg2</source>
        <translation>Arg2</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="194"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="315"/>
        <source>and</source>
        <translation>och</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="310"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_libraryeditargdialog.h" line="324"/>
        <source>Valid values</source>
        <translation>Tillåtna värden</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.ui" line="687"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="568"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="677"/>
        <location filename="mainwindow.cpp" line="463"/>
        <location filename="mainwindow.cpp" line="667"/>
        <location filename="mainwindow.cpp" line="752"/>
        <location filename="mainwindow.cpp" line="857"/>
        <location filename="mainwindow.cpp" line="879"/>
        <location filename="mainwindow.cpp" line="1468"/>
        <location filename="mainwindow.cpp" line="1598"/>
        <location filename="mainwindow.cpp" line="1937"/>
        <location filename="mainwindow.cpp" line="1945"/>
        <location filename="mainwindow.cpp" line="1993"/>
        <location filename="mainwindow.cpp" line="2002"/>
        <location filename="mainwindow.cpp" line="2074"/>
        <location filename="mainwindow.cpp" line="2148"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="209"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="738"/>
        <source>A&amp;nalyze</source>
        <translation>Analysera</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="266"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="742"/>
        <source>Standard</source>
        <translation>Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="132"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="733"/>
        <source>&amp;File</source>
        <translation>&amp;Arkiv</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="152"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="734"/>
        <source>&amp;View</source>
        <translation>&amp;Visa</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="156"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="735"/>
        <source>&amp;Toolbars</source>
        <translation>Verktygsfält</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="213"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="739"/>
        <source>C++ standard</source>
        <translation>C++ standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="225"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="740"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation>C standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="253"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="741"/>
        <source>&amp;Edit</source>
        <translation>&amp;Redigera</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="314"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="569"/>
        <source>&amp;License...</source>
        <translation>&amp;Licens...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="319"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="570"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Utvecklat av...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="328"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="571"/>
        <source>&amp;About...</source>
        <translation>&amp;Om...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="333"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="572"/>
        <source>&amp;Files...</source>
        <translation>&amp;Filer...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="336"/>
        <location filename="mainwindow.ui" line="339"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="573"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="575"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation>Analysera filer</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="342"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="578"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="351"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="580"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Katalog...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="354"/>
        <location filename="mainwindow.ui" line="357"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="581"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="583"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation>Analysera mapp</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="360"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="586"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="372"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="590"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="390"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="593"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stoppa</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="393"/>
        <location filename="mainwindow.ui" line="396"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="594"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="596"/>
        <source>Stop analysis</source>
        <oldsource>Stop checking</oldsource>
        <translation>Stoppa analys</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="399"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="599"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="408"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="601"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Spara resultat till fil...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="411"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="603"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="416"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="605"/>
        <source>&amp;Quit</source>
        <translation>&amp;Avsluta</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="428"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="609"/>
        <source>&amp;Clear results</source>
        <translation>&amp;Töm resultat</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="437"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="610"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Inställningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="470"/>
        <location filename="mainwindow.ui" line="473"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="617"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="619"/>
        <location filename="mainwindow.cpp" line="2374"/>
        <source>Show errors</source>
        <translation>Visa fel</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="616"/>
        <location filename="mainwindow.ui" line="619"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="659"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="661"/>
        <location filename="mainwindow.cpp" line="2375"/>
        <source>Show warnings</source>
        <translation>Visa varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="634"/>
        <location filename="mainwindow.ui" line="637"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="664"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="666"/>
        <source>Show performance warnings</source>
        <translation>Visa prestanda varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="645"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="668"/>
        <source>Show &amp;hidden</source>
        <translation>Visa dolda</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="983"/>
        <location filename="mainwindow.cpp" line="1024"/>
        <source>Information</source>
        <translation>Information</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="660"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="671"/>
        <source>Show information messages</source>
        <translation>Visa informations meddelanden</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="675"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="675"/>
        <source>Show portability warnings</source>
        <translation>Visa portabilitets varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="690"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="679"/>
        <source>Show Cppcheck results</source>
        <translation>Visa Cppcheck resultat</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="702"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="681"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="705"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="683"/>
        <source>Show Clang results</source>
        <translation>Visa Clang resultat</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="713"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="685"/>
        <source>&amp;Filter</source>
        <translation>&amp;Filter</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="716"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="687"/>
        <source>Filter results</source>
        <translation>Filtrera resultat</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="732"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="689"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSI</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="740"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="690"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="748"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="691"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="756"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="692"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="764"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="693"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="858"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="701"/>
        <source>&amp;Print...</source>
        <translation>Skriv ut...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="861"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="703"/>
        <source>Print the Current Report</source>
        <translation>Skriv ut aktuell rapport</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="866"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="705"/>
        <source>Print Pre&amp;view...</source>
        <translation>Förhandsgranska utskrift...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="869"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="707"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>Öppnar förhandsgranskning för nuvarande resultat</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="877"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="711"/>
        <source>Open library editor</source>
        <translation>Öppna library editor</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="478"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="621"/>
        <source>&amp;Check all</source>
        <translation>&amp;Kryssa alla</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="84"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="731"/>
        <source>Checking for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="100"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="732"/>
        <source>Hide</source>
        <translation type="unfinished">Dölj</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="164"/>
        <location filename="mainwindow.ui" line="979"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="722"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="736"/>
        <source>Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="303"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="744"/>
        <source>Filter</source>
        <translation>Filter</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="369"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="588"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation>Analysera om ändrade filer</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="381"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="592"/>
        <source>Reanal&amp;yze all files</source>
        <translation>Analysera om alla filer</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="419"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="607"/>
        <source>Ctrl+Q</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="449"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="611"/>
        <source>Style war&amp;nings</source>
        <translation>Style varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="467"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="616"/>
        <source>E&amp;rrors</source>
        <translation>Fel</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="483"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="622"/>
        <source>&amp;Uncheck all</source>
        <translation>Kryssa &amp;ur alla</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="488"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="623"/>
        <source>Collapse &amp;all</source>
        <translatorcomment>Ingen bra översättning!</translatorcomment>
        <translation>&amp;Fäll ihop alla</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="493"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="624"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Expandera alla</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="501"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="625"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standard</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="504"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="627"/>
        <source>Standard items</source>
        <translation>Standard poster</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="520"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="636"/>
        <source>Toolbar</source>
        <translation>Verktygsfält</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="528"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="637"/>
        <source>&amp;Categories</source>
        <translation>&amp;Kategorier</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="531"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="639"/>
        <source>Error categories</source>
        <translation>Fel kategorier</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="536"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="641"/>
        <source>&amp;Open XML...</source>
        <translation>&amp;Öppna XML...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="545"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="642"/>
        <source>Open P&amp;roject File...</source>
        <translation>Öppna Projektfil...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="548"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="644"/>
        <source>Ctrl+Shift+O</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="557"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="646"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation>Visa Scratchpad...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="562"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="647"/>
        <source>&amp;New Project File...</source>
        <translation>Ny projektfil...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="565"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="649"/>
        <source>Ctrl+Shift+N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="570"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="651"/>
        <source>&amp;Log View</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="573"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="653"/>
        <source>Log View</source>
        <translation>Logg vy</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="581"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="655"/>
        <source>C&amp;lose Project File</source>
        <translation>Stäng projektfil</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="589"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="656"/>
        <source>&amp;Edit Project File...</source>
        <translation>Redigera projektfil...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="601"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="657"/>
        <source>&amp;Statistics</source>
        <translation>Statistik</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="613"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="658"/>
        <source>&amp;Warnings</source>
        <translation>Varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="631"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="663"/>
        <source>Per&amp;formance warnings</source>
        <translation>Optimerings varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="657"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="669"/>
        <source>&amp;Information</source>
        <translation>Information</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="672"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="673"/>
        <source>&amp;Portability</source>
        <translation>Portabilitet</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="772"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="694"/>
        <source>P&amp;latforms</source>
        <translation>Plattformar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="786"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="695"/>
        <source>C++&amp;11</source>
        <translation>C++11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="797"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="696"/>
        <source>C&amp;99</source>
        <translation>C99</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="805"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="697"/>
        <source>&amp;Posix</source>
        <translation>Posix</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="813"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="698"/>
        <source>C&amp;11</source>
        <translation>C11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="845"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="699"/>
        <source>&amp;C89</source>
        <translation>C89</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="853"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="700"/>
        <source>&amp;C++03</source>
        <translation>C++03</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="874"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="709"/>
        <source>&amp;Library Editor...</source>
        <translation>Library Editor...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="885"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="713"/>
        <source>&amp;Auto-detect language</source>
        <translation>Detektera språk automatiskt</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="893"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="714"/>
        <source>&amp;Enforce C++</source>
        <translation>Tvinga C++</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="901"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="715"/>
        <source>E&amp;nforce C</source>
        <translation>Tvinga C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="912"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="716"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="920"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="717"/>
        <source>Reanalyze and check library</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="928"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="718"/>
        <source>Check configuration (defines, includes)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="936"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="719"/>
        <source>C++17</source>
        <translation type="unfinished">C++17</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="947"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="720"/>
        <source>C++20</source>
        <translation type="unfinished">C++20</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="974"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="721"/>
        <source>Compliance report...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="987"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="723"/>
        <source>Normal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="995"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="724"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1003"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="725"/>
        <source>Misra C++ 2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1011"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="726"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1019"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="727"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1027"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="728"/>
        <source>Misra C++ 2023</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1035"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="729"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1040"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="730"/>
        <source>EULA...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="509"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="629"/>
        <source>&amp;Contents</source>
        <translation>&amp;Innehåll</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="284"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="743"/>
        <source>Categories</source>
        <translation>Kategorier</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="452"/>
        <location filename="mainwindow.ui" line="455"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="612"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="614"/>
        <source>Show style warnings</source>
        <translation>Visa stil varningar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="512"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="631"/>
        <source>Open the help contents</source>
        <translation>Öppna hjälp</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="515"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="634"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="198"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_mainwindow.h" line="737"/>
        <source>&amp;Help</source>
        <translation>&amp;Hjälp</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="155"/>
        <location filename="mainwindow.cpp" line="1782"/>
        <source>Quick Filter:</source>
        <translation>Snabbfilter:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="828"/>
        <source>Select configuration</source>
        <translation>Välj konfiguration</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="858"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>Hittade projektfil: %1

Vill du ladda denna projektfil istället?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="992"/>
        <source>File not found</source>
        <translation>Filen hittades ej</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="995"/>
        <source>Bad XML</source>
        <translation>Ogiltig XML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="998"/>
        <source>Missing attribute</source>
        <translation>Attribut finns ej</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1001"/>
        <source>Bad attribute value</source>
        <translation>Ogiltigt attribut värde</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1004"/>
        <source>Unsupported format</source>
        <translation>Format stöds ej</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1013"/>
        <source>Duplicate define</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1024"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>Misslyckades att ladda valda library &apos;%1&apos;.
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1035"/>
        <source>File not found: &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1058"/>
        <source>Failed to load/setup addon %1: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1080"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1090"/>
        <source>Failed to load %1 - %2

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1100"/>
        <location filename="mainwindow.cpp" line="1199"/>
        <source>%1

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1650"/>
        <source>License</source>
        <translation>Licens</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1657"/>
        <source>Authors</source>
        <translation>Utvecklare</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1677"/>
        <source>Save the report file</source>
        <translation>Spara rapport</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1484"/>
        <location filename="mainwindow.cpp" line="1684"/>
        <source>XML files (*.xml)</source>
        <translation>XML filer (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="458"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>Det uppstod ett problem när programinställningarna skulle laddas.

En trolig orsak är att inställningarna ändrats för olika Cppcheck versioner. Kontrollera programinställningarna.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="753"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Du måste stänga projektfilen innan nya filer eller sökvägar kan väljas!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="983"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>Library filen &apos;%1&apos; har element som ej hanteras:
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1007"/>
        <source>Duplicate platform type</source>
        <translation>Dubbel plattformstyp</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1010"/>
        <source>Platform type redefined</source>
        <translation>Plattformstyp definieras igen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1016"/>
        <source>Unknown element</source>
        <translation>Element hanteras ej</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1019"/>
        <source>Unknown issue</source>
        <translation>Något problem</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1080"/>
        <location filename="mainwindow.cpp" line="1090"/>
        <location filename="mainwindow.cpp" line="1100"/>
        <location filename="mainwindow.cpp" line="1199"/>
        <source>Error</source>
        <translation>Fel</translation>
    </message>
    <message>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation type="vanished">Misslyckades att ladda %1. Din Cppcheck installation är ej komplett. Du kan använda --data-dir&lt;directory&gt; på kommandoraden för att specificera var denna fil finns. Det är meningen att --data-dir kommandot skall köras under installationen,så GUIt kommer ej visas när --data-dir används allt som händer är att en inställning görs.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1486"/>
        <source>Open the report file</source>
        <translation>Öppna rapportfilen</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1688"/>
        <source>Text files (*.txt)</source>
        <translation>Text filer (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1692"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1820"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Projektfiler (*.cppcheck);;Alla filer(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1822"/>
        <source>Select Project File</source>
        <translation>Välj projektfil</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="239"/>
        <location filename="mainwindow.cpp" line="1784"/>
        <location filename="mainwindow.cpp" line="1850"/>
        <location filename="mainwindow.cpp" line="2042"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="668"/>
        <source>No suitable files found to analyze!</source>
        <translation>Inga filer hittades att analysera!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="767"/>
        <source>C/C++ Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="768"/>
        <source>Compile database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="769"/>
        <source>Visual Studio</source>
        <translation type="unfinished">Visual Studio</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="770"/>
        <source>Borland C++ Builder 6</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="773"/>
        <source>Select files to analyze</source>
        <translation>Välj filer att analysera</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="788"/>
        <source>Select directory to analyze</source>
        <translation>Välj mapp att analysera</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="828"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>Välj konfiguration som kommer analyseras</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="880"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation>Hittade projekt filer i mappen.

Vill du fortsätta analysen utan att använda någon av dessa projekt filer?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1469"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.
Do you want to proceed?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1594"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>Analys körs.

Vill du stoppa analysen och avsluta Cppcheck?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1636"/>
        <source>About</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1675"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML filer (*.xml);;Text filer (*.txt);;CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1715"/>
        <source>Cannot generate a compliance report right now, an analysis must finish successfully. Try to reanalyze the code and ensure there are no critical errors.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1938"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>Build dir &apos;%1&apos; existerar ej, skapa den?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1946"/>
        <source>To check the project using addons, you need a build directory.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1978"/>
        <source>Failed to open file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1981"/>
        <source>Unknown project file format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1984"/>
        <source>Failed to import project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1994"/>
        <source>Failed to import &apos;%1&apos;: %2

Analysis is stopped.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2003"/>
        <source>Failed to import &apos;%1&apos; (%2), analysis is stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2347"/>
        <source>Show Mandatory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2348"/>
        <source>Show Required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2349"/>
        <source>Show Advisory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2350"/>
        <source>Show Document</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2367"/>
        <source>Show L1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2368"/>
        <source>Show L2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2369"/>
        <source>Show L3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2376"/>
        <source>Show style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2377"/>
        <source>Show portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2378"/>
        <source>Show performance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2379"/>
        <source>Show information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation type="vanished">Misslyckades att importera &apos;%1&apos;, analysen stoppas</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2027"/>
        <source>Project files (*.cppcheck)</source>
        <translation>Projekt filer (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2029"/>
        <source>Select Project Filename</source>
        <translation>Välj Projektfil</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2075"/>
        <source>No project file loaded</source>
        <translation>Inget projekt laddat</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2143"/>
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
        <location filename="mainwindow.cpp" line="2305"/>
        <source>Install</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2309"/>
        <source>New version available: %1. %2</source>
        <translation type="unfinished"></translation>
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
        <location filename="main.cpp" line="115"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation>Cppcheck GUI - Command line parameters</translation>
    </message>
</context>
<context>
    <name>NewSuppressionDialog</name>
    <message>
        <location filename="newsuppressiondialog.ui" line="17"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_newsuppressiondialog.h" line="114"/>
        <source>New suppression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="25"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_newsuppressiondialog.h" line="115"/>
        <source>Error ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="32"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_newsuppressiondialog.h" line="116"/>
        <source>File name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="42"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_newsuppressiondialog.h" line="117"/>
        <source>Line number</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="52"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_newsuppressiondialog.h" line="118"/>
        <source>Symbol name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.cpp" line="83"/>
        <source>Edit suppression</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Native</source>
        <translation>Native</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="39"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="40"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="41"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSI</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="42"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="43"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="911"/>
        <source>Project File</source>
        <translation>Projektfil</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="938"/>
        <source>Paths and Defines</source>
        <translation>Sökvägar och defines</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="30"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="912"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <oldsource>Import Project (Visual studio / compile database)</oldsource>
        <translation type="unfinished">Importera Projekt (Visual Studio / compile database)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="231"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="926"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <oldsource>Defines must be separated by a semicolon &apos;;&apos;</oldsource>
        <translation type="unfinished">Defines separeras med semicolon &apos;;&apos;</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="389"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="941"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>Obs: Lägg dina egna .cfg filer i samma folder som projekt filen. De skall isåfall visas ovan.</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="890"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1000"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="73"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="916"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Du har ett val:&lt;/p&gt;&lt;p&gt; * Analysera alla Debug och Release konfigurationer&lt;/p&gt;&lt;p&gt; * Analysera bara den första matchande Debug konfigurationen&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="60"/>
        <location filename="projectfile.ui" line="422"/>
        <location filename="projectfile.ui" line="616"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="914"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="947"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="966"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="76"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="918"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation>Analysera alla Visual Studio konfigurationer</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="113"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="919"/>
        <source>Selected VS Configurations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="147"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="920"/>
        <source>Paths:</source>
        <translation>Sökvägar:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="181"/>
        <location filename="projectfile.ui" line="296"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="921"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="933"/>
        <source>Add...</source>
        <translation>Lägg till...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="188"/>
        <location filename="projectfile.ui" line="303"/>
        <location filename="projectfile.ui" line="690"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="922"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="934"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="979"/>
        <source>Edit</source>
        <translation>Redigera</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="195"/>
        <location filename="projectfile.ui" line="310"/>
        <location filename="projectfile.ui" line="697"/>
        <location filename="projectfile.ui" line="740"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="923"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="935"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="980"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="983"/>
        <source>Remove</source>
        <translation>Ta bort</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="242"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="928"/>
        <source>Undefines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="252"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="930"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="265"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="932"/>
        <source>Include Paths:</source>
        <translation>Include sökvägar:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="415"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="945"/>
        <source>This is a workfolder that Cppcheck will use for various purposes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="448"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="950"/>
        <source>Clang (experimental)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="458"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="951"/>
        <source>Check level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="464"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="952"/>
        <source>Reduced -- meant for usage where developer wants results directly. Limited and faster analysis with fewer results.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="471"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="953"/>
        <source>Normal -- meant for normal analysis in CI. Analysis should finish in reasonable time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="478"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="954"/>
        <source>Exhaustive -- meant for nightly builds etc. Analysis time can be longer (10x slower than compilation is OK).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="494"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="957"/>
        <source>If you want to design your classes to be as flexible and robust as possible then the public interface must be very robust. Cppcheck will asumme that arguments can take *any* value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="519"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="961"/>
        <source>Check code in headers  (should be ON normally. if you want a limited quick analysis then turn this OFF)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="572"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="964"/>
        <source>Max recursion in template instantiation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="607"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="965"/>
        <source>Premium License</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="639"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="970"/>
        <source>Filepaths in warnings will be relative to this path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="655"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="974"/>
        <source>If tags are added, you will be able to right click on warnings and set one of these tags. You can manually categorize warnings.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="665"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="976"/>
        <source>Exclude source files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="676"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="977"/>
        <source>Exclude folder...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="683"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="978"/>
        <source>Exclude file...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="765"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="984"/>
        <source>Enable inline suppressions</source>
        <translation type="unfinished">Använd inline suppressions</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="833"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="991"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="841"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="992"/>
        <source>2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="846"/>
        <location filename="projectfile.ui" line="914"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="993"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1003"/>
        <source>2023</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="851"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="994"/>
        <source>2025</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="876"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="996"/>
        <source>MISRA rule texts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="883"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="998"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="901"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1001"/>
        <source>Misra C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="909"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1002"/>
        <source>2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="958"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1007"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="975"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1009"/>
        <source>Bug hunting (Premium)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="991"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1011"/>
        <source>External tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="330"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="936"/>
        <source>Up</source>
        <translation>Upp</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="337"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="937"/>
        <source>Down</source>
        <translation>Ned</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="368"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="939"/>
        <source>Platform</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="403"/>
        <location filename="projectfile.ui" line="488"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="955"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="967"/>
        <source>Analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="432"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="948"/>
        <source>Parser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="438"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="949"/>
        <source>Cppcheck (built in)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="497"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="959"/>
        <source>Check that each class has a safe public interface</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="513"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="960"/>
        <source>Limit analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="529"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="962"/>
        <source>Check code in unused templates (should be ON normally, however in theory you can safely ignore warnings in unused templates)</source>
        <oldsource>Check code in unused templates  (slower and less accurate analysis)</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="541"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="963"/>
        <source>Max CTU depth</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="627"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="985"/>
        <source>Warning options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="633"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="968"/>
        <source>Root path:</source>
        <translation>Bas sökväg:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="649"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="972"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>Varnings taggar (separerade med semikolon)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="409"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="943"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation>Cppcheck build dir (whole program analys, incremental analys, statistik, etc)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="362"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="942"/>
        <source>Types and Functions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="380"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="940"/>
        <source>Libraries</source>
        <translation>Libraries</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="722"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="981"/>
        <source>Suppressions</source>
        <translation>Suppressions</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="733"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="982"/>
        <source>Add</source>
        <translation>Lägg till</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="786"/>
        <location filename="projectfile.ui" line="792"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="986"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1014"/>
        <source>Addons</source>
        <translation>Addons</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="798"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="987"/>
        <source>Note: Addons require &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt; being installed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="808"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="988"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="815"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="989"/>
        <source>Thread safety</source>
        <translation>Tråd säkerhet</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="825"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="990"/>
        <source>Coding standards</source>
        <translation>Kodstandarder</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="937"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1005"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="946"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1006"/>
        <source>CERT-INT35-C:  int precision (if size equals precision, you can leave empty)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="965"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1008"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="981"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1010"/>
        <source>Bug hunting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="1004"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1013"/>
        <source>Clang analyzer</source>
        <translation>Clang analyzer</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="997"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="1012"/>
        <source>Clang-tidy</source>
        <translation>Clang-tidy</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="221"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_projectfile.h" line="924"/>
        <source>Defines:</source>
        <translation>Defines:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="119"/>
        <source>Project file: %1</source>
        <translation>Projektfil: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="396"/>
        <source>Note: Open source Cppcheck does not fully implement Misra C 2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="459"/>
        <source>Clang-tidy (not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="586"/>
        <source>Select Cppcheck build dir</source>
        <translation>Välj Cppcheck build dir</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="847"/>
        <source>Select include directory</source>
        <translation>Välj include sökväg</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="875"/>
        <source>Source files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="876"/>
        <source>All files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="877"/>
        <source>Exclude file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="960"/>
        <source>Select MISRA rule texts file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="962"/>
        <source>MISRA rule texts file (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="827"/>
        <source>Select a directory to check</source>
        <translation>Välj mapp att analysera</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="625"/>
        <source>Visual Studio</source>
        <translation type="unfinished">Visual Studio</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="626"/>
        <source>Compile database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="627"/>
        <source>Borland C++ Builder 6</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="628"/>
        <source>Import Project</source>
        <translation>Importera Projekt</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="867"/>
        <source>Select directory to ignore</source>
        <translation>Välj sökväg att ignorera</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="91"/>
        <source>Unknown language specified!</source>
        <translation>Okänt språk valt!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="117"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation>Språk filen %1 hittades ej!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="122"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Misslyckades med att ladda översättningen för %1 från filen %2</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="38"/>
        <source>line %1: Unhandled element %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="43"/>
        <source>line %1: Mandatory attribute &apos;%2&apos; missing in &apos;%3&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="284"/>
        <source> (Not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="76"/>
        <source>Thin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="78"/>
        <source>ExtraLight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="80"/>
        <source>Light</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="82"/>
        <source>Normal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="84"/>
        <source>Medium</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="86"/>
        <source>DemiBold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="88"/>
        <source>Bold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="90"/>
        <source>ExtraBold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="92"/>
        <source>Black</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="81"/>
        <source>Editor Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="84"/>
        <source>Editor Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="87"/>
        <source>Highlight Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="90"/>
        <source>Line Number Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="93"/>
        <source>Line Number Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="96"/>
        <source>Keyword Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="99"/>
        <source>Keyword Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="102"/>
        <source>Class Foreground Color</source>
        <oldsource>Class ForegroundColor</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="105"/>
        <source>Class Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="108"/>
        <source>Quote Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="111"/>
        <source>Quote Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="114"/>
        <source>Comment Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="117"/>
        <source>Comment Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="120"/>
        <source>Symbol Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="123"/>
        <source>Symbol Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="126"/>
        <source>Symbol Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="142"/>
        <source>Set to Default Light</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="144"/>
        <source>Set to Default Dark</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="133"/>
        <source>File</source>
        <translation type="unfinished">Fil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="134"/>
        <source>Line</source>
        <translation type="unfinished">Rad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="135"/>
        <source>Severity</source>
        <translation type="unfinished">Typ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="136"/>
        <source>Classification</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="137"/>
        <source>Level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="138"/>
        <source>Inconclusive</source>
        <translation type="unfinished">Inconclusive</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="139"/>
        <source>Summary</source>
        <translation type="unfinished">Sammanfattning</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="140"/>
        <source>Id</source>
        <translation type="unfinished">Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="141"/>
        <source>Guideline</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="142"/>
        <source>Rule</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="143"/>
        <source>Since date</source>
        <translation type="unfinished">Sedan datum</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="144"/>
        <source>Tags</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="145"/>
        <source>CWE</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="42"/>
        <source>Cancel</source>
        <translation>Avbryt</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="43"/>
        <source>Close</source>
        <translation>Stäng</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="44"/>
        <source>Save</source>
        <translation>Spara</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <source>File</source>
        <translation type="vanished">Fil</translation>
    </message>
    <message>
        <source>Severity</source>
        <translation type="vanished">Typ</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="vanished">Rad</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation type="vanished">Sammanfattning</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="262"/>
        <source>Undefined file</source>
        <translation>Odefinierad fil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="765"/>
        <source>Copy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="948"/>
        <source>Could not find file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="952"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="953"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="955"/>
        <source>Please select the directory where file is located.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="462"/>
        <source>debug</source>
        <translation>debug</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="388"/>
        <source>note</source>
        <translation>note</translation>
    </message>
    <message>
        <source>Recheck</source>
        <translation type="vanished">Analysera om</translation>
    </message>
    <message>
        <source>Hide</source>
        <translation type="vanished">Dölj</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="767"/>
        <source>Hide all with id</source>
        <translation>Dölj alla med id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="789"/>
        <source>Suppress selected id(s)</source>
        <translation>Stäng av valda id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="768"/>
        <source>Open containing folder</source>
        <translation>Öppna mapp</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="465"/>
        <source>internal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="764"/>
        <source>Recheck %1 file(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="766"/>
        <source>Hide %1 result(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="812"/>
        <source>Tag</source>
        <translation>Tag</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="814"/>
        <source>No tag</source>
        <translation>Ingen tag</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="845"/>
        <location filename="resultstree.cpp" line="859"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="846"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>Ingen editor konfigurerad.

Konfigurera program i inställningar/program.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="860"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>Ingen standard editor vald.

Vänligen välj standard editor i inställningar/Program.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="886"/>
        <source>Could not find the file!</source>
        <translation>Kunde inte hitta filen!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="934"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>Kunde inte starta %1

Kontrollera att sökvägen och parametrarna är korrekta.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="956"/>
        <source>Select Directory</source>
        <translation>Välj mapp</translation>
    </message>
    <message>
        <source>Id</source>
        <translation type="vanished">Id</translation>
    </message>
    <message>
        <source>Inconclusive</source>
        <translation type="vanished">Inconclusive</translation>
    </message>
    <message>
        <source>Since date</source>
        <translation type="vanished">Sedan datum</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="444"/>
        <source>style</source>
        <translation>stil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="447"/>
        <source>error</source>
        <translation>fel</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="450"/>
        <source>warning</source>
        <translation>varning</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="453"/>
        <source>performance</source>
        <translation>prestanda</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="456"/>
        <source>portability</source>
        <translation>portabilitet</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="459"/>
        <source>information</source>
        <translation>information</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="257"/>
        <source>Print Report</source>
        <translation>Skriv ut rapport</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="276"/>
        <source>No errors found, nothing to print.</source>
        <translation>Inga fel hittades, inget att skriva ut.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="328"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 av %2 filer analyserade)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="352"/>
        <location filename="resultsview.cpp" line="363"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="353"/>
        <source>No errors found.</source>
        <translation>Inga fel hittades.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="360"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Fel hittades, men de visas ej.
För att ställa in vilka fel som skall visas använd visa menyn.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="404"/>
        <location filename="resultsview.cpp" line="423"/>
        <source>Failed to read the report.</source>
        <translation>Misslyckades att läsa rapporten.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="411"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>XML format version 1 stöds ej längre.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="472"/>
        <source>First included by</source>
        <translation>Först inkluderad av</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="477"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="479"/>
        <source>Bug hunting analysis is incomplete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="553"/>
        <source>Clear Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="554"/>
        <source>Copy this Log entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="555"/>
        <source>Copy complete Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="563"/>
        <source>Analysis was stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="577"/>
        <source>There was a critical error with id &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="579"/>
        <source>when checking %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="581"/>
        <source>when checking a file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="582"/>
        <source>Analysis was aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="239"/>
        <location filename="resultsview.cpp" line="247"/>
        <source>Failed to save the report.</source>
        <translation>Misslyckades med att spara rapporten.</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_resultsview.h" line="147"/>
        <source>Results</source>
        <translation>Resultat</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="60"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_resultsview.h" line="148"/>
        <source>Critical errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="92"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_resultsview.h" line="149"/>
        <source>Analysis Log</source>
        <translation>Analys Log</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="114"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_resultsview.h" line="150"/>
        <source>Warning Details</source>
        <translation>Varningsdetaljer</translation>
    </message>
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_scratchpad.h" line="107"/>
        <source>Scratchpad</source>
        <translation>Scratchpad</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="20"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_scratchpad.h" line="108"/>
        <source>Copy or write some C/C++ code here:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="37"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_scratchpad.h" line="109"/>
        <source>Optionally enter a filename (mainly for automatic language detection) and click on &quot;Check&quot;:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="71"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_scratchpad.h" line="110"/>
        <source>filename</source>
        <translation>Filnamn</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="78"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_scratchpad.h" line="111"/>
        <source>Check</source>
        <translation>Analysera</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="490"/>
        <source>Preferences</source>
        <translation>Inställningar</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="502"/>
        <source>General</source>
        <translation>Allmänt</translation>
    </message>
    <message>
        <location filename="settings.ui" line="196"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="503"/>
        <source>Add...</source>
        <translation>Lägg till...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="491"/>
        <source>Number of threads: </source>
        <translation>Antal trådar:</translation>
    </message>
    <message>
        <source>Ideal count:</source>
        <translation type="vanished">Optimalt värde:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="108"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="493"/>
        <source>Force checking all #ifdef configurations</source>
        <oldsource>Check all #ifdef configurations</oldsource>
        <translation>Kontrollera alla #ifdef konfigurationer</translation>
    </message>
    <message>
        <location filename="settings.ui" line="115"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="494"/>
        <source>Show full path of files</source>
        <translation>Visa den fulla sökvägen för filer</translation>
    </message>
    <message>
        <location filename="settings.ui" line="122"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="495"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Visa &quot;Inga fel hittades&quot; meddelande när inga fel hittas</translation>
    </message>
    <message>
        <location filename="settings.ui" line="129"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="496"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>Visa meddelande id i kolumn &quot;Id&quot;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="136"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="497"/>
        <source>Enable inline suppressions</source>
        <translation>Använd inline suppressions</translation>
    </message>
    <message>
        <location filename="settings.ui" line="143"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="498"/>
        <source>Check for inconclusive errors also</source>
        <translation>Kör inconclusive analys</translation>
    </message>
    <message>
        <location filename="settings.ui" line="150"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="499"/>
        <source>Show statistics on check completion</source>
        <translation>Visa statistik när analys är klar</translation>
    </message>
    <message>
        <location filename="settings.ui" line="157"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="500"/>
        <source>Check for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="177"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="501"/>
        <source>Show internal warnings in log</source>
        <translation>Visa interna fel i loggen</translation>
    </message>
    <message>
        <location filename="settings.ui" line="288"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="521"/>
        <source>Addons</source>
        <translation>Addons</translation>
    </message>
    <message>
        <location filename="settings.ui" line="294"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="512"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation>Python binär fil (lämna tom för att använda python i PATH)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="305"/>
        <location filename="settings.ui" line="346"/>
        <location filename="settings.ui" line="391"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="513"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="520"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="523"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="324"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="515"/>
        <source>MISRA addon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="332"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="516"/>
        <source>MISRA rule texts file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="339"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="518"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="372"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="526"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="settings.ui" line="378"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="522"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation>Clang sökväg (lämna tom för att använda PATH)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="401"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="524"/>
        <source>Visual Studio headers</source>
        <translation>Visual Studio headers</translation>
    </message>
    <message>
        <location filename="settings.ui" line="407"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="525"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Sökvägar till Visual Studio headers, separerade med semikolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;Du kan öppna en Visual Studio command prompt, och skriva &amp;quot;SET INCLUDE&amp;quot;. Sedan kopiera och klistra in sökvägarna.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="437"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="533"/>
        <source>Code Editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="443"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="527"/>
        <source>Code Editor Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="449"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="528"/>
        <source>System Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="456"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="529"/>
        <source>Default Light Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="463"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="530"/>
        <source>Default Dark Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="472"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="531"/>
        <source>Custom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="210"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="505"/>
        <source>Remove</source>
        <translation>Ta bort</translation>
    </message>
    <message>
        <location filename="settings.ui" line="79"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="492"/>
        <source>Max count:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="185"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="507"/>
        <source>Applications</source>
        <translation>Program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="203"/>
        <location filename="settings.ui" line="479"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="504"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="532"/>
        <source>Edit...</source>
        <translation>Redigera...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="217"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="506"/>
        <source>Set as default</source>
        <translation>Sätt förvald</translation>
    </message>
    <message>
        <location filename="settings.ui" line="240"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="510"/>
        <source>Reports</source>
        <translation>Rapporter</translation>
    </message>
    <message>
        <location filename="settings.ui" line="246"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="508"/>
        <source>Save all errors when creating report</source>
        <translation>Spara alla fel</translation>
    </message>
    <message>
        <location filename="settings.ui" line="253"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="509"/>
        <source>Save full path to files in reports</source>
        <translation>Spara fulla sökvägar</translation>
    </message>
    <message>
        <location filename="settings.ui" line="274"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_settings.h" line="511"/>
        <source>Language</source>
        <translation>Språk</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <source>N/A</source>
        <translation type="vanished">Ej tillgängligt</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="232"/>
        <source>The executable file &quot;%1&quot; is not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="243"/>
        <source>Add a new application</source>
        <translation>Lägg till program</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="273"/>
        <source>Modify an application</source>
        <translation>Ändra program</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="278"/>
        <source> [Default]</source>
        <translation> [Vald]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="303"/>
        <source>[Default]</source>
        <translation>[Förvald]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="358"/>
        <source>Select python binary</source>
        <translation>Välj python binär</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="365"/>
        <source>Select MISRA File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="396"/>
        <source>Select clang path</source>
        <translation>Välj Clang sökväg</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="statsdialog.ui" line="14"/>
        <location filename="statsdialog.ui" line="248"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="395"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="415"/>
        <location filename="statsdialog.cpp" line="183"/>
        <location filename="statsdialog.cpp" line="230"/>
        <source>Statistics</source>
        <translation>Statistik</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="27"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="401"/>
        <location filename="statsdialog.cpp" line="221"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="33"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="396"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="53"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="397"/>
        <source>Paths:</source>
        <translation>Sökvägar:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="85"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="398"/>
        <source>Include paths:</source>
        <translation>Include sökvägar:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="108"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="399"/>
        <source>Defines:</source>
        <translation>Defines:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="131"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="400"/>
        <source>Undefines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="165"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="407"/>
        <location filename="statsdialog.cpp" line="226"/>
        <source>Previous Scan</source>
        <translation>Föregående analys</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="171"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="402"/>
        <source>Path Selected:</source>
        <translation>Vald sökväg:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="181"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="403"/>
        <source>Number of Files Scanned:</source>
        <translation>Antal analyserade filer:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="201"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="405"/>
        <source>Scan Duration:</source>
        <translation>Analys tid:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="254"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="408"/>
        <source>Errors:</source>
        <translation>Fel:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="271"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="409"/>
        <source>Warnings:</source>
        <translation>Varningar:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="288"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="410"/>
        <source>Stylistic warnings:</source>
        <translation>Stil varningar:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="305"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="411"/>
        <source>Portability warnings:</source>
        <translation>Portabilitets varningar:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="322"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="412"/>
        <source>Performance issues:</source>
        <translation>Prestanda varningar:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="339"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="413"/>
        <source>Information messages:</source>
        <translation>Informations meddelanden:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="356"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="414"/>
        <source>Active checkers:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="374"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="416"/>
        <source>Checkers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="399"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="418"/>
        <source>History</source>
        <translation>Historik</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="405"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="417"/>
        <source>File:</source>
        <translation>Fil:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="443"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="419"/>
        <source>Copy to Clipboard</source>
        <translation>Kopiera</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="450"/>
        <location filename="build-cppcheck-Desktop-Debug/gui/ui_statsdialog.h" line="420"/>
        <source>Pdf Export</source>
        <translation>Pdf Export</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="159"/>
        <source>1 day</source>
        <translation>1 dag</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="159"/>
        <source>%1 days</source>
        <translation>%1 dagar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="161"/>
        <source>1 hour</source>
        <translation>1 timme</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="161"/>
        <source>%1 hours</source>
        <translation>%1 timmar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="163"/>
        <source>1 minute</source>
        <translation>1 minut</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="163"/>
        <source>%1 minutes</source>
        <translation>%1 minuter</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>1 second</source>
        <translation>1 sekund</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>%1 seconds</source>
        <translation>%1 sekunder</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="169"/>
        <source>0.%1 seconds</source>
        <translation>0.%1 sekunder</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="171"/>
        <source> and </source>
        <translation> och </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="198"/>
        <source>Export PDF</source>
        <translation>Exportera PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="220"/>
        <source>Project Settings</source>
        <translation>Projekt inställningar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="222"/>
        <source>Paths</source>
        <translation>Sökvägar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="223"/>
        <source>Include paths</source>
        <translation>Include sökvägar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="224"/>
        <source>Defines</source>
        <translation>Definitioner</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="225"/>
        <source>Undefines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="227"/>
        <source>Path selected</source>
        <translation>Vald sökväg</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="228"/>
        <source>Number of files scanned</source>
        <translation>Antal analyserade filer</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="229"/>
        <source>Scan duration</source>
        <translation>Tid</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="185"/>
        <location filename="statsdialog.cpp" line="231"/>
        <source>Errors</source>
        <translation>Fel</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>File: </source>
        <translation>Fil:</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="112"/>
        <source>No cppcheck build dir</source>
        <translation>Ingen Cppcheck build dir</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="187"/>
        <location filename="statsdialog.cpp" line="232"/>
        <source>Warnings</source>
        <translation>Varningar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="189"/>
        <location filename="statsdialog.cpp" line="233"/>
        <source>Style warnings</source>
        <translation>Stil varningar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="191"/>
        <location filename="statsdialog.cpp" line="234"/>
        <source>Portability warnings</source>
        <translation>Portabilitetsvarningar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="193"/>
        <location filename="statsdialog.cpp" line="235"/>
        <source>Performance warnings</source>
        <translation>Prestanda varningar</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="195"/>
        <location filename="statsdialog.cpp" line="236"/>
        <source>Information messages</source>
        <translation>Informationsmeddelanden</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="46"/>
        <source>%1 of %2 files checked</source>
        <translation>%1 av %2 filer analyserade</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="130"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>Misslyckades att ändra språk:

%1

Språket har nollställts till Engelska. Öppna Preferences och välj något av de tillgängliga språken.</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="136"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="74"/>
        <source>inconclusive</source>
        <translation>inconclusive</translation>
    </message>
</context>
<context>
    <name>toFilterString</name>
    <message>
        <location filename="common.cpp" line="58"/>
        <source>All supported files (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="common.cpp" line="63"/>
        <source>All files (%1)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
