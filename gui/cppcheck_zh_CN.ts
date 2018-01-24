<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>关于 Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>版本 %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - C/C++ 静态代码分析工具。</translation>
    </message>
    <message>
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-2018 Cppcheck team.</source>
        <translation>版权所有 © 2007-2018 Daniel Marjamäki 与 Cppcheck 团队。</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>该程序在 GNU 通用公共授权版本 3 的条款下发布</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>访问 Cppcheck 主页: %1</translation>
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
        <translation>在这里，你可以添加一个应用程序，用于打开错误文件。请为该应用程序指定一个名称、可执行文件和命令行参数。

当应用程序执行时，在参数中的以下文本将会替换为适当的值:
(file) - 包含错误的文件名称
(line) - 包含错误的行号
(message) - 错误消息
(severity) - 错误严重性

示例：使用 Kate 打开一个文件，并使之滚动到相应的行:
可执行文件: kate
参数: -l(line) (file)</translation>
    </message>
    <message>
        <location filename="application.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>名称(&amp;N):</translation>
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
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>可执行文件(*.exe);;所有文件(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="62"/>
        <source>Select viewer application</source>
        <translation>选择查看应用程序</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="77"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="78"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>你必须为应用程序指定名称、路径以及可选参数！</translation>
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
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">编辑</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="82"/>
        <location filename="librarydialog.cpp" line="154"/>
        <source>Library files (*.cfg)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="84"/>
        <source>Open library file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="95"/>
        <location filename="librarydialog.cpp" line="107"/>
        <location filename="librarydialog.cpp" line="144"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="96"/>
        <source>Can not open file %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="108"/>
        <source>Failed to load %1. %2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="145"/>
        <source>Can not save file %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="157"/>
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
        <translation type="vanished">正在检查记录</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">清空</translation>
    </message>
    <message>
        <source>Save Log</source>
        <translation type="vanished">保存记录</translation>
    </message>
    <message>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation type="vanished">文本文件(*.txt *.log);;所有文件(*.*)</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation type="vanished">无法打开并写入文件: “%1”</translation>
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
        <location filename="mainwindow.ui" line="70"/>
        <source>&amp;File</source>
        <translation>文件(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="89"/>
        <source>&amp;View</source>
        <translation>查看(&amp;V)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="93"/>
        <source>&amp;Toolbars</source>
        <translation>工具栏(&amp;T)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="121"/>
        <source>&amp;Help</source>
        <translation>帮助(&amp;H)</translation>
    </message>
    <message>
        <source>&amp;Check</source>
        <translation type="obsolete">检查(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="135"/>
        <source>C++ standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="143"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="167"/>
        <source>&amp;Edit</source>
        <translation>编辑(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="180"/>
        <source>Standard</source>
        <translation>标准</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="198"/>
        <source>Categories</source>
        <translation>分类</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="228"/>
        <source>&amp;License...</source>
        <translation>许可证(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="233"/>
        <source>A&amp;uthors...</source>
        <translation>作者(&amp;U)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="242"/>
        <source>&amp;About...</source>
        <translation>关于(&amp;A)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="247"/>
        <source>&amp;Files...</source>
        <translation>文件(&amp;F)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="250"/>
        <location filename="mainwindow.ui" line="253"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation type="unfinished">检查文件</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="256"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="265"/>
        <source>&amp;Directory...</source>
        <translation>目录(&amp;D)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="268"/>
        <location filename="mainwindow.ui" line="271"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation type="unfinished">检查目录</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="274"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>&amp;Recheck files</source>
        <translation type="obsolete">重新检查文件(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="286"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="304"/>
        <source>&amp;Stop</source>
        <translation>停止(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="307"/>
        <location filename="mainwindow.ui" line="310"/>
        <source>Stop analysis</source>
        <oldsource>Stop checking</oldsource>
        <translation type="unfinished">停止检查</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="313"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="322"/>
        <source>&amp;Save results to file...</source>
        <translation>保存结果到文件(&amp;S)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="325"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="330"/>
        <source>&amp;Quit</source>
        <translation>退出(&amp;Q)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="339"/>
        <source>&amp;Clear results</source>
        <translation>清空结果(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="348"/>
        <source>&amp;Preferences</source>
        <translation>首选项(&amp;P)</translation>
    </message>
    <message>
        <source>Style warnings</source>
        <translation type="vanished">风格警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="363"/>
        <location filename="mainwindow.ui" line="366"/>
        <source>Show style warnings</source>
        <translation>显示风格警告</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation type="vanished">错误</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="381"/>
        <location filename="mainwindow.ui" line="384"/>
        <source>Show errors</source>
        <translation>显示错误</translation>
    </message>
    <message>
        <source>Show S&amp;cratchpad...</source>
        <translation type="vanished">显示便条(&amp;C)...</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="761"/>
        <location filename="mainwindow.cpp" line="799"/>
        <source>Information</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="562"/>
        <source>Show information messages</source>
        <translation>显示信息消息</translation>
    </message>
    <message>
        <source>Portability</source>
        <translation type="vanished">移植可能性</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="577"/>
        <source>Show portability warnings</source>
        <translation>显示可移植性警告</translation>
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
        <translation>滤器(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="618"/>
        <source>Filter results</source>
        <translation>过滤结果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="634"/>
        <source>Windows 32-bit ANSI</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="642"/>
        <source>Windows 32-bit Unicode</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="650"/>
        <source>Unix 32-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="658"/>
        <source>Unix 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="666"/>
        <source>Windows 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <source>Platforms</source>
        <translation type="vanished">平台</translation>
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
        <translation type="obsolete">C11</translation>
    </message>
    <message>
        <source>C89</source>
        <translation type="obsolete">C89</translation>
    </message>
    <message>
        <source>C++03</source>
        <translation type="obsolete">C++03</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="736"/>
        <source>&amp;Print...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="739"/>
        <source>Print the Current Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="744"/>
        <source>Print Pre&amp;view...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="747"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="755"/>
        <source>Open library editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="486"/>
        <source>C&amp;lose Project File</source>
        <translation>关闭项目文件(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="494"/>
        <source>&amp;Edit Project File...</source>
        <translation>编辑项目文件(&amp;E)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="503"/>
        <source>&amp;Statistics</source>
        <translation>统计(&amp;S)</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation type="vanished">警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="518"/>
        <location filename="mainwindow.ui" line="521"/>
        <source>Show warnings</source>
        <translation>显示警告</translation>
    </message>
    <message>
        <source>Performance warnings</source>
        <translation type="vanished">性能警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="536"/>
        <location filename="mainwindow.ui" line="539"/>
        <source>Show performance warnings</source>
        <translation>显示性能警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="547"/>
        <source>Show &amp;hidden</source>
        <translation>显示隐藏项(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="389"/>
        <source>&amp;Check all</source>
        <translation>全部选中(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="131"/>
        <source>A&amp;nalyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="217"/>
        <source>Filter</source>
        <translation>滤器</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="283"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation type="unfinished"></translation>
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
        <translation>全部取消选中(&amp;U)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="399"/>
        <source>Collapse &amp;all</source>
        <translation>全部折叠(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="404"/>
        <source>&amp;Expand all</source>
        <translation>全部展开(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="412"/>
        <source>&amp;Standard</source>
        <translation>标准(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="415"/>
        <source>Standard items</source>
        <translation>标准项</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="420"/>
        <source>&amp;Contents</source>
        <translation>内容(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="423"/>
        <source>Open the help contents</source>
        <translation>打开帮助内容</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="426"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="431"/>
        <source>Toolbar</source>
        <translation>工具栏</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="439"/>
        <source>&amp;Categories</source>
        <translation>分类(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="442"/>
        <source>Error categories</source>
        <translation>错误分类</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="447"/>
        <source>&amp;Open XML...</source>
        <translation>打开 XML (&amp;O)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="456"/>
        <source>Open P&amp;roject File...</source>
        <translation>打开项目文件(&amp;R)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="465"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="470"/>
        <source>&amp;New Project File...</source>
        <translation>新建项目文件(&amp;N)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="475"/>
        <source>&amp;Log View</source>
        <translation>日志视图(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="478"/>
        <source>Log View</source>
        <translation>日志视图</translation>
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
        <location filename="mainwindow.cpp" line="322"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>加载编辑器应用程序设置出错。

这可能是因为 Cppcheck 不同版本间的设置有所不同。请检查(并修复)编辑器应用程序设置，否则编辑器程序可能不会正确启动。</translation>
    </message>
    <message>
        <source>No suitable files found to check!</source>
        <translation type="obsolete">未发现适合检查的文件！</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="555"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>在选择新的文件或目录之前，你必须先关闭此项目文件！</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="572"/>
        <source>C/C++ Source, Compile database, Visual Studio (%1 %2 *.sln *.vcxproj)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select directory to check</source>
        <translation type="obsolete">选择目录来检查</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="85"/>
        <source>Quick Filter:</source>
        <translation>快速滤器:</translation>
    </message>
    <message>
        <source>Select files to check</source>
        <translation type="obsolete">选择要检查的文件</translation>
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
        <translation>找到项目文件: %1

你是否想加载该项目文件？</translation>
    </message>
    <message>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation type="obsolete">在目录中找到项目文件。

你是否想在不使用这些项目文件的情况下，执行检查？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="761"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="770"/>
        <source>File not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="773"/>
        <source>Bad XML</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="776"/>
        <source>Missing attribute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="779"/>
        <source>Bad attribute value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="782"/>
        <source>Unsupported format</source>
        <translation type="unfinished"></translation>
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
        <location filename="mainwindow.cpp" line="799"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
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
        <translation>当前结果将被清空。

打开一个新的 XML 文件将会清空当前结果。你要继续吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1092"/>
        <location filename="mainwindow.cpp" line="1262"/>
        <source>XML files (*.xml)</source>
        <translation>XML 文件(*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1094"/>
        <source>Open the report file</source>
        <translation>打开报告文件</translation>
    </message>
    <message>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?</source>
        <translation type="obsolete">检查正在执行。

你是否需要停止检查并退出 Cppcheck？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1233"/>
        <source>License</source>
        <translation>许可证</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1240"/>
        <source>Authors</source>
        <translation>作者</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation type="obsolete">XML 文件版本 2 (*.xml);;XML 文件版本 1 (*.xml);; 文本文件(*.txt);; CSV 文件(*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1255"/>
        <source>Save the report file</source>
        <translation>保存报告文件</translation>
    </message>
    <message>
        <source>XML files version 1 (*.xml)</source>
        <translation type="obsolete">XML 文件版本 1 (*.xml)</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml)</source>
        <translation type="obsolete">XML 文件版本 2 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1266"/>
        <source>Text files (*.txt)</source>
        <translation>文本文件(*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1270"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 文件(*.csv)</translation>
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
        <translation type="obsolete">更改用户界面语言失败:

%1

用户界面语言已被重置为英语。打开“首选项”对话框，选择任何可用的语言。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1361"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>项目文件(*.cppcheck);;所有文件(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1363"/>
        <source>Select Project File</source>
        <translation>选择项目文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="154"/>
        <location filename="mainwindow.cpp" line="1391"/>
        <location filename="mainwindow.cpp" line="1515"/>
        <source>Project:</source>
        <translation>项目:</translation>
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
        <translation>选择项目文件名</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1547"/>
        <source>No project file loaded</source>
        <translation>项目文件未加载</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1607"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>项目文件

%1

未找到！

你要从最近使用的项目列表中删除此文件吗？</translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <source>Built-in</source>
        <translation type="obsolete">内置</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="37"/>
        <source>Native</source>
        <translation type="unfinished"></translation>
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
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <source>Could not read the project file.</source>
        <translation type="vanished">无法读取项目文件。</translation>
    </message>
    <message>
        <source>Could not write the project file.</source>
        <translation type="vanished">无法写入项目文件。</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfiledialog.ui" line="14"/>
        <source>Project File</source>
        <translation>项目文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="273"/>
        <source>Project</source>
        <translation>项目</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Root:</source>
        <oldsource>Root:</oldsource>
        <translation type="obsolete">根目录:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="331"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation type="unfinished"></translation>
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
        <translation>路径:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="113"/>
        <location filename="projectfiledialog.ui" line="207"/>
        <location filename="projectfiledialog.ui" line="378"/>
        <source>Add...</source>
        <translation>添加...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="120"/>
        <location filename="projectfiledialog.ui" line="214"/>
        <location filename="projectfiledialog.ui" line="385"/>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="127"/>
        <location filename="projectfiledialog.ui" line="221"/>
        <location filename="projectfiledialog.ui" line="392"/>
        <location filename="projectfiledialog.ui" line="455"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="176"/>
        <source>Include Paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Includes</source>
        <translation type="obsolete">包含</translation>
    </message>
    <message>
        <source>Include directories:</source>
        <translation type="obsolete">Include 目录:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="241"/>
        <source>Up</source>
        <translation>向上</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="248"/>
        <source>Down</source>
        <translation>向下</translation>
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
        <translation>排除</translation>
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
        <translation type="unfinished"></translation>
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
        <translation>定义:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="47"/>
        <source>Project file: %1</source>
        <translation>项目文件: %1</translation>
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
        <translation>选择 Include 目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="477"/>
        <source>Select a directory to check</source>
        <translation>选择一个检查目录</translation>
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
        <translation>选择忽略的目录</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Close</source>
        <translation type="unfinished">关闭</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="103"/>
        <source>Unknown language specified!</source>
        <translation>指定了未知语言！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="132"/>
        <source>Language file %1 not found!</source>
        <translation>语言文件 %1 不存在！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="138"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>无法从文件 %2 中为语言 %1 加载翻译文件</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Close</source>
        <translation type="unfinished">关闭</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>File</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Severity</source>
        <translation>严重性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Line</source>
        <translation>行</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Summary</source>
        <translation>概要</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="137"/>
        <source>Undefined file</source>
        <translation>未定义文件</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="841"/>
        <source>[Inconclusive]</source>
        <translation>[不确定的]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="308"/>
        <source>debug</source>
        <translation>调试</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="245"/>
        <source>note</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="608"/>
        <source>Recheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="609"/>
        <source>Copy filename</source>
        <translation>复制文件名</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="610"/>
        <source>Copy full path</source>
        <translation>复制完整路径</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="611"/>
        <source>Copy message</source>
        <translation>复制消息</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="612"/>
        <source>Copy message id</source>
        <translation>复制消息 ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="613"/>
        <source>Hide</source>
        <translation>隐藏</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="614"/>
        <source>Hide all with id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="615"/>
        <source>Suppress selected id(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="616"/>
        <source>Open containing folder</source>
        <translation type="unfinished"></translation>
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
        <translation>编辑应用程序未配置。

在“首先项 / 应用程序”中为 Cppcheck 配置编辑应用程序。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="716"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>未选中默认编辑应用程序。

请在“首先项 / 应用程序”中选择默认应用程序。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="745"/>
        <source>Could not find the file!</source>
        <translation>找不到文件！</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="791"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>无法启动 %1

请检查此应用程序的路径与参数是否正确。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="805"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>无法找到文件:
%1
请选择文件所在目录。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="812"/>
        <source>Select Directory</source>
        <translation>选择目录</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Inconclusive</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Since date</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="290"/>
        <source>style</source>
        <translation>风格</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="293"/>
        <source>error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="296"/>
        <source>warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="299"/>
        <source>performance</source>
        <translation>性能</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="302"/>
        <source>portability</source>
        <translation>移植可能性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="305"/>
        <source>information</source>
        <translation>信息</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>结果</translation>
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
    <message>
        <location filename="resultsview.cpp" line="156"/>
        <source>No errors found, nothing to save.</source>
        <translation>未发现错误，没有结果可保存。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="180"/>
        <location filename="resultsview.cpp" line="188"/>
        <source>Failed to save the report.</source>
        <translation>保存报告失败。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="198"/>
        <source>Print Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="217"/>
        <source>No errors found, nothing to print.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="255"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%2 个文件已检查 %1 个)</translation>
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
        <translation>未发现错误。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="276"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>发现错误，但它们被设为隐藏。
打开“查看”菜单，切换需要显示的错误。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="322"/>
        <location filename="resultsview.cpp" line="343"/>
        <location filename="resultsview.cpp" line="351"/>
        <source>Failed to read the report.</source>
        <translation>读取报告失败。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="329"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="388"/>
        <source>Summary</source>
        <translation>概要</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="389"/>
        <source>Message</source>
        <translation>消息</translation>
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
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <source>Scratchpad</source>
        <translation>便条</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="48"/>
        <source>filename</source>
        <translation>文件名</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="55"/>
        <source>Check</source>
        <translation>检查</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>首选项</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>常规</translation>
    </message>
    <message>
        <source>Include paths:</source>
        <translation type="vanished">Include 路径:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="195"/>
        <source>Add...</source>
        <translation>添加...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>线程个数:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="85"/>
        <source>Ideal count:</source>
        <translation>理想个数:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="114"/>
        <source>Force checking all #ifdef configurations</source>
        <oldsource>Check all #ifdef configurations</oldsource>
        <translation>强制检查所有 #ifdef 配置</translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Show full path of files</source>
        <translation>显示文件的完整路径</translation>
    </message>
    <message>
        <location filename="settings.ui" line="128"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>当未找到错误，显示“未发现错误”消息</translation>
    </message>
    <message>
        <location filename="settings.ui" line="135"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>在列“Id”中显示错误 Id</translation>
    </message>
    <message>
        <location filename="settings.ui" line="142"/>
        <source>Enable inline suppressions</source>
        <translation>启用内联方案</translation>
    </message>
    <message>
        <location filename="settings.ui" line="149"/>
        <source>Check for inconclusive errors also</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="156"/>
        <source>Show statistics on check completion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="176"/>
        <source>Show internal warnings in log</source>
        <translation type="unfinished"></translation>
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
        <translation type="vanished">路径</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="vanished">编辑</translation>
    </message>
    <message>
        <location filename="settings.ui" line="209"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="settings.ui" line="184"/>
        <source>Applications</source>
        <translation>应用程序</translation>
    </message>
    <message>
        <location filename="settings.ui" line="202"/>
        <source>Edit...</source>
        <translation>编辑...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="216"/>
        <source>Set as default</source>
        <translation>设为默认</translation>
    </message>
    <message>
        <location filename="settings.ui" line="239"/>
        <source>Reports</source>
        <translation>报告</translation>
    </message>
    <message>
        <location filename="settings.ui" line="245"/>
        <source>Save all errors when creating report</source>
        <translation>创建报告时，保存所有错误</translation>
    </message>
    <message>
        <location filename="settings.ui" line="252"/>
        <source>Save full path to files in reports</source>
        <translation>在报告中保存文件的完整路径</translation>
    </message>
    <message>
        <location filename="settings.ui" line="273"/>
        <source>Language</source>
        <translation>语言</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="obsolete">高级</translation>
    </message>
    <message>
        <source>&amp;Show inconclusive errors</source>
        <translation type="obsolete">显示不确定的错误(&amp;S)</translation>
    </message>
    <message>
        <source>S&amp;how internal warnings in log</source>
        <translation type="obsolete">在日记中显示内部警告(&amp;H)</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="88"/>
        <source>N/A</source>
        <translation>N/A</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="186"/>
        <source>Add a new application</source>
        <translation>添加一个新的应用程序</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="219"/>
        <source>Modify an application</source>
        <translation>修改一个应用程序</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="224"/>
        <source> [Default]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="249"/>
        <source>[Default]</source>
        <translation>[默认]</translation>
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
        <translation type="vanished">选择包含目录</translation>
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
        <translation>统计</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="173"/>
        <source>Project</source>
        <translation>项目</translation>
    </message>
    <message>
        <location filename="stats.ui" line="33"/>
        <source>Project:</source>
        <translation>项目:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="53"/>
        <source>Paths:</source>
        <translation>路径:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="85"/>
        <source>Include paths:</source>
        <translation>包含路径:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation>定义:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="145"/>
        <location filename="statsdialog.cpp" line="177"/>
        <source>Previous Scan</source>
        <translation>上一次扫描</translation>
    </message>
    <message>
        <location filename="stats.ui" line="151"/>
        <source>Path Selected:</source>
        <translation>选中的路径:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="161"/>
        <source>Number of Files Scanned:</source>
        <translation>扫描的文件数:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Scan Duration:</source>
        <translation>扫描时间:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="236"/>
        <source>Errors:</source>
        <translation>错误:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="254"/>
        <source>Warnings:</source>
        <translation>警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="272"/>
        <source>Stylistic warnings:</source>
        <translation>Stylistic 警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="290"/>
        <source>Portability warnings:</source>
        <translation>可移植性警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="308"/>
        <source>Performance issues:</source>
        <translation>性能警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="326"/>
        <source>Information messages:</source>
        <translation>信息:</translation>
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
        <translation>复制到剪贴板</translation>
    </message>
    <message>
        <location filename="stats.ui" line="394"/>
        <source>Pdf Export</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>1 day</source>
        <translation>1 天</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>%1 days</source>
        <translation>%1 天</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>1 hour</source>
        <translation>1 小时</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>%1 hours</source>
        <translation>%1 小时</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>1 minute</source>
        <translation>1 分钟</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>%1 minutes</source>
        <translation>%1 分钟</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>1 second</source>
        <translation>1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>%1 seconds</source>
        <translation>%1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="121"/>
        <source>0.%1 seconds</source>
        <translation>0.%1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="123"/>
        <source> and </source>
        <translation> 与 </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="150"/>
        <source>Export PDF</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="172"/>
        <source>Project Settings</source>
        <translation>项目设置</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="174"/>
        <source>Paths</source>
        <translation>路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="175"/>
        <source>Include paths</source>
        <translation>包含路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="176"/>
        <source>Defines</source>
        <translation>定义</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="178"/>
        <source>Path selected</source>
        <translation>选中的路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="179"/>
        <source>Number of files scanned</source>
        <translation>扫描的文件数</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="180"/>
        <source>Scan duration</source>
        <translation>扫描时间</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="137"/>
        <location filename="statsdialog.cpp" line="182"/>
        <source>Errors</source>
        <translation>错误</translation>
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
        <translation>警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="141"/>
        <location filename="statsdialog.cpp" line="184"/>
        <source>Style warnings</source>
        <translation>风格警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="143"/>
        <location filename="statsdialog.cpp" line="185"/>
        <source>Portability warnings</source>
        <translation>移植可能性警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="145"/>
        <location filename="statsdialog.cpp" line="186"/>
        <source>Performance warnings</source>
        <translation>性能警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="147"/>
        <location filename="statsdialog.cpp" line="187"/>
        <source>Information messages</source>
        <translation>信息</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="54"/>
        <source>%1 of %2 files checked</source>
        <translation>%2 个文件已检查 %1 个</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="144"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation type="unfinished">更改用户界面语言失败:

%1

用户界面语言已被重置为英语。打开“首选项”对话框，选择任何可用的语言。</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="150"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="73"/>
        <source>inconclusive</source>
        <translation>不确定的</translation>
    </message>
</context>
</TS>
