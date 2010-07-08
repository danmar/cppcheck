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
        <source>Copyright (C) 2007-2010 Daniel Marjamäki and cppcheck team.</source>
        <oldsource>Copyright (C) 2007-2009 Daniel Marjamäki and cppcheck team.</oldsource>
        <translation>Copyright (C) 2007-2010 Daniel Marjamäki and cppcheck team.</translation>
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
        <location filename="application.ui" line="14"/>
        <source>Add an application</source>
        <translation>Lägg till program</translation>
    </message>
    <message>
        <location filename="application.ui" line="20"/>
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
        <location filename="application.ui" line="39"/>
        <source>Application&apos;s name</source>
        <translation>Programmets namn</translation>
    </message>
    <message>
        <location filename="application.ui" line="46"/>
        <source>Application to execute</source>
        <translation>Kommandorad</translation>
    </message>
    <message>
        <location filename="application.ui" line="59"/>
        <source>Browse</source>
        <translation>Bläddra</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="57"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Exekverbara filer (*.exe);;Alla filer(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="60"/>
        <source>Select viewer application</source>
        <translation>Välj program</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="97"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="98"/>
        <source>You must specify a name and a path for the application!</source>
        <translation>Du måste ange programmets namn och kommandorad!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="47"/>
        <source>Could not find the file: %1</source>
        <oldsource>Could not find the file:
</oldsource>
        <translation>Kunde inte hitta filen: %1</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="51"/>
        <location filename="fileviewdialog.cpp" line="66"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="62"/>
        <source>Could not read the file: %1</source>
        <translation>Kunde inte läsa filen: %1</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="main.ui" line="26"/>
        <location filename="mainwindow.cpp" line="220"/>
        <location filename="mainwindow.cpp" line="483"/>
        <location filename="mainwindow.cpp" line="585"/>
        <location filename="mainwindow.cpp" line="603"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="143"/>
        <source>Standard</source>
        <translation>Standard</translation>
    </message>
    <message>
        <location filename="main.ui" line="74"/>
        <source>&amp;File</source>
        <translation>&amp;Arkiv</translation>
    </message>
    <message>
        <location filename="main.ui" line="83"/>
        <source>&amp;View</source>
        <translation>&amp;Visa</translation>
    </message>
    <message>
        <location filename="main.ui" line="87"/>
        <source>&amp;Toolbars</source>
        <translation>Verktygsfält</translation>
    </message>
    <message>
        <location filename="main.ui" line="120"/>
        <source>&amp;Check</source>
        <translation>&amp;Check</translation>
    </message>
    <message>
        <location filename="main.ui" line="129"/>
        <source>&amp;Edit</source>
        <translation>&amp;Redigera</translation>
    </message>
    <message>
        <location filename="main.ui" line="174"/>
        <source>&amp;License...</source>
        <translation>&amp;Licens...</translation>
    </message>
    <message>
        <location filename="main.ui" line="179"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Utvecklat av...</translation>
    </message>
    <message>
        <location filename="main.ui" line="188"/>
        <source>&amp;About...</source>
        <translation>&amp;Om...</translation>
    </message>
    <message>
        <location filename="main.ui" line="193"/>
        <source>&amp;Files...</source>
        <translation>&amp;Filer...</translation>
    </message>
    <message>
        <location filename="main.ui" line="196"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="205"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Katalog...</translation>
    </message>
    <message>
        <location filename="main.ui" line="208"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="217"/>
        <source>&amp;Recheck files</source>
        <translation>Starta &amp;om check</translation>
    </message>
    <message>
        <location filename="main.ui" line="220"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="229"/>
        <source>&amp;Stop</source>
        <translation>&amp;Stoppa</translation>
    </message>
    <message>
        <location filename="main.ui" line="232"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="241"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Spara resultat till fil...</translation>
    </message>
    <message>
        <location filename="main.ui" line="244"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="249"/>
        <source>&amp;Quit</source>
        <translation>&amp;Avsluta</translation>
    </message>
    <message>
        <location filename="main.ui" line="258"/>
        <source>&amp;Clear results</source>
        <translation>&amp;Töm resultat</translation>
    </message>
    <message>
        <location filename="main.ui" line="267"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Inställningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="279"/>
        <source>Show style errors</source>
        <translation>Visa stilvarningar</translation>
    </message>
    <message>
        <location filename="main.ui" line="291"/>
        <source>Show common errors</source>
        <translation>Visa vanliga fel</translation>
    </message>
    <message>
        <location filename="main.ui" line="296"/>
        <source>&amp;Check all</source>
        <translation>&amp;Kryssa alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="301"/>
        <source>&amp;Uncheck all</source>
        <translation>Kryssa &amp;ur alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="306"/>
        <source>Collapse &amp;all</source>
        <translatorcomment>Ingen bra översättning!</translatorcomment>
        <translation>&amp;Fäll ihop alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="311"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Expandera alla</translation>
    </message>
    <message>
        <location filename="main.ui" line="319"/>
        <source>&amp;Standard</source>
        <translation>&amp;Standard</translation>
    </message>
    <message>
        <location filename="main.ui" line="322"/>
        <source>Standard items</source>
        <translation>Standard poster</translation>
    </message>
    <message>
        <location filename="main.ui" line="338"/>
        <source>Toolbar</source>
        <translation>Verktygsfält</translation>
    </message>
    <message>
        <location filename="main.ui" line="346"/>
        <source>&amp;Categories</source>
        <translation>&amp;Kategorier</translation>
    </message>
    <message>
        <location filename="main.ui" line="349"/>
        <source>Error categories</source>
        <translation>Fel kategorier</translation>
    </message>
    <message>
        <location filename="main.ui" line="354"/>
        <source>Open P&amp;roject File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="359"/>
        <source>&amp;New Project File...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="327"/>
        <source>&amp;Contents</source>
        <translation>&amp;Innehåll</translation>
    </message>
    <message>
        <location filename="main.ui" line="161"/>
        <source>Categories</source>
        <translation>Kategorier</translation>
    </message>
    <message>
        <location filename="main.ui" line="330"/>
        <source>Open the help contents</source>
        <translation>Öppna hjälp</translation>
    </message>
    <message>
        <location filename="main.ui" line="333"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="main.ui" line="105"/>
        <source>&amp;Language</source>
        <translation>&amp;Språk</translation>
    </message>
    <message>
        <location filename="main.ui" line="110"/>
        <source>&amp;Help</source>
        <translation>&amp;Hjälp</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="253"/>
        <source>Select files to check</source>
        <translation>Välj filer att kontrollera</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="267"/>
        <source>Select directory to check</source>
        <translation>Välj katalog som skall kontrolleras</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="221"/>
        <source>No suitable files found to check!</source>
        <translation>Inga lämpliga filer hittades!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="479"/>
        <source>Cannot exit while checking.

Stop the checking before exiting.</source>
        <translation>Kan ej avsluta medans check pågår.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="510"/>
        <source>License</source>
        <translation>Licens</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="517"/>
        <source>Authors</source>
        <translation>Utvecklare</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="525"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML filer (*.xml);;Text filer (*.txt);;CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="527"/>
        <source>Save the report file</source>
        <translation>Spara rapport</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="535"/>
        <source>XML files (*.xml)</source>
        <translation>XML filer (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="541"/>
        <source>Text files (*.txt)</source>
        <translation>Text filer (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="547"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV filer (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="587"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="604"/>
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
        <location filename="mainwindow.cpp" line="671"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="673"/>
        <source>Select Project File</source>
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
        <source>Include paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="39"/>
        <source>Defines:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="48"/>
        <location filename="projectfiledialog.cpp" line="106"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="49"/>
        <source>Could not read the project file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="87"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="89"/>
        <source>Save Project File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="107"/>
        <source>Could not write the project file.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="93"/>
        <source>Incorrect language specified!</source>
        <translation>valt språk är ej korrekt!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="103"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation>Språk filen %1 hittades ej!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="109"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Misslyckades med att ladda översättningen för %1 från filen %2</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="42"/>
        <location filename="resultstree.cpp" line="796"/>
        <source>File</source>
        <translation>Fil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="42"/>
        <location filename="resultstree.cpp" line="796"/>
        <source>Severity</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="42"/>
        <location filename="resultstree.cpp" line="796"/>
        <source>Line</source>
        <translation>Rad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="42"/>
        <location filename="resultstree.cpp" line="796"/>
        <source>Message</source>
        <translation>Meddelande</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="88"/>
        <source>Undefined file</source>
        <translation>Odefinierad fil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="419"/>
        <source>Copy filename</source>
        <translation>Kopiera filnamn</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="420"/>
        <source>Copy full path</source>
        <translation>Kopiera full sökväg</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="421"/>
        <source>Copy message</source>
        <translation>Kopiera meddelande</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="459"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="460"/>
        <source>Configure the text file viewer program in Cppcheck preferences/Applications.</source>
        <oldsource>You can open this error by specifying applications in program&apos;s settings.</oldsource>
        <translation>Konfigurera program i inställningar/program.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="497"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>Kunde inte starta %1

Kontrollera att sökvägen och parametrarna är korrekta.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="661"/>
        <source>style</source>
        <translation>stil</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="665"/>
        <source>error</source>
        <translation>fel</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="174"/>
        <location filename="resultsview.cpp" line="186"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="175"/>
        <source>No errors found.</source>
        <translation>Inga fel hittades.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="183"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Fel hittades, men de visas ej.
För att ställa in vilka fel som skall visas använd visa menyn.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="101"/>
        <source>No errors found, nothing to save.</source>
        <translation>Inga fel hittades, ingenting att spara.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="128"/>
        <location filename="resultsview.cpp" line="138"/>
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
        <location filename="settings.ui" line="34"/>
        <source>Number of threads: </source>
        <translation>Antal trådar:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="46"/>
        <source>Check all #ifdef configurations</source>
        <translation>Kontrollera alla #ifdef konfigurationer</translation>
    </message>
    <message>
        <location filename="settings.ui" line="53"/>
        <source>Show full path of files</source>
        <translation>Visa den fulla sökvägen för filer</translation>
    </message>
    <message>
        <location filename="settings.ui" line="60"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Visa &quot;Inga fel hittades&quot; meddelande när inga fel hittas</translation>
    </message>
    <message>
        <location filename="settings.ui" line="83"/>
        <source>Applications</source>
        <translation>Program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="92"/>
        <source>Add application</source>
        <translation>Lägg till program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="99"/>
        <source>Delete application</source>
        <translation>Ta bort program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="106"/>
        <source>Modify application</source>
        <translation>Ändra program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="113"/>
        <source>Set as default application</source>
        <translation>Ange som standard program</translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Reports</source>
        <translation>Rapporter</translation>
    </message>
    <message>
        <location filename="settings.ui" line="127"/>
        <source>Save all errors when creating report</source>
        <translation>Spara alla fel</translation>
    </message>
    <message>
        <location filename="settings.ui" line="134"/>
        <source>Save full path to files in reports</source>
        <translation>Spara fulla sökvägar</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="132"/>
        <source>Add a new application</source>
        <translation>Lägg till program</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="165"/>
        <source>Modify an application</source>
        <translation>Ändra program</translation>
    </message>
</context>
</TS>
