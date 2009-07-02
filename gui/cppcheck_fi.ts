<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="fi_FI">
<context>
    <name>AboutDialog</name>
    <message>
        <location filename="aboutdialog.cpp" line="29"/>
        <source>About cppcheck</source>
        <translation>Tietoa ohjelmasta Cppcheck</translation>
    </message>
    <message>
        <location filename="aboutdialog.cpp" line="42"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - Työkalu C/C++ koodin staattiseen analysointiin.</translation>
    </message>
    <message>
        <location filename="aboutdialog.cpp" line="43"/>
        <source>Version %1</source>
        <translation>Versio %1</translation>
    </message>
    <message>
        <location filename="aboutdialog.cpp" line="46"/>
        <source>This program is licensed under the terms of the GNU General Public License version 3</source>
        <translation>Tämä ohjelma on lisensoitu GNU General Public Lisenssin version 3 alaiseksi</translation>
    </message>
    <message>
        <location filename="aboutdialog.cpp" line="49"/>
        <source>Close</source>
        <translation>Sulje</translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="application.ui" line="14"/>
        <source>Add an application</source>
        <translation>Lisää uusi ohjelma</translation>
    </message>
    <message>
        <location filename="application.ui" line="20"/>
        <source>Here you can add applications that can open error files.
Specify a name for the application and the application to execute.

The following texts are replaced with appriproate values when application is executed:
(file) - Filename containing the error
(line) - Line number containing the error
(message) - Error message
(severity) - Error severity

Example opening a file with Kate and make Kate scroll to the correct line:
kate -l(line) (file)</source>
        <translation>Voi lisätä ohjelmia joilla avata virheen sisältämiä tiedostoja
määrittämällä suoritettavan ohjelman ja nimen ohjelmalle.

Suoritettavalle ohjelmalle voi määrittää parametreja jotka Cppcheck korvaa sopivilla arvoilla.
Parametrit ovat seuraavat:
(file) - Tiedosto joka sisältää virheen
(line) - Rivinumero josta virhe löytyy
(message) - Virheilmoitus
(severity) - Virheen vakavuus

Esimerkki virheen avaamisesta Kate nimisellä tekstinkäsittelyohjelmalla:
kate -l(line) (file)
</translation>
    </message>
    <message>
        <location filename="application.ui" line="39"/>
        <source>Application&apos;s name</source>
        <translation>Ohjelman nimi</translation>
    </message>
    <message>
        <location filename="application.ui" line="46"/>
        <source>Application to execute</source>
        <translation>Suoritettava ohjelma</translation>
    </message>
    <message>
        <location filename="application.ui" line="59"/>
        <source>Browse</source>
        <translation>Selaa</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="57"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Suoritettavat tiedostot (*.exe);;Kaikki tiedostot(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="60"/>
        <source>Select viewer application</source>
        <translation>Valitse ohjelma jolla avata virhetiedosto</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="97"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="98"/>
        <source>You must specify a name and a path for the application!</source>
        <translation>Suoritettava ohjelma tulee määrittää!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="37"/>
        <source>Close</source>
        <translation>Sulje</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="57"/>
        <source>Could not find the file: %1</source>
        <oldsource>Could not find the file:
</oldsource>
        <translation>Tiedostoa %1 ei löytynyt</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="61"/>
        <location filename="fileviewdialog.cpp" line="76"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="72"/>
        <source>Could not read the file: %1</source>
        <translation>Tiedoston %1 lukeminen epäonnistui</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="main.ui" line="26"/>
        <location filename="mainwindow.cpp" line="243"/>
        <location filename="mainwindow.cpp" line="450"/>
        <location filename="mainwindow.cpp" line="548"/>
        <location filename="mainwindow.cpp" line="566"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="253"/>
        <source>Show possible false positives</source>
        <translation>Näytä epävarmat virheet</translation>
    </message>
    <message>
        <location filename="main.ui" line="74"/>
        <source>&amp;File</source>
        <translation>&amp;Tiedosto</translation>
    </message>
    <message>
        <location filename="main.ui" line="81"/>
        <source>&amp;View</source>
        <translation>&amp;Näytä</translation>
    </message>
    <message>
        <location filename="main.ui" line="110"/>
        <source>&amp;Check</source>
        <translation>&amp;Tarkista</translation>
    </message>
    <message>
        <location filename="main.ui" line="119"/>
        <source>&amp;Edit</source>
        <translation>&amp;Muokkaa</translation>
    </message>
    <message>
        <location filename="main.ui" line="134"/>
        <source>toolBar</source>
        <translation>työkalupalkki</translation>
    </message>
    <message>
        <location filename="main.ui" line="152"/>
        <source>&amp;License...</source>
        <translation>&amp;Lisenssi...</translation>
    </message>
    <message>
        <location filename="main.ui" line="157"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Tekijät...</translation>
    </message>
    <message>
        <location filename="main.ui" line="166"/>
        <source>&amp;About...</source>
        <translation>&amp;Tietoa ohjelmasta Cppcheck...</translation>
    </message>
    <message>
        <location filename="main.ui" line="171"/>
        <source>&amp;Files...</source>
        <translation>&amp;Tiedostot...</translation>
    </message>
    <message>
        <location filename="main.ui" line="174"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="183"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Hakemisto...</translation>
    </message>
    <message>
        <location filename="main.ui" line="186"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="195"/>
        <source>&amp;Recheck files</source>
        <translation>Tarkista tiedostot &amp;uudelleen</translation>
    </message>
    <message>
        <location filename="main.ui" line="198"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="207"/>
        <source>&amp;Stop</source>
        <translation>&amp;Pysäytä</translation>
    </message>
    <message>
        <location filename="main.ui" line="210"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="219"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Tallenna tulokset tiedostoon...</translation>
    </message>
    <message>
        <location filename="main.ui" line="222"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="227"/>
        <source>&amp;Quit</source>
        <translation>&amp;Lopeta</translation>
    </message>
    <message>
        <location filename="main.ui" line="236"/>
        <source>&amp;Clear results</source>
        <translation>&amp;Tyhjennä tulokset</translation>
    </message>
    <message>
        <location filename="main.ui" line="245"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Asetukset</translation>
    </message>
    <message>
        <location filename="main.ui" line="261"/>
        <source>Show security errors</source>
        <translation>Näytä tietoturvavirheet</translation>
    </message>
    <message>
        <location filename="main.ui" line="269"/>
        <source>Show style errors</source>
        <translation>Näytä tyylivirheet</translation>
    </message>
    <message>
        <location filename="main.ui" line="277"/>
        <source>Show common errors</source>
        <translation>Näytä yleiset virheet</translation>
    </message>
    <message>
        <location filename="main.ui" line="282"/>
        <source>&amp;Check all</source>
        <translation>&amp;Valitse kaikki</translation>
    </message>
    <message>
        <location filename="main.ui" line="287"/>
        <source>&amp;Uncheck all</source>
        <translation>&amp;Poista kaikista valinta</translation>
    </message>
    <message>
        <location filename="main.ui" line="292"/>
        <source>Collapse &amp;all</source>
        <translation>&amp;Pienennä kaikki</translation>
    </message>
    <message>
        <location filename="main.ui" line="297"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Laajenna kaikki</translation>
    </message>
    <message>
        <location filename="main.ui" line="305"/>
        <source>&amp;Toolbar</source>
        <translation>&amp;Työkalupalkki</translation>
    </message>
    <message>
        <location filename="main.ui" line="97"/>
        <source>&amp;Language</source>
        <translation>&amp;Kieli</translation>
    </message>
    <message>
        <location filename="main.ui" line="102"/>
        <source>&amp;Help</source>
        <translation>&amp;Ohje</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="206"/>
        <source>Select files to check</source>
        <translation>Valitse tarkistettavat tiedostot</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="215"/>
        <source>Select directory to check</source>
        <translation>Valitse tarkistettava hakemisto</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="244"/>
        <source>No suitable files found to check!</source>
        <translation>Tarkistettavaksi sopivia tiedostoja ei löytynyt!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="446"/>
        <source>Cannot exit while checking.

Stop the checking before exiting.</source>
        <translation>Ohjelmaa ei voi sammuttaa tarkistuksen ollessa käynnissä.

Lopeta tarkistus ennen ohjelman sammuttamista.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="507"/>
        <source>XML files (*.xml);;Text files (*.txt)</source>
        <translation>XML-tiedostot (*.xml);;Tekstitiedostot (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="509"/>
        <source>Save the report file</source>
        <translation>Tallenna raportti</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="517"/>
        <source>XML files (*.xml)</source>
        <translation>XML-tiedostot (*xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="550"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="567"/>
        <source>Failed to change language:

%1</source>
        <translation>Kielen %1 vaihto epäonnistui</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="12"/>
        <source>English</source>
        <translation>Englanti</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="13"/>
        <source>Finnish</source>
        <translation>Suomi</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="14"/>
        <source>Swedish</source>
        <translation>Ruotsi</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="15"/>
        <source>German</source>
        <translation>Saksa</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="16"/>
        <source>Russian</source>
        <translation>Venäjä</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="69"/>
        <source>Incorrect language specified!</source>
        <translation>Virheellinen kieli valittu!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="77"/>
        <source>Failed to load language from file %1</source>
        <translation>Kielen lataaminen tiedostosta %1 epäonnistui</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="39"/>
        <location filename="resultstree.cpp" line="754"/>
        <source>File</source>
        <translation>Tiedosto</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="39"/>
        <location filename="resultstree.cpp" line="754"/>
        <source>Severity</source>
        <translation>Tyyppi</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="39"/>
        <location filename="resultstree.cpp" line="754"/>
        <source>Line</source>
        <translation>Rivi</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="39"/>
        <location filename="resultstree.cpp" line="754"/>
        <source>Message</source>
        <translation>Virhe</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="85"/>
        <source>Undefined file</source>
        <translation>Määrittelemätön tiedosto</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="388"/>
        <source>Copy filename</source>
        <translation>Kopioi tiedostonimi</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="389"/>
        <source>Copy full path</source>
        <translation>Kopioi tiedoston koko polku</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="425"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="426"/>
        <source>You can open this error by specifying applications in program&apos;s settings.</source>
        <translation>Voit asetuksista määritellä muita ohjelmia joilla avata tämän virheen sisältävän tiedoston.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="471"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>Ohjelman %1 käynnistäminen epäonnistui

Tarkista että ohjelman polku ja parametrit ovat oikeat.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="612"/>
        <source>all</source>
        <translation>Epävarmat</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="616"/>
        <source>style</source>
        <translation>Tyyli</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="620"/>
        <source>security</source>
        <translation>Tietoturva</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="624"/>
        <source>error</source>
        <translation>Yleinen</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="77"/>
        <location filename="resultsview.cpp" line="89"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="78"/>
        <source>No errors found.</source>
        <translation>Virheitä ei löytynyt.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="86"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Virheitä löytyi, mutta asetuksissa kyseiset virheet on määritelty piilotettavaksi.
Määrittääksesi minkä tyyppisiä virheitä näytetään, avaa näkymä valikko.</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>Tulokset</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Settings</source>
        <translation>Asetukset</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>Yleiset</translation>
    </message>
    <message>
        <location filename="settings.ui" line="34"/>
        <source>Number of threads: </source>
        <translation>Säikeiden lukumäärä: </translation>
    </message>
    <message>
        <location filename="settings.ui" line="46"/>
        <source>Check all #ifdef configurations</source>
        <translation>Tarkista kaikki #ifdef kombinaatiot</translation>
    </message>
    <message>
        <location filename="settings.ui" line="53"/>
        <source>Show full path of files</source>
        <translation>Näytä tiedostojen täysi polku</translation>
    </message>
    <message>
        <location filename="settings.ui" line="60"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Näytä &quot;virheitä ei löytynyt&quot;-viesti jos virheitä ei löydy</translation>
    </message>
    <message>
        <location filename="settings.ui" line="83"/>
        <source>Applications</source>
        <translation>Ohjelmat</translation>
    </message>
    <message>
        <location filename="settings.ui" line="92"/>
        <source>Add application</source>
        <translation>Lisää ohjelma</translation>
    </message>
    <message>
        <location filename="settings.ui" line="99"/>
        <source>Delete application</source>
        <translation>Poista ohjelma</translation>
    </message>
    <message>
        <location filename="settings.ui" line="106"/>
        <source>Modify application</source>
        <translation>Muokkaa ohjelmaa</translation>
    </message>
    <message>
        <location filename="settings.ui" line="113"/>
        <source>Set as default application</source>
        <translation>Aseta oletusohjelmaksi</translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Reports</source>
        <translation>Raportit</translation>
    </message>
    <message>
        <location filename="settings.ui" line="127"/>
        <source>Save all errors when creating report</source>
        <translation>Tallenna kaikki virheet raporttia luodessa</translation>
    </message>
    <message>
        <location filename="settings.ui" line="134"/>
        <source>Save full path to files in reports</source>
        <translation>Tallenna tiedostojen koko polku raportteihin</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="132"/>
        <source>Add a new application</source>
        <translation>Lisää uusi ohjelma</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="165"/>
        <source>Modify an application</source>
        <translation>Muokkaa ohjelmaa</translation>
    </message>
</context>
</TS>
