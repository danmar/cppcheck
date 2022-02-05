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
        <source>Copyright © 2007-%1 Cppcheck team.</source>
        <oldsource>Copyright © 2007-2021 Cppcheck team.</oldsource>
        <translation>版权所有 © 2007-%1 Cppcheck 团队。</translation>
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
    <message>
        <location filename="about.ui" line="115"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul&gt;
&lt;li&gt;pcre&lt;/li&gt;
&lt;li&gt;picojson&lt;/li&gt;
&lt;li&gt;qt&lt;/li&gt;
&lt;li&gt;tinyxml2&lt;/li&gt;
&lt;li&gt;z3&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <oldsource>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul&gt;
&lt;li&gt;tinyxml2&lt;/li&gt;
&lt;li&gt;picojson&lt;/li&gt;
&lt;li&gt;pcre&lt;/li&gt;
&lt;li&gt;qt&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</oldsource>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;非常感谢我们使用的这些库:&lt;/p&gt;&lt;ul&gt;
&lt;li&gt;pcre&lt;/li&gt;
&lt;li&gt;picojson&lt;/li&gt;
&lt;li&gt;qt&lt;/li&gt;
&lt;li&gt;tinyxml2&lt;/li&gt;
&lt;li&gt;z3&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
        <location filename="applicationdialog.cpp" line="60"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>可执行文件(*.exe);;所有文件(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="63"/>
        <source>Select viewer application</source>
        <translation>选择查看应用程序</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="78"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="79"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>你必须为应用程序指定名称、路径以及可选参数！</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="43"/>
        <source>Could not find the file: %1</source>
        <translation>无法找到文件: %1</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="47"/>
        <location filename="fileviewdialog.cpp" line="61"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="57"/>
        <source>Could not read the file: %1</source>
        <translation>无法读取文件: %1</translation>
    </message>
</context>
<context>
    <name>FunctionContractDialog</name>
    <message>
        <location filename="functioncontractdialog.ui" line="14"/>
        <source>Function contract</source>
        <translation>函数约定</translation>
    </message>
    <message>
        <location filename="functioncontractdialog.ui" line="20"/>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="functioncontractdialog.ui" line="27"/>
        <source>Requirements for parameters</source>
        <translation>必须的参数</translation>
    </message>
</context>
<context>
    <name>HelpDialog</name>
    <message>
        <location filename="helpdialog.ui" line="14"/>
        <source>Cppcheck GUI help</source>
        <translation>Cppcheck GUI 帮助</translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="34"/>
        <source>Contents</source>
        <translation>内容</translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="44"/>
        <source>Index</source>
        <translation>索引</translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="77"/>
        <source>Helpfile &apos;%1&apos; was not found</source>
        <translation>帮助文件 &apos;%1&apos; 未找到</translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="79"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>LibraryAddFunctionDialog</name>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="23"/>
        <source>Add function</source>
        <translation>添加函数</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <source>Function name(s)</source>
        <translation>函数名</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <source>Number of arguments</source>
        <translation>参数个数</translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <source>Library Editor</source>
        <translation>库编辑器</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <source>Open</source>
        <translation>打开</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <source>Save as</source>
        <translation>另存为</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <source>Functions</source>
        <translation>函数</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <source>Sort</source>
        <translation>排序</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <source>Add</source>
        <translation>添加</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <source>Filter:</source>
        <translation>过滤：</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <source>Comments</source>
        <translation>注释</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="204"/>
        <source>noreturn</source>
        <translation>无返回</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="212"/>
        <source>False</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="217"/>
        <source>True</source>
        <translation>是</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="222"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="232"/>
        <source>return value must be used</source>
        <translation>返回值必须被使用</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <source>ignore function in leaks checking</source>
        <translation>在泄漏检查中忽略函数</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <source>Arguments</source>
        <translation>参数</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="82"/>
        <location filename="librarydialog.cpp" line="154"/>
        <source>Library files (*.cfg)</source>
        <translation>库文件 (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="84"/>
        <source>Open library file</source>
        <translation>打开库文件</translation>
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
        <source>Cannot open file %1.</source>
        <oldsource>Can not open file %1.</oldsource>
        <translation>无法打开文件 %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="108"/>
        <source>Failed to load %1. %2.</source>
        <translation>加载文件 %1 失败。%2。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="145"/>
        <source>Cannot save file %1.</source>
        <oldsource>Can not save file %1.</oldsource>
        <translation>无法保存文件 %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="157"/>
        <source>Save the library as</source>
        <translation>库另存为</translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <source>Edit argument</source>
        <translation>编辑参数</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="20"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is bool value allowed? For instance result from comparison or from &apos;!&apos; operator.&lt;/p&gt;
&lt;p&gt;Typically, set this if the argument is a pointer, size, etc.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // last argument should not have a bool value&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;是否允许布尔值？ 例如，来自比较结果或来自 &apos;!&apos; 操作符。&lt;/p&gt;
&lt;p&gt;通常，如果参数是指针、大小等，则设置此参数。&lt;/p&gt;
&lt;p&gt;例子：&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // 最后一个参数不应该有bool值&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="28"/>
        <source>Not bool</source>
        <translation>非布尔值</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="35"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is a null parameter value allowed?&lt;/p&gt;
&lt;p&gt;Typically this should be used on any pointer parameter that does not allow null.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // neither x or y is allowed to be null.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;是否允许空参数值?&lt;/p&gt;
&lt;p&gt;通常这应该用于任何不允许空指针的参数。&lt;/p&gt;
&lt;p&gt;例子：&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // 无论 x 或 y 都不允许为空。&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="43"/>
        <source>Not null</source>
        <translation>非空</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="50"/>
        <source>Not uninit</source>
        <translation>非未初始化</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="57"/>
        <source>String</source>
        <translation>字符串</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="70"/>
        <source>Format string</source>
        <translation>格式化字符串</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="92"/>
        <source>Min size of buffer</source>
        <translation>最小缓冲区大小</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="101"/>
        <location filename="libraryeditargdialog.ui" line="203"/>
        <source>Type</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="214"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <source>argvalue</source>
        <translation>argvalue</translation>
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
        <translation>参数1</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="159"/>
        <location filename="libraryeditargdialog.ui" line="264"/>
        <source>Arg2</source>
        <translation>参数2</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="194"/>
        <source>and</source>
        <translation>并且</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="310"/>
        <source>Valid values</source>
        <translation>有效值</translation>
    </message>
</context>
<context>
    <name>LogView</name>
    <message>
        <source>Checking Log</source>
        <translation type="obsolete">正在检查记录</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="obsolete">清空</translation>
    </message>
    <message>
        <source>Save Log</source>
        <translation type="obsolete">保存记录</translation>
    </message>
    <message>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation type="obsolete">文本文件(*.txt *.log);;所有文件(*.*)</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="obsolete">Cppcheck</translation>
    </message>
    <message>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation type="obsolete">无法打开并写入文件: “%1”</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.ui" line="604"/>
        <location filename="mainwindow.cpp" line="328"/>
        <location filename="mainwindow.cpp" line="485"/>
        <location filename="mainwindow.cpp" line="561"/>
        <location filename="mainwindow.cpp" line="693"/>
        <location filename="mainwindow.cpp" line="715"/>
        <location filename="mainwindow.cpp" line="1198"/>
        <location filename="mainwindow.cpp" line="1323"/>
        <location filename="mainwindow.cpp" line="1597"/>
        <location filename="mainwindow.cpp" line="1605"/>
        <location filename="mainwindow.cpp" line="1628"/>
        <location filename="mainwindow.cpp" line="1705"/>
        <location filename="mainwindow.cpp" line="1780"/>
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
        <translation>C++ 标准</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="145"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation>&amp;C 标准</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="170"/>
        <source>&amp;Edit</source>
        <translation>编辑(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="183"/>
        <source>Standard</source>
        <translation>标准</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="201"/>
        <source>Categories</source>
        <translation>分类</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="231"/>
        <source>&amp;License...</source>
        <translation>许可证(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="236"/>
        <source>A&amp;uthors...</source>
        <translation>作者(&amp;U)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="245"/>
        <source>&amp;About...</source>
        <translation>关于(&amp;A)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="250"/>
        <source>&amp;Files...</source>
        <translation>文件(&amp;F)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="253"/>
        <location filename="mainwindow.ui" line="256"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation>分析文件</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="259"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="268"/>
        <source>&amp;Directory...</source>
        <translation>目录(&amp;D)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="271"/>
        <location filename="mainwindow.ui" line="274"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation>分析目录</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="277"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>&amp;Recheck files</source>
        <translation type="obsolete">重新检查文件(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="289"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="307"/>
        <source>&amp;Stop</source>
        <translation>停止(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="310"/>
        <location filename="mainwindow.ui" line="313"/>
        <source>Stop analysis</source>
        <oldsource>Stop checking</oldsource>
        <translation>停止分析</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="316"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="325"/>
        <source>&amp;Save results to file...</source>
        <translation>保存结果到文件(&amp;S)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="328"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="333"/>
        <source>&amp;Quit</source>
        <translation>退出(&amp;Q)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="345"/>
        <source>&amp;Clear results</source>
        <translation>清空结果(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="354"/>
        <source>&amp;Preferences</source>
        <translation>首选项(&amp;P)</translation>
    </message>
    <message>
        <source>Style warnings</source>
        <translation type="obsolete">风格警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="369"/>
        <location filename="mainwindow.ui" line="372"/>
        <source>Show style warnings</source>
        <translation>显示风格警告</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation type="obsolete">错误</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="387"/>
        <location filename="mainwindow.ui" line="390"/>
        <source>Show errors</source>
        <translation>显示错误</translation>
    </message>
    <message>
        <source>Show S&amp;cratchpad...</source>
        <translation type="obsolete">显示便条(&amp;C)...</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="804"/>
        <location filename="mainwindow.cpp" line="842"/>
        <source>Information</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="577"/>
        <source>Show information messages</source>
        <translation>显示信息消息</translation>
    </message>
    <message>
        <source>Portability</source>
        <translation type="obsolete">移植可能性</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="592"/>
        <source>Show portability warnings</source>
        <translation>显示可移植性警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="607"/>
        <source>Show Cppcheck results</source>
        <translation>显示 Cppcheck 结果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="619"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="622"/>
        <source>Show Clang results</source>
        <translation>显示 Clang 结果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="630"/>
        <source>&amp;Filter</source>
        <translation>滤器(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="633"/>
        <source>Filter results</source>
        <translation>过滤结果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="649"/>
        <source>Windows 32-bit ANSI</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="657"/>
        <source>Windows 32-bit Unicode</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="665"/>
        <source>Unix 32-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="673"/>
        <source>Unix 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="681"/>
        <source>Windows 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <source>Platforms</source>
        <translation type="obsolete">平台</translation>
    </message>
    <message>
        <source>C++11</source>
        <translation type="obsolete">C++11</translation>
    </message>
    <message>
        <source>C99</source>
        <translation type="obsolete">C99</translation>
    </message>
    <message>
        <source>Posix</source>
        <translation type="obsolete">Posix</translation>
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
        <location filename="mainwindow.ui" line="751"/>
        <source>&amp;Print...</source>
        <translation>打印(&amp;P)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="754"/>
        <source>Print the Current Report</source>
        <translation>打印当前报告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="759"/>
        <source>Print Pre&amp;view...</source>
        <translation>打印预览(&amp;v)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="762"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>打开当前结果的打印预览窗口</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="770"/>
        <source>Open library editor</source>
        <translation>打开库编辑器</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="498"/>
        <source>C&amp;lose Project File</source>
        <translation>关闭项目文件(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="506"/>
        <source>&amp;Edit Project File...</source>
        <translation>编辑项目文件(&amp;E)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="518"/>
        <source>&amp;Statistics</source>
        <translation>统计(&amp;S)</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation type="obsolete">警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="533"/>
        <location filename="mainwindow.ui" line="536"/>
        <source>Show warnings</source>
        <translation>显示警告</translation>
    </message>
    <message>
        <source>Performance warnings</source>
        <translation type="obsolete">性能警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="551"/>
        <location filename="mainwindow.ui" line="554"/>
        <source>Show performance warnings</source>
        <translation>显示性能警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="562"/>
        <source>Show &amp;hidden</source>
        <translation>显示隐藏项(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="395"/>
        <source>&amp;Check all</source>
        <translation>全部选中(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="131"/>
        <source>A&amp;nalyze</source>
        <translation>分析(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="220"/>
        <source>Filter</source>
        <translation>滤器</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="286"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation>重新分析已修改的文件(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="298"/>
        <source>Reanal&amp;yze all files</source>
        <translation>重新分析全部文件(&amp;y)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="336"/>
        <source>Ctrl+Q</source>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="366"/>
        <source>Style war&amp;nings</source>
        <translation>风格警告(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="384"/>
        <source>E&amp;rrors</source>
        <translation>编辑(&amp;r)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="400"/>
        <source>&amp;Uncheck all</source>
        <translation>全部取消选中(&amp;U)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="405"/>
        <source>Collapse &amp;all</source>
        <translation>全部折叠(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="410"/>
        <source>&amp;Expand all</source>
        <translation>全部展开(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="418"/>
        <source>&amp;Standard</source>
        <translation>标准(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="421"/>
        <source>Standard items</source>
        <translation>标准项</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="426"/>
        <source>&amp;Contents</source>
        <translation>内容(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="429"/>
        <source>Open the help contents</source>
        <translation>打开帮助内容</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="432"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="437"/>
        <source>Toolbar</source>
        <translation>工具栏</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="445"/>
        <source>&amp;Categories</source>
        <translation>分类(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="448"/>
        <source>Error categories</source>
        <translation>错误分类</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="453"/>
        <source>&amp;Open XML...</source>
        <translation>打开 XML (&amp;O)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="462"/>
        <source>Open P&amp;roject File...</source>
        <translation>打开项目文件(&amp;R)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="465"/>
        <source>Ctrl+Shift+O</source>
        <translation>Ctrl+Shift+O</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="474"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation>显示便条(&amp;o)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="479"/>
        <source>&amp;New Project File...</source>
        <translation>新建项目文件(&amp;N)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="482"/>
        <source>Ctrl+Shift+N</source>
        <translation>Ctrl+Shift+N</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="487"/>
        <source>&amp;Log View</source>
        <translation>日志视图(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="490"/>
        <source>Log View</source>
        <translation>日志视图</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="530"/>
        <source>&amp;Warnings</source>
        <translation>警告(&amp;W)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="548"/>
        <source>Per&amp;formance warnings</source>
        <translation>性能警告(&amp;f)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="574"/>
        <source>&amp;Information</source>
        <translation>信息(&amp;I)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="589"/>
        <source>&amp;Portability</source>
        <translation>可移植性(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="689"/>
        <source>P&amp;latforms</source>
        <translation>平台(&amp;l)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="703"/>
        <source>C++&amp;11</source>
        <translation>C++&amp;11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="714"/>
        <source>C&amp;99</source>
        <translation>C&amp;99</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="722"/>
        <source>&amp;Posix</source>
        <translation>&amp;Posix</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="730"/>
        <source>C&amp;11</source>
        <translation>C&amp;11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="738"/>
        <source>&amp;C89</source>
        <translation>&amp;C89</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="746"/>
        <source>&amp;C++03</source>
        <translation>&amp;C++03</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="767"/>
        <source>&amp;Library Editor...</source>
        <translation>库编辑器(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="778"/>
        <source>&amp;Auto-detect language</source>
        <translation>自动检测语言(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="786"/>
        <source>&amp;Enforce C++</source>
        <translation>&amp;Enforce C++</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="794"/>
        <source>E&amp;nforce C</source>
        <translation>E&amp;nforce C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="805"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="813"/>
        <source>Reanalyze and check library</source>
        <translation>重新分析并检查库</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="821"/>
        <source>Check configuration (defines, includes)</source>
        <translation>检查配置(defines, includes)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="829"/>
        <source>C++17</source>
        <translation>C++17</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="840"/>
        <source>C++20</source>
        <translation>C++20</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="323"/>
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
        <location filename="mainwindow.cpp" line="562"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>在选择新的文件或目录之前，你必须先关闭此项目文件！</translation>
    </message>
    <message>
        <source>Select directory to check</source>
        <translation type="obsolete">选择目录来检查</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="83"/>
        <location filename="mainwindow.cpp" line="1466"/>
        <source>Quick Filter:</source>
        <translation>快速滤器:</translation>
    </message>
    <message>
        <source>Select files to check</source>
        <translation type="obsolete">选择要检查的文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="664"/>
        <source>Select configuration</source>
        <translation>选择配置</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="694"/>
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
        <location filename="mainwindow.cpp" line="804"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>库 &apos;%1&apos; 包含未知元素：
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="813"/>
        <source>File not found</source>
        <translation>文件未找到</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="816"/>
        <source>Bad XML</source>
        <translation>无效的 XML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="819"/>
        <source>Missing attribute</source>
        <translation>缺失属性</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="822"/>
        <source>Bad attribute value</source>
        <translation>无效的属性值</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="825"/>
        <source>Unsupported format</source>
        <translation>不支持的格式</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="828"/>
        <source>Duplicate platform type</source>
        <translation>重复的平台类型</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="831"/>
        <source>Platform type redefined</source>
        <translation>平台类型重定义</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="834"/>
        <source>Unknown element</source>
        <translation>位置元素</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="837"/>
        <source>Unknown issue</source>
        <translation>未知问题</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="842"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>选择的库 &apos;%1&apos; 加载失败。
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="865"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="865"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation>加载 %1 失败。您的 Cppcheck 安装已损坏。您可以在命令行添加 --data-dir=&lt;目录&gt; 参数来指定文件位置。请注意，&apos;--data-dir&apos; 参数应当由安装脚本使用，因此，当使用此参数时，GUI不会启动，所发生的一切只是配置了设置。</translation>
    </message>
    <message>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation type="vanished">当前结果将被清空。

打开一个新的 XML 文件将会清空当前结果。你要继续吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1214"/>
        <location filename="mainwindow.cpp" line="1394"/>
        <source>XML files (*.xml)</source>
        <translation>XML 文件(*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1216"/>
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
        <location filename="mainwindow.cpp" line="1365"/>
        <source>License</source>
        <translation>许可证</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1372"/>
        <source>Authors</source>
        <translation>作者</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation type="obsolete">XML 文件版本 2 (*.xml);;XML 文件版本 1 (*.xml);; 文本文件(*.txt);; CSV 文件(*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1387"/>
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
        <location filename="mainwindow.cpp" line="1398"/>
        <source>Text files (*.txt)</source>
        <translation>文本文件(*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1402"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 文件(*.csv)</translation>
    </message>
    <message>
        <source>Cppcheck - %1</source>
        <translation type="vanished">Cppcheck - %1</translation>
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
        <location filename="mainwindow.cpp" line="1504"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>项目文件(*.cppcheck);;所有文件(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1506"/>
        <source>Select Project File</source>
        <translation>选择项目文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="159"/>
        <location filename="mainwindow.cpp" line="1468"/>
        <location filename="mainwindow.cpp" line="1534"/>
        <location filename="mainwindow.cpp" line="1668"/>
        <source>Project:</source>
        <translation>项目:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="486"/>
        <source>No suitable files found to analyze!</source>
        <translation>没有找到合适的文件来分析!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="576"/>
        <source>C/C++ Source</source>
        <translation>C/C++ 源码</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="577"/>
        <source>Compile database</source>
        <translation>Compile database</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="578"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="579"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="582"/>
        <source>Select files to analyze</source>
        <translation>选择要分析的文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="597"/>
        <source>Select directory to analyze</source>
        <translation>选择要分析的目录</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="664"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>选择要分析的配置</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="716"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation>在目录中发现项目文件。

您想在不使用这些项目文件的情况下进行分析吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1199"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.
Do you want to proceed?</source>
        <translation>当前结果将被清除。

打开一个新的XML文件将清除当前的结果。
你想继续吗?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1319"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>分析正在运行。

您想停止分析并退出 Cppcheck 吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1385"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML 文件 (*.xml);;文本文件 (*.txt);;CSV 文件 (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1598"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>构建文件夹 &apos;%1&apos; 不能存在，创建它吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1606"/>
        <source>To check the project using addons, you need a build directory.</source>
        <translation>要使用插件检查项目，您需要一个构建目录。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1629"/>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation>导入 &apos;%1&apos; 失败，分析已停止</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1653"/>
        <source>Project files (*.cppcheck)</source>
        <translation>项目文件 (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1655"/>
        <source>Select Project Filename</source>
        <translation>选择项目文件名</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1706"/>
        <source>No project file loaded</source>
        <translation>项目文件未加载</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1775"/>
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
        <location filename="main.cpp" line="99"/>
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

语法:
    cppcheck-gui [选项] [文件或路径]

选项:
    -h, --help              打印此帮助
    -p &lt;file&gt;               打开指定的项目文件并开始检查它
    -l &lt;file&gt;               打开指定的 xml 结果文件
    -d &lt;directory&gt;          指定检查的目录，用以生成用 -l 指定的 xml 结果
    -v, --version           显示程序版本
    --data-dir=&lt;directory&gt;  这个选项用于安装脚本，这样他们可以配置数据文件所在的目录(translations, cfg)。
                            当使用这个选项时，GUI不会启动。</translation>
    </message>
    <message>
        <location filename="main.cpp" line="114"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation>Cppcheck GUI - 命令行参数</translation>
    </message>
</context>
<context>
    <name>NewSuppressionDialog</name>
    <message>
        <location filename="newsuppressiondialog.ui" line="17"/>
        <source>New suppression</source>
        <translation>新建抑制</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="25"/>
        <source>Error ID</source>
        <translation>错误 ID</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="32"/>
        <source>File name</source>
        <translation>文件名</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="42"/>
        <source>Line number</source>
        <translation>行号</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="52"/>
        <source>Symbol name</source>
        <translation>符号名</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.cpp" line="74"/>
        <source>Edit suppression</source>
        <translation>编辑抑制</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <source>Built-in</source>
        <translation type="obsolete">内置</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Native</source>
        <translation>本地</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="39"/>
        <source>Unix 32-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="40"/>
        <source>Unix 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="41"/>
        <source>Windows 32-bit ANSI</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="42"/>
        <source>Windows 32-bit Unicode</source>
        <translation></translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="43"/>
        <source>Windows 64-bit</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Project</name>
    <message>
        <source>Cppcheck</source>
        <translation type="obsolete">Cppcheck</translation>
    </message>
    <message>
        <source>Could not read the project file.</source>
        <translation type="obsolete">无法读取项目文件。</translation>
    </message>
    <message>
        <source>Could not write the project file.</source>
        <translation type="obsolete">无法写入项目文件。</translation>
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
        <source>Project</source>
        <translation type="obsolete">项目</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation>路径和定义</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="30"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <oldsource>Import Project (Visual studio / compile database)</oldsource>
        <translation>导入项目 (Visual studio / compile database/ Borland C++ Builder 6)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="231"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <oldsource>Defines must be separated by a semicolon &apos;;&apos;</oldsource>
        <translation>定义必须用分号分隔。例如：DEF1;DEF2=5;DEF3=int</translation>
    </message>
    <message>
        <source>&amp;Root:</source>
        <oldsource>Root:</oldsource>
        <translation type="obsolete">根目录:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="389"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>注意：把你自己的 .cfg 文件放在和项目文件相同的文件夹中。你应该在上面看到它们。</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="636"/>
        <source>If tags are added, you will be able to right click on warnings and set one of these tags. You can manually categorize warnings.</source>
        <translation>如果添加了标记，您将能够右键单击警告并设置其中一个标记。您可以手动对警告进行分类。</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="646"/>
        <source>Exclude source files</source>
        <translation>排除源文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="657"/>
        <source>Exclude folder...</source>
        <translation>排除文件夹...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="664"/>
        <source>Exclude file...</source>
        <translation>排除文件...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="810"/>
        <source>MISRA C 2012</source>
        <translation>MISRA C 2012</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="819"/>
        <source>MISRA rule texts</source>
        <translation>MISRA 规则文本</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="826"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;从 MISRA C 2012 PDF 的附录 A &amp;quot;指南摘要&amp;quot; 复制/粘贴文本到一个文本文件。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="833"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="73"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;您有一个选择：&lt;/p&gt;&lt;p&gt; * 分析所有的 Debug 和 Release 配置&lt;/p&gt;&lt;p&gt; * 只分析第一个匹配的 Debug 配置&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="60"/>
        <location filename="projectfiledialog.ui" line="422"/>
        <source>Browse...</source>
        <translation>浏览...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="76"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation>分析全部 Visual Studio 配置</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="113"/>
        <source>Selected VS Configurations</source>
        <translation>已选择的 VS 配置</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="147"/>
        <source>Paths:</source>
        <translation>路径:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="181"/>
        <location filename="projectfiledialog.ui" line="296"/>
        <source>Add...</source>
        <translation>添加...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="188"/>
        <location filename="projectfiledialog.ui" line="303"/>
        <location filename="projectfiledialog.ui" line="671"/>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="195"/>
        <location filename="projectfiledialog.ui" line="310"/>
        <location filename="projectfiledialog.ui" line="678"/>
        <location filename="projectfiledialog.ui" line="721"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="242"/>
        <source>Undefines:</source>
        <translation>未定义：</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="252"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation>未定义必须用分号分隔。例如：UNDEF1;UNDEF2;UNDEF3</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="265"/>
        <source>Include Paths:</source>
        <translation>包含目录：</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="362"/>
        <source>Types and Functions</source>
        <translation>类型和函数</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="403"/>
        <location filename="projectfiledialog.ui" line="458"/>
        <source>Analysis</source>
        <translation>分析</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="415"/>
        <source>This is a workfolder that Cppcheck will use for various purposes.</source>
        <translation>这是一个 Cppcheck 将用于各种目的的工作文件夹。</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="432"/>
        <source>Parser</source>
        <translation>解析器</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="438"/>
        <source>Cppcheck (built in)</source>
        <translation>Cppcheck (内建)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="484"/>
        <source>Check that each class has a safe public interface</source>
        <translation>检查每个类是否有一个安全的公共接口</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="500"/>
        <source>Limit analysis</source>
        <translation>极限分析</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="516"/>
        <source>Check code in unused templates (should be ON normally, however in theory you can safely ignore warnings in unused templates)</source>
        <oldsource>Check code in unused templates  (slower and less accurate analysis)</oldsource>
        <translation>检查未使用模板中的代码（正常情况下应该是打开，但理论上可以忽略未使用模板中的警告）</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="528"/>
        <source>Max CTU depth</source>
        <translation>最大 CTU 深度</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="845"/>
        <source>External tools</source>
        <translation>外部工具</translation>
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
        <location filename="projectfiledialog.ui" line="330"/>
        <source>Up</source>
        <translation>向上</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="337"/>
        <source>Down</source>
        <translation>向下</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="368"/>
        <source>Platform</source>
        <translation>平台</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="448"/>
        <source>Clang (experimental)</source>
        <translation>Clang (实验性的)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="464"/>
        <source>Normal analysis -- Avoid false positives.</source>
        <translation>常规分析 -- 避免误报。</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="474"/>
        <source>Bug hunting -- Generates mostly noise. The goal is to be &quot;soundy&quot; and detect most bugs.</source>
        <translation>错误搜寻 -- 生成几乎所有提示。其目的是为了检测出大多数错误并使代码更加 &quot;牢固&quot;。</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="481"/>
        <source>If you want to design your classes to be as flexible and robust as possible then the public interface must be very robust. Cppcheck will asumme that arguments can take *any* value.</source>
        <translation>如果你想要设计你的类尽可能的灵活和健壮，那么公共接口必须非常健壮。Cppcheck 将假设参数可以取 *任何* 值。</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="506"/>
        <source>Check code in headers  (should be ON normally. if you want a limited quick analysis then turn this OFF)</source>
        <translation>检查头文件中的代码（通常应该是打开的。如果您想要一个有限的快速分析，那么关掉它)）</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="559"/>
        <source>Max recursion in template instantiation</source>
        <translation>模板实例化中的最大递归</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="608"/>
        <source>Warning options</source>
        <translation>警告选项</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="614"/>
        <source>Root path:</source>
        <translation>根路径：</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="620"/>
        <source>Filepaths in warnings will be relative to this path</source>
        <translation>警告中的文件路径将相对于此路径</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="630"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>警告标志（用分号隔开）</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="409"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation>Cppcheck 构建目录 (整个程序分析、增量分析、统计数据等)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="380"/>
        <source>Libraries</source>
        <translation>库</translation>
    </message>
    <message>
        <source>Exclude</source>
        <translation type="obsolete">排除</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="703"/>
        <source>Suppressions</source>
        <translation>抑制</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="714"/>
        <source>Add</source>
        <translation>添加</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="760"/>
        <location filename="projectfiledialog.ui" line="766"/>
        <source>Addons</source>
        <translation>插件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="772"/>
        <source>Note: Addons require &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt; being installed.</source>
        <translation>注意：插件需要安装  &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="782"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="789"/>
        <source>Thread safety</source>
        <translation>线程安全</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="796"/>
        <source>Coding standards</source>
        <translation>编码标准</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="803"/>
        <source>CERT</source>
        <translation>CERT</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="858"/>
        <source>Clang analyzer</source>
        <translation>Clang analyzer</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="851"/>
        <source>Clang-tidy</source>
        <translation>Clang-tidy</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="221"/>
        <source>Defines:</source>
        <translation>定义:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="81"/>
        <source>Project file: %1</source>
        <translation>项目文件: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="455"/>
        <source>Select Cppcheck build dir</source>
        <translation>选择 Cppcheck 构建目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="751"/>
        <source>Select include directory</source>
        <translation>选择 Include 目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="731"/>
        <source>Select a directory to check</source>
        <translation>选择一个检查目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="342"/>
        <source>(no rule texts file)</source>
        <translation>(无规则文本文件)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="348"/>
        <source>Clang-tidy (not found)</source>
        <translation>Clang-tidy (未找到)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="495"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="496"/>
        <source>Compile database</source>
        <translation>Compile database</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="497"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="498"/>
        <source>Import Project</source>
        <translation>导入项目</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="771"/>
        <source>Select directory to ignore</source>
        <translation>选择忽略的目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="779"/>
        <source>Source files</source>
        <translation>源文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="780"/>
        <source>All files</source>
        <translation>全部文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="781"/>
        <source>Exclude file</source>
        <translation>排除文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="865"/>
        <source>Select MISRA rule texts file</source>
        <translation>选择 MISRA 规则文本文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="867"/>
        <source>MISRA rule texts file (%1)</source>
        <translation>MISRA 规则文本文件 (%1)</translation>
    </message>
</context>
<context>
    <name>QDialogButtonBox</name>
    <message>
        <source>Close</source>
        <translation type="obsolete">关闭</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="86"/>
        <source>Unknown language specified!</source>
        <translation>指定了未知语言！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="111"/>
        <source>Language file %1 not found!</source>
        <translation>语言文件 %1 不存在！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="117"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>无法从文件 %2 中为语言 %1 加载翻译文件</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="34"/>
        <source>line %1: Unhandled element %2</source>
        <translation>第%1行：未处理元素 %2</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="39"/>
        <source>line %1: Mandatory attribute &apos;%2&apos; missing in &apos;%3&apos;</source>
        <translation>第%1行：在 &quot;%3&quot; 中缺失的必选属性 &quot;%2&quot;</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="237"/>
        <source> (Not found)</source>
        <translation> (未找到)</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="71"/>
        <source>Thin</source>
        <translation>极细</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="73"/>
        <source>ExtraLight</source>
        <translation>更细</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="75"/>
        <source>Light</source>
        <translation>细</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="77"/>
        <source>Normal</source>
        <translation>常规</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="79"/>
        <source>Medium</source>
        <translation>中等</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="81"/>
        <source>DemiBold</source>
        <translation>较粗</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="83"/>
        <source>Bold</source>
        <translation>粗</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="85"/>
        <source>ExtraBold</source>
        <translation>更粗</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="87"/>
        <source>Black</source>
        <translation>极粗</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="71"/>
        <source>Editor Foreground Color</source>
        <translation>编辑器前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="74"/>
        <source>Editor Background Color</source>
        <translation>编辑器背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="77"/>
        <source>Highlight Background Color</source>
        <translation>高亮背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="80"/>
        <source>Line Number Foreground Color</source>
        <translation>行号前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="83"/>
        <source>Line Number Background Color</source>
        <translation>行号背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="86"/>
        <source>Keyword Foreground Color</source>
        <translation>关键字前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="89"/>
        <source>Keyword Font Weight</source>
        <translation>关键字字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="92"/>
        <source>Class Foreground Color</source>
        <oldsource>Class ForegroundColor</oldsource>
        <translation>类前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="95"/>
        <source>Class Font Weight</source>
        <translation>类字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="98"/>
        <source>Quote Foreground Color</source>
        <translation>引用前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="101"/>
        <source>Quote Font Weight</source>
        <translation>引用字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="104"/>
        <source>Comment Foreground Color</source>
        <translation>注释前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="107"/>
        <source>Comment Font Weight</source>
        <translation>注释字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="110"/>
        <source>Symbol Foreground Color</source>
        <translation>符号前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="113"/>
        <source>Symbol Background Color</source>
        <translation>符号背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="116"/>
        <source>Symbol Font Weight</source>
        <translation>符号字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="136"/>
        <source>Set to Default Light</source>
        <translation>设置为默认亮色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="138"/>
        <source>Set to Default Dark</source>
        <translation>设置为默认暗色</translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="1445"/>
        <source>File</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1445"/>
        <source>Severity</source>
        <translation>严重性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1445"/>
        <source>Line</source>
        <translation>行</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1445"/>
        <source>Summary</source>
        <translation>概要</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="158"/>
        <source>Undefined file</source>
        <translation>未定义文件</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="659"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="846"/>
        <source>Could not find file:</source>
        <translation>找不到文件：</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="850"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation>请选择文件夹 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="851"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation>选择目录 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="853"/>
        <source>Please select the directory where file is located.</source>
        <translation>请选择文件所在的目录。</translation>
    </message>
    <message>
        <source>[Inconclusive]</source>
        <translation type="obsolete">[不确定的]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="347"/>
        <source>debug</source>
        <translation>调试</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="284"/>
        <source>note</source>
        <translation>注意</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="658"/>
        <source>Recheck</source>
        <translation>重新检查</translation>
    </message>
    <message>
        <source>Copy filename</source>
        <translation type="obsolete">复制文件名</translation>
    </message>
    <message>
        <source>Copy full path</source>
        <translation type="obsolete">复制完整路径</translation>
    </message>
    <message>
        <source>Copy message</source>
        <translation type="obsolete">复制消息</translation>
    </message>
    <message>
        <source>Copy message id</source>
        <translation type="obsolete">复制消息 ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="660"/>
        <source>Hide</source>
        <translation>隐藏</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="661"/>
        <source>Hide all with id</source>
        <translation>隐藏全部 ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="681"/>
        <source>Suppress selected id(s)</source>
        <translation>抑制选择的 ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="662"/>
        <source>Open containing folder</source>
        <translation>打开包含的文件夹</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="651"/>
        <source>Edit contract..</source>
        <translation>编辑约定..</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="685"/>
        <source>Suppress</source>
        <translation>抑制</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="701"/>
        <location filename="resultstree.cpp" line="1445"/>
        <source>Tag</source>
        <translation>标记</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="703"/>
        <source>No tag</source>
        <translation>取消标记</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="745"/>
        <location filename="resultstree.cpp" line="759"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="746"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>编辑应用程序未配置。

在“首先项 / 应用程序”中为 Cppcheck 配置编辑应用程序。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="760"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>未选中默认编辑应用程序。

请在“首先项 / 应用程序”中选择默认应用程序。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="786"/>
        <source>Could not find the file!</source>
        <translation>找不到文件！</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="832"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>无法启动 %1

请检查此应用程序的路径与参数是否正确。</translation>
    </message>
    <message>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation type="obsolete">无法找到文件:
%1
请选择文件所在目录。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="854"/>
        <source>Select Directory</source>
        <translation>选择目录</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1445"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1445"/>
        <source>Inconclusive</source>
        <translation>不确定的</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1445"/>
        <source>Since date</source>
        <translation>日期</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="329"/>
        <source>style</source>
        <translation>风格</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="332"/>
        <source>error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="335"/>
        <source>warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="338"/>
        <source>performance</source>
        <translation>性能</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="341"/>
        <source>portability</source>
        <translation>移植可能性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="344"/>
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
        <translation>分析日志</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="104"/>
        <source>Warning Details</source>
        <translation>警告详情</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="158"/>
        <source>Functions</source>
        <translation>函数</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="196"/>
        <source>Variables</source>
        <translation>变量</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="217"/>
        <source>Only show variable names that contain text:</source>
        <translation>只显示包含文本的变量名：</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="164"/>
        <location filename="resultsview.ui" line="229"/>
        <source>Configured contracts:</source>
        <translation>已配置的约定：</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="178"/>
        <location filename="resultsview.ui" line="243"/>
        <source>Missing contracts:</source>
        <translation>缺失的约定：</translation>
    </message>
    <message>
        <source>No errors found, nothing to save.</source>
        <translation type="vanished">未发现错误，没有结果可保存。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="226"/>
        <location filename="resultsview.cpp" line="234"/>
        <source>Failed to save the report.</source>
        <translation>保存报告失败。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="244"/>
        <source>Print Report</source>
        <translation>打印报告</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="263"/>
        <source>No errors found, nothing to print.</source>
        <translation>没有错误发现，没有可打印内容。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="307"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%2 个文件已检查 %1 个)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="324"/>
        <location filename="resultsview.cpp" line="335"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="325"/>
        <source>No errors found.</source>
        <translation>未发现错误。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="332"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>发现错误，但它们被设为隐藏。
打开“查看”菜单，切换需要显示的错误。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="379"/>
        <location filename="resultsview.cpp" line="398"/>
        <source>Failed to read the report.</source>
        <translation>读取报告失败。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="386"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>不再支持 XML 格式版本 1。</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation type="obsolete">概要</translation>
    </message>
    <message>
        <source>Message</source>
        <translation type="obsolete">消息</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="447"/>
        <source>First included by</source>
        <translation>首次包含于</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="452"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="454"/>
        <source>Bug hunting analysis is incomplete</source>
        <translation>错误搜寻分析未完成</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="567"/>
        <source>Clear Log</source>
        <translation>清空日志</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="568"/>
        <source>Copy this Log entry</source>
        <translation>复制此日志条目</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="569"/>
        <source>Copy complete Log</source>
        <translation>复制完整日志</translation>
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
        <location filename="scratchpad.ui" line="20"/>
        <source>Copy or write some C/C++ code here:</source>
        <translation>在这里复制或输入一些 C/C++ 代码：</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="37"/>
        <source>Optionally enter a filename (mainly for automatic language detection) and click on &quot;Check&quot;:</source>
        <translation>可选择输入文件名 (主要用来自动语言检测) 然后点击 &quot;检查&quot;：</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="71"/>
        <source>filename</source>
        <translation>文件名</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="78"/>
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
        <translation type="obsolete">Include 路径:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="195"/>
        <source>Add...</source>
        <translation>添加...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>线程个数: </translation>
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
        <translation>检查不确定的错误</translation>
    </message>
    <message>
        <location filename="settings.ui" line="156"/>
        <source>Show statistics on check completion</source>
        <translation>检查完成后显示统计数据</translation>
    </message>
    <message>
        <location filename="settings.ui" line="176"/>
        <source>Show internal warnings in log</source>
        <translation>在日志中显示内建警告</translation>
    </message>
    <message>
        <location filename="settings.ui" line="287"/>
        <source>Addons</source>
        <translation>插件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="293"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation>Python 二进制 (留空将使用 PATH 路径中的 python)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="304"/>
        <location filename="settings.ui" line="345"/>
        <location filename="settings.ui" line="390"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="323"/>
        <source>MISRA addon</source>
        <translation>MISRA 插件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="331"/>
        <source>MISRA rule texts file</source>
        <translation>MISRA 规则文本文件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="338"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;从 MISRA C 2012 PDF 的附录 A &amp;quot;指南摘要&amp;quot; 复制/粘贴文本到一个文本文件。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="371"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="settings.ui" line="377"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation>Clang 路径 (留空将使用系统 PATH 路径)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="400"/>
        <source>Visual Studio headers</source>
        <translation>Visual Studio 头文件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="406"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Visual Studio 头文件路径，用分号 &apos;;&apos; 分割。&lt;/p&gt;&lt;p&gt;你可以打开一个 Visual Studio 命令提示符，输入 &amp;quot;SET INCLUDE&amp;quot;。然后复制/粘贴路径。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="436"/>
        <source>Code Editor</source>
        <translation>代码编辑器</translation>
    </message>
    <message>
        <location filename="settings.ui" line="442"/>
        <source>Code Editor Style</source>
        <translation>代码编辑器风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="448"/>
        <source>System Style</source>
        <translation>系统风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="455"/>
        <source>Default Light Style</source>
        <translation>默认浅色风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="462"/>
        <source>Default Dark Style</source>
        <translation>默认深色风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="471"/>
        <source>Custom</source>
        <translation>自定义</translation>
    </message>
    <message>
        <source>Paths</source>
        <translation type="obsolete">路径</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="obsolete">编辑</translation>
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
        <location filename="settings.ui" line="478"/>
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
        <location filename="settingsdialog.cpp" line="105"/>
        <source>N/A</source>
        <translation>N/A</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="216"/>
        <source>The executable file &quot;%1&quot; is not available</source>
        <translation>可执行文件 &quot;%1&quot; 不可用</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="227"/>
        <source>Add a new application</source>
        <translation>添加一个新的应用程序</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="260"/>
        <source>Modify an application</source>
        <translation>修改一个应用程序</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="265"/>
        <source> [Default]</source>
        <translation> [默认]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="290"/>
        <source>[Default]</source>
        <translation>[默认]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="345"/>
        <source>Select python binary</source>
        <translation>选择 python 二进制</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="352"/>
        <source>Select MISRA File</source>
        <translation>选择 MISRA 文件</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="383"/>
        <source>Select clang path</source>
        <translation>选择 clang 路径</translation>
    </message>
    <message>
        <source>Select include directory</source>
        <translation type="obsolete">选择包含目录</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="248"/>
        <location filename="statsdialog.cpp" line="150"/>
        <location filename="statsdialog.cpp" line="197"/>
        <source>Statistics</source>
        <translation>统计</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="188"/>
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
        <location filename="stats.ui" line="131"/>
        <source>Undefines:</source>
        <translation>未定义：</translation>
    </message>
    <message>
        <location filename="stats.ui" line="165"/>
        <location filename="statsdialog.cpp" line="193"/>
        <source>Previous Scan</source>
        <translation>上一次扫描</translation>
    </message>
    <message>
        <location filename="stats.ui" line="171"/>
        <source>Path Selected:</source>
        <translation>选中的路径:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Number of Files Scanned:</source>
        <translation>扫描的文件数:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="201"/>
        <source>Scan Duration:</source>
        <translation>扫描时间:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="256"/>
        <source>Errors:</source>
        <translation>错误:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="274"/>
        <source>Warnings:</source>
        <translation>警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="292"/>
        <source>Stylistic warnings:</source>
        <translation>Stylistic 警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="310"/>
        <source>Portability warnings:</source>
        <translation>可移植性警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="328"/>
        <source>Performance issues:</source>
        <translation>性能警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="346"/>
        <source>Information messages:</source>
        <translation>信息:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="363"/>
        <source>History</source>
        <translation>历史</translation>
    </message>
    <message>
        <location filename="stats.ui" line="369"/>
        <source>File:</source>
        <translation>文件：</translation>
    </message>
    <message>
        <location filename="stats.ui" line="407"/>
        <source>Copy to Clipboard</source>
        <translation>复制到剪贴板</translation>
    </message>
    <message>
        <location filename="stats.ui" line="414"/>
        <source>Pdf Export</source>
        <translation>导出 PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="126"/>
        <source>1 day</source>
        <translation>1 天</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="126"/>
        <source>%1 days</source>
        <translation>%1 天</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="128"/>
        <source>1 hour</source>
        <translation>1 小时</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="128"/>
        <source>%1 hours</source>
        <translation>%1 小时</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="130"/>
        <source>1 minute</source>
        <translation>1 分钟</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="130"/>
        <source>%1 minutes</source>
        <translation>%1 分钟</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="132"/>
        <source>1 second</source>
        <translation>1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="132"/>
        <source>%1 seconds</source>
        <translation>%1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="136"/>
        <source>0.%1 seconds</source>
        <translation>0.%1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="138"/>
        <source> and </source>
        <translation> 与 </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>Export PDF</source>
        <translation>导出 PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="187"/>
        <source>Project Settings</source>
        <translation>项目设置</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="189"/>
        <source>Paths</source>
        <translation>路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="190"/>
        <source>Include paths</source>
        <translation>包含路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="191"/>
        <source>Defines</source>
        <translation>定义</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="192"/>
        <source>Undefines</source>
        <translation>未定义</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="194"/>
        <source>Path selected</source>
        <translation>选中的路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="195"/>
        <source>Number of files scanned</source>
        <translation>扫描的文件数</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="196"/>
        <source>Scan duration</source>
        <translation>扫描时间</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="152"/>
        <location filename="statsdialog.cpp" line="198"/>
        <source>Errors</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="78"/>
        <source>File: </source>
        <translation>文件: </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="78"/>
        <source>No cppcheck build dir</source>
        <translation>没有 cppcheck 构建目录</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="154"/>
        <location filename="statsdialog.cpp" line="199"/>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="156"/>
        <location filename="statsdialog.cpp" line="200"/>
        <source>Style warnings</source>
        <translation>风格警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="158"/>
        <location filename="statsdialog.cpp" line="201"/>
        <source>Portability warnings</source>
        <translation>移植可能性警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="160"/>
        <location filename="statsdialog.cpp" line="202"/>
        <source>Performance warnings</source>
        <translation>性能警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="162"/>
        <location filename="statsdialog.cpp" line="203"/>
        <source>Information messages</source>
        <translation>信息</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="51"/>
        <source>%1 of %2 files checked</source>
        <translation>%2 个文件已检查 %1 个</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="124"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>更改用户界面语言失败:

%1

用户界面语言已被重置为英语。打开“首选项”对话框，选择任何可用的语言。</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="130"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="74"/>
        <source>inconclusive</source>
        <translation>不确定的</translation>
    </message>
</context>
<context>
    <name>VariableContractsDialog</name>
    <message>
        <location filename="variablecontractsdialog.ui" line="14"/>
        <source>Dialog</source>
        <translation>对话框</translation>
    </message>
    <message>
        <location filename="variablecontractsdialog.ui" line="20"/>
        <source>You can specify min and max value for the variable here</source>
        <translation>你可以在这里指定变量的最小值和最大值</translation>
    </message>
    <message>
        <location filename="variablecontractsdialog.ui" line="29"/>
        <source>Min</source>
        <translation>最小</translation>
    </message>
    <message>
        <location filename="variablecontractsdialog.ui" line="39"/>
        <source>Max</source>
        <translation>最大</translation>
    </message>
</context>
<context>
    <name>toFilterString</name>
    <message>
        <location filename="common.cpp" line="53"/>
        <source>All supported files (%1)</source>
        <translation>全部支持的文件 (%1)</translation>
    </message>
    <message>
        <location filename="common.cpp" line="58"/>
        <source>All files (%1)</source>
        <translation>全部文件 (%1)</translation>
    </message>
</context>
</TS>
