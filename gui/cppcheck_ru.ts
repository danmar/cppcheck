<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU" sourcelanguage="ru_RU">
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
        <translation>Cppcheck - программа для статического анализа кода на языках С/С++.</translation>
    </message>
    <message utf8="true">
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-2016 Cppcheck team.</source>
        <oldsource>Copyright © 2007-2016 Daniel Marjamäki and cppcheck team.</oldsource>
        <translation type="unfinished">Copyright © 2007-2013 Daniel Marjamäki and cppcheck team.</translation>
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
        <location filename="application.ui" line="23"/>
        <source>Add an application</source>
        <translation>Добавление приложения</translation>
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
        <translation>Вы можете добавить приложение, которое можно будет использовать для открытия файла с ошибками. Задайте название приложения, путь до него и параметры командной строки.

Следующие текстовые параметры будут заменены реальными значениями при запуске приложения:
(file) - файл, содержащий ошибку
(line) - номер строки с ошибкой
(message) - текст ошибки
(severity) - тип ошибки

Пример открытия файла с помощью Kate (скролл переместится на нужную строчку):
Программа: kate
Параметры: -l(line) (file)</translation>
    </message>
    <message>
        <location filename="application.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>&amp;Название:</translation>
    </message>
    <message>
        <location filename="application.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>&amp;Программа:</translation>
    </message>
    <message>
        <location filename="application.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>&amp;Параметры:</translation>
    </message>
    <message>
        <location filename="application.ui" line="138"/>
        <source>Browse</source>
        <translation>Просмотреть</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Выполняемые файлы  (*.exe);;Все файлы(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="62"/>
        <source>Select viewer application</source>
        <translation>Выберите приложение</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="77"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="78"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>Вы должны задать название и путь к приложению!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="42"/>
        <source>Could not find the file: %1</source>
        <oldsource>Could not find the file:
</oldsource>
        <translation>Невозможно найти файл: %1</translation>
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
        <translation>Невозможно прочитать файл: %1</translation>
    </message>
</context>
<context>
    <name>LibraryAddFunctionDialog</name>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="23"/>
        <source>Add function</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <source>Function name(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <source>Number of arguments</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <source>Library Editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <source>Open</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <source>Save</source>
        <translation type="unfinished">Сохранить</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <source>Save as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <source>Functions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <source>Sort</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <source>Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <source>Comments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="204"/>
        <source>noreturn</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="212"/>
        <source>False</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="217"/>
        <source>True</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="222"/>
        <source>Unknown</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="232"/>
        <source>return value must be used</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <source>ignore function in leaks checking</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <source>Arguments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <source>Edit</source>
        <translation type="unfinished">Изменить</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="82"/>
        <location filename="librarydialog.cpp" line="140"/>
        <source>Library files (*.cfg)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="84"/>
        <source>Open library file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="110"/>
        <location filename="librarydialog.cpp" line="130"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="111"/>
        <source>Can not open file %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="131"/>
        <source>Can not save file %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="143"/>
        <source>Save the library as</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <source>Edit argument</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="20"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is bool value allowed? For instance result from comparison or from &apos;!&apos; operator.&lt;/p&gt;
&lt;p&gt;Typically, set this if the argument is a pointer, size, etc.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // last argument should not have a bool value&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="28"/>
        <source>Not bool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="35"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is a null parameter value allowed?&lt;/p&gt;
&lt;p&gt;Typically this should be used on any pointer parameter that does not allow null.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // neither x or y is allowed to be null.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="43"/>
        <source>Not null</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="50"/>
        <source>Not uninit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="57"/>
        <source>String</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="70"/>
        <source>Format string</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="92"/>
        <source>Min size of buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="101"/>
        <location filename="libraryeditargdialog.ui" line="208"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <source>None</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <source>argvalue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="229"/>
        <source>constant</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="124"/>
        <location filename="libraryeditargdialog.ui" line="234"/>
        <source>mul</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="129"/>
        <location filename="libraryeditargdialog.ui" line="239"/>
        <source>strlen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="137"/>
        <location filename="libraryeditargdialog.ui" line="247"/>
        <source>Arg</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="164"/>
        <location filename="libraryeditargdialog.ui" line="274"/>
        <source>Arg2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="199"/>
        <source>and</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="320"/>
        <source>Valid values</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LogView</name>
    <message>
        <location filename="logview.ui" line="17"/>
        <source>Checking Log</source>
        <translation>Лог проверки</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="34"/>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="68"/>
        <source>Save Log</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="69"/>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation>Текстовые файлы (*.txt *.log);;Все файлы (*.*)</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="73"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="74"/>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation>Не удалось записать в файл: &quot;%1&quot;</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="main.ui" line="26"/>
        <location filename="mainwindow.cpp" line="305"/>
        <location filename="mainwindow.cpp" line="373"/>
        <location filename="mainwindow.cpp" line="430"/>
        <location filename="mainwindow.cpp" line="495"/>
        <location filename="mainwindow.cpp" line="517"/>
        <location filename="mainwindow.cpp" line="871"/>
        <location filename="mainwindow.cpp" line="978"/>
        <location filename="mainwindow.cpp" line="1103"/>
        <location filename="mainwindow.cpp" line="1273"/>
        <location filename="mainwindow.cpp" line="1360"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="180"/>
        <source>Standard</source>
        <translation>Стандартные</translation>
    </message>
    <message>
        <location filename="main.ui" line="70"/>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <location filename="main.ui" line="89"/>
        <source>&amp;View</source>
        <translation>&amp;Вид</translation>
    </message>
    <message>
        <location filename="main.ui" line="93"/>
        <source>&amp;Toolbars</source>
        <translation>&amp;Панель инструментов</translation>
    </message>
    <message>
        <location filename="main.ui" line="132"/>
        <source>&amp;Check</source>
        <translation>&amp;Проверить</translation>
    </message>
    <message>
        <location filename="main.ui" line="136"/>
        <source>C++ standard</source>
        <translation>Стандарт C++</translation>
    </message>
    <message>
        <location filename="main.ui" line="143"/>
        <source>C standard</source>
        <translation>Стандарт C</translation>
    </message>
    <message>
        <location filename="main.ui" line="167"/>
        <source>&amp;Edit</source>
        <translation>&amp;Правка</translation>
    </message>
    <message>
        <location filename="main.ui" line="229"/>
        <source>&amp;License...</source>
        <translation>&amp;Лицензия...</translation>
    </message>
    <message>
        <location filename="main.ui" line="234"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Авторы...</translation>
    </message>
    <message>
        <location filename="main.ui" line="243"/>
        <source>&amp;About...</source>
        <translation>&amp;О программе...</translation>
    </message>
    <message>
        <location filename="main.ui" line="248"/>
        <source>&amp;Files...</source>
        <translation>&amp;Файлы...</translation>
    </message>
    <message>
        <location filename="main.ui" line="251"/>
        <location filename="main.ui" line="254"/>
        <source>Check files</source>
        <translation>Проверить файлы</translation>
    </message>
    <message>
        <location filename="main.ui" line="257"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="266"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Каталог...</translation>
    </message>
    <message>
        <location filename="main.ui" line="269"/>
        <location filename="main.ui" line="272"/>
        <source>Check directory</source>
        <translation>Проверка директории</translation>
    </message>
    <message>
        <location filename="main.ui" line="275"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>&amp;Recheck files</source>
        <translation type="obsolete">&amp;Перепроверить файлы</translation>
    </message>
    <message>
        <location filename="main.ui" line="287"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="296"/>
        <source>&amp;Recheck all files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="305"/>
        <source>&amp;Stop</source>
        <translation>Остановить</translation>
    </message>
    <message>
        <location filename="main.ui" line="308"/>
        <location filename="main.ui" line="311"/>
        <source>Stop checking</source>
        <translation>Остановить проверку</translation>
    </message>
    <message>
        <location filename="main.ui" line="314"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="323"/>
        <source>&amp;Save results to file...</source>
        <translation>Сохранить отчёт в файл...</translation>
    </message>
    <message>
        <location filename="main.ui" line="326"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="331"/>
        <source>&amp;Quit</source>
        <translation>Выход</translation>
    </message>
    <message>
        <location filename="main.ui" line="340"/>
        <source>&amp;Clear results</source>
        <translation>Очистить отчёт</translation>
    </message>
    <message>
        <location filename="main.ui" line="349"/>
        <source>&amp;Preferences</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="main.ui" line="379"/>
        <source>Errors</source>
        <translation>Ошибки</translation>
    </message>
    <message>
        <location filename="main.ui" line="382"/>
        <location filename="main.ui" line="385"/>
        <source>Show errors</source>
        <translation>Показать ошибки</translation>
    </message>
    <message>
        <location filename="main.ui" line="466"/>
        <source>Show S&amp;cratchpad...</source>
        <translation>Показать блокнот</translation>
    </message>
    <message>
        <location filename="main.ui" line="516"/>
        <source>Warnings</source>
        <translation>Предупреждения</translation>
    </message>
    <message>
        <location filename="main.ui" line="519"/>
        <location filename="main.ui" line="522"/>
        <source>Show warnings</source>
        <translation>Показать предупреждения</translation>
    </message>
    <message>
        <location filename="main.ui" line="534"/>
        <source>Performance warnings</source>
        <translation>Предупреждения производительности</translation>
    </message>
    <message>
        <location filename="main.ui" line="537"/>
        <location filename="main.ui" line="540"/>
        <source>Show performance warnings</source>
        <translation>Показать предупреждения производительности</translation>
    </message>
    <message>
        <location filename="main.ui" line="548"/>
        <source>Show &amp;hidden</source>
        <translation>Показать скрытые</translation>
    </message>
    <message>
        <location filename="main.ui" line="560"/>
        <location filename="mainwindow.cpp" line="593"/>
        <location filename="mainwindow.cpp" line="631"/>
        <source>Information</source>
        <translation>Информационные сообщения</translation>
    </message>
    <message>
        <location filename="main.ui" line="563"/>
        <source>Show information messages</source>
        <translation>Показать информационные сообщения</translation>
    </message>
    <message>
        <location filename="main.ui" line="575"/>
        <source>Portability</source>
        <translation>Переносимость</translation>
    </message>
    <message>
        <location filename="main.ui" line="578"/>
        <source>Show portability warnings</source>
        <translation>Показать предупреждения переносимости</translation>
    </message>
    <message>
        <location filename="main.ui" line="586"/>
        <source>&amp;Filter</source>
        <translation>Фильтр</translation>
    </message>
    <message>
        <location filename="main.ui" line="589"/>
        <source>Filter results</source>
        <translation>Результаты фильтрации</translation>
    </message>
    <message>
        <location filename="main.ui" line="605"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSI</translation>
    </message>
    <message>
        <location filename="main.ui" line="613"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
    </message>
    <message>
        <location filename="main.ui" line="621"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="main.ui" line="629"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="main.ui" line="637"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <location filename="main.ui" line="645"/>
        <source>Platforms</source>
        <translation>Платформы</translation>
    </message>
    <message>
        <location filename="main.ui" line="659"/>
        <source>C++11</source>
        <translation>C++11</translation>
    </message>
    <message>
        <location filename="main.ui" line="670"/>
        <source>C99</source>
        <translation>C99</translation>
    </message>
    <message>
        <location filename="main.ui" line="678"/>
        <source>Posix</source>
        <translation>Posix</translation>
    </message>
    <message>
        <location filename="main.ui" line="686"/>
        <source>C11</source>
        <translation>C11</translation>
    </message>
    <message>
        <location filename="main.ui" line="694"/>
        <source>C89</source>
        <translation>C89</translation>
    </message>
    <message>
        <location filename="main.ui" line="702"/>
        <source>C++03</source>
        <translation>C++03</translation>
    </message>
    <message>
        <location filename="main.ui" line="707"/>
        <source>&amp;Print...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="710"/>
        <source>Print the Current Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="715"/>
        <source>Print Pre&amp;view...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="718"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="723"/>
        <source>Library Editor...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="726"/>
        <source>Open library editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="734"/>
        <source>Auto-detect language</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="742"/>
        <source>Enforce C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="750"/>
        <source>Enforce C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="390"/>
        <source>&amp;Check all</source>
        <translation>Отметить все</translation>
    </message>
    <message>
        <location filename="main.ui" line="218"/>
        <source>Filter</source>
        <translation>Фильтр</translation>
    </message>
    <message>
        <location filename="main.ui" line="284"/>
        <source>&amp;Recheck modified files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="395"/>
        <source>&amp;Uncheck all</source>
        <translation>Сбросить все</translation>
    </message>
    <message>
        <location filename="main.ui" line="400"/>
        <source>Collapse &amp;all</source>
        <translation>Свернуть все</translation>
    </message>
    <message>
        <location filename="main.ui" line="405"/>
        <source>&amp;Expand all</source>
        <translation>Развернуть все</translation>
    </message>
    <message>
        <location filename="main.ui" line="413"/>
        <source>&amp;Standard</source>
        <translation>Стандартные</translation>
    </message>
    <message>
        <location filename="main.ui" line="416"/>
        <source>Standard items</source>
        <translation>Стандартные элементы</translation>
    </message>
    <message>
        <location filename="main.ui" line="432"/>
        <source>Toolbar</source>
        <translation>Панель инструментов</translation>
    </message>
    <message>
        <location filename="main.ui" line="440"/>
        <source>&amp;Categories</source>
        <translation>Категории</translation>
    </message>
    <message>
        <location filename="main.ui" line="443"/>
        <source>Error categories</source>
        <translation>Категории ошибок</translation>
    </message>
    <message>
        <location filename="main.ui" line="448"/>
        <source>&amp;Open XML...</source>
        <translation>&amp;Открыть XML...</translation>
    </message>
    <message>
        <location filename="main.ui" line="457"/>
        <source>Open P&amp;roject File...</source>
        <translation>Открыть файл &amp;проекта...</translation>
    </message>
    <message>
        <location filename="main.ui" line="471"/>
        <source>&amp;New Project File...</source>
        <translation>&amp;Новый файл проекта...</translation>
    </message>
    <message>
        <location filename="main.ui" line="476"/>
        <source>&amp;Log View</source>
        <translation>Посмотреть &amp;лог</translation>
    </message>
    <message>
        <location filename="main.ui" line="479"/>
        <source>Log View</source>
        <translation>Посмотреть лог</translation>
    </message>
    <message>
        <location filename="main.ui" line="487"/>
        <source>C&amp;lose Project File</source>
        <translation>&amp;Закрыть файл проекта</translation>
    </message>
    <message>
        <location filename="main.ui" line="495"/>
        <source>&amp;Edit Project File...</source>
        <translation>&amp;Изменить файл проекта...</translation>
    </message>
    <message>
        <location filename="main.ui" line="504"/>
        <source>&amp;Statistics</source>
        <translation>&amp;Статистика</translation>
    </message>
    <message>
        <location filename="main.ui" line="421"/>
        <source>&amp;Contents</source>
        <translation>Помощь</translation>
    </message>
    <message>
        <location filename="main.ui" line="201"/>
        <source>Categories</source>
        <translation>Категории</translation>
    </message>
    <message>
        <location filename="main.ui" line="361"/>
        <source>Style warnings</source>
        <translation>Стилистические предупреждения</translation>
    </message>
    <message>
        <location filename="main.ui" line="364"/>
        <location filename="main.ui" line="367"/>
        <source>Show style warnings</source>
        <translation>Показать стилистические предупреждения</translation>
    </message>
    <message>
        <location filename="main.ui" line="424"/>
        <source>Open the help contents</source>
        <translation>Открыть помощь</translation>
    </message>
    <message>
        <location filename="main.ui" line="427"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="main.ui" line="122"/>
        <source>&amp;Help</source>
        <translation>Помощь</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="458"/>
        <source>Select directory to check</source>
        <translation>Выберите каталог для проверки</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="374"/>
        <source>No suitable files found to check!</source>
        <translation>Не найдено подходящих файлов для проверки!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="76"/>
        <source>Quick Filter:</source>
        <translation>Быстрый фильтр:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="496"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>Найден файл проекта: %1

Вы хотите загрузить этот проект?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="518"/>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation>Найдены файлы проекта из каталога.
Вы хотите продолжить проверку, не используя ни одного из этих файлов проекта?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="602"/>
        <source>File not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="605"/>
        <source>Bad XML</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="608"/>
        <source>Missing attribute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="611"/>
        <source>Bad attribute value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="614"/>
        <source>Unsupported format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="631"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="712"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1020"/>
        <source>License</source>
        <translation>Лицензия</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1027"/>
        <source>Authors</source>
        <translation>Авторы</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1040"/>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation>XML файлы версии 2 (*.xml);;XML файлы версии 1 (*.xml);;Текстовые файлы  (*.txt);;CSV файлы (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1042"/>
        <source>Save the report file</source>
        <translation>Сохранить файл с отчетом</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="887"/>
        <source>XML files (*.xml)</source>
        <translation>XML-файлы (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="300"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>Возникла проблема при загрузке настроек программы.

Возможно, это связано с изменениями в версии программы. Пожалуйста, проверьте (и исправьте) настройки приложения.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="431"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Вы должны закрыть проект перед выбором новых файлов или директорий!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="446"/>
        <source>Select files to check</source>
        <translation>Выберите файлы для проверки</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="448"/>
        <source>C/C++ Source (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="593"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="617"/>
        <source>Duplicate platform type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="620"/>
        <source>Platform type redefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="623"/>
        <source>Unknown element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="626"/>
        <source>Unknown issue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="712"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="872"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation>Текущие результаты будут удалены.

Открытые нового XML файла приведет к очистке текущих результатов. Продолжить?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="889"/>
        <source>Open the report file</source>
        <translation>Открыть файл с отчетом</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="974"/>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?</source>
        <translation>Идет проверка.

Вы хотите завершить проверку и выйти?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1049"/>
        <source>XML files version 1 (*.xml)</source>
        <translation>XML файлы версии 1 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1053"/>
        <source>XML files version 2 (*.xml)</source>
        <translation>XML файлы версии 2 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1057"/>
        <source>Text files (*.txt)</source>
        <translation>Текстовые файлы (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1061"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV файлы(*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1105"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation type="obsolete">Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1150"/>
        <location filename="mainwindow.cpp" line="1235"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Файлы проекта (*.cppcheck);;Все файлы(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1152"/>
        <source>Select Project File</source>
        <translation>Выберите файл проекта</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1180"/>
        <location filename="mainwindow.cpp" line="1249"/>
        <source>Project:</source>
        <translation>Проект:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1237"/>
        <source>Select Project Filename</source>
        <translation>Выберите имя файла для проекта</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1274"/>
        <source>No project file loaded</source>
        <translation>Файл с проектом не загружен</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1355"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>Файл с проектом

%1

не найден!
Хотите удалить его из списка проектов?</translation>
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
        <translation type="obsolete">Cppcheck Графический Интерфейс Пользователя .

Синтаксис:
     cppcheck-gui [ОПЦИИ] [файлы или пути]
	 
Опции:
	 -h, --help     Показать эту справку
	 -p &lt;file&gt;     Откройте данный файл проекта и начните проверять его
	 -l &lt;file&gt;     Откройте данные результаты xml файл
	 -d &lt;directory&gt;   Укажите каталог, который был проверен, чтобы генерировать результаты xml определенный с -l
	 -v, --version Показать версию программы</translation>
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
        <translation>Cppcheck GUI - параметры Командной строки</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <source>Built-in</source>
        <translation type="obsolete">Встроенная</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="37"/>
        <source>Native</source>
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
        <location filename="project.cpp" line="71"/>
        <location filename="project.cpp" line="123"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="project.cpp" line="72"/>
        <source>Could not read the project file.</source>
        <translation>Не удалось прочитать файл проекта.</translation>
    </message>
    <message>
        <location filename="project.cpp" line="124"/>
        <source>Could not write the project file.</source>
        <translation>Не удалось записать файл проекта.</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation>Файл проекта</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <source>Project</source>
        <translation>Проект</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="42"/>
        <source>Defines must be separated by a semicolon &apos;;&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="53"/>
        <source>Root:</source>
        <translation>Корневая директория:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="70"/>
        <source>Libraries:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="79"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="93"/>
        <location filename="projectfile.ui" line="242"/>
        <source>Paths:</source>
        <translation>Пути:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="120"/>
        <location filename="projectfile.ui" line="183"/>
        <location filename="projectfile.ui" line="256"/>
        <source>Add...</source>
        <translation>Добавить...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="127"/>
        <location filename="projectfile.ui" line="190"/>
        <location filename="projectfile.ui" line="263"/>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="134"/>
        <location filename="projectfile.ui" line="197"/>
        <location filename="projectfile.ui" line="270"/>
        <location filename="projectfile.ui" line="333"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="159"/>
        <source>Includes</source>
        <translation>Пути для заголовочных файлов</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="165"/>
        <source>Include directories:</source>
        <translation>Пути для поиска заголовочных файлов:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="217"/>
        <source>Up</source>
        <translation>Вверх</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="224"/>
        <source>Down</source>
        <translation>Вниз</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="236"/>
        <source>Exclude</source>
        <translation>Исключенные пути</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="295"/>
        <source>Suppressions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="301"/>
        <source>Suppression list:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="326"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="32"/>
        <source>Defines:</source>
        <translation>Макросы:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="42"/>
        <source>Project file: %1</source>
        <translation>Файл проекта: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="290"/>
        <source>Select include directory</source>
        <translation>Выберите директорию для поиска заголовочных файлов</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="313"/>
        <source>Select a directory to check</source>
        <translation>Выберите директорию для проверки</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="353"/>
        <source>Select directory to ignore</source>
        <translation>Выберите директорию, которую надо проигнорировать</translation>
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
        <location filename="translationhandler.cpp" line="33"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="100"/>
        <source>Unknown language specified!</source>
        <translation>Неизвестный язык!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="130"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation>Языковой файл %1 не найден!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="136"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Ошибка загрузки переводов для языка %1 из файла %2</translation>
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
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="1138"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1138"/>
        <source>Severity</source>
        <translation>Важность</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1138"/>
        <source>Line</source>
        <translation>Строка</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1138"/>
        <source>Summary</source>
        <translation>Кратко</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="124"/>
        <source>Undefined file</source>
        <translation>Неопределенный файл</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="780"/>
        <source>[Inconclusive]</source>
        <translation>[Неубедительный]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="296"/>
        <source>debug</source>
        <translation>отлаживать</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="573"/>
        <source>Recheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="574"/>
        <source>Copy filename</source>
        <translation>Скопировать имя файла</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="575"/>
        <source>Copy full path</source>
        <translation>Скопировать полный путь</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="576"/>
        <source>Copy message</source>
        <translation>Скопировать сообщение</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="577"/>
        <source>Copy message id</source>
        <translation>Скопировать номер сообщения</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="578"/>
        <source>Hide</source>
        <translation>Скрыть</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="579"/>
        <source>Hide all with id</source>
        <translation>Скрыть все с id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="580"/>
        <source>Open containing folder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="640"/>
        <location filename="resultstree.cpp" line="654"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="641"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>Никакое приложение редактора не сконфигурировано.
Сконфигурируйте приложение редактора для Cppcheck в предпочтениях/Приложениях.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="655"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>Никакое приложение редактора по умолчанию не выбрано.
Выберите приложение редактора по умолчанию в предпочтениях/Приложениях.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="684"/>
        <source>Could not find the file!</source>
        <translation>Не удается найти файл!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="730"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>Не удалось запустить %1
Пожалуйста, проверьте путь приложения, и верны ли параметры.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="744"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>Не удается найти файл:
%1		
Пожалуйста, выберите каталог, в котором находится файл.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="751"/>
        <source>Select Directory</source>
        <translation>Выберите директорию</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1138"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1138"/>
        <source>Inconclusive</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="278"/>
        <source>style</source>
        <translation>стиль</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="281"/>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="284"/>
        <source>warning</source>
        <translation>предупреждение</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="287"/>
        <source>performance</source>
        <translation>производительность</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="290"/>
        <source>portability</source>
        <translation>переносимость</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="293"/>
        <source>information</source>
        <translation>информация</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="186"/>
        <source>Print Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="205"/>
        <source>No errors found, nothing to print.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="238"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 из %2 файлов проверено)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="251"/>
        <location filename="resultsview.cpp" line="262"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="252"/>
        <source>No errors found.</source>
        <translation>Ошибок не найдено.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="259"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Были обнаружены ошибки, но они настроены быть скрыты.
Для переключения какие ошибки отображаются, откройте меню представления.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="305"/>
        <location filename="resultsview.cpp" line="323"/>
        <location filename="resultsview.cpp" line="331"/>
        <source>Failed to read the report.</source>
        <translation>Не удалось прочитать отчет.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="368"/>
        <source>Summary</source>
        <translation>Кратко</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="369"/>
        <source>Message</source>
        <translation>Сообщение</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="373"/>
        <source>First included by</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="376"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="141"/>
        <source>No errors found, nothing to save.</source>
        <translation>Ошибки не найдены, нечего сохранять.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="168"/>
        <location filename="resultsview.cpp" line="176"/>
        <source>Failed to save the report.</source>
        <translation>Не удалось сохранить отчет.</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>Результаты</translation>
    </message>
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <source>Scratchpad</source>
        <translation>Блокнот</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="48"/>
        <source>filename</source>
        <translation>имя файла</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="55"/>
        <source>Check</source>
        <translation>Проверить</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="settings.ui" line="183"/>
        <source>Include paths:</source>
        <translation>Пути для поиска заголовочных файлов:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="201"/>
        <location filename="settings.ui" line="251"/>
        <source>Add...</source>
        <translation>Добавить...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>Количество потоков исполнения:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="85"/>
        <source>Ideal count:</source>
        <translation>Рекомендуемое значение:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="114"/>
        <source>Force checking all #ifdef configurations</source>
        <oldsource>Check all #ifdef configurations</oldsource>
        <translation>Проверять все варианты #ifdef конфигураций</translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Show full path of files</source>
        <translation>Показывать полные пути к файлам</translation>
    </message>
    <message>
        <location filename="settings.ui" line="128"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Показывать сообщение, если ошибок не найдено</translation>
    </message>
    <message>
        <location filename="settings.ui" line="135"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>Отображать номер ошибки в колонке &quot;id&quot;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="142"/>
        <source>Enable inline suppressions</source>
        <translation>Включить inline-подавление ошибок</translation>
    </message>
    <message>
        <location filename="settings.ui" line="149"/>
        <source>Check for inconclusive errors also</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="169"/>
        <source>Show internal warnings in log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="177"/>
        <source>Paths</source>
        <translation>Пути</translation>
    </message>
    <message>
        <location filename="settings.ui" line="208"/>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <location filename="settings.ui" line="215"/>
        <location filename="settings.ui" line="265"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="settings.ui" line="240"/>
        <source>Applications</source>
        <translation>Приложения</translation>
    </message>
    <message>
        <location filename="settings.ui" line="258"/>
        <source>Edit...</source>
        <translation>Изменить...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="272"/>
        <source>Set as default</source>
        <translation>Установить по умолчанию</translation>
    </message>
    <message>
        <location filename="settings.ui" line="295"/>
        <source>Reports</source>
        <translation>Отчёты</translation>
    </message>
    <message>
        <location filename="settings.ui" line="301"/>
        <source>Save all errors when creating report</source>
        <translation>Сохранять все ошибки при создании отчёта</translation>
    </message>
    <message>
        <location filename="settings.ui" line="308"/>
        <source>Save full path to files in reports</source>
        <translation>Сохранять полные пути к файлам в отчётах</translation>
    </message>
    <message>
        <location filename="settings.ui" line="329"/>
        <source>Language</source>
        <translation>Язык</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="obsolete">Прочие</translation>
    </message>
    <message>
        <source>&amp;Show inconclusive errors</source>
        <translation type="obsolete">&amp;Показывать незначительные ошибки</translation>
    </message>
    <message>
        <source>S&amp;how internal warnings in log</source>
        <translation type="obsolete">&amp;Записывать внутренние предупреждения в лог</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="82"/>
        <source>N/A</source>
        <translation>Нет данных</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="203"/>
        <source>Add a new application</source>
        <translation>Добавить новое приложение</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="236"/>
        <source>Modify an application</source>
        <translation>Изменить приложение</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="241"/>
        <source> [Default]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="266"/>
        <source>[Default]</source>
        <translation>[По умолчанию]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="322"/>
        <source>Select include directory</source>
        <translation>Выберите директорию</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="228"/>
        <location filename="statsdialog.cpp" line="105"/>
        <source>Statistics</source>
        <translation>Статистика</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="97"/>
        <source>Project</source>
        <translation>Проект</translation>
    </message>
    <message>
        <location filename="stats.ui" line="33"/>
        <source>Project:</source>
        <translation>Проект:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="53"/>
        <source>Paths:</source>
        <translation>Пути:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="85"/>
        <source>Include paths:</source>
        <translation>Включенные пути:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation>Макросы:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="145"/>
        <location filename="statsdialog.cpp" line="101"/>
        <source>Previous Scan</source>
        <translation>Последнее сканирование</translation>
    </message>
    <message>
        <location filename="stats.ui" line="151"/>
        <source>Path Selected:</source>
        <translation>Выбранный путь:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="161"/>
        <source>Number of Files Scanned:</source>
        <translation>Количество просканированных файлов:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Scan Duration:</source>
        <translation>Продолжительность сканирования:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="236"/>
        <source>Errors:</source>
        <translation>Ошибки:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="254"/>
        <source>Warnings:</source>
        <translation>Предупреждения:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="272"/>
        <source>Stylistic warnings:</source>
        <translation>Стилистические предупреждения:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="290"/>
        <source>Portability warnings:</source>
        <translation>Предупреждения переносимости:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="308"/>
        <source>Performance issues:</source>
        <translation>Проблемы с производительностью:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="326"/>
        <source>Information messages:</source>
        <translation>Информационные сообщения:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="364"/>
        <source>Copy to Clipboard</source>
        <translation>Скопировать в буфер обмена</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="77"/>
        <source>1 day</source>
        <translation>1 день</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="77"/>
        <source>%1 days</source>
        <translation>%1 дней</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="79"/>
        <source>1 hour</source>
        <translation>1 час</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="79"/>
        <source>%1 hours</source>
        <translation>%1 часов</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>1 minute</source>
        <translation>1 минута</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>%1 minutes</source>
        <translation>%1 минут</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>1 second</source>
        <translation>1 секунда</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>%1 seconds</source>
        <translation>%1 секунд</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="87"/>
        <source>0.%1 seconds</source>
        <translation>0.1%1 секунд</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="89"/>
        <source> and </source>
        <translation>и</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="96"/>
        <source>Project Settings</source>
        <translation>Настройки проекта</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="98"/>
        <source>Paths</source>
        <translation>Пути</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="99"/>
        <source>Include paths</source>
        <translation>Включенные пути</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="100"/>
        <source>Defines</source>
        <translation>Макросы</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="102"/>
        <source>Path selected</source>
        <translation>Выбранные пути</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="103"/>
        <source>Number of files scanned</source>
        <translation>Количество просканированных файлов</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="104"/>
        <source>Scan duration</source>
        <translation>Продолжительность сканирования</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="106"/>
        <source>Errors</source>
        <translation>Ошибки</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="107"/>
        <source>Warnings</source>
        <translation>Предупреждения</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="108"/>
        <source>Style warnings</source>
        <translation>Стилистические предупреждения</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="109"/>
        <source>Portability warnings</source>
        <translation>Предупреждения переносимости</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="110"/>
        <source>Performance warnings</source>
        <translation>Предупреждения производительности</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>Information messages</source>
        <translation>Информационные сообщения</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="54"/>
        <source>%1 of %2 files checked</source>
        <translation>%1 из %2 файлов проверены</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="142"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>Не удалось изменить язык пользовательского интерфейса:

%1

Язык пользовательского интерфейса был сброшен на английский. Откройте Настройки-диалог для выбора любого из доступных языков.</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="148"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="73"/>
        <source>inconclusive</source>
        <translation>незначительная</translation>
    </message>
</context>
</TS>
