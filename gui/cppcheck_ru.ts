<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>О Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>Версия %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - программа для статического анализа кода на языках С/С++</translation>
    </message>
    <message utf8="true">
        <location filename="about.ui" line="81"/>
        <source>Copyright (C) 2007-2010 Daniel Marjamäki and cppcheck team.</source>
        <oldsource>Copyright (C) 2007-2009 Daniel Marjamäki and cppcheck team.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>Эта программа распространяется на
условиях лицензии GNU General Public License, версии 3</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Посетите домашнюю страницу: %1</translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="application.ui" line="14"/>
        <source>Add an application</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="application.ui" line="39"/>
        <source>Application&apos;s name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="application.ui" line="46"/>
        <source>Application to execute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="application.ui" line="59"/>
        <source>Browse</source>
        <translation>Просмотреть</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="57"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="60"/>
        <source>Select viewer application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="97"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="98"/>
        <source>You must specify a name and a path for the application!</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="47"/>
        <source>Could not find the file: %1</source>
        <oldsource>Could not find the file:
</oldsource>
        <translation>Невозможно найти файл: %1</translation>
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
        <translation>Невозможно прочитать файл: %1</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="74"/>
        <source>&amp;File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="main.ui" line="83"/>
        <source>&amp;View</source>
        <translation>Вид</translation>
    </message>
    <message>
        <location filename="main.ui" line="87"/>
        <source>&amp;Toolbars</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="120"/>
        <source>&amp;Check</source>
        <translation>Проверить</translation>
    </message>
    <message>
        <location filename="main.ui" line="129"/>
        <source>&amp;Edit</source>
        <translation>Правка</translation>
    </message>
    <message>
        <location filename="main.ui" line="174"/>
        <source>&amp;License...</source>
        <translation>Лицензия...</translation>
    </message>
    <message>
        <location filename="main.ui" line="179"/>
        <source>A&amp;uthors...</source>
        <translation>Авторы...</translation>
    </message>
    <message>
        <location filename="main.ui" line="188"/>
        <source>&amp;About...</source>
        <translation>О программе...</translation>
    </message>
    <message>
        <location filename="main.ui" line="193"/>
        <source>&amp;Files...</source>
        <translation>Файлы...</translation>
    </message>
    <message>
        <location filename="main.ui" line="196"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="205"/>
        <source>&amp;Directory...</source>
        <translation>Каталог...</translation>
    </message>
    <message>
        <location filename="main.ui" line="208"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="217"/>
        <source>&amp;Recheck files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="220"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="229"/>
        <source>&amp;Stop</source>
        <translation>Остановить</translation>
    </message>
    <message>
        <location filename="main.ui" line="232"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="241"/>
        <source>&amp;Save results to file...</source>
        <translation>Сохранить отчёт в файл...</translation>
    </message>
    <message>
        <location filename="main.ui" line="244"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="249"/>
        <source>&amp;Quit</source>
        <translation>Выход</translation>
    </message>
    <message>
        <location filename="main.ui" line="258"/>
        <source>&amp;Clear results</source>
        <translation>Очистить отчёт</translation>
    </message>
    <message>
        <location filename="main.ui" line="267"/>
        <source>&amp;Preferences</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="main.ui" line="279"/>
        <source>Show style errors</source>
        <translation>Показывать ошибки стиля</translation>
    </message>
    <message>
        <location filename="main.ui" line="291"/>
        <source>Show common errors</source>
        <translation>Показывать общие ошибки</translation>
    </message>
    <message>
        <location filename="main.ui" line="296"/>
        <source>&amp;Check all</source>
        <translation>Отметить все</translation>
    </message>
    <message>
        <location filename="main.ui" line="301"/>
        <source>&amp;Uncheck all</source>
        <translation>Сбросить все</translation>
    </message>
    <message>
        <location filename="main.ui" line="306"/>
        <source>Collapse &amp;all</source>
        <translation>Свернуть все</translation>
    </message>
    <message>
        <location filename="main.ui" line="311"/>
        <source>&amp;Expand all</source>
        <translation>Развернуть все</translation>
    </message>
    <message>
        <location filename="main.ui" line="319"/>
        <source>&amp;Standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="322"/>
        <source>Standard items</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="338"/>
        <source>Toolbar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="346"/>
        <source>&amp;Categories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="349"/>
        <source>Error categories</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="161"/>
        <source>Categories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="330"/>
        <source>Open the help contents</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="333"/>
        <source>F1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="105"/>
        <source>&amp;Language</source>
        <translation>Язык</translation>
    </message>
    <message>
        <location filename="main.ui" line="110"/>
        <source>&amp;Help</source>
        <translation>Помощь</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="253"/>
        <source>Select files to check</source>
        <translation>Выберите файлы для проверки</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="267"/>
        <source>Select directory to check</source>
        <translation>Выберите каталог для проверки</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="221"/>
        <source>No suitable files found to check!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="479"/>
        <source>Cannot exit while checking.

Stop the checking before exiting.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="510"/>
        <source>License</source>
        <translation>Лицензия</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="517"/>
        <source>Authors</source>
        <translation>Авторы</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="525"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="527"/>
        <source>Save the report file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="535"/>
        <source>XML files (*.xml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="541"/>
        <source>Text files (*.txt)</source>
        <translation>Текстовые файлы (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="547"/>
        <source>CSV files (*.csv)</source>
        <translation type="unfinished"></translation>
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
        <translation>Невозможно изменить язык приложения:

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
        <translation>Финский</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="32"/>
        <source>English</source>
        <translation>Английский</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="33"/>
        <source>Dutch</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Swedish</source>
        <translation>Швецкий</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>German</source>
        <translation>Немецкий</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Russian</source>
        <translation>Русский</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Polish</source>
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
        <translation>Выбран неверный язык!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="103"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="109"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Ошибка загрузки переводов для языка %1 из файла %2</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="42"/>
        <location filename="resultstree.cpp" line="796"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="42"/>
        <location filename="resultstree.cpp" line="796"/>
        <source>Severity</source>
        <translation>Важность</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="42"/>
        <location filename="resultstree.cpp" line="796"/>
        <source>Line</source>
        <translation>Строка</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="42"/>
        <location filename="resultstree.cpp" line="796"/>
        <source>Message</source>
        <translation>Сообщение</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="88"/>
        <source>Undefined file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="419"/>
        <source>Copy filename</source>
        <translation>Скопировать имя файла</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="420"/>
        <source>Copy full path</source>
        <translation>Скопировать полный путь</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="421"/>
        <source>Copy message</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="497"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="661"/>
        <source>style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="665"/>
        <source>error</source>
        <translation type="unfinished"></translation>
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
        <translation>Ошибок не найдено.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="183"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="101"/>
        <source>No errors found, nothing to save.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="128"/>
        <location filename="resultsview.cpp" line="138"/>
        <source>Failed to save the report.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="settings.ui" line="34"/>
        <source>Number of threads: </source>
        <translation>Количество потоков исполнения:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="46"/>
        <source>Check all #ifdef configurations</source>
        <translation>Проверять все варианты #ifdef конфигураций</translation>
    </message>
    <message>
        <location filename="settings.ui" line="53"/>
        <source>Show full path of files</source>
        <translation>Показывать полные пути к файлам</translation>
    </message>
    <message>
        <location filename="settings.ui" line="60"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Показывать сообщение, если ошибок не найдено</translation>
    </message>
    <message>
        <location filename="settings.ui" line="83"/>
        <source>Applications</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="92"/>
        <source>Add application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="99"/>
        <source>Delete application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="106"/>
        <source>Modify application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="113"/>
        <source>Set as default application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Reports</source>
        <translation>Отчёты</translation>
    </message>
    <message>
        <location filename="settings.ui" line="127"/>
        <source>Save all errors when creating report</source>
        <translation>Сохранять все ошибки при создании отчёта</translation>
    </message>
    <message>
        <location filename="settings.ui" line="134"/>
        <source>Save full path to files in reports</source>
        <translation>Сохранять полные пути к файлам в отчётах</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="132"/>
        <source>Add a new application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="165"/>
        <source>Modify an application</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
