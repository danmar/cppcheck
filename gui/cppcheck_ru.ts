<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU" sourcelanguage="ru_RU">
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
    <message>
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-2018 Cppcheck team.</source>
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
        <translation>Добавить функцию</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <source>Function name(s)</source>
        <translation>Имя(имена) функции</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <source>Number of arguments</source>
        <translation>Количество аргументов</translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <source>Library Editor</source>
        <translation>Редактор библиотек</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <source>Open</source>
        <translation>Открыть</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <source>Save as</source>
        <translation>Сохранить как</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <source>Functions</source>
        <translation>Функции</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <source>Sort</source>
        <translation>Сортировать</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <source>Filter:</source>
        <translation>Фильтр:</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <source>Comments</source>
        <translation>Комментарии</translation>
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
        <translation>должно быть использовано возвращаемое значение</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <source>ignore function in leaks checking</source>
        <translation>пропускать функцию при проверке на утечки</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <source>Arguments</source>
        <translation>Аргументы</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="82"/>
        <location filename="librarydialog.cpp" line="154"/>
        <source>Library files (*.cfg)</source>
        <translation>Файлы библиотек (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="84"/>
        <source>Open library file</source>
        <translation>Открыть файл библиотеки</translation>
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
        <translation>Невозможно открыть файл %1.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="108"/>
        <source>Failed to load %1. %2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="145"/>
        <source>Can not save file %1.</source>
        <translation>Невозможно сохранить файл %1.</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="157"/>
        <source>Save the library as</source>
        <translation>Сохранить библиотеку как</translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <source>Edit argument</source>
        <translation>Редактировать аргумент</translation>
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
        <location filename="libraryeditargdialog.ui" line="203"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="214"/>
        <source>None</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <source>argvalue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <source>mul</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="124"/>
        <location filename="libraryeditargdialog.ui" line="229"/>
        <source>strlen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="132"/>
        <location filename="libraryeditargdialog.ui" line="237"/>
        <source>Arg</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="159"/>
        <location filename="libraryeditargdialog.ui" line="264"/>
        <source>Arg2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="194"/>
        <source>and</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="310"/>
        <source>Valid values</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LogView</name>
    <message>
        <source>Checking Log</source>
        <translation type="vanished">Лог проверки</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">Очистить</translation>
    </message>
    <message>
        <source>Save Log</source>
        <translation type="vanished">Сохранить</translation>
    </message>
    <message>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation type="vanished">Текстовые файлы (*.txt *.log);;Все файлы (*.*)</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation type="vanished">Не удалось записать в файл: &quot;%1&quot;</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="180"/>
        <source>Standard</source>
        <translation>Стандартные</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="70"/>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="89"/>
        <source>&amp;View</source>
        <translation>&amp;Вид</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="93"/>
        <source>&amp;Toolbars</source>
        <translation>&amp;Панель инструментов</translation>
    </message>
    <message>
        <source>&amp;Check</source>
        <translation type="obsolete">&amp;Проверить</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="135"/>
        <source>C++ standard</source>
        <translation>Стандарт C++</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="143"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation type="unfinished">Стандарт C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="167"/>
        <source>&amp;Edit</source>
        <translation>&amp;Правка</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="228"/>
        <source>&amp;License...</source>
        <translation>&amp;Лицензия...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="233"/>
        <source>A&amp;uthors...</source>
        <translation>&amp;Авторы...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="242"/>
        <source>&amp;About...</source>
        <translation>&amp;О программе...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="247"/>
        <source>&amp;Files...</source>
        <translation>&amp;Файлы...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="250"/>
        <location filename="mainwindow.ui" line="253"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation type="unfinished">Проверить файлы</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="256"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="265"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Каталог...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="268"/>
        <location filename="mainwindow.ui" line="271"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation type="unfinished">Проверка директории</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="274"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>&amp;Recheck files</source>
        <translation type="obsolete">&amp;Перепроверить файлы</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="286"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <source>&amp;Reanalyze all files</source>
        <oldsource>&amp;Recheck all files</oldsource>
        <translation type="obsolete">Заново проверить все файлы</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="304"/>
        <source>&amp;Stop</source>
        <translation>Остановить</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="307"/>
        <location filename="mainwindow.ui" line="310"/>
        <source>Stop analysis</source>
        <oldsource>Stop checking</oldsource>
        <translation type="unfinished">Остановить проверку</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="313"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="322"/>
        <source>&amp;Save results to file...</source>
        <translation>Сохранить отчёт в файл...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="325"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="330"/>
        <source>&amp;Quit</source>
        <translation>Выход</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="339"/>
        <source>&amp;Clear results</source>
        <translation>Очистить отчёт</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="348"/>
        <source>&amp;Preferences</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation type="vanished">Ошибки</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="381"/>
        <location filename="mainwindow.ui" line="384"/>
        <source>Show errors</source>
        <translation>Показать ошибки</translation>
    </message>
    <message>
        <source>Show S&amp;cratchpad...</source>
        <translation type="vanished">Показать блокнот</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation type="vanished">Предупреждения</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="518"/>
        <location filename="mainwindow.ui" line="521"/>
        <source>Show warnings</source>
        <translation>Показать предупреждения</translation>
    </message>
    <message>
        <source>Performance warnings</source>
        <translation type="vanished">Предупреждения производительности</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="536"/>
        <location filename="mainwindow.ui" line="539"/>
        <source>Show performance warnings</source>
        <translation>Показать предупреждения производительности</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="547"/>
        <source>Show &amp;hidden</source>
        <translation>Показать скрытые</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="761"/>
        <location filename="mainwindow.cpp" line="799"/>
        <source>Information</source>
        <translation>Информационные сообщения</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="562"/>
        <source>Show information messages</source>
        <translation>Показать информационные сообщения</translation>
    </message>
    <message>
        <source>Portability</source>
        <translation type="vanished">Переносимость</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="577"/>
        <source>Show portability warnings</source>
        <translation>Показать предупреждения переносимости</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="592"/>
        <source>Show Cppcheck results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="604"/>
        <source>Clang</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="607"/>
        <source>Show Clang results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="615"/>
        <source>&amp;Filter</source>
        <translation>Фильтр</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="618"/>
        <source>Filter results</source>
        <translation>Результаты фильтрации</translation>
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
        <translation type="vanished">Платформы</translation>
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
        <translation>Печать...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="739"/>
        <source>Print the Current Report</source>
        <translation>Напечатать текущий отчет</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="744"/>
        <source>Print Pre&amp;view...</source>
        <translation>Предварительный просмотр...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="747"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>Открыть диалог печати для текущих результатов</translation>
    </message>
    <message>
        <source>Library Editor...</source>
        <translation type="vanished">Редактор библиотек...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="755"/>
        <source>Open library editor</source>
        <translation>Открыть редактор библиотек</translation>
    </message>
    <message>
        <source>Auto-detect language</source>
        <translation type="vanished">Автоопределение языка</translation>
    </message>
    <message>
        <source>Enforce C++</source>
        <translation type="vanished">Принудительно C++</translation>
    </message>
    <message>
        <source>Enforce C</source>
        <translation type="vanished">Принудительно C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="389"/>
        <source>&amp;Check all</source>
        <translation>Отметить все</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="217"/>
        <source>Filter</source>
        <translation>Фильтр</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="283"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation type="unfinished">Заново проверить измененные файлы</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="295"/>
        <source>Reanal&amp;yze all files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="360"/>
        <source>Style war&amp;nings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="378"/>
        <source>E&amp;rrors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="394"/>
        <source>&amp;Uncheck all</source>
        <translation>Сбросить все</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="399"/>
        <source>Collapse &amp;all</source>
        <translation>Свернуть все</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="404"/>
        <source>&amp;Expand all</source>
        <translation>Развернуть все</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="412"/>
        <source>&amp;Standard</source>
        <translation>Стандартные</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="415"/>
        <source>Standard items</source>
        <translation>Стандартные элементы</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="431"/>
        <source>Toolbar</source>
        <translation>Панель инструментов</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="439"/>
        <source>&amp;Categories</source>
        <translation>Категории</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="442"/>
        <source>Error categories</source>
        <translation>Категории ошибок</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="447"/>
        <source>&amp;Open XML...</source>
        <translation>&amp;Открыть XML...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="456"/>
        <source>Open P&amp;roject File...</source>
        <translation>Открыть файл &amp;проекта...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="465"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="470"/>
        <source>&amp;New Project File...</source>
        <translation>&amp;Новый файл проекта...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="475"/>
        <source>&amp;Log View</source>
        <translation>Посмотреть &amp;лог</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="478"/>
        <source>Log View</source>
        <translation>Посмотреть лог</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="486"/>
        <source>C&amp;lose Project File</source>
        <translation>&amp;Закрыть файл проекта</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="494"/>
        <source>&amp;Edit Project File...</source>
        <translation>&amp;Изменить файл проекта...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="503"/>
        <source>&amp;Statistics</source>
        <translation>&amp;Статистика</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="515"/>
        <source>&amp;Warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="533"/>
        <source>Per&amp;formance warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="559"/>
        <source>&amp;Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="574"/>
        <source>&amp;Portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="674"/>
        <source>P&amp;latforms</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="688"/>
        <source>C++&amp;11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="699"/>
        <source>C&amp;99</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="707"/>
        <source>&amp;Posix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="715"/>
        <source>C&amp;11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="723"/>
        <source>&amp;C89</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="731"/>
        <source>&amp;C++03</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="752"/>
        <source>&amp;Library Editor...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="763"/>
        <source>&amp;Auto-detect language</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="771"/>
        <source>&amp;Enforce C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="779"/>
        <source>E&amp;nforce C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="790"/>
        <source>C++14</source>
        <translation type="unfinished">C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="420"/>
        <source>&amp;Contents</source>
        <translation>Помощь</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="198"/>
        <source>Categories</source>
        <translation>Категории</translation>
    </message>
    <message>
        <source>Style warnings</source>
        <translation type="vanished">Стилистические предупреждения</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="363"/>
        <location filename="mainwindow.ui" line="366"/>
        <source>Show style warnings</source>
        <translation>Показать стилистические предупреждения</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="423"/>
        <source>Open the help contents</source>
        <translation>Открыть помощь</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="426"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="121"/>
        <source>&amp;Help</source>
        <translation>Помощь</translation>
    </message>
    <message>
        <source>Select directory to check</source>
        <translation type="obsolete">Выберите каталог для проверки</translation>
    </message>
    <message>
        <source>No suitable files found to check!</source>
        <translation type="obsolete">Не найдено подходящих файлов для проверки!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="85"/>
        <source>Quick Filter:</source>
        <translation>Быстрый фильтр:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="572"/>
        <source>C/C++ Source, Compile database, Visual Studio (%1 %2 *.sln *.vcxproj)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="621"/>
        <source>Select configuration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="651"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>Найден файл проекта: %1

Вы хотите загрузить этот проект?</translation>
    </message>
    <message>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation type="obsolete">Найдены файлы проекта из каталога.
Вы хотите продолжить проверку, не используя ни одного из этих файлов проекта?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="770"/>
        <source>File not found</source>
        <translation>Файл не найден</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="773"/>
        <source>Bad XML</source>
        <translation>Некорректный XML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="776"/>
        <source>Missing attribute</source>
        <translation>Пропущен атрибут</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="779"/>
        <source>Bad attribute value</source>
        <translation>Некорректное значение атрибута</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="782"/>
        <source>Unsupported format</source>
        <translation>Неподдерживаемый формат</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="799"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>Не удалось загрузить выбранную библиотеку &apos;%1&apos;.
%2</translation>
    </message>
    <message>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located.</source>
        <translation type="obsolete">Не удалось загрузить %1. Установленный Cppcheck поврежден. Вы можете использовать ключ --data-dir=&lt;directory&gt; в командной строке, чтобы указать, где расположен этот файл.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1233"/>
        <source>License</source>
        <translation>Лицензия</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1240"/>
        <source>Authors</source>
        <translation>Авторы</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation type="obsolete">XML файлы версии 2 (*.xml);;XML файлы версии 1 (*.xml);;Текстовые файлы  (*.txt);;CSV файлы (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1255"/>
        <source>Save the report file</source>
        <translation>Сохранить файл с отчетом</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1092"/>
        <location filename="mainwindow.cpp" line="1262"/>
        <source>XML files (*.xml)</source>
        <translation>XML-файлы (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="322"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>Возникла проблема при загрузке настроек программы.

Возможно, это связано с изменениями в версии программы. Пожалуйста, проверьте (и исправьте) настройки приложения.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="555"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Вы должны закрыть проект перед выбором новых файлов или директорий!</translation>
    </message>
    <message>
        <source>Select files to check</source>
        <translation type="obsolete">Выберите файлы для проверки</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="761"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>Библиотека &apos;%1&apos; содержит неизвестные элементы:
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="785"/>
        <source>Duplicate platform type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="788"/>
        <source>Platform type redefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Unknown element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="794"/>
        <source>Unknown issue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="895"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="895"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1077"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation>Текущие результаты будут очищены.

Открытые нового XML файла приведет к очистке текущих результатов. Продолжить?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1094"/>
        <source>Open the report file</source>
        <translation>Открыть файл с отчетом</translation>
    </message>
    <message>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?</source>
        <translation type="obsolete">Идет проверка.

Вы хотите завершить проверку и выйти?</translation>
    </message>
    <message>
        <source>XML files version 1 (*.xml)</source>
        <translation type="obsolete">XML файлы версии 1 (*.xml)</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml)</source>
        <translation type="obsolete">XML файлы версии 2 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1266"/>
        <source>Text files (*.txt)</source>
        <translation>Текстовые файлы (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1270"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV файлы(*.csv)</translation>
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
        <translation type="obsolete">Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1361"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Файлы проекта (*.cppcheck);;Все файлы(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1363"/>
        <source>Select Project File</source>
        <translation>Выберите файл проекта</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="154"/>
        <location filename="mainwindow.cpp" line="1391"/>
        <location filename="mainwindow.cpp" line="1515"/>
        <source>Project:</source>
        <translation>Проект:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="491"/>
        <source>No suitable files found to analyze!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="570"/>
        <source>Select files to analyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="584"/>
        <source>Select directory to analyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="621"/>
        <source>Select the configuration that will be analyzed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="673"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1187"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1253"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1450"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1467"/>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1500"/>
        <source>Project files (*.cppcheck)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1502"/>
        <source>Select Project Filename</source>
        <translation>Выберите имя файла для проекта</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1547"/>
        <source>No project file loaded</source>
        <translation>Файл с проектом не загружен</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1607"/>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.cpp" line="120"/>
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
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <source>Could not read the project file.</source>
        <translation type="vanished">Не удалось прочитать файл проекта.</translation>
    </message>
    <message>
        <source>Could not write the project file.</source>
        <translation type="vanished">Не удалось записать файл проекта.</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfiledialog.ui" line="14"/>
        <source>Project File</source>
        <translation>Файл проекта</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="273"/>
        <source>Project</source>
        <translation>Проект</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="30"/>
        <source>Import Project (Visual studio / compile database)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="163"/>
        <source>Defines must be separated by a semicolon &apos;;&apos;</source>
        <translation>Defines должны быть разделены точкой с запятой &apos;;&apos;</translation>
    </message>
    <message>
        <source>&amp;Root:</source>
        <oldsource>Root:</oldsource>
        <translation type="obsolete">Корневая директория:</translation>
    </message>
    <message>
        <source>Libraries:</source>
        <translation type="vanished">Библиотеки:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="331"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>Note: Положите свои .cfg-файлы в одну директорию с файлом проекта. Вы увидите их сверху.</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="60"/>
        <location filename="projectfiledialog.ui" line="312"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="69"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="72"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="86"/>
        <location filename="projectfiledialog.ui" line="364"/>
        <source>Paths:</source>
        <translation>Пути:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="113"/>
        <location filename="projectfiledialog.ui" line="207"/>
        <location filename="projectfiledialog.ui" line="378"/>
        <source>Add...</source>
        <translation>Добавить...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="120"/>
        <location filename="projectfiledialog.ui" line="214"/>
        <location filename="projectfiledialog.ui" line="385"/>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="127"/>
        <location filename="projectfiledialog.ui" line="221"/>
        <location filename="projectfiledialog.ui" line="392"/>
        <location filename="projectfiledialog.ui" line="455"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="176"/>
        <source>Include Paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Includes</source>
        <translation type="obsolete">Пути для заголовочных файлов</translation>
    </message>
    <message>
        <source>Include directories:</source>
        <translation type="obsolete">Пути для поиска заголовочных файлов:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="241"/>
        <source>Up</source>
        <translation>Вверх</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="248"/>
        <source>Down</source>
        <translation>Вниз</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="279"/>
        <source>Root path:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="291"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="303"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="322"/>
        <source>Libraries</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="358"/>
        <source>Exclude</source>
        <translation>Исключенные пути</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="417"/>
        <source>Suppressions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="423"/>
        <source>Suppression list:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="448"/>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="465"/>
        <source>Addons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="471"/>
        <source>Y2038</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="478"/>
        <source>Thread safety</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="485"/>
        <source>Coding standards</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="492"/>
        <source>Cert</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="513"/>
        <source>Extra Tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="519"/>
        <source>It is common best practice to use several tools.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="526"/>
        <source>Clang analyzer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="533"/>
        <source>Clang-tidy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="153"/>
        <source>Defines:</source>
        <translation>Макросы:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="47"/>
        <source>Project file: %1</source>
        <translation>Файл проекта: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="247"/>
        <source>Select Cppcheck build dir</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="283"/>
        <source>Visual Studio (*.sln *.vcxproj);;Compile database (compile_commands.json)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="497"/>
        <source>Select include directory</source>
        <translation>Выберите директорию для поиска заголовочных файлов</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="477"/>
        <source>Select a directory to check</source>
        <translation>Выберите директорию для проверки</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="172"/>
        <source>Clang-tidy (not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="281"/>
        <source>Import Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="517"/>
        <source>Select directory to ignore</source>
        <translation>Выберите директорию, которую надо проигнорировать</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="571"/>
        <source>Add Suppression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="572"/>
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
        <location filename="translationhandler.cpp" line="103"/>
        <source>Unknown language specified!</source>
        <translation>Неизвестный язык!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="132"/>
        <source>Language file %1 not found!</source>
        <oldsource>Language file %1.qm not found!</oldsource>
        <translation>Языковой файл %1 не найден!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="138"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <oldsource>Failed to load translation for language %1 from file %2.qm</oldsource>
        <translation>Ошибка загрузки переводов для языка %1 из файла %2</translation>
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
        <location filename="resultstree.cpp" line="1341"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Severity</source>
        <translation>Важность</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Line</source>
        <translation>Строка</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Summary</source>
        <translation>Кратко</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="137"/>
        <source>Undefined file</source>
        <translation>Неопределенный файл</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="841"/>
        <source>[Inconclusive]</source>
        <translation>[Неубедительный]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="308"/>
        <source>debug</source>
        <translation>отлаживать</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="245"/>
        <source>note</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="608"/>
        <source>Recheck</source>
        <translation>Проверить заново</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="609"/>
        <source>Copy filename</source>
        <translation>Скопировать имя файла</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="610"/>
        <source>Copy full path</source>
        <translation>Скопировать полный путь</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="611"/>
        <source>Copy message</source>
        <translation>Скопировать сообщение</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="612"/>
        <source>Copy message id</source>
        <translation>Скопировать номер сообщения</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="613"/>
        <source>Hide</source>
        <translation>Скрыть</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="614"/>
        <source>Hide all with id</source>
        <translation>Скрыть все с id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="615"/>
        <source>Suppress selected id(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="616"/>
        <source>Open containing folder</source>
        <translation>Открыть содержащую папку</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="656"/>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Tag</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="658"/>
        <source>No tag</source>
        <translation type="unfinished"></translation>
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
        <translation>Никакое приложение редактора не сконфигурировано.
Сконфигурируйте приложение редактора для Cppcheck в предпочтениях/Приложениях.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="716"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>Никакое приложение редактора по умолчанию не выбрано.
Выберите приложение редактора по умолчанию в предпочтениях/Приложениях.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="745"/>
        <source>Could not find the file!</source>
        <translation>Не удается найти файл!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="791"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>Не удалось запустить %1
Пожалуйста, проверьте путь приложения, и верны ли параметры.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="805"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>Не удается найти файл:
%1		
Пожалуйста, выберите каталог, в котором находится файл.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="812"/>
        <source>Select Directory</source>
        <translation>Выберите директорию</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Inconclusive</source>
        <translation>Спорное</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Since date</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="290"/>
        <source>style</source>
        <translation>стиль</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="293"/>
        <source>error</source>
        <translation>ошибка</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="296"/>
        <source>warning</source>
        <translation>предупреждение</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="299"/>
        <source>performance</source>
        <translation>производительность</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="302"/>
        <source>portability</source>
        <translation>переносимость</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="305"/>
        <source>information</source>
        <translation>информация</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.cpp" line="198"/>
        <source>Print Report</source>
        <translation>Распечатать отчет</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="217"/>
        <source>No errors found, nothing to print.</source>
        <translation>Ошибок не найдено, нечего распечатывать.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="255"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 из %2 файлов проверено)</translation>
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
        <translation>Ошибок не найдено.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="276"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Были обнаружены ошибки, но они настроены быть скрыты.
Для переключения какие ошибки отображаются, откройте меню представления.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="322"/>
        <location filename="resultsview.cpp" line="343"/>
        <location filename="resultsview.cpp" line="351"/>
        <source>Failed to read the report.</source>
        <translation>Не удалось прочитать отчет.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="329"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="388"/>
        <source>Summary</source>
        <translation>Кратко</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="389"/>
        <source>Message</source>
        <translation>Сообщение</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="393"/>
        <source>First included by</source>
        <translation type="unfinished"></translation>
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
        <translation>Ошибки не найдены, нечего сохранять.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="180"/>
        <location filename="resultsview.cpp" line="188"/>
        <source>Failed to save the report.</source>
        <translation>Не удалось сохранить отчет.</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>Результаты</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="82"/>
        <source>Analysis Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="104"/>
        <source>Warning Details</source>
        <translation type="unfinished"></translation>
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
        <source>Include paths:</source>
        <translation type="vanished">Пути для поиска заголовочных файлов:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="195"/>
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
        <translation>Показывать также спорные ошибки</translation>
    </message>
    <message>
        <location filename="settings.ui" line="156"/>
        <source>Show statistics on check completion</source>
        <translation>Показывать статистику после завершения проверки</translation>
    </message>
    <message>
        <location filename="settings.ui" line="176"/>
        <source>Show internal warnings in log</source>
        <translation>Показывать внутренние предупреждения в логе</translation>
    </message>
    <message>
        <location filename="settings.ui" line="287"/>
        <source>Addons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="293"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="302"/>
        <location filename="settings.ui" line="345"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="326"/>
        <source>Clang</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="332"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="355"/>
        <source>Visual Studio headers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="361"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Paths</source>
        <translation type="vanished">Пути</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="vanished">Изменить</translation>
    </message>
    <message>
        <location filename="settings.ui" line="209"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="settings.ui" line="184"/>
        <source>Applications</source>
        <translation>Приложения</translation>
    </message>
    <message>
        <location filename="settings.ui" line="202"/>
        <source>Edit...</source>
        <translation>Изменить...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="216"/>
        <source>Set as default</source>
        <translation>Установить по умолчанию</translation>
    </message>
    <message>
        <location filename="settings.ui" line="239"/>
        <source>Reports</source>
        <translation>Отчёты</translation>
    </message>
    <message>
        <location filename="settings.ui" line="245"/>
        <source>Save all errors when creating report</source>
        <translation>Сохранять все ошибки при создании отчёта</translation>
    </message>
    <message>
        <location filename="settings.ui" line="252"/>
        <source>Save full path to files in reports</source>
        <translation>Сохранять полные пути к файлам в отчётах</translation>
    </message>
    <message>
        <location filename="settings.ui" line="273"/>
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
        <location filename="settingsdialog.cpp" line="88"/>
        <source>N/A</source>
        <translation>Нет данных</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="186"/>
        <source>Add a new application</source>
        <translation>Добавить новое приложение</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="219"/>
        <source>Modify an application</source>
        <translation>Изменить приложение</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="224"/>
        <source> [Default]</source>
        <translation> [По умолчанию]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="249"/>
        <source>[Default]</source>
        <translation>[По умолчанию]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="304"/>
        <source>Select python binary</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="312"/>
        <source>Select clang path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select include directory</source>
        <translation type="vanished">Выберите директорию</translation>
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
        <translation>Статистика</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="173"/>
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
        <location filename="statsdialog.cpp" line="177"/>
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
        <location filename="stats.ui" line="343"/>
        <source>History</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="349"/>
        <source>File:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="387"/>
        <source>Copy to Clipboard</source>
        <translation>Скопировать в буфер обмена</translation>
    </message>
    <message>
        <location filename="stats.ui" line="394"/>
        <source>Pdf Export</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>1 day</source>
        <translation>1 день</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>%1 days</source>
        <translation>%1 дней</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>1 hour</source>
        <translation>1 час</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>%1 hours</source>
        <translation>%1 часов</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>1 minute</source>
        <translation>1 минута</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>%1 minutes</source>
        <translation>%1 минут</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>1 second</source>
        <translation>1 секунда</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>%1 seconds</source>
        <translation>%1 секунд</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="121"/>
        <source>0.%1 seconds</source>
        <translation>0.1%1 секунд</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="123"/>
        <source> and </source>
        <translation>и</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="150"/>
        <source>Export PDF</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="172"/>
        <source>Project Settings</source>
        <translation>Настройки проекта</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="174"/>
        <source>Paths</source>
        <translation>Пути</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="175"/>
        <source>Include paths</source>
        <translation>Включенные пути</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="176"/>
        <source>Defines</source>
        <translation>Макросы</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="178"/>
        <source>Path selected</source>
        <translation>Выбранные пути</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="179"/>
        <source>Number of files scanned</source>
        <translation>Количество просканированных файлов</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="180"/>
        <source>Scan duration</source>
        <translation>Продолжительность сканирования</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="137"/>
        <location filename="statsdialog.cpp" line="182"/>
        <source>Errors</source>
        <translation>Ошибки</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="64"/>
        <source>File: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="64"/>
        <source>No cppcheck build dir</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="139"/>
        <location filename="statsdialog.cpp" line="183"/>
        <source>Warnings</source>
        <translation>Предупреждения</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="141"/>
        <location filename="statsdialog.cpp" line="184"/>
        <source>Style warnings</source>
        <translation>Стилистические предупреждения</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="143"/>
        <location filename="statsdialog.cpp" line="185"/>
        <source>Portability warnings</source>
        <translation>Предупреждения переносимости</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="145"/>
        <location filename="statsdialog.cpp" line="186"/>
        <source>Performance warnings</source>
        <translation>Предупреждения производительности</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="147"/>
        <location filename="statsdialog.cpp" line="187"/>
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
        <location filename="translationhandler.cpp" line="144"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>Не удалось изменить язык пользовательского интерфейса:

%1

Язык пользовательского интерфейса был сброшен на английский. Откройте Настройки-диалог для выбора любого из доступных языков.</translation>
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
        <translation>незначительная</translation>
    </message>
</context>
</TS>
