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
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;pcre&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;picojson&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;tinyxml2&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="applicationdialog.ui" line="23"/>
        <source>Add an application</source>
        <translation>添加应用程序</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="41"/>
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
        <location filename="applicationdialog.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>名称(&amp;N):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>可执行文件(&amp;E):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>参数(&amp;P):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="138"/>
        <source>Browse</source>
        <translation>浏览</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="63"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>可执行文件(*.exe);;所有文件(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="66"/>
        <source>Select viewer application</source>
        <translation>选择查看应用程序</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="81"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="82"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>你必须为应用程序指定名称、路径以及可选参数！</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="51"/>
        <source>Could not find the file: %1</source>
        <translation>无法找到文件: %1</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="55"/>
        <location filename="fileviewdialog.cpp" line="69"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="65"/>
        <source>Could not read the file: %1</source>
        <translation>无法读取文件: %1</translation>
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
        <location filename="helpdialog.cpp" line="79"/>
        <source>Helpfile &apos;%1&apos; was not found</source>
        <translation>帮助文件 &apos;%1&apos; 未找到</translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="81"/>
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
        <location filename="librarydialog.cpp" line="87"/>
        <location filename="librarydialog.cpp" line="159"/>
        <source>Library files (*.cfg)</source>
        <translation>库文件 (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="89"/>
        <source>Open library file</source>
        <translation>打开库文件</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="100"/>
        <location filename="librarydialog.cpp" line="112"/>
        <location filename="librarydialog.cpp" line="149"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="101"/>
        <source>Cannot open file %1.</source>
        <oldsource>Can not open file %1.</oldsource>
        <translation>无法打开文件 %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="113"/>
        <source>Failed to load %1. %2.</source>
        <translation>加载文件 %1 失败。%2。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="150"/>
        <source>Cannot save file %1.</source>
        <oldsource>Can not save file %1.</oldsource>
        <translation>无法保存文件 %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="162"/>
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
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.ui" line="667"/>
        <location filename="mainwindow.cpp" line="384"/>
        <location filename="mainwindow.cpp" line="541"/>
        <location filename="mainwindow.cpp" line="616"/>
        <location filename="mainwindow.cpp" line="721"/>
        <location filename="mainwindow.cpp" line="743"/>
        <location filename="mainwindow.cpp" line="1230"/>
        <location filename="mainwindow.cpp" line="1355"/>
        <location filename="mainwindow.cpp" line="1639"/>
        <location filename="mainwindow.cpp" line="1647"/>
        <location filename="mainwindow.cpp" line="1670"/>
        <location filename="mainwindow.cpp" line="1741"/>
        <location filename="mainwindow.cpp" line="1815"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="132"/>
        <source>&amp;File</source>
        <translation>文件(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="151"/>
        <source>&amp;View</source>
        <translation>查看(&amp;V)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="155"/>
        <source>&amp;Toolbars</source>
        <translation>工具栏(&amp;T)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="183"/>
        <source>&amp;Help</source>
        <translation>帮助(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="197"/>
        <source>C++ standard</source>
        <translation>C++ 标准</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="208"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation>&amp;C 标准</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="233"/>
        <source>&amp;Edit</source>
        <translation>编辑(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="246"/>
        <source>Standard</source>
        <translation>标准</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="264"/>
        <source>Categories</source>
        <translation>分类</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="294"/>
        <source>&amp;License...</source>
        <translation>许可证(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="299"/>
        <source>A&amp;uthors...</source>
        <translation>作者(&amp;U)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="308"/>
        <source>&amp;About...</source>
        <translation>关于(&amp;A)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="313"/>
        <source>&amp;Files...</source>
        <translation>文件(&amp;F)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="316"/>
        <location filename="mainwindow.ui" line="319"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation>分析文件</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="322"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="331"/>
        <source>&amp;Directory...</source>
        <translation>目录(&amp;D)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="334"/>
        <location filename="mainwindow.ui" line="337"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation>分析目录</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="340"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="352"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="370"/>
        <source>&amp;Stop</source>
        <translation>停止(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="373"/>
        <location filename="mainwindow.ui" line="376"/>
        <source>Stop analysis</source>
        <oldsource>Stop checking</oldsource>
        <translation>停止分析</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="379"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="388"/>
        <source>&amp;Save results to file...</source>
        <translation>保存结果到文件(&amp;S)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="391"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="396"/>
        <source>&amp;Quit</source>
        <translation>退出(&amp;Q)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="408"/>
        <source>&amp;Clear results</source>
        <translation>清空结果(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="417"/>
        <source>&amp;Preferences</source>
        <translation>首选项(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="432"/>
        <location filename="mainwindow.ui" line="435"/>
        <source>Show style warnings</source>
        <translation>显示风格警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="450"/>
        <location filename="mainwindow.ui" line="453"/>
        <source>Show errors</source>
        <translation>显示错误</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="831"/>
        <location filename="mainwindow.cpp" line="869"/>
        <source>Information</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="640"/>
        <source>Show information messages</source>
        <translation>显示信息消息</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="655"/>
        <source>Show portability warnings</source>
        <translation>显示可移植性警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="670"/>
        <source>Show Cppcheck results</source>
        <translation>显示 Cppcheck 结果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="682"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="685"/>
        <source>Show Clang results</source>
        <translation>显示 Clang 结果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="693"/>
        <source>&amp;Filter</source>
        <translation>滤器(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="696"/>
        <source>Filter results</source>
        <translation>过滤结果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="712"/>
        <source>Windows 32-bit ANSI</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="720"/>
        <source>Windows 32-bit Unicode</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="728"/>
        <source>Unix 32-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="736"/>
        <source>Unix 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="744"/>
        <source>Windows 64-bit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="814"/>
        <source>&amp;Print...</source>
        <translation>打印(&amp;P)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="817"/>
        <source>Print the Current Report</source>
        <translation>打印当前报告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="822"/>
        <source>Print Pre&amp;view...</source>
        <translation>打印预览(&amp;v)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="825"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>打开当前结果的打印预览窗口</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="833"/>
        <source>Open library editor</source>
        <translation>打开库编辑器</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="561"/>
        <source>C&amp;lose Project File</source>
        <translation>关闭项目文件(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="569"/>
        <source>&amp;Edit Project File...</source>
        <translation>编辑项目文件(&amp;E)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="581"/>
        <source>&amp;Statistics</source>
        <translation>统计(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="596"/>
        <location filename="mainwindow.ui" line="599"/>
        <source>Show warnings</source>
        <translation>显示警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="614"/>
        <location filename="mainwindow.ui" line="617"/>
        <source>Show performance warnings</source>
        <translation>显示性能警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="625"/>
        <source>Show &amp;hidden</source>
        <translation>显示隐藏项(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="458"/>
        <source>&amp;Check all</source>
        <translation>全部选中(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="84"/>
        <source>Checking for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="100"/>
        <source>Hide</source>
        <translation type="unfinished">隐藏</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="193"/>
        <source>A&amp;nalyze</source>
        <translation>分析(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="283"/>
        <source>Filter</source>
        <translation>滤器</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="349"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation>重新分析已修改的文件(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="361"/>
        <source>Reanal&amp;yze all files</source>
        <translation>重新分析全部文件(&amp;y)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="399"/>
        <source>Ctrl+Q</source>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="429"/>
        <source>Style war&amp;nings</source>
        <translation>风格警告(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="447"/>
        <source>E&amp;rrors</source>
        <translation>编辑(&amp;r)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="463"/>
        <source>&amp;Uncheck all</source>
        <translation>全部取消选中(&amp;U)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="468"/>
        <source>Collapse &amp;all</source>
        <translation>全部折叠(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="473"/>
        <source>&amp;Expand all</source>
        <translation>全部展开(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="481"/>
        <source>&amp;Standard</source>
        <translation>标准(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="484"/>
        <source>Standard items</source>
        <translation>标准项</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="489"/>
        <source>&amp;Contents</source>
        <translation>内容(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="492"/>
        <source>Open the help contents</source>
        <translation>打开帮助内容</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="495"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="500"/>
        <source>Toolbar</source>
        <translation>工具栏</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="508"/>
        <source>&amp;Categories</source>
        <translation>分类(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="511"/>
        <source>Error categories</source>
        <translation>错误分类</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="516"/>
        <source>&amp;Open XML...</source>
        <translation>打开 XML (&amp;O)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="525"/>
        <source>Open P&amp;roject File...</source>
        <translation>打开项目文件(&amp;R)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="528"/>
        <source>Ctrl+Shift+O</source>
        <translation>Ctrl+Shift+O</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="537"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation>显示便条(&amp;o)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="542"/>
        <source>&amp;New Project File...</source>
        <translation>新建项目文件(&amp;N)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="545"/>
        <source>Ctrl+Shift+N</source>
        <translation>Ctrl+Shift+N</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="550"/>
        <source>&amp;Log View</source>
        <translation>日志视图(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="553"/>
        <source>Log View</source>
        <translation>日志视图</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="593"/>
        <source>&amp;Warnings</source>
        <translation>警告(&amp;W)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="611"/>
        <source>Per&amp;formance warnings</source>
        <translation>性能警告(&amp;f)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="637"/>
        <source>&amp;Information</source>
        <translation>信息(&amp;I)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="652"/>
        <source>&amp;Portability</source>
        <translation>可移植性(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="752"/>
        <source>P&amp;latforms</source>
        <translation>平台(&amp;l)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="766"/>
        <source>C++&amp;11</source>
        <translation>C++&amp;11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="777"/>
        <source>C&amp;99</source>
        <translation>C&amp;99</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="785"/>
        <source>&amp;Posix</source>
        <translation>&amp;Posix</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="793"/>
        <source>C&amp;11</source>
        <translation>C&amp;11</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="801"/>
        <source>&amp;C89</source>
        <translation>&amp;C89</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="809"/>
        <source>&amp;C++03</source>
        <translation>&amp;C++03</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="830"/>
        <source>&amp;Library Editor...</source>
        <translation>库编辑器(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="841"/>
        <source>&amp;Auto-detect language</source>
        <translation>自动检测语言(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="849"/>
        <source>&amp;Enforce C++</source>
        <translation>&amp;Enforce C++</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="857"/>
        <source>E&amp;nforce C</source>
        <translation>E&amp;nforce C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="868"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="876"/>
        <source>Reanalyze and check library</source>
        <translation>重新分析并检查库</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="884"/>
        <source>Check configuration (defines, includes)</source>
        <translation>检查配置(defines, includes)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="892"/>
        <source>C++17</source>
        <translation>C++17</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="903"/>
        <source>C++20</source>
        <translation>C++20</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="379"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>加载编辑器应用程序设置出错。

这可能是因为 Cppcheck 不同版本间的设置有所不同。请检查(并修复)编辑器应用程序设置，否则编辑器程序可能不会正确启动。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="617"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>在选择新的文件或目录之前，你必须先关闭此项目文件！</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="119"/>
        <location filename="mainwindow.cpp" line="1511"/>
        <source>Quick Filter:</source>
        <translation>快速滤器:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="692"/>
        <source>Select configuration</source>
        <translation>选择配置</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="722"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>找到项目文件: %1

你是否想加载该项目文件？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="831"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>库 &apos;%1&apos; 包含未知元素：
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="840"/>
        <source>File not found</source>
        <translation>文件未找到</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="843"/>
        <source>Bad XML</source>
        <translation>无效的 XML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="846"/>
        <source>Missing attribute</source>
        <translation>缺失属性</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="849"/>
        <source>Bad attribute value</source>
        <translation>无效的属性值</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="852"/>
        <source>Unsupported format</source>
        <translation>不支持的格式</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="855"/>
        <source>Duplicate platform type</source>
        <translation>重复的平台类型</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="858"/>
        <source>Platform type redefined</source>
        <translation>平台类型重定义</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="861"/>
        <source>Unknown element</source>
        <translation>位置元素</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="864"/>
        <source>Unknown issue</source>
        <translation>未知问题</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="869"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>选择的库 &apos;%1&apos; 加载失败。
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="885"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="885"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation>加载 %1 失败。您的 Cppcheck 安装已损坏。您可以在命令行添加 --data-dir=&lt;目录&gt; 参数来指定文件位置。请注意，&apos;--data-dir&apos; 参数应当由安装脚本使用，因此，当使用此参数时，GUI不会启动，所发生的一切只是配置了设置。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1246"/>
        <location filename="mainwindow.cpp" line="1436"/>
        <source>XML files (*.xml)</source>
        <translation>XML 文件(*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1248"/>
        <source>Open the report file</source>
        <translation>打开报告文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1407"/>
        <source>License</source>
        <translation>许可证</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1414"/>
        <source>Authors</source>
        <translation>作者</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1429"/>
        <source>Save the report file</source>
        <translation>保存报告文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1440"/>
        <source>Text files (*.txt)</source>
        <translation>文本文件(*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1444"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 文件(*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1549"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>项目文件(*.cppcheck);;所有文件(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1551"/>
        <source>Select Project File</source>
        <translation>选择项目文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1970"/>
        <source>Install</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1974"/>
        <source>New version available: %1. %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="190"/>
        <location filename="mainwindow.cpp" line="1513"/>
        <location filename="mainwindow.cpp" line="1579"/>
        <location filename="mainwindow.cpp" line="1710"/>
        <source>Project:</source>
        <translation>项目:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="542"/>
        <source>No suitable files found to analyze!</source>
        <translation>没有找到合适的文件来分析!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="631"/>
        <source>C/C++ Source</source>
        <translation>C/C++ 源码</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="632"/>
        <source>Compile database</source>
        <translation>Compile database</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="633"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="634"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="637"/>
        <source>Select files to analyze</source>
        <translation>选择要分析的文件</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="652"/>
        <source>Select directory to analyze</source>
        <translation>选择要分析的目录</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="692"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>选择要分析的配置</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="744"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation>在目录中发现项目文件。

您想在不使用这些项目文件的情况下进行分析吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1231"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.
Do you want to proceed?</source>
        <translation>当前结果将被清除。

打开一个新的XML文件将清除当前的结果。
你想继续吗?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1351"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>分析正在运行。

您想停止分析并退出 Cppcheck 吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1393"/>
        <source>About</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1427"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML 文件 (*.xml);;文本文件 (*.txt);;CSV 文件 (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1640"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>构建文件夹 &apos;%1&apos; 不能存在，创建它吗？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1648"/>
        <source>To check the project using addons, you need a build directory.</source>
        <translation>要使用插件检查项目，您需要一个构建目录。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1671"/>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation>导入 &apos;%1&apos; 失败，分析已停止</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1695"/>
        <source>Project files (*.cppcheck)</source>
        <translation>项目文件 (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1697"/>
        <source>Select Project Filename</source>
        <translation>选择项目文件名</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1742"/>
        <source>No project file loaded</source>
        <translation>项目文件未加载</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1810"/>
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
        <location filename="main.cpp" line="102"/>
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
        <location filename="main.cpp" line="117"/>
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
        <location filename="newsuppressiondialog.cpp" line="78"/>
        <source>Edit suppression</source>
        <translation>编辑抑制</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
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
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation>项目文件</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation>路径和定义</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="30"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <oldsource>Import Project (Visual studio / compile database)</oldsource>
        <translation>导入项目 (Visual studio / compile database/ Borland C++ Builder 6)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="231"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <oldsource>Defines must be separated by a semicolon &apos;;&apos;</oldsource>
        <translation>定义必须用分号分隔。例如：DEF1;DEF2=5;DEF3=int</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="389"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>注意：把你自己的 .cfg 文件放在和项目文件相同的文件夹中。你应该在上面看到它们。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="619"/>
        <source>If tags are added, you will be able to right click on warnings and set one of these tags. You can manually categorize warnings.</source>
        <translation>如果添加了标记，您将能够右键单击警告并设置其中一个标记。您可以手动对警告进行分类。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="629"/>
        <source>Exclude source files</source>
        <translation>排除源文件</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="640"/>
        <source>Exclude folder...</source>
        <translation>排除文件夹...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="647"/>
        <source>Exclude file...</source>
        <translation>排除文件...</translation>
    </message>
    <message>
        <source>MISRA C 2012</source>
        <translation type="vanished">MISRA C 2012</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="797"/>
        <source>MISRA rule texts</source>
        <translation>MISRA 规则文本</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="804"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;从 MISRA C 2012 PDF 的附录 A &amp;quot;指南摘要&amp;quot; 复制/粘贴文本到一个文本文件。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="811"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="73"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;您有一个选择：&lt;/p&gt;&lt;p&gt; * 分析所有的 Debug 和 Release 配置&lt;/p&gt;&lt;p&gt; * 只分析第一个匹配的 Debug 配置&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="60"/>
        <location filename="projectfile.ui" line="422"/>
        <source>Browse...</source>
        <translation>浏览...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="76"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation>分析全部 Visual Studio 配置</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="113"/>
        <source>Selected VS Configurations</source>
        <translation>已选择的 VS 配置</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="147"/>
        <source>Paths:</source>
        <translation>路径:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="181"/>
        <location filename="projectfile.ui" line="296"/>
        <source>Add...</source>
        <translation>添加...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="188"/>
        <location filename="projectfile.ui" line="303"/>
        <location filename="projectfile.ui" line="654"/>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="195"/>
        <location filename="projectfile.ui" line="310"/>
        <location filename="projectfile.ui" line="661"/>
        <location filename="projectfile.ui" line="704"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="242"/>
        <source>Undefines:</source>
        <translation>未定义：</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="252"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation>未定义必须用分号分隔。例如：UNDEF1;UNDEF2;UNDEF3</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="265"/>
        <source>Include Paths:</source>
        <translation>包含目录：</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="362"/>
        <source>Types and Functions</source>
        <translation>类型和函数</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="403"/>
        <location filename="projectfile.ui" line="458"/>
        <source>Analysis</source>
        <translation>分析</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="415"/>
        <source>This is a workfolder that Cppcheck will use for various purposes.</source>
        <translation>这是一个 Cppcheck 将用于各种目的的工作文件夹。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="432"/>
        <source>Parser</source>
        <translation>解析器</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="438"/>
        <source>Cppcheck (built in)</source>
        <translation>Cppcheck (内建)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="467"/>
        <source>Check that each class has a safe public interface</source>
        <translation>检查每个类是否有一个安全的公共接口</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="483"/>
        <source>Limit analysis</source>
        <translation>极限分析</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="499"/>
        <source>Check code in unused templates (should be ON normally, however in theory you can safely ignore warnings in unused templates)</source>
        <oldsource>Check code in unused templates  (slower and less accurate analysis)</oldsource>
        <translation>检查未使用模板中的代码（正常情况下应该是打开，但理论上可以忽略未使用模板中的警告）</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="511"/>
        <source>Max CTU depth</source>
        <translation>最大 CTU 深度</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="788"/>
        <source>Misra C 2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="827"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="836"/>
        <source>CERT-INT35-C:  int precision (if size equals precision, you can leave empty)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="820"/>
        <source>Misra C++ 2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="855"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="871"/>
        <source>Bug hunting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="881"/>
        <source>External tools</source>
        <translation>外部工具</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="330"/>
        <source>Up</source>
        <translation>向上</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="337"/>
        <source>Down</source>
        <translation>向下</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="368"/>
        <source>Platform</source>
        <translation>平台</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="448"/>
        <source>Clang (experimental)</source>
        <translation>Clang (实验性的)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="464"/>
        <source>If you want to design your classes to be as flexible and robust as possible then the public interface must be very robust. Cppcheck will asumme that arguments can take *any* value.</source>
        <translation>如果你想要设计你的类尽可能的灵活和健壮，那么公共接口必须非常健壮。Cppcheck 将假设参数可以取 *任何* 值。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="489"/>
        <source>Check code in headers  (should be ON normally. if you want a limited quick analysis then turn this OFF)</source>
        <translation>检查头文件中的代码（通常应该是打开的。如果您想要一个有限的快速分析，那么关掉它)）</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="542"/>
        <source>Max recursion in template instantiation</source>
        <translation>模板实例化中的最大递归</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="591"/>
        <source>Warning options</source>
        <translation>警告选项</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="597"/>
        <source>Root path:</source>
        <translation>根路径：</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="603"/>
        <source>Filepaths in warnings will be relative to this path</source>
        <translation>警告中的文件路径将相对于此路径</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="613"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>警告标志（用分号隔开）</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="409"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation>Cppcheck 构建目录 (整个程序分析、增量分析、统计数据等)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="380"/>
        <source>Libraries</source>
        <translation>库</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="686"/>
        <source>Suppressions</source>
        <translation>抑制</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="697"/>
        <source>Add</source>
        <translation>添加</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="743"/>
        <location filename="projectfile.ui" line="749"/>
        <source>Addons</source>
        <translation>插件</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="755"/>
        <source>Note: Addons require &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt; being installed.</source>
        <translation>注意：插件需要安装  &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt;。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="765"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="772"/>
        <source>Thread safety</source>
        <translation>线程安全</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="782"/>
        <source>Coding standards</source>
        <translation>编码标准</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="848"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="865"/>
        <source>Bug hunting (Premium)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="894"/>
        <source>Clang analyzer</source>
        <translation>Clang analyzer</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="887"/>
        <source>Clang-tidy</source>
        <translation>Clang-tidy</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="221"/>
        <source>Defines:</source>
        <translation>定义:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="100"/>
        <source>Project file: %1</source>
        <translation>项目文件: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="504"/>
        <source>Select Cppcheck build dir</source>
        <translation>选择 Cppcheck 构建目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="800"/>
        <source>Select include directory</source>
        <translation>选择 Include 目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="780"/>
        <source>Select a directory to check</source>
        <translation>选择一个检查目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="388"/>
        <source>Clang-tidy (not found)</source>
        <translation>Clang-tidy (未找到)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="544"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="545"/>
        <source>Compile database</source>
        <translation>Compile database</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="546"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="547"/>
        <source>Import Project</source>
        <translation>导入项目</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="820"/>
        <source>Select directory to ignore</source>
        <translation>选择忽略的目录</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="828"/>
        <source>Source files</source>
        <translation>源文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="829"/>
        <source>All files</source>
        <translation>全部文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="830"/>
        <source>Exclude file</source>
        <translation>排除文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="914"/>
        <source>Select MISRA rule texts file</source>
        <translation>选择 MISRA 规则文本文件</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="916"/>
        <source>MISRA rule texts file (%1)</source>
        <translation>MISRA 规则文本文件 (%1)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="87"/>
        <source>Unknown language specified!</source>
        <translation>指定了未知语言！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="112"/>
        <source>Language file %1 not found!</source>
        <translation>语言文件 %1 不存在！</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="118"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>无法从文件 %2 中为语言 %1 加载翻译文件</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="35"/>
        <source>line %1: Unhandled element %2</source>
        <translation>第%1行：未处理元素 %2</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="40"/>
        <source>line %1: Mandatory attribute &apos;%2&apos; missing in &apos;%3&apos;</source>
        <translation>第%1行：在 &quot;%3&quot; 中缺失的必选属性 &quot;%2&quot;</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="259"/>
        <source> (Not found)</source>
        <translation> (未找到)</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="73"/>
        <source>Thin</source>
        <translation>极细</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="75"/>
        <source>ExtraLight</source>
        <translation>更细</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="77"/>
        <source>Light</source>
        <translation>细</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="79"/>
        <source>Normal</source>
        <translation>常规</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="81"/>
        <source>Medium</source>
        <translation>中等</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="83"/>
        <source>DemiBold</source>
        <translation>较粗</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="85"/>
        <source>Bold</source>
        <translation>粗</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="87"/>
        <source>ExtraBold</source>
        <translation>更粗</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="89"/>
        <source>Black</source>
        <translation>极粗</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="73"/>
        <source>Editor Foreground Color</source>
        <translation>编辑器前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="76"/>
        <source>Editor Background Color</source>
        <translation>编辑器背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="79"/>
        <source>Highlight Background Color</source>
        <translation>高亮背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="82"/>
        <source>Line Number Foreground Color</source>
        <translation>行号前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="85"/>
        <source>Line Number Background Color</source>
        <translation>行号背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="88"/>
        <source>Keyword Foreground Color</source>
        <translation>关键字前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="91"/>
        <source>Keyword Font Weight</source>
        <translation>关键字字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="94"/>
        <source>Class Foreground Color</source>
        <oldsource>Class ForegroundColor</oldsource>
        <translation>类前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="97"/>
        <source>Class Font Weight</source>
        <translation>类字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="100"/>
        <source>Quote Foreground Color</source>
        <translation>引用前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="103"/>
        <source>Quote Font Weight</source>
        <translation>引用字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="106"/>
        <source>Comment Foreground Color</source>
        <translation>注释前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="109"/>
        <source>Comment Font Weight</source>
        <translation>注释字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="112"/>
        <source>Symbol Foreground Color</source>
        <translation>符号前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="115"/>
        <source>Symbol Background Color</source>
        <translation>符号背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="118"/>
        <source>Symbol Font Weight</source>
        <translation>符号字体大小</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="138"/>
        <source>Set to Default Light</source>
        <translation>设置为默认亮色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="140"/>
        <source>Set to Default Dark</source>
        <translation>设置为默认暗色</translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="1429"/>
        <source>File</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1429"/>
        <source>Severity</source>
        <translation>严重性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1429"/>
        <source>Line</source>
        <translation>行</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1429"/>
        <source>Summary</source>
        <translation>概要</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="156"/>
        <source>Undefined file</source>
        <translation>未定义文件</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="656"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="839"/>
        <source>Could not find file:</source>
        <translation>找不到文件：</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="843"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation>请选择文件夹 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="844"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation>选择目录 &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="846"/>
        <source>Please select the directory where file is located.</source>
        <translation>请选择文件所在的目录。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="353"/>
        <source>debug</source>
        <translation>调试</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="278"/>
        <source>note</source>
        <translation>注意</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="655"/>
        <source>Recheck</source>
        <translation>重新检查</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="657"/>
        <source>Hide</source>
        <translation>隐藏</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="658"/>
        <source>Hide all with id</source>
        <translation>隐藏全部 ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="677"/>
        <source>Suppress selected id(s)</source>
        <translation>抑制选择的 ID</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="659"/>
        <source>Open containing folder</source>
        <translation>打开包含的文件夹</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="693"/>
        <location filename="resultstree.cpp" line="1429"/>
        <source>Tag</source>
        <translation>标记</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="695"/>
        <source>No tag</source>
        <translation>取消标记</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="737"/>
        <location filename="resultstree.cpp" line="751"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="738"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>编辑应用程序未配置。

在“首先项 / 应用程序”中为 Cppcheck 配置编辑应用程序。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="752"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>未选中默认编辑应用程序。

请在“首先项 / 应用程序”中选择默认应用程序。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="778"/>
        <source>Could not find the file!</source>
        <translation>找不到文件！</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="825"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>无法启动 %1

请检查此应用程序的路径与参数是否正确。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="847"/>
        <source>Select Directory</source>
        <translation>选择目录</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1429"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1429"/>
        <source>Inconclusive</source>
        <translation>不确定的</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1429"/>
        <source>Since date</source>
        <translation>日期</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="335"/>
        <source>style</source>
        <translation>风格</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="338"/>
        <source>error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="341"/>
        <source>warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="344"/>
        <source>performance</source>
        <translation>性能</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="347"/>
        <source>portability</source>
        <translation>移植可能性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="350"/>
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
        <location filename="resultsview.cpp" line="183"/>
        <location filename="resultsview.cpp" line="191"/>
        <source>Failed to save the report.</source>
        <translation>保存报告失败。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="201"/>
        <source>Print Report</source>
        <translation>打印报告</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="220"/>
        <source>No errors found, nothing to print.</source>
        <translation>没有错误发现，没有可打印内容。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="264"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%2 个文件已检查 %1 个)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="281"/>
        <location filename="resultsview.cpp" line="292"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="282"/>
        <source>No errors found.</source>
        <translation>未发现错误。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="289"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>发现错误，但它们被设为隐藏。
打开“查看”菜单，切换需要显示的错误。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="336"/>
        <location filename="resultsview.cpp" line="355"/>
        <source>Failed to read the report.</source>
        <translation>读取报告失败。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="343"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>不再支持 XML 格式版本 1。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="403"/>
        <source>First included by</source>
        <translation>首次包含于</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="408"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="410"/>
        <source>Bug hunting analysis is incomplete</source>
        <translation>错误搜寻分析未完成</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="484"/>
        <source>Clear Log</source>
        <translation>清空日志</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="485"/>
        <source>Copy this Log entry</source>
        <translation>复制此日志条目</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="486"/>
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
        <location filename="settings.ui" line="202"/>
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
        <location filename="settings.ui" line="163"/>
        <source>Check for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="183"/>
        <source>Show internal warnings in log</source>
        <translation>在日志中显示内建警告</translation>
    </message>
    <message>
        <location filename="settings.ui" line="294"/>
        <source>Addons</source>
        <translation>插件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="300"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation>Python 二进制 (留空将使用 PATH 路径中的 python)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="311"/>
        <location filename="settings.ui" line="352"/>
        <location filename="settings.ui" line="397"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="330"/>
        <source>MISRA addon</source>
        <translation>MISRA 插件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="338"/>
        <source>MISRA rule texts file</source>
        <translation>MISRA 规则文本文件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="345"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;从 MISRA C 2012 PDF 的附录 A &amp;quot;指南摘要&amp;quot; 复制/粘贴文本到一个文本文件。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="378"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="settings.ui" line="384"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation>Clang 路径 (留空将使用系统 PATH 路径)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="407"/>
        <source>Visual Studio headers</source>
        <translation>Visual Studio 头文件</translation>
    </message>
    <message>
        <location filename="settings.ui" line="413"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Visual Studio 头文件路径，用分号 &apos;;&apos; 分割。&lt;/p&gt;&lt;p&gt;你可以打开一个 Visual Studio 命令提示符，输入 &amp;quot;SET INCLUDE&amp;quot;。然后复制/粘贴路径。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="443"/>
        <source>Code Editor</source>
        <translation>代码编辑器</translation>
    </message>
    <message>
        <location filename="settings.ui" line="449"/>
        <source>Code Editor Style</source>
        <translation>代码编辑器风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="455"/>
        <source>System Style</source>
        <translation>系统风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="462"/>
        <source>Default Light Style</source>
        <translation>默认浅色风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="469"/>
        <source>Default Dark Style</source>
        <translation>默认深色风格</translation>
    </message>
    <message>
        <location filename="settings.ui" line="478"/>
        <source>Custom</source>
        <translation>自定义</translation>
    </message>
    <message>
        <location filename="settings.ui" line="216"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="settings.ui" line="191"/>
        <source>Applications</source>
        <translation>应用程序</translation>
    </message>
    <message>
        <location filename="settings.ui" line="209"/>
        <location filename="settings.ui" line="485"/>
        <source>Edit...</source>
        <translation>编辑...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="223"/>
        <source>Set as default</source>
        <translation>设为默认</translation>
    </message>
    <message>
        <location filename="settings.ui" line="246"/>
        <source>Reports</source>
        <translation>报告</translation>
    </message>
    <message>
        <location filename="settings.ui" line="252"/>
        <source>Save all errors when creating report</source>
        <translation>创建报告时，保存所有错误</translation>
    </message>
    <message>
        <location filename="settings.ui" line="259"/>
        <source>Save full path to files in reports</source>
        <translation>在报告中保存文件的完整路径</translation>
    </message>
    <message>
        <location filename="settings.ui" line="280"/>
        <source>Language</source>
        <translation>语言</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="114"/>
        <source>N/A</source>
        <translation>N/A</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="228"/>
        <source>The executable file &quot;%1&quot; is not available</source>
        <translation>可执行文件 &quot;%1&quot; 不可用</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="239"/>
        <source>Add a new application</source>
        <translation>添加一个新的应用程序</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="269"/>
        <source>Modify an application</source>
        <translation>修改一个应用程序</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="274"/>
        <source> [Default]</source>
        <translation> [默认]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="299"/>
        <source>[Default]</source>
        <translation>[默认]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="354"/>
        <source>Select python binary</source>
        <translation>选择 python 二进制</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="361"/>
        <source>Select MISRA File</source>
        <translation>选择 MISRA 文件</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="392"/>
        <source>Select clang path</source>
        <translation>选择 clang 路径</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="statsdialog.ui" line="14"/>
        <location filename="statsdialog.ui" line="248"/>
        <location filename="statsdialog.cpp" line="162"/>
        <location filename="statsdialog.cpp" line="209"/>
        <source>Statistics</source>
        <translation>统计</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="27"/>
        <location filename="statsdialog.cpp" line="200"/>
        <source>Project</source>
        <translation>项目</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="33"/>
        <source>Project:</source>
        <translation>项目:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="53"/>
        <source>Paths:</source>
        <translation>路径:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="85"/>
        <source>Include paths:</source>
        <translation>包含路径:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="108"/>
        <source>Defines:</source>
        <translation>定义:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="131"/>
        <source>Undefines:</source>
        <translation>未定义：</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="165"/>
        <location filename="statsdialog.cpp" line="205"/>
        <source>Previous Scan</source>
        <translation>上一次扫描</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="171"/>
        <source>Path Selected:</source>
        <translation>选中的路径:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="181"/>
        <source>Number of Files Scanned:</source>
        <translation>扫描的文件数:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="201"/>
        <source>Scan Duration:</source>
        <translation>扫描时间:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="256"/>
        <source>Errors:</source>
        <translation>错误:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="274"/>
        <source>Warnings:</source>
        <translation>警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="292"/>
        <source>Stylistic warnings:</source>
        <translation>Stylistic 警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="310"/>
        <source>Portability warnings:</source>
        <translation>可移植性警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="328"/>
        <source>Performance issues:</source>
        <translation>性能警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="346"/>
        <source>Information messages:</source>
        <translation>信息:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="363"/>
        <source>History</source>
        <translation>历史</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="369"/>
        <source>File:</source>
        <translation>文件：</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="407"/>
        <source>Copy to Clipboard</source>
        <translation>复制到剪贴板</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="414"/>
        <source>Pdf Export</source>
        <translation>导出 PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="138"/>
        <source>1 day</source>
        <translation>1 天</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="138"/>
        <source>%1 days</source>
        <translation>%1 天</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="140"/>
        <source>1 hour</source>
        <translation>1 小时</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="140"/>
        <source>%1 hours</source>
        <translation>%1 小时</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="142"/>
        <source>1 minute</source>
        <translation>1 分钟</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="142"/>
        <source>%1 minutes</source>
        <translation>%1 分钟</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="144"/>
        <source>1 second</source>
        <translation>1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="144"/>
        <source>%1 seconds</source>
        <translation>%1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="148"/>
        <source>0.%1 seconds</source>
        <translation>0.%1 秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="150"/>
        <source> and </source>
        <translation> 与 </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="177"/>
        <source>Export PDF</source>
        <translation>导出 PDF</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="199"/>
        <source>Project Settings</source>
        <translation>项目设置</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="201"/>
        <source>Paths</source>
        <translation>路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="202"/>
        <source>Include paths</source>
        <translation>包含路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="203"/>
        <source>Defines</source>
        <translation>定义</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="204"/>
        <source>Undefines</source>
        <translation>未定义</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="206"/>
        <source>Path selected</source>
        <translation>选中的路径</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="207"/>
        <source>Number of files scanned</source>
        <translation>扫描的文件数</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="208"/>
        <source>Scan duration</source>
        <translation>扫描时间</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="164"/>
        <location filename="statsdialog.cpp" line="210"/>
        <source>Errors</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="90"/>
        <source>File: </source>
        <translation>文件: </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="90"/>
        <source>No cppcheck build dir</source>
        <translation>没有 cppcheck 构建目录</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="166"/>
        <location filename="statsdialog.cpp" line="211"/>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="168"/>
        <location filename="statsdialog.cpp" line="212"/>
        <source>Style warnings</source>
        <translation>风格警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="170"/>
        <location filename="statsdialog.cpp" line="213"/>
        <source>Portability warnings</source>
        <translation>移植可能性警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="172"/>
        <location filename="statsdialog.cpp" line="214"/>
        <source>Performance warnings</source>
        <translation>性能警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="174"/>
        <location filename="statsdialog.cpp" line="215"/>
        <source>Information messages</source>
        <translation>信息</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="52"/>
        <source>%1 of %2 files checked</source>
        <translation>%2 个文件已检查 %1 个</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="125"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>更改用户界面语言失败:

%1

用户界面语言已被重置为英语。打开“首选项”对话框，选择任何可用的语言。</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="131"/>
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
