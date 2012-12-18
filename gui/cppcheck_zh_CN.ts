<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="zh_CN">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>关于 cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>版本 %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - 一个 C/C++ 静态代码分析工具.</translation>
    </message>
    <message utf8="true">
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-2012 Daniel Marjamäki and cppcheck team.</source>
        <oldsource>Copyright © 2007-2011 Daniel Marjamäki and cppcheck team.</oldsource>
        <translation>版权所有 © 2007-2012 Daniel Marjamäki 与 cppcheck 团队.</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>访问 Cppcheck 主页于 %1</translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="application.ui" line="23"/>
        <source>Add an application</source>
        <translation>添加应用程序</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="application.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>名称(&amp;N)：</translation>
    </message>
    <message>
        <location filename="application.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>可执行文件(&amp;E):</translation>
    </message>
    <message>
        <location filename="application.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>参数(&amp;P):</translation>
    </message>
    <message>
        <location filename="application.ui" line="138"/>
        <source>Browse</source>
        <translation>浏览</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="58"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>可执行文件 (*.exe);;所有文件 (*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="61"/>
        <source>Select viewer application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="75"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="76"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>你必须为此应用程序指定名称、路径以及可选参数！</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="42"/>
        <source>Could not find the file: %1</source>
        <translation>无法找到文件: %1</translation>
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
        <translation>无法读取文件: %1</translation>
    </message>
</context>
<context>
    <name>LogView</name>
    <message>
        <location filename="logview.ui" line="17"/>
        <source>Checking Log</source>
        <translation>正在检查记录</translation>
    </message>
    <message>
        <location filename="logview.ui" line="48"/>
        <source>&amp;Save</source>
        <translation>保存(&amp;S)</translation>
    </message>
    <message>
        <location filename="logview.ui" line="55"/>
        <source>Clear</source>
        <translation>清空</translation>
    </message>
    <message>
        <location filename="logview.ui" line="62"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="66"/>
        <source>Save Log</source>
        <translation>保存记录</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="67"/>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation>文本文件 (*.txt *.log);;所有文件 (*.*)</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="71"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="logview.cpp" line="72"/>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation>无法打开并写入文件: “%1”</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="main.ui" line="26"/>
        <location filename="mainwindow.cpp" line="249"/>
        <location filename="mainwindow.cpp" line="307"/>
        <location filename="mainwindow.cpp" line="363"/>
        <location filename="mainwindow.cpp" line="430"/>
        <location filename="mainwindow.cpp" line="452"/>
        <location filename="mainwindow.cpp" line="644"/>
        <location filename="mainwindow.cpp" line="735"/>
        <location filename="mainwindow.cpp" line="854"/>
        <location filename="mainwindow.cpp" line="874"/>
        <location filename="mainwindow.cpp" line="1031"/>
        <location filename="mainwindow.cpp" line="1112"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="70"/>
        <source>&amp;File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="85"/>
        <source>&amp;View</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="89"/>
        <source>&amp;Toolbars</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="117"/>
        <source>&amp;Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="127"/>
        <source>&amp;Check</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="142"/>
        <source>&amp;Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="155"/>
        <source>Standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="175"/>
        <source>Categories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="203"/>
        <source>&amp;License...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="208"/>
        <source>A&amp;uthors...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="217"/>
        <source>&amp;About...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="222"/>
        <source>&amp;Files...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="225"/>
        <location filename="main.ui" line="228"/>
        <source>Check files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="231"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="240"/>
        <source>&amp;Directory...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="243"/>
        <location filename="main.ui" line="246"/>
        <source>Check directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="249"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="258"/>
        <source>&amp;Recheck files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="261"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="270"/>
        <source>&amp;Stop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="273"/>
        <location filename="main.ui" line="276"/>
        <source>Stop checking</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="279"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="288"/>
        <source>&amp;Save results to file...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="291"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="296"/>
        <source>&amp;Quit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="305"/>
        <source>&amp;Clear results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="314"/>
        <source>&amp;Preferences</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="326"/>
        <source>Style warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="329"/>
        <location filename="main.ui" line="332"/>
        <source>Show style warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="344"/>
        <source>Errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="347"/>
        <location filename="main.ui" line="350"/>
        <source>Show errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="432"/>
        <source>Show S&amp;cratchpad...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="526"/>
        <source>Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="529"/>
        <source>Show information messages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="541"/>
        <source>Portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="544"/>
        <source>Show portability warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="552"/>
        <source>&amp;Filter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="555"/>
        <source>Filter results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="571"/>
        <source>Windows 32-bit ANSI</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="579"/>
        <source>Windows 32-bit Unicode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="587"/>
        <source>Unix 32-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="595"/>
        <source>Unix 64-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="603"/>
        <source>Windows 64-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="611"/>
        <source>Platforms</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main.ui" line="622"/>
        <source>C++11</source>
        <translation type="unfinished">C++11</translation>
    </message>
    <message>
        <location filename="main.ui" line="630"/>
        <source>C99</source>
        <translation type="unfinished">C99</translation>
    </message>
    <message>
        <location filename="main.ui" line="638"/>
        <source>Posix</source>
        <translation type="unfinished">Posix</translation>
    </message>
    <message>
        <location filename="main.ui" line="453"/>
        <source>C&amp;lose Project File</source>
        <translation type="unfinished">プロジェクトを閉じる(&amp;l)</translation>
    </message>
    <message>
        <location filename="main.ui" line="461"/>
        <source>&amp;Edit Project File...</source>
        <translation type="unfinished">プロジェクトの編集(&amp;E)...</translation>
    </message>
    <message>
        <location filename="main.ui" line="470"/>
        <source>&amp;Statistics</source>
        <translation type="unfinished">統計情報(&amp;S)</translation>
    </message>
    <message>
        <location filename="main.ui" line="482"/>
        <source>Warnings</source>
        <translation type="unfinished">警告</translation>
    </message>
    <message>
        <location filename="main.ui" line="485"/>
        <location filename="main.ui" line="488"/>
        <source>Show warnings</source>
        <translation type="unfinished">警告を表示</translation>
    </message>
    <message>
        <location filename="main.ui" line="500"/>
        <source>Performance warnings</source>
        <translation type="unfinished">パフォーマンス警告</translation>
    </message>
    <message>
        <location filename="main.ui" line="503"/>
        <location filename="main.ui" line="506"/>
        <source>Show performance warnings</source>
        <translation type="unfinished">パフォーマンス警告を表示</translation>
    </message>
    <message>
        <location filename="main.ui" line="514"/>
        <source>Show &amp;hidden</source>
        <translation type="unfinished">非表示を表示(&amp;h)</translation>
    </message>
    <message>
        <location filename="main.ui" line="355"/>
        <source>&amp;Check all</source>
        <translation type="unfinished">すべてのエラーを表示(&amp;C)</translation>
    </message>
    <message>
        <location filename="main.ui" line="192"/>
        <source>Filter</source>
        <translation type="unfinished">フィルター</translation>
    </message>
    <message>
        <location filename="main.ui" line="360"/>
        <source>&amp;Uncheck all</source>
        <translation type="unfinished">すべてのエラーを非表示(&amp;U)</translation>
    </message>
    <message>
        <location filename="main.ui" line="365"/>
        <source>Collapse &amp;all</source>
        <translation type="unfinished">ツリーを折り畳む(&amp;A)</translation>
    </message>
    <message>
        <location filename="main.ui" line="370"/>
        <source>&amp;Expand all</source>
        <translation type="unfinished">ツリーを展開(&amp;E)</translation>
    </message>
    <message>
        <location filename="main.ui" line="378"/>
        <source>&amp;Standard</source>
        <translation type="unfinished">標準(&amp;S)</translation>
    </message>
    <message>
        <location filename="main.ui" line="381"/>
        <source>Standard items</source>
        <translation type="unfinished">標準項目</translation>
    </message>
    <message>
        <location filename="main.ui" line="386"/>
        <source>&amp;Contents</source>
        <translation type="unfinished">コンテンツ(&amp;C)</translation>
    </message>
    <message>
        <location filename="main.ui" line="389"/>
        <source>Open the help contents</source>
        <translation type="unfinished">ヘルプファイルを開く</translation>
    </message>
    <message>
        <location filename="main.ui" line="392"/>
        <source>F1</source>
        <translation type="unfinished">F1</translation>
    </message>
    <message>
        <location filename="main.ui" line="397"/>
        <source>Toolbar</source>
        <translation type="unfinished">ツールバー</translation>
    </message>
    <message>
        <location filename="main.ui" line="405"/>
        <source>&amp;Categories</source>
        <translation type="unfinished">カテゴリ(&amp;C)</translation>
    </message>
    <message>
        <location filename="main.ui" line="408"/>
        <source>Error categories</source>
        <translation type="unfinished">エラーカテゴリ</translation>
    </message>
    <message>
        <location filename="main.ui" line="413"/>
        <source>&amp;Open XML...</source>
        <translation type="unfinished">XMLを開く(&amp;O)...</translation>
    </message>
    <message>
        <location filename="main.ui" line="422"/>
        <source>Open P&amp;roject File...</source>
        <translation type="unfinished">プロジェクトを開く(&amp;R)...</translation>
    </message>
    <message>
        <location filename="main.ui" line="437"/>
        <source>&amp;New Project File...</source>
        <translation type="unfinished">新規プロジェクト(&amp;N)...</translation>
    </message>
    <message>
        <location filename="main.ui" line="442"/>
        <source>&amp;Log View</source>
        <translation type="unfinished">ログを表示(&amp;L)</translation>
    </message>
    <message>
        <location filename="main.ui" line="445"/>
        <source>Log View</source>
        <translation type="unfinished">ログ表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="244"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="308"/>
        <source>No suitable files found to check!</source>
        <translation type="unfinished">解析可能なファイルではありません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="364"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation type="unfinished">新しいファイル／ディレクトリを解析するには現在のプロジェクトを閉じてください</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="395"/>
        <source>Select directory to check</source>
        <translation type="unfinished">チェック対象のディレクトリを選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="73"/>
        <source>Quick Filter:</source>
        <translation type="unfinished">クイックフィルタ：</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="431"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="453"/>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="645"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="660"/>
        <source>XML files (*.xml)</source>
        <translation type="unfinished">XML ファイル (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="662"/>
        <source>Open the report file</source>
        <translation type="unfinished">レポートを開く</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="731"/>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?.</source>
        <translation type="unfinished">解析中です.

解析を停止してCppcheckを終了しますか？.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="777"/>
        <source>License</source>
        <translation type="unfinished">ライセンス</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="784"/>
        <source>Authors</source>
        <translation type="unfinished">作者</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="792"/>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation type="unfinished">XML ファイル (*.xml);;テキストファイル (*.txt);;CSV形式ファイル (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="794"/>
        <source>Save the report file</source>
        <translation type="unfinished">レポートを保存</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="801"/>
        <source>XML files version 1 (*.xml)</source>
        <translation type="unfinished">XMLファイルのバージョン1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="805"/>
        <source>XML files version 2 (*.xml)</source>
        <translation type="unfinished">XMLファイルのバージョン2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="809"/>
        <source>Text files (*.txt)</source>
        <translation type="unfinished">テキストファイル (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="813"/>
        <source>CSV files (*.csv)</source>
        <translation type="unfinished">CSV形式ファイル (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="856"/>
        <source>Cppcheck - %1</source>
        <translation type="unfinished">Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="868"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="912"/>
        <location filename="mainwindow.cpp" line="995"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation type="unfinished">プロジェクトファイル (*.cppcheck);;すべてのファイル(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="914"/>
        <source>Select Project File</source>
        <translation type="unfinished">プロジェクトファイルを選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="942"/>
        <location filename="mainwindow.cpp" line="1007"/>
        <source>Project:</source>
        <translation type="unfinished">プロジェクト:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="997"/>
        <source>Select Project Filename</source>
        <translation type="unfinished">プロジェクトファイル名を選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1032"/>
        <source>No project file loaded</source>
        <translation type="unfinished">プロジェクトファイルが読み込まれていません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1107"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="33"/>
        <source>English</source>
        <translation type="unfinished">英語</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="32"/>
        <source>Dutch</source>
        <translation type="unfinished">オランダ語</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Finnish</source>
        <translation type="unfinished">フィンランド語</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>French</source>
        <translation type="unfinished">フランス語</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Italian</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>Korean</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="42"/>
        <source>Spanish</source>
        <translation type="unfinished">スペイン語</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="43"/>
        <source>Swedish</source>
        <translation type="unfinished">スウェーデン語</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>German</source>
        <translation type="unfinished">ドイツ語</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Russian</source>
        <translation type="unfinished">ロシア語</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Japanese</source>
        <oldsource>Japanease</oldsource>
        <translation type="unfinished">日本語</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>Serbian</source>
        <translation type="unfinished">セルビア語</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <location filename="platforms.cpp" line="37"/>
        <source>Built-in</source>
        <translation>内置</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Unix 32-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="39"/>
        <source>Unix 64-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="40"/>
        <source>Windows 32-bit ANSI</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="41"/>
        <source>Windows 32-bit Unicode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="42"/>
        <source>Windows 64-bit</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Project</name>
    <message>
        <location filename="project.cpp" line="71"/>
        <location filename="project.cpp" line="115"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="project.cpp" line="72"/>
        <source>Could not read the project file.</source>
        <translation>无法读取项目文件。</translation>
    </message>
    <message>
        <location filename="project.cpp" line="116"/>
        <source>Could not write the project file.</source>
        <translation>无法写入项目文件。</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation>项目文件</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <source>Project</source>
        <translation>项目</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="49"/>
        <source>Root:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="68"/>
        <location filename="projectfile.ui" line="217"/>
        <source>Paths:</source>
        <translation>路径:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="95"/>
        <location filename="projectfile.ui" line="158"/>
        <location filename="projectfile.ui" line="231"/>
        <source>Add...</source>
        <translation>添加...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="102"/>
        <location filename="projectfile.ui" line="165"/>
        <location filename="projectfile.ui" line="238"/>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="109"/>
        <location filename="projectfile.ui" line="172"/>
        <location filename="projectfile.ui" line="245"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="134"/>
        <source>Includes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="140"/>
        <source>Include directories:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="192"/>
        <source>Up</source>
        <translation>向上</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="199"/>
        <source>Down</source>
        <translation>向下</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="211"/>
        <source>Exclude</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="32"/>
        <source>Defines:</source>
        <translation>定义:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="38"/>
        <source>Project file: %1</source>
        <translation>项目文件: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="209"/>
        <source>Select include directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="232"/>
        <source>Select a directory to check</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="272"/>
        <source>Select directory to ignore</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="77"/>
        <source>Unknown language specified!</source>
        <translation>指定了未知语言！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="89"/>
        <source>Language file %1 not found!</source>
        <translation>语言文件 %1 不存在！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="95"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>无法从文件 %2 中为语言 %1 加载翻译文件</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="1000"/>
        <source>File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1000"/>
        <source>Severity</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1000"/>
        <source>Line</source>
        <translation type="unfinished">行</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1000"/>
        <source>Summary</source>
        <translation type="unfinished">内容</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="102"/>
        <source>Undefined file</source>
        <translation type="unfinished">未定義ファイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="205"/>
        <location filename="resultstree.cpp" line="724"/>
        <source>[Inconclusive]</source>
        <translation type="unfinished">[結論の出ない]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="273"/>
        <source>debug</source>
        <translation type="unfinished">デバッグ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="535"/>
        <source>Copy filename</source>
        <translation type="unfinished">ファイル名をコピー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="536"/>
        <source>Copy full path</source>
        <translation type="unfinished">フルパスをコピー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="537"/>
        <source>Copy message</source>
        <translation type="unfinished">メッセージをコピー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="538"/>
        <source>Copy message id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="539"/>
        <source>Hide</source>
        <translation type="unfinished">非表示</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="585"/>
        <location filename="resultstree.cpp" line="599"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="586"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation type="unfinished">メニューの「編集」→「設定」からテキストファイルを表示するアプリケーションを設定してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="600"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="629"/>
        <source>Could not find the file!</source>
        <translation type="unfinished">ファイルが見つかりません</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="675"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation type="unfinished">%1 が実行できません。

実行ファイルパスや引数の設定を確認してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="689"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation type="unfinished">ファイルが見つかりません:
%1
ディレクトリにファイルが存在するか確認してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="696"/>
        <source>Select Directory</source>
        <translation type="unfinished">ディレクトリを選択</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1000"/>
        <source>Id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="255"/>
        <source>style</source>
        <translation type="unfinished">スタイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="258"/>
        <source>error</source>
        <translation type="unfinished">エラー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="261"/>
        <source>warning</source>
        <translation type="unfinished">警告</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="264"/>
        <source>performance</source>
        <translation type="unfinished">パフォーマンス</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="267"/>
        <source>portability</source>
        <translation type="unfinished">移植可能性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="270"/>
        <source>information</source>
        <translation type="unfinished">情報</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation type="unfinished">結果</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="139"/>
        <source>No errors found, nothing to save.</source>
        <translation type="unfinished">警告/エラーが見つからなかったため、保存しません。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="166"/>
        <location filename="resultsview.cpp" line="174"/>
        <source>Failed to save the report.</source>
        <translation type="unfinished">レポートの保存に失敗しました。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="200"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="213"/>
        <location filename="resultsview.cpp" line="224"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="214"/>
        <source>No errors found.</source>
        <translation type="unfinished">警告/エラーは見つかりませんでした。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="221"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation type="unfinished">警告/エラーが見つかりましたが、非表示設定になっています。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="267"/>
        <location filename="resultsview.cpp" line="285"/>
        <location filename="resultsview.cpp" line="293"/>
        <source>Failed to read the report.</source>
        <translation type="unfinished">レポートの読み込みに失敗.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="330"/>
        <source>Summary</source>
        <translation type="unfinished">内容</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="331"/>
        <source>Message</source>
        <translation type="unfinished">メッセージ</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="333"/>
        <source>Id</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <source>Scratchpad</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="48"/>
        <source>filename</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="55"/>
        <source>Check</source>
        <translation type="unfinished">チェック</translation>
    </message>
</context>
<context>
    <name>SelectFilesDialog</name>
    <message>
        <location filename="selectfilesdialog.ui" line="14"/>
        <source>Select files to check</source>
        <translation type="unfinished">选择要检查的文件</translation>
    </message>
    <message>
        <location filename="selectfilesdialog.cpp" line="230"/>
        <source>Check</source>
        <translation type="unfinished">检查</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation type="unfinished">設定</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation type="unfinished">全般</translation>
    </message>
    <message>
        <location filename="settings.ui" line="169"/>
        <source>Include paths:</source>
        <translation type="unfinished">Include ディレクトリ:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="187"/>
        <location filename="settings.ui" line="237"/>
        <source>Add...</source>
        <translation type="unfinished">追加...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation type="unfinished">解析スレッド数:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="85"/>
        <source>Ideal count:</source>
        <translation type="unfinished">理想的な数：</translation>
    </message>
    <message>
        <location filename="settings.ui" line="114"/>
        <source>Force checking all #ifdef configurations</source>
        <oldsource>Check all #ifdef configurations</oldsource>
        <translation type="unfinished">すべての #ifdef をチェックする</translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Show full path of files</source>
        <translation type="unfinished">ファイルのフルパスを表示</translation>
    </message>
    <message>
        <location filename="settings.ui" line="128"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation type="unfinished">エラーが無いときは&quot;エラーなし&quot;を表示</translation>
    </message>
    <message>
        <location filename="settings.ui" line="135"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="142"/>
        <source>Enable inline suppressions</source>
        <translation type="unfinished">inline抑制を有効</translation>
    </message>
    <message>
        <location filename="settings.ui" line="163"/>
        <source>Paths</source>
        <translation type="unfinished">パス</translation>
    </message>
    <message>
        <location filename="settings.ui" line="194"/>
        <source>Edit</source>
        <translation type="unfinished">編集</translation>
    </message>
    <message>
        <location filename="settings.ui" line="201"/>
        <location filename="settings.ui" line="251"/>
        <source>Remove</source>
        <translation type="unfinished">削除</translation>
    </message>
    <message>
        <location filename="settings.ui" line="226"/>
        <source>Applications</source>
        <translation type="unfinished">アプリケーション</translation>
    </message>
    <message>
        <location filename="settings.ui" line="244"/>
        <source>Edit...</source>
        <translation type="unfinished">編集...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="258"/>
        <source>Set as default</source>
        <translation type="unfinished">デフォルトとして設定</translation>
    </message>
    <message>
        <location filename="settings.ui" line="281"/>
        <source>Reports</source>
        <translation type="unfinished">レポート</translation>
    </message>
    <message>
        <location filename="settings.ui" line="287"/>
        <source>Save all errors when creating report</source>
        <translation type="unfinished">すべての警告/エラーを保存</translation>
    </message>
    <message>
        <location filename="settings.ui" line="294"/>
        <source>Save full path to files in reports</source>
        <translation type="unfinished">ファイルのフルパスを保存</translation>
    </message>
    <message>
        <location filename="settings.ui" line="315"/>
        <source>Language</source>
        <translation type="unfinished">言語</translation>
    </message>
    <message>
        <location filename="settings.ui" line="329"/>
        <source>Advanced</source>
        <translation type="unfinished">高度</translation>
    </message>
    <message>
        <location filename="settings.ui" line="335"/>
        <source>&amp;Show inconclusive errors</source>
        <translation type="unfinished">結論の出ないのエラーを表示</translation>
    </message>
    <message>
        <location filename="settings.ui" line="342"/>
        <source>S&amp;how internal warnings in log</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="82"/>
        <source>N/A</source>
        <translation type="unfinished">N/A</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="201"/>
        <source>Add a new application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="234"/>
        <source>Modify an application</source>
        <translation type="unfinished">アプリケーションの変更</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="261"/>
        <source>[Default]</source>
        <translation type="unfinished">[デフォルト]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="312"/>
        <source>Select include directory</source>
        <translation type="unfinished">include ディレクトリを選択</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="228"/>
        <location filename="statsdialog.cpp" line="105"/>
        <source>Statistics</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="97"/>
        <source>Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="33"/>
        <source>Project:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="53"/>
        <source>Paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="85"/>
        <source>Include paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation type="unfinished">Defines:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="145"/>
        <location filename="statsdialog.cpp" line="101"/>
        <source>Previous Scan</source>
        <translation type="unfinished">前回の解析</translation>
    </message>
    <message>
        <location filename="stats.ui" line="151"/>
        <source>Path Selected:</source>
        <translation type="unfinished">ディレクトリ選択:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="161"/>
        <source>Number of Files Scanned:</source>
        <translation type="unfinished">解析済みファイル数:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Scan Duration:</source>
        <translation type="unfinished">解析にかかった時間:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="236"/>
        <source>Errors:</source>
        <translation type="unfinished">エラー:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="254"/>
        <source>Warnings:</source>
        <translation type="unfinished">警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="272"/>
        <source>Stylistic warnings:</source>
        <translation type="unfinished">スタイル警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="290"/>
        <source>Portability warnings:</source>
        <translation type="unfinished">移植可能性の警告</translation>
    </message>
    <message>
        <location filename="stats.ui" line="308"/>
        <source>Performance issues:</source>
        <translation type="unfinished">パフォーマンス警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="326"/>
        <source>Information messages:</source>
        <translation type="unfinished">情報メッセージ</translation>
    </message>
    <message>
        <location filename="stats.ui" line="364"/>
        <source>Copy to Clipboard</source>
        <translation type="unfinished">クリップボードにコピー</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="77"/>
        <source>1 day</source>
        <translation type="unfinished">一日</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="77"/>
        <source>%1 days</source>
        <translation type="unfinished">%1日</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="79"/>
        <source>1 hour</source>
        <translation type="unfinished">一時間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="79"/>
        <source>%1 hours</source>
        <translation type="unfinished">%1時間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>1 minute</source>
        <translation type="unfinished">一分</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="81"/>
        <source>%1 minutes</source>
        <translation type="unfinished">%1分</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>1 second</source>
        <translation type="unfinished">一秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="83"/>
        <source>%1 seconds</source>
        <translation type="unfinished">%1秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="87"/>
        <source>0.%1 seconds</source>
        <translation type="unfinished">0.%1秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="89"/>
        <source> and </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="96"/>
        <source>Project Settings</source>
        <translation type="unfinished">プロジェクトの設定</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="98"/>
        <source>Paths</source>
        <translation type="unfinished">パス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="99"/>
        <source>Include paths</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="100"/>
        <source>Defines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="102"/>
        <source>Path selected</source>
        <translation type="unfinished">選択されたパス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="103"/>
        <source>Number of files scanned</source>
        <translation type="unfinished">スキャンしたファイルの数</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="104"/>
        <source>Scan duration</source>
        <translation type="unfinished">スキャン期間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="106"/>
        <source>Errors</source>
        <translation type="unfinished">エラー</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="107"/>
        <source>Warnings</source>
        <translation type="unfinished">警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="108"/>
        <source>Style warnings</source>
        <translation type="unfinished">スタイル警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="109"/>
        <source>Portability warnings</source>
        <translation type="unfinished">移植可能性警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="110"/>
        <source>Performance warnings</source>
        <translation type="unfinished">パフォーマンス警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>Information messages</source>
        <translation type="unfinished">情報メッセージ</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="54"/>
        <source>%1 of %2 files checked</source>
        <translation>%1 / %2 文件已检查</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="73"/>
        <source>inconclusive</source>
        <translation>未知错误</translation>
    </message>
</context>
</TS>
