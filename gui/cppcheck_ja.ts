<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ja_JP">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>cppcheckについて</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>Version %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>CppcheckはC/C++ 静的コード解析ツールです.</translation>
    </message>
    <message>
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-%1 Cppcheck team.</source>
        <oldsource>Copyright © 2007-2021 Cppcheck team.</oldsource>
        <translation>Copyright © 2007-%1 Cppcheck team.</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>本ソフトウェアはGNU General Public License Version3 ライセンスの元で配布されます</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Cppcheckのホームページはこちら %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="115"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PCRE&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PicoJSON&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;TinyXML2&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Boost&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <oldsource>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;pcre&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;picojson&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;tinyxml2&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</oldsource>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;ライブラリの開発者に感謝を捧げます:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PCRE&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PicoJSON&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;TinyXML2&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Boost&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="applicationdialog.ui" line="23"/>
        <source>Add an application</source>
        <translation>アプリケーションの追加</translation>
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
        <translatorcomment>fix Japanese word 名前 to 表示名</translatorcomment>
        <translation>ここにエラー指摘のあるファイルを開くアプリケーションを追加できます。そのアプリケーションの表示名、実行ファイル名、コマンドラインパラメータを指定してください。

パラメータ中の以下の文字列を使用してパラメータ(Parameters:)に設定します。これらの文字列はアプリケーションが実行されたときに、適切な値に変換されます。:
(file) - エラー指摘のあるファイル
(line) - エラー指摘のある行
(message) - エラー指摘メッセージ
(severity) - エラー指摘重大度

Kate テキストエディタでファイルを開き、該当する行に移動する例:
Executable: kate
Parameters: -l(line) (file)</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>表示名(&amp;N):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>実行ファイルのパス(&amp;E):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>パラメータ(&amp;P):</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui" line="138"/>
        <source>Browse</source>
        <translation>参照</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="65"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>実行ファイル (*.exe);;すべてのファイル(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="68"/>
        <source>Select viewer application</source>
        <translation>表示アプリケーションの選択</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="83"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="84"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>アプリケーションの表示名と実行ファイルのパスと(オプションの)引数を指定してください!</translation>
    </message>
</context>
<context>
    <name>ComplianceReportDialog</name>
    <message>
        <location filename="compliancereportdialog.ui" line="14"/>
        <source>Compliance Report</source>
        <translation>コンプライアンスレポート</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="29"/>
        <source>Project name</source>
        <translation>プロジェクト名</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="22"/>
        <source>Project version</source>
        <translation>プロジェクトバージョン</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="42"/>
        <source>Coding Standard</source>
        <translation>コーディング標準</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="50"/>
        <source>Misra C</source>
        <translation>MISRA C</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="55"/>
        <source>Cert C</source>
        <translation>CERT C</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="60"/>
        <source>Cert C++</source>
        <translation>Cert C++</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui" line="70"/>
        <source>List of files with md5 checksums</source>
        <translation>md5チェックサムつきファイルリスト</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="133"/>
        <source>Compliance report</source>
        <translation>コンプライアンスレポート</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="135"/>
        <source>HTML files (*.html)</source>
        <translation>HTMLファイル(*.html)</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="176"/>
        <location filename="compliancereportdialog.cpp" line="239"/>
        <source>Save compliance report</source>
        <translation>コンプライアンスレポートの保存</translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="177"/>
        <source>Failed to import &apos;%1&apos; (%2), can not show files in compliance report</source>
        <translation>&apos;%1&apos; (%2)のインポートに失敗しました。コンプライアンスレポートに表示できません</translation>
    </message>
    <message>
        <source>Failed to import &apos;%1&apos;, can not show files in compliance report</source>
        <translation type="vanished">&apos;%1&apos;のインポートに失敗しました、コンプライアンスレポートでファイルを表示できません</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="53"/>
        <source>Could not find the file: %1</source>
        <translation>ファイル：%1 が見つかりません</translation>
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
        <translation>ファイル：%1 が読み込めません</translation>
    </message>
</context>
<context>
    <name>HelpDialog</name>
    <message>
        <location filename="helpdialog.ui" line="14"/>
        <source>Cppcheck GUI help</source>
        <translation>Cppcheck GUI ヘルプ</translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="34"/>
        <source>Contents</source>
        <translation>目次</translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="44"/>
        <source>Index</source>
        <translation>インデックス</translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="83"/>
        <source>Helpfile &apos;%1&apos; was not found</source>
        <translation>ヘルプファイル &apos;%1&apos; が見つかりません</translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="85"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>LibraryAddFunctionDialog</name>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="23"/>
        <source>Add function</source>
        <translation>関数の追加</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="34"/>
        <source>Function name(s)</source>
        <translation>関数の名称</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui" line="44"/>
        <source>Number of arguments</source>
        <translation>引数の数</translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui" line="14"/>
        <source>Library Editor</source>
        <translation>ライブラリエディタ</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="22"/>
        <source>Open</source>
        <translation>開く</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="29"/>
        <source>Save</source>
        <translation>保存する</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="36"/>
        <source>Save as</source>
        <translation>別名で保存する</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="62"/>
        <source>Functions</source>
        <translation>関数</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <source>Sort</source>
        <translation>ソート</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <source>Add</source>
        <translation>追加</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="131"/>
        <source>Filter:</source>
        <translation>フィルタ:</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="164"/>
        <source>Comments</source>
        <translation>コメント</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="204"/>
        <source>noreturn</source>
        <translation>noreturn(返り値なし)</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="212"/>
        <source>False</source>
        <translation>False(偽)</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="217"/>
        <source>True</source>
        <translation>True(真)</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="222"/>
        <source>Unknown</source>
        <translation>Unknown(不明)</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="232"/>
        <source>return value must be used</source>
        <translation>返り値は使用されなければならない</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="239"/>
        <source>ignore function in leaks checking</source>
        <translation>リークの解析中に無視する関数</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="246"/>
        <source>Arguments</source>
        <translation>Arguments(引数)</translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="258"/>
        <source>Edit</source>
        <translation>編集</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="98"/>
        <location filename="librarydialog.cpp" line="170"/>
        <source>Library files (*.cfg)</source>
        <translation>ライブラリファイル(*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="100"/>
        <source>Open library file</source>
        <translation>ライブラリファイルを開く</translation>
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
        <translation>ファイルが見つかりません %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="124"/>
        <source>Failed to load %1. %2.</source>
        <translation>読み込みに失敗しました(%1.%2)。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="161"/>
        <source>Cannot save file %1.</source>
        <oldsource>Can not save file %1.</oldsource>
        <translation>ファイルが保存できません %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="173"/>
        <source>Save the library as</source>
        <translation>このライブラリに名前をつけて保存する</translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui" line="14"/>
        <source>Edit argument</source>
        <translation>引数の編集</translation>
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
&lt;p&gt;ブール値は許可されていますか?  例えば、比較の結果または &apos;!&apos; 演算子&lt;/p&gt;
&lt;p&gt;典型的に引数がポインタやサイズを表す場合、これを設定します。&lt;/p&gt;
&lt;p&gt;例:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // 最後の引数は、ブール型であってはならない
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="28"/>
        <source>Not bool</source>
        <translatorcomment>非ブール型</translatorcomment>
        <translation>非bool値</translation>
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
&lt;p&gt;null値が許可されていますか？&lt;/p&gt;
&lt;p&gt;典型的には、nullを渡してはいけないポインタのパラメータに使用してください。&lt;/p&gt;
&lt;p&gt;例:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // x も y も null であってはならない。&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="43"/>
        <source>Not null</source>
        <translation>非NULL</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="50"/>
        <source>Not uninit</source>
        <translation>未初期化</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="57"/>
        <source>String</source>
        <translation>文字列</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="70"/>
        <source>Format string</source>
        <translation>フォーマット文字列</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="92"/>
        <source>Min size of buffer</source>
        <translation>バッファの最小サイズ</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="101"/>
        <location filename="libraryeditargdialog.ui" line="203"/>
        <source>Type</source>
        <translation>Type(型)</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="109"/>
        <location filename="libraryeditargdialog.ui" line="214"/>
        <source>None</source>
        <translation>None(無)</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="114"/>
        <location filename="libraryeditargdialog.ui" line="219"/>
        <source>argvalue</source>
        <translation>argvalue(引数の値)</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="119"/>
        <location filename="libraryeditargdialog.ui" line="224"/>
        <source>mul</source>
        <translation>mul(積)</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="124"/>
        <location filename="libraryeditargdialog.ui" line="229"/>
        <source>strlen</source>
        <translation>strlen(文字数)</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="132"/>
        <location filename="libraryeditargdialog.ui" line="237"/>
        <source>Arg</source>
        <translation>Arg(引数)</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="159"/>
        <location filename="libraryeditargdialog.ui" line="264"/>
        <source>Arg2</source>
        <translation>Arg2(第二引数)</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="194"/>
        <source>and</source>
        <translation>and(和)</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui" line="310"/>
        <source>Valid values</source>
        <translation>妥当な値</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.ui" line="685"/>
        <location filename="mainwindow.cpp" line="448"/>
        <location filename="mainwindow.cpp" line="635"/>
        <location filename="mainwindow.cpp" line="715"/>
        <location filename="mainwindow.cpp" line="820"/>
        <location filename="mainwindow.cpp" line="842"/>
        <location filename="mainwindow.cpp" line="1422"/>
        <location filename="mainwindow.cpp" line="1551"/>
        <location filename="mainwindow.cpp" line="1859"/>
        <location filename="mainwindow.cpp" line="1867"/>
        <location filename="mainwindow.cpp" line="1915"/>
        <location filename="mainwindow.cpp" line="1924"/>
        <location filename="mainwindow.cpp" line="1996"/>
        <location filename="mainwindow.cpp" line="2070"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="132"/>
        <source>&amp;File</source>
        <translation>ファイル(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="152"/>
        <source>&amp;View</source>
        <translation>表示(&amp;V)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="156"/>
        <source>&amp;Toolbars</source>
        <translation>ツールバー(&amp;T)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="198"/>
        <source>&amp;Help</source>
        <translation>ヘルプ(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="212"/>
        <source>C++ standard</source>
        <translation>C++標準</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="224"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation>&amp;C標準</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="251"/>
        <source>&amp;Edit</source>
        <translation>編集(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="264"/>
        <source>Standard</source>
        <translation>言語規格</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="282"/>
        <source>Categories</source>
        <translation>カテゴリ</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="312"/>
        <source>&amp;License...</source>
        <translation>ライセンス(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="317"/>
        <source>A&amp;uthors...</source>
        <translation>作者(&amp;u)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="326"/>
        <source>&amp;About...</source>
        <translation>Cppcheckについて(&amp;A)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="331"/>
        <source>&amp;Files...</source>
        <translation>ファイル選択(&amp;F)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="334"/>
        <location filename="mainwindow.ui" line="337"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation>ファイルをチェックする</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="340"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="349"/>
        <source>&amp;Directory...</source>
        <translation>ディレクトリ選択(&amp;D)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="352"/>
        <location filename="mainwindow.ui" line="355"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation>ディレクトリをチェックする</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="358"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="370"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="388"/>
        <source>&amp;Stop</source>
        <translation>停止(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="391"/>
        <location filename="mainwindow.ui" line="394"/>
        <source>Stop analysis</source>
        <oldsource>Stop checking</oldsource>
        <translation>チェックを停止する</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="397"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="406"/>
        <source>&amp;Save results to file...</source>
        <translation>結果をファイルに保存(&amp;S)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="409"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="414"/>
        <source>&amp;Quit</source>
        <translation>終了(&amp;Q)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="426"/>
        <source>&amp;Clear results</source>
        <translation>結果をクリア(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="435"/>
        <source>&amp;Preferences</source>
        <translation>設定(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="450"/>
        <location filename="mainwindow.ui" line="453"/>
        <source>Show style warnings</source>
        <translation>スタイル警告を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="468"/>
        <location filename="mainwindow.ui" line="471"/>
        <location filename="mainwindow.cpp" line="2294"/>
        <source>Show errors</source>
        <translation>エラーを表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="946"/>
        <location filename="mainwindow.cpp" line="987"/>
        <source>Information</source>
        <translation>情報</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="658"/>
        <source>Show information messages</source>
        <translation>情報メッセージを表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="673"/>
        <source>Show portability warnings</source>
        <translation>移植可能性の問題を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="688"/>
        <source>Show Cppcheck results</source>
        <translation>Cppcheck結果を表示する</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="700"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="703"/>
        <source>Show Clang results</source>
        <translation>Clangの結果を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="711"/>
        <source>&amp;Filter</source>
        <translation>フィルター(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="714"/>
        <source>Filter results</source>
        <translation>フィルタ結果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="730"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSIエンコード</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="738"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="746"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="754"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="762"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="848"/>
        <source>&amp;Print...</source>
        <translation>印刷(&amp;P)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="851"/>
        <source>Print the Current Report</source>
        <translation>現在のレポートを印刷</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="856"/>
        <source>Print Pre&amp;view...</source>
        <translation>印刷プレビュー(&amp;v)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="859"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>現在のレポートをプレビュー表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="867"/>
        <source>Open library editor</source>
        <translation>ライブラリエディタを開く</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="579"/>
        <source>C&amp;lose Project File</source>
        <translation>プロジェクトを閉じる(&amp;l)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="587"/>
        <source>&amp;Edit Project File...</source>
        <translation>プロジェクトの編集(&amp;E)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="599"/>
        <source>&amp;Statistics</source>
        <translation>統計情報(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="614"/>
        <location filename="mainwindow.ui" line="617"/>
        <location filename="mainwindow.cpp" line="2295"/>
        <source>Show warnings</source>
        <translation>警告を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="632"/>
        <location filename="mainwindow.ui" line="635"/>
        <source>Show performance warnings</source>
        <translation>パフォーマンス警告を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="643"/>
        <source>Show &amp;hidden</source>
        <translation>非表示を表示(&amp;h)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="476"/>
        <source>&amp;Check all</source>
        <translation>すべてのエラーを表示(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="84"/>
        <source>Checking for updates</source>
        <translation>更新の確認</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="100"/>
        <source>Hide</source>
        <translation>非表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="164"/>
        <location filename="mainwindow.ui" line="969"/>
        <source>Report</source>
        <translation>レポート</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="208"/>
        <source>A&amp;nalyze</source>
        <translation>チェック(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="301"/>
        <source>Filter</source>
        <translation>フィルター</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="367"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation>変更ありファイルを再解析(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="379"/>
        <source>Reanal&amp;yze all files</source>
        <translation>全ファイル再解析(&amp;y)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="417"/>
        <source>Ctrl+Q</source>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="447"/>
        <source>Style war&amp;nings</source>
        <translation>スタイル警告(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="465"/>
        <source>E&amp;rrors</source>
        <translation>エラー(&amp;r)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="481"/>
        <source>&amp;Uncheck all</source>
        <translation>すべてのエラーを非表示(&amp;U)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="486"/>
        <source>Collapse &amp;all</source>
        <translation>ツリーを折り畳む(&amp;a)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="491"/>
        <source>&amp;Expand all</source>
        <translation>ツリーを展開(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="499"/>
        <source>&amp;Standard</source>
        <translation>言語規格(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="502"/>
        <source>Standard items</source>
        <translation>標準項目</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="507"/>
        <source>&amp;Contents</source>
        <translation>コンテンツ(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="510"/>
        <source>Open the help contents</source>
        <translation>ヘルプファイルを開く</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="513"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="518"/>
        <source>Toolbar</source>
        <translation>ツールバー</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="526"/>
        <source>&amp;Categories</source>
        <translation>カテゴリ(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="529"/>
        <source>Error categories</source>
        <translation>エラーカテゴリ</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="534"/>
        <source>&amp;Open XML...</source>
        <translation>XMLを開く(&amp;O)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="543"/>
        <source>Open P&amp;roject File...</source>
        <translation>プロジェクトを開く(&amp;R)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="546"/>
        <source>Ctrl+Shift+O</source>
        <translation>Ctrl+Shift+O</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="555"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation>スクラッチパッドを表示(&amp;o)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="560"/>
        <source>&amp;New Project File...</source>
        <translation>新規プロジェクト(&amp;N)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="563"/>
        <source>Ctrl+Shift+N</source>
        <translation>Ctrl+Shift+N</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="568"/>
        <source>&amp;Log View</source>
        <translation>ログを表示(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="571"/>
        <source>Log View</source>
        <translation>ログ表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="611"/>
        <source>&amp;Warnings</source>
        <translation>警告(&amp;W)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="629"/>
        <source>Per&amp;formance warnings</source>
        <translation>パフォーマンス警告(&amp;f)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="655"/>
        <source>&amp;Information</source>
        <translation>情報(&amp;I)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="670"/>
        <source>&amp;Portability</source>
        <translation>移植可能性(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="770"/>
        <source>P&amp;latforms</source>
        <translation>プラットフォーム(&amp;l)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="784"/>
        <source>C++&amp;11</source>
        <translation>C++11(&amp;1)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="795"/>
        <source>C&amp;99</source>
        <translation>C99(&amp;9)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="803"/>
        <source>&amp;Posix</source>
        <translation>Posix(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="811"/>
        <source>C&amp;11</source>
        <translation>C11(&amp;1)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="835"/>
        <source>&amp;C89</source>
        <translation>C89(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="843"/>
        <source>&amp;C++03</source>
        <translation>C++03(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="864"/>
        <source>&amp;Library Editor...</source>
        <translation>ライブラリエディタ(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="875"/>
        <source>&amp;Auto-detect language</source>
        <translation>自動言語検出(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="883"/>
        <source>&amp;Enforce C++</source>
        <translation>C++ 強制(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="891"/>
        <source>E&amp;nforce C</source>
        <translation>C 強制(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="902"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="910"/>
        <source>Reanalyze and check library</source>
        <translation>ライブラリを再チェックする</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="918"/>
        <source>Check configuration (defines, includes)</source>
        <translation>チェックの設定(define、インクルード)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="926"/>
        <source>C++17</source>
        <translation>C++17</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="937"/>
        <source>C++20</source>
        <translation>C++20</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="964"/>
        <source>Compliance report...</source>
        <translation>コンプライアンスレポート...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="977"/>
        <source>Normal</source>
        <translation>ノーマル</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="985"/>
        <source>Misra C</source>
        <translation>MISRA C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="993"/>
        <source>Misra C++ 2008</source>
        <translation>MISRA C++ 2008</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1001"/>
        <source>Cert C</source>
        <translation>CERT C</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1009"/>
        <source>Cert C++</source>
        <translation>Cert C++</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1017"/>
        <source>Misra C++ 2023</source>
        <translation>MISRA C++ 2023</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="1025"/>
        <source>Autosar</source>
        <translation>AUTOSAR</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="443"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>エディタアプリの設定の読み込みで問題が発生しました。

Cppcheckの古いバージョンの設定には互換性がありません。エディタアプリケーションの設定を確認して修正してください、そうしないと正しく起動できないかもしれません。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="716"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>新しいファイル／ディレクトリをチェックするには現在のプロジェクトを閉じてください!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="145"/>
        <location filename="mainwindow.cpp" line="1729"/>
        <source>Quick Filter:</source>
        <translation>クイックフィルタ：</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Select configuration</source>
        <translation>コンフィグレーションの選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="821"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>プロジェクトファイルを検出しました: %1

現在のプロジェクトの代わりにこのプロジェクトファイルを読み込んでもかまいませんか？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="946"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>このライブラリ &apos;%1&apos; には次の不明な要素が含まれています。
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="955"/>
        <source>File not found</source>
        <translation>ファイルがありません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="958"/>
        <source>Bad XML</source>
        <translation>不正なXML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="961"/>
        <source>Missing attribute</source>
        <translation>属性がありません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="964"/>
        <source>Bad attribute value</source>
        <translation>不正な属性があります</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="967"/>
        <source>Unsupported format</source>
        <translation>サポートされていないフォーマット</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="970"/>
        <source>Duplicate platform type</source>
        <translation>プラットフォームの種類が重複しています</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="973"/>
        <source>Platform type redefined</source>
        <translation>プラットフォームの種類が再定義されました</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="979"/>
        <source>Unknown element</source>
        <translation>不明な要素</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="982"/>
        <source>Unknown issue</source>
        <translation>不明な課題</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="987"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>選択したライブラリの読み込みに失敗しました &apos;%1&apos;
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1046"/>
        <location filename="mainwindow.cpp" line="1056"/>
        <location filename="mainwindow.cpp" line="1066"/>
        <location filename="mainwindow.cpp" line="1153"/>
        <source>Error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation type="vanished">%1のロードに失敗しました。あなたの Cppcheck は正しくインストールされていません。あなたは --data-dir=&lt;directory&gt; コマンドラインオプションでロードするファイルの場所を指定できます。ただし、この --data-dir はインストールスクリプトによってサポートされており、GUI版ではサポートされていません。全ての設定は調整済みでなければなりません。</translation>
    </message>
    <message>
        <source>Failed to load %1 - %2</source>
        <translation type="vanished">%1 - %2 の読み込みに失敗</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1438"/>
        <location filename="mainwindow.cpp" line="1632"/>
        <source>XML files (*.xml)</source>
        <translation>XML ファイル (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1440"/>
        <source>Open the report file</source>
        <translation>レポートを開く</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1603"/>
        <source>License</source>
        <translation>ライセンス</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1610"/>
        <source>Authors</source>
        <translation>作者</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1625"/>
        <source>Save the report file</source>
        <translation>レポートを保存</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1636"/>
        <source>Text files (*.txt)</source>
        <translation>テキストファイル (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1640"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV形式ファイル (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1663"/>
        <source>Cannot generate a compliance report right now, an analysis must finish successfully. Try to reanalyze the code and ensure there are no critical errors.</source>
        <translation>コンプライアンスレポートをすぐに生成できません。解析が完了し成功していなければなりません。コードを再解析して、致命的なエラーがないことを確認してください。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1768"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>プロジェクトファイル (*.cppcheck);;すべてのファイル(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1770"/>
        <source>Select Project File</source>
        <translation>プロジェクトファイルを選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1900"/>
        <source>Failed to open file</source>
        <translation>ファイルを開くのに失敗しました</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1903"/>
        <source>Unknown project file format</source>
        <translation>プロジェクトファイルの形式が不明です</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1906"/>
        <source>Failed to import project file</source>
        <translation>プロジェクトファイルのインポートに失敗しました</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1916"/>
        <source>Failed to import &apos;%1&apos;: %2

Analysis is stopped.</source>
        <translation>インポートに失敗&apos;%1&apos;: %2

解析を停止しました。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1925"/>
        <source>Failed to import &apos;%1&apos; (%2), analysis is stopped</source>
        <translation>&apos;%1&apos; (%2) のインポートに失敗しました。解析は停止</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2225"/>
        <source>Install</source>
        <translation>インストール</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2229"/>
        <source>New version available: %1. %2</source>
        <translation>新しいバージョンが利用可能です。: %1. %2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="226"/>
        <location filename="mainwindow.cpp" line="1731"/>
        <location filename="mainwindow.cpp" line="1798"/>
        <location filename="mainwindow.cpp" line="1964"/>
        <source>Project:</source>
        <translation>プロジェクト:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="636"/>
        <source>No suitable files found to analyze!</source>
        <translation>チェック対象のファイルがみつかりません!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="730"/>
        <source>C/C++ Source</source>
        <translation>C/C++のソースコード</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="731"/>
        <source>Compile database</source>
        <translation>コンパイルデータベース</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="732"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="733"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="736"/>
        <source>Select files to analyze</source>
        <translation>チェック対象のファイルを選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="751"/>
        <source>Select directory to analyze</source>
        <translation>チェックするディレクトリを選択してください</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>チェックの設定を選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="843"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation>ディレクトリ内にプロジェクトファイルがありました。

みつかったプロジェクトファイルを使用せずにチェックしますか?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="976"/>
        <source>Duplicate define</source>
        <translation>重複した定義</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="998"/>
        <source>File not found: &apos;%1&apos;</source>
        <translation>ファイルがありません: &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1024"/>
        <source>Failed to load/setup addon %1: %2</source>
        <translation>アドオンの読み込みまたは設定に失敗 %1 - %2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1046"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.

Analysis is aborted.</source>
        <translation>%1のロードに失敗しました。あなたの Cppcheck は正しくインストールされていません。あなたは --data-dir=&lt;directory&gt; コマンドラインオプションを使ってこのファイルの場所を指定できます。ただし、この --data-dir はインストールスクリプトによって使用されていなければなりません。またGUI版はこれを使用しません。さらに、全ての設定は調整済みでなければなりません。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1056"/>
        <source>Failed to load %1 - %2

Analysis is aborted.</source>
        <translation>読み込みに失敗 %1 - %2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1066"/>
        <location filename="mainwindow.cpp" line="1153"/>
        <source>%1

Analysis is aborted.</source>
        <translation>%1

解析は中止した。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1423"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.
Do you want to proceed?</source>
        <translation>現在の結果を作成します。

新しくXMLファイルを開くと現在の結果が削除されます。実行しますか？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1547"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>チェック中です。

チェックを中断して、Cppcheckを終了しますか?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1589"/>
        <source>About</source>
        <translation>CppCheckについて</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1623"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML ファイル (*.xml);;テキストファイル (*.txt);;CSVファイル (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1860"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>ビルドディレクトリ&apos;%1&apos;がありません。作成しますか?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1868"/>
        <source>To check the project using addons, you need a build directory.</source>
        <translation>アドオンを使用してプロジェクトをチェックするためには、ビルドディレクトリが必要です。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2267"/>
        <source>Show Mandatory</source>
        <translation>必須を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2268"/>
        <source>Show Required</source>
        <translation>要求を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2269"/>
        <source>Show Advisory</source>
        <translation>推奨を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2270"/>
        <source>Show Document</source>
        <translation>ドキュメントを表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2287"/>
        <source>Show L1</source>
        <translation>L1を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2288"/>
        <source>Show L2</source>
        <translation>L2を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2289"/>
        <source>Show L3</source>
        <translation>L3を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2296"/>
        <source>Show style</source>
        <translation>スタイルを表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2297"/>
        <source>Show portability</source>
        <translation>移植可能性を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2298"/>
        <source>Show performance</source>
        <translation>パフォーマンスを表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2299"/>
        <source>Show information</source>
        <translation>情報を表示</translation>
    </message>
    <message>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation type="vanished">&apos;%1&apos;のインポートに失敗しました。(チェック中断)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1949"/>
        <source>Project files (*.cppcheck)</source>
        <translation>プロジェクトファイル (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1951"/>
        <source>Select Project Filename</source>
        <translation>プロジェクトファイル名を選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1997"/>
        <source>No project file loaded</source>
        <translation>プロジェクトファイルが読み込まれていません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2065"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>このプロジェクトファイル %1 が見つかりません。
最近使用したプロジェクトのリストからこのファイルを取り除きますか？</translation>
    </message>
    <message>
        <location filename="main.cpp" line="106"/>
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

シンタックス:
    cppcheck-gui [OPTIONS] [files または paths]

オプション:
    -h, --help              このヘルプを表示する。
    -p &lt;file&gt;               指定のプロジェクトファイルを開き、チェックを開始する。
    -l &lt;file&gt;               指定の、結果XMLファイルを開く
    -d &lt;directory&gt;          フォルダを指定してチェックする。これは -l オプションで 指定した、結果XMLファイルを生成する。
    -v, --version           バージョンを表示する。
    --data-dir=&lt;directory&gt;  GUI のデータファイル(翻訳やcfg)のあるディレクトリを指定する。このオプションを指定した場合、GUIで起動しません。</translation>
    </message>
    <message>
        <location filename="main.cpp" line="121"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation>Cppcheck GUI - コマンドラインパラメータ</translation>
    </message>
</context>
<context>
    <name>NewSuppressionDialog</name>
    <message>
        <location filename="newsuppressiondialog.ui" line="17"/>
        <source>New suppression</source>
        <translation>新しい指摘の抑制</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="25"/>
        <source>Error ID</source>
        <translation>エラーID</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="32"/>
        <source>File name</source>
        <translation>ファイル名</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="42"/>
        <source>Line number</source>
        <translation>行数</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="52"/>
        <source>Symbol name</source>
        <translation>シンボル名</translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.cpp" line="82"/>
        <source>Edit suppression</source>
        <translation>抑制の編集</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Native</source>
        <translation>ネイティブ</translation>
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
        <translation>Windows 32-bit ANSIエンコード</translation>
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
        <source>Project File</source>
        <translation>プロジェクトファイル</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation>パスと定義</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="30"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <oldsource>Import Project (Visual studio / compile database)</oldsource>
        <translation>プロジェクトのインポート(Visual studio / compile database Borland C++ Builder 6))</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="231"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <oldsource>Defines must be separated by a semicolon &apos;;&apos;</oldsource>
        <translation>定義(Define)はセミコロン&apos;;&apos;で区切る必要があります。 例: DEF1;DEF2=5;DEF3=int</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="389"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>カスタマイズした cfgファイルを同じフォルダにプロジェクトファイルとして保存してください。ここに表示できるようになります。</translation>
    </message>
    <message>
        <source>MISRA C 2012</source>
        <translation type="vanished">MISRA C 2012</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="858"/>
        <source>MISRA rule texts</source>
        <translation>MISRA ルールテキスト</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="865"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;MISRA C 2012 pdfのAppendix A &amp;quot;Summary of guidelines&amp;quot; からテキストをコピーペーストしてください。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="872"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="73"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;選択済み:&lt;/p&gt;&lt;p&gt; * 全Debug と Release設定をチェックする&lt;/p&gt;&lt;p&gt; * 最初にマッチした Debug 設定のみチェックする&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="60"/>
        <location filename="projectfile.ui" line="422"/>
        <source>Browse...</source>
        <translation>参照...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="76"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation>Visual Studioの全ての設定をチェックする</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="113"/>
        <source>Selected VS Configurations</source>
        <translation>選択したVS設定</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="147"/>
        <source>Paths:</source>
        <translation>パス:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="181"/>
        <location filename="projectfile.ui" line="296"/>
        <source>Add...</source>
        <translation>追加...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="188"/>
        <location filename="projectfile.ui" line="303"/>
        <location filename="projectfile.ui" line="677"/>
        <source>Edit</source>
        <translation>編集</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="195"/>
        <location filename="projectfile.ui" line="310"/>
        <location filename="projectfile.ui" line="684"/>
        <location filename="projectfile.ui" line="727"/>
        <source>Remove</source>
        <translation>取り除く</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="242"/>
        <source>Undefines:</source>
        <translation>定義取り消し(Undefines):</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="252"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation>定義の取り消しはセミコロンで区切ります。例: UNDEF1;UNDEF2;UNDEF3</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="265"/>
        <source>Include Paths:</source>
        <translation>インクルードパス:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="362"/>
        <source>Types and Functions</source>
        <translation>型と関数</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="403"/>
        <location filename="projectfile.ui" line="481"/>
        <source>Analysis</source>
        <translation>チェック</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="415"/>
        <source>This is a workfolder that Cppcheck will use for various purposes.</source>
        <translation>Cppcheckがさまざまな目的で使用するワークディレクトリ。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="432"/>
        <source>Parser</source>
        <translation>パーサー</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="438"/>
        <source>Cppcheck (built in)</source>
        <translation>Cppcheckビルトイン</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="458"/>
        <source>Check level</source>
        <translation>チェックレベル</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="464"/>
        <source>Normal -- meant for normal analysis in CI. Analysis should finish in reasonable time.</source>
        <translation>#N 通常 -- CIでの通常の解析を意味します。解析が合理的な時間内に完了すべきです。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="471"/>
        <source>Exhaustive -- meant for nightly builds etc. Analysis time can be longer (10x slower than compilation is OK).</source>
        <translation>#E 徹底的 -- ナイトリービルド等を意味します。解析時間はより長くなることがあります (コンパイルの10倍以上時間がかかってもよい)。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="512"/>
        <source>Check code in headers  (should be ON normally. if you want a limited quick analysis then turn this OFF)</source>
        <translation>ヘッダファイルのコードもチェック  (通常はONにしてください、制限するときのみOFF)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="642"/>
        <source>If tags are added, you will be able to right click on warnings and set one of these tags. You can manually categorize warnings.</source>
        <translation>タグが追加された場合、警告上で右クリックしてそれらのタグの中の一つを設定できます。警告を分類できます。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="652"/>
        <source>Exclude source files</source>
        <translation>除外するソースファイル</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="663"/>
        <source>Exclude folder...</source>
        <translation>フォルダで除外...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="670"/>
        <source>Exclude file...</source>
        <translation>ファイルで除外...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="490"/>
        <source>Check that each class has a safe public interface</source>
        <translation>クラスが安全で公開されたインターフェースをもっているか確認</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="506"/>
        <source>Limit analysis</source>
        <translation>解析の制限</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="522"/>
        <source>Check code in unused templates (should be ON normally, however in theory you can safely ignore warnings in unused templates)</source>
        <oldsource>Check code in unused templates  (slower and less accurate analysis)</oldsource>
        <translation>未使用テンプレートのコードもチェック (解析に時間がかかり、また正確性は低い)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="534"/>
        <source>Max CTU depth</source>
        <translation>CTUの最大深さ</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="752"/>
        <source>Enable inline suppressions</source>
        <translation>inline抑制を有効にする</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="820"/>
        <source>Misra C</source>
        <translation>MISRA C</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="828"/>
        <source>2012</source>
        <translation>2012</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="833"/>
        <location filename="projectfile.ui" line="896"/>
        <source>2023</source>
        <translation>2023</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="883"/>
        <source>Misra C++</source>
        <translation>MISRA C++</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="891"/>
        <source>2008</source>
        <translation>2008</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="940"/>
        <source>Cert C++</source>
        <translation>Cert C++</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="957"/>
        <source>Bug hunting (Premium)</source>
        <translation>バグハンティング(プレミアム)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="973"/>
        <source>External tools</source>
        <translation>外部ツール</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="330"/>
        <source>Up</source>
        <translation>上</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="337"/>
        <source>Down</source>
        <translation>下</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="368"/>
        <source>Platform</source>
        <translation>プラットフォーム</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="448"/>
        <source>Clang (experimental)</source>
        <translation>Clang (実験的)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="487"/>
        <source>If you want to design your classes to be as flexible and robust as possible then the public interface must be very robust. Cppcheck will asumme that arguments can take *any* value.</source>
        <translation>可能な限りクラスが柔軟であり堅牢であることを望む場合、公開されたインターフェースが非常に堅牢です。Cppcheckは引数があらゆる値をとりうると仮定します。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="565"/>
        <source>Max recursion in template instantiation</source>
        <translation>テンプレートインスタンス化の最大再帰回数</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="614"/>
        <source>Warning options</source>
        <translation>警告オプション</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="620"/>
        <source>Root path:</source>
        <translation>ルートパス:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="626"/>
        <source>Filepaths in warnings will be relative to this path</source>
        <translation>警告中のファイルパスはこのパスからの相対パスになります</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="636"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>警告タグ(セミコロン区切り)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="409"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation>Cppcheck ビルドディレクトリ (全プログラムチェック, 差分チェック, 統計等)</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="380"/>
        <source>Libraries</source>
        <translation>ライブラリ</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="709"/>
        <source>Suppressions</source>
        <translation>指摘の抑制</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="720"/>
        <source>Add</source>
        <translation>追加</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="773"/>
        <location filename="projectfile.ui" line="779"/>
        <source>Addons</source>
        <translation>アドオン</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="785"/>
        <source>Note: Addons require &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt; being installed.</source>
        <translation>注意: アドオンには&lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt;が必要です。</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="795"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="802"/>
        <source>Thread safety</source>
        <translation>スレッドセーフ</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="812"/>
        <source>Coding standards</source>
        <translation>コーディング標準</translation>
    </message>
    <message>
        <source>Misra C 2012</source>
        <translation type="vanished">MISRA C 2012</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="919"/>
        <source>Cert C</source>
        <translation>CERT C</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="928"/>
        <source>CERT-INT35-C:  int precision (if size equals precision, you can leave empty)</source>
        <translation>CERT-INT35-C:  int型の精度 (もしサイズが精度と一致する場合空のままにしてください)</translation>
    </message>
    <message>
        <source>Misra C++ 2008</source>
        <translation type="vanished">MISRA C++ 2008</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="947"/>
        <source>Autosar</source>
        <translation>AUTOSAR</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="963"/>
        <source>Bug hunting</source>
        <translation>バグハント</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="986"/>
        <source>Clang analyzer</source>
        <translation>Clang Analyzer</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="979"/>
        <source>Clang-tidy</source>
        <translation>Clang-tidy</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="221"/>
        <source>Defines:</source>
        <translation>定義(Defines):</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="118"/>
        <source>Project file: %1</source>
        <translation>プロジェクトファイル:%1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="556"/>
        <source>Select Cppcheck build dir</source>
        <translation>Cppcheckビルドディレクトリ</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="825"/>
        <source>Select include directory</source>
        <translation>includeディレクトリを選択</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="805"/>
        <source>Select a directory to check</source>
        <translation>チェックするディレクトリを選択してください</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="434"/>
        <source>Clang-tidy (not found)</source>
        <translation>Clang-tidy (みつかりません)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="595"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="596"/>
        <source>Compile database</source>
        <translation>コンパイルデータベース</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="597"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="598"/>
        <source>Import Project</source>
        <translation>プロジェクトのインポート</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="845"/>
        <source>Select directory to ignore</source>
        <translation>除外するディレクトリを選択してください</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="853"/>
        <source>Source files</source>
        <translation>ソースファイル</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="854"/>
        <source>All files</source>
        <translation>全ファイル</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="855"/>
        <source>Exclude file</source>
        <translation>除外ファイル</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="939"/>
        <source>Select MISRA rule texts file</source>
        <translation>MISRAルールテキストファイルを選択</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="941"/>
        <source>MISRA rule texts file (%1)</source>
        <translation>MISRAルールテキストファイル (%1)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="86"/>
        <source>Unknown language specified!</source>
        <translation>未知の言語が指定されました!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="112"/>
        <source>Language file %1 not found!</source>
        <translation>言語ファイル %1 が見つかりません!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="117"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>言語 %2 から %1 への翻訳ファイルの読み込みに失敗</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="42"/>
        <source>line %1: Unhandled element %2</source>
        <translation>行 %1: 扱われていない要素(Unhandled element) %2</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="47"/>
        <source>line %1: Mandatory attribute &apos;%2&apos; missing in &apos;%3&apos;</source>
        <translation>行 %1: 必須の属性 &apos;%2&apos; が &apos;%3&apos;にない</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="277"/>
        <source> (Not found)</source>
        <translation> (見つかりません)</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="76"/>
        <source>Thin</source>
        <translation>シン(細)</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="78"/>
        <source>ExtraLight</source>
        <translation>エクストラライト</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="80"/>
        <source>Light</source>
        <translation>ライト</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="82"/>
        <source>Normal</source>
        <translation>ノーマル</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="84"/>
        <source>Medium</source>
        <translation>メディウム</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="86"/>
        <source>DemiBold</source>
        <translation>デミボールト</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="88"/>
        <source>Bold</source>
        <translation>ボールド</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="90"/>
        <source>ExtraBold</source>
        <translation>エクストラボールド</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="92"/>
        <source>Black</source>
        <translation>黒</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="80"/>
        <source>Editor Foreground Color</source>
        <translation>エディタの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="83"/>
        <source>Editor Background Color</source>
        <translation>エディタの背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="86"/>
        <source>Highlight Background Color</source>
        <translation>ハイライトの背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="89"/>
        <source>Line Number Foreground Color</source>
        <translation>行番号の前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="92"/>
        <source>Line Number Background Color</source>
        <translation>行番号の背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="95"/>
        <source>Keyword Foreground Color</source>
        <translation>キーワードの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="98"/>
        <source>Keyword Font Weight</source>
        <translation>キーワードのフォントのウェイト</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="101"/>
        <source>Class Foreground Color</source>
        <oldsource>Class ForegroundColor</oldsource>
        <translation>クラスの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="104"/>
        <source>Class Font Weight</source>
        <translation>クラスフォントのウェイト</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="107"/>
        <source>Quote Foreground Color</source>
        <translation>クォートの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="110"/>
        <source>Quote Font Weight</source>
        <translation>クォートのフォントウェイト</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="113"/>
        <source>Comment Foreground Color</source>
        <translation>コメントの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="116"/>
        <source>Comment Font Weight</source>
        <translation>コメントフォントのウェイト</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="119"/>
        <source>Symbol Foreground Color</source>
        <translation>シンボルの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="122"/>
        <source>Symbol Background Color</source>
        <translation>シンボルの背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="125"/>
        <source>Symbol Font Weight</source>
        <translation>シンボルのフォントウェイト</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="145"/>
        <source>Set to Default Light</source>
        <translation>デフォルトをライトに設定</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="147"/>
        <source>Set to Default Dark</source>
        <translation>デフォルトをダークに設定</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="233"/>
        <source>File</source>
        <translation>ファイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="234"/>
        <source>Line</source>
        <translation>行</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="235"/>
        <source>Severity</source>
        <translation>警告の種別</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="236"/>
        <source>Classification</source>
        <translation>分類</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="237"/>
        <source>Level</source>
        <translation>レベル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="238"/>
        <source>Inconclusive</source>
        <translation>結論のでない</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="239"/>
        <source>Summary</source>
        <translation>要約</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="240"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="241"/>
        <source>Guideline</source>
        <translation>ガイドライン</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="242"/>
        <source>Rule</source>
        <translation>ルール</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="243"/>
        <source>Since date</source>
        <translation>日付</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="244"/>
        <source>Tags</source>
        <translation>タグ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="245"/>
        <source>CWE</source>
        <translation>CWE</translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Save</source>
        <translation>保存する</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <source>File</source>
        <translation type="vanished">ファイル</translation>
    </message>
    <message>
        <source>Severity</source>
        <translation type="vanished">警告の種別</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="vanished">行</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation type="vanished">要約</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="387"/>
        <source>Undefined file</source>
        <translation>未定義ファイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="880"/>
        <source>Copy</source>
        <translation>コピー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1065"/>
        <source>Could not find file:</source>
        <translation>ファイルが見つかりません:</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1069"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation>フォルダ &apos;%1&apos; を選択してください</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1070"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation>ディレクトリ &apos;%1&apos; 選択</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1072"/>
        <source>Please select the directory where file is located.</source>
        <translation>ファイルのあるディレクトリを選択してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="587"/>
        <source>debug</source>
        <translation>デバッグ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="513"/>
        <source>note</source>
        <translation>注意</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="879"/>
        <source>Recheck</source>
        <translation>再チェック</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="881"/>
        <source>Hide</source>
        <translation>非表示</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="882"/>
        <source>Hide all with id</source>
        <translation>IDで非表示を指定</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="901"/>
        <source>Suppress selected id(s)</source>
        <translation>選択したidを抑制</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="883"/>
        <source>Open containing folder</source>
        <translation>含まれるフォルダを開く</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="590"/>
        <source>internal</source>
        <translation>内部</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="922"/>
        <source>Tag</source>
        <translation>タグ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="924"/>
        <source>No tag</source>
        <translation>タグなし</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="955"/>
        <location filename="resultstree.cpp" line="969"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="956"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>エディタアプリが設定されていません。

Cppcheckの「設定」からテキストファイルを編集するアプリケーションを設定してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="970"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>デフォルトのエディタアプリケーションが指定されていません。

設定からデフォルトのエディタアプリケーションを設定してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="996"/>
        <source>Could not find the file!</source>
        <translation>ファイルが見つかりません!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1051"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>%1 が実行できません。

実行ファイルパスや引数の設定を確認してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1073"/>
        <source>Select Directory</source>
        <translation>ディレクトリを選択</translation>
    </message>
    <message>
        <source>Id</source>
        <translation type="vanished">Id</translation>
    </message>
    <message>
        <source>Inconclusive</source>
        <translation type="vanished">結論のでない</translation>
    </message>
    <message>
        <source>Since date</source>
        <translation type="vanished">日付</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="569"/>
        <source>style</source>
        <translation>スタイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="572"/>
        <source>error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="575"/>
        <source>warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="578"/>
        <source>performance</source>
        <translation>パフォーマンス</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="581"/>
        <source>portability</source>
        <translation>移植可能性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="584"/>
        <source>information</source>
        <translation>情報</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>結果</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="60"/>
        <source>Critical errors</source>
        <translation>致命的なエラー</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="92"/>
        <source>Analysis Log</source>
        <translation>チェックログ</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="114"/>
        <source>Warning Details</source>
        <translation>警告の詳細</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="232"/>
        <location filename="resultsview.cpp" line="240"/>
        <source>Failed to save the report.</source>
        <translation>レポートの保存に失敗しました。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="250"/>
        <source>Print Report</source>
        <translation>レポートの印刷</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="269"/>
        <source>No errors found, nothing to print.</source>
        <translation>指摘がないため、印刷するものがありません。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="321"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 / %2 :ファイル数)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="345"/>
        <location filename="resultsview.cpp" line="356"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="346"/>
        <source>No errors found.</source>
        <translation>警告/エラーは見つかりませんでした。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="353"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>警告/エラーが見つかりましたが、非表示設定になっています。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="402"/>
        <location filename="resultsview.cpp" line="421"/>
        <source>Failed to read the report.</source>
        <translation>レポートの読み込みに失敗.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="409"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>XML フォーマットバージョン 1 はもうサポートされていません。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="470"/>
        <source>First included by</source>
        <translation>は次のものが最初にインクルードしました</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="475"/>
        <source>Id</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="477"/>
        <source>Bug hunting analysis is incomplete</source>
        <translation>バグハントの解析は不完全です</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="551"/>
        <source>Clear Log</source>
        <translation>ログの消去</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="552"/>
        <source>Copy this Log entry</source>
        <translation>このログ項目をコピー</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="553"/>
        <source>Copy complete Log</source>
        <translation>ログ全体をコピー</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="561"/>
        <source>Analysis was stopped</source>
        <translation>解析は停止しした</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="575"/>
        <source>There was a critical error with id &apos;%1&apos;</source>
        <translation>id &apos;%1&apos;の致命的なエラーがあります</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="577"/>
        <source>when checking %1</source>
        <translation>%1 をチェックするとき</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="579"/>
        <source>when checking a file</source>
        <translation>ファイルをチェックするとき</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="580"/>
        <source>Analysis was aborted.</source>
        <translation>解析は中止した。</translation>
    </message>
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui" line="14"/>
        <source>Scratchpad</source>
        <translation>スクラッチパッド</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="20"/>
        <source>Copy or write some C/C++ code here:</source>
        <translation>ここに C/C++のコードをコピーペーストまたは記入してください:</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="37"/>
        <source>Optionally enter a filename (mainly for automatic language detection) and click on &quot;Check&quot;:</source>
        <translation>オプション: ファイル名を入力(言語は自動判定)して&quot;チェック&quot;をクリックしてください:</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="71"/>
        <source>filename</source>
        <translation>ファイル名</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="78"/>
        <source>Check</source>
        <translation>チェック</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>設定</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>全般</translation>
    </message>
    <message>
        <location filename="settings.ui" line="202"/>
        <source>Add...</source>
        <translation>追加...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>解析用のスレッド数: </translation>
    </message>
    <message>
        <location filename="settings.ui" line="85"/>
        <source>Ideal count:</source>
        <translation>理想的な数：</translation>
    </message>
    <message>
        <location filename="settings.ui" line="114"/>
        <source>Force checking all #ifdef configurations</source>
        <oldsource>Check all #ifdef configurations</oldsource>
        <translation>すべての #ifdef をチェックする</translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Show full path of files</source>
        <translation>ファイルのフルパスを表示</translation>
    </message>
    <message>
        <location filename="settings.ui" line="128"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>エラーが無いときは&quot;エラーなし&quot;を表示</translation>
    </message>
    <message>
        <location filename="settings.ui" line="135"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>エラーIDを &quot;Id&quot; に表示する</translation>
    </message>
    <message>
        <location filename="settings.ui" line="142"/>
        <source>Enable inline suppressions</source>
        <translation>inline抑制を有効にする</translation>
    </message>
    <message>
        <location filename="settings.ui" line="149"/>
        <source>Check for inconclusive errors also</source>
        <translation>結論のでない指摘もチェックする</translation>
    </message>
    <message>
        <location filename="settings.ui" line="156"/>
        <source>Show statistics on check completion</source>
        <translation>チェック完了時に統計情報を表示する</translation>
    </message>
    <message>
        <location filename="settings.ui" line="163"/>
        <source>Check for updates</source>
        <translation>更新の確認</translation>
    </message>
    <message>
        <location filename="settings.ui" line="183"/>
        <source>Show internal warnings in log</source>
        <translation>ログの内部警告も表示する</translation>
    </message>
    <message>
        <location filename="settings.ui" line="294"/>
        <source>Addons</source>
        <translation>アドオン</translation>
    </message>
    <message>
        <location filename="settings.ui" line="300"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation>Pythonインタプリタの場所(空白の場合システムのPATHから検索)</translation>
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
        <translation>MISRAアドオン</translation>
    </message>
    <message>
        <location filename="settings.ui" line="338"/>
        <source>MISRA rule texts file</source>
        <translation>MISRA ルールテキストファイル</translation>
    </message>
    <message>
        <location filename="settings.ui" line="345"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdfのテキストをテキストファイルにコピー&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="378"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="settings.ui" line="384"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation>Clangの場所(空白の場合システムのPATHから検索)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="407"/>
        <source>Visual Studio headers</source>
        <translation>Visual Studioのヘッダ</translation>
    </message>
    <message>
        <location filename="settings.ui" line="413"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Visual Studioのヘッダーファイル(セミコロン区切り&apos;;&apos;)。&lt;/p&gt;&lt;p&gt;Visual Studio コマンドプロンプトを開き、 &amp;quot;SET INCLUDE&amp;quot;. と入力後、そのパスをコピーペーストしてください。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="443"/>
        <source>Code Editor</source>
        <translation>コードエディタ</translation>
    </message>
    <message>
        <location filename="settings.ui" line="449"/>
        <source>Code Editor Style</source>
        <translation>コードエディタスタイル</translation>
    </message>
    <message>
        <location filename="settings.ui" line="455"/>
        <source>System Style</source>
        <translation>システムのデフォルトのスタイル</translation>
    </message>
    <message>
        <location filename="settings.ui" line="462"/>
        <source>Default Light Style</source>
        <translation>ライトスタイルをデフォルトに</translation>
    </message>
    <message>
        <location filename="settings.ui" line="469"/>
        <source>Default Dark Style</source>
        <translation>ダークスタイルをデフォルトに</translation>
    </message>
    <message>
        <location filename="settings.ui" line="478"/>
        <source>Custom</source>
        <translation>カスタム</translation>
    </message>
    <message>
        <location filename="settings.ui" line="216"/>
        <source>Remove</source>
        <translation>削除</translation>
    </message>
    <message>
        <location filename="settings.ui" line="191"/>
        <source>Applications</source>
        <translation>アプリケーション</translation>
    </message>
    <message>
        <location filename="settings.ui" line="209"/>
        <location filename="settings.ui" line="485"/>
        <source>Edit...</source>
        <translation>編集...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="223"/>
        <source>Set as default</source>
        <translation>デフォルトとして設定</translation>
    </message>
    <message>
        <location filename="settings.ui" line="246"/>
        <source>Reports</source>
        <translation>レポート</translation>
    </message>
    <message>
        <location filename="settings.ui" line="252"/>
        <source>Save all errors when creating report</source>
        <translation>レポート作成時にすべての警告/エラーを保存</translation>
    </message>
    <message>
        <location filename="settings.ui" line="259"/>
        <source>Save full path to files in reports</source>
        <translation>レポートにファイルのフルパスを保存</translation>
    </message>
    <message>
        <location filename="settings.ui" line="280"/>
        <source>Language</source>
        <translation>言語</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="126"/>
        <source>N/A</source>
        <translation>N/A</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="237"/>
        <source>The executable file &quot;%1&quot; is not available</source>
        <translation>実行ファイル &quot;%1&quot; が利用できません</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="248"/>
        <source>Add a new application</source>
        <translation>新しいアプリケーションの追加</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="278"/>
        <source>Modify an application</source>
        <translation>アプリケーションの変更</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="283"/>
        <source> [Default]</source>
        <translation> [デフォルト]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="308"/>
        <source>[Default]</source>
        <translation>[デフォルト]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="363"/>
        <source>Select python binary</source>
        <translation>pythonの場所の選択</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="370"/>
        <source>Select MISRA File</source>
        <translation>MISRAファイルの選択</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="401"/>
        <source>Select clang path</source>
        <translation>clangのパスの選択</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="statsdialog.ui" line="14"/>
        <location filename="statsdialog.ui" line="248"/>
        <location filename="statsdialog.cpp" line="187"/>
        <location filename="statsdialog.cpp" line="234"/>
        <source>Statistics</source>
        <translation>統計情報</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="27"/>
        <location filename="statsdialog.cpp" line="225"/>
        <source>Project</source>
        <translation>プロジェクト</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="33"/>
        <source>Project:</source>
        <translation>プロジェクト:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="53"/>
        <source>Paths:</source>
        <translation>パス:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="85"/>
        <source>Include paths:</source>
        <translation>インクルードパス:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="108"/>
        <source>Defines:</source>
        <translation>定義(define):</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="131"/>
        <source>Undefines:</source>
        <translation>定義取り消し(undef):</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="165"/>
        <location filename="statsdialog.cpp" line="230"/>
        <source>Previous Scan</source>
        <translation>前回の解析</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="171"/>
        <source>Path Selected:</source>
        <translation>ディレクトリ選択:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="181"/>
        <source>Number of Files Scanned:</source>
        <translation>解析済みファイル数:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="201"/>
        <source>Scan Duration:</source>
        <translation>解析にかかった時間:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="254"/>
        <source>Errors:</source>
        <translation>エラー:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="271"/>
        <source>Warnings:</source>
        <translation>警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="288"/>
        <source>Stylistic warnings:</source>
        <translation>スタイル警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="305"/>
        <source>Portability warnings:</source>
        <translation>移植可能性の警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="322"/>
        <source>Performance issues:</source>
        <translation>パフォーマンス警告:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="339"/>
        <source>Information messages:</source>
        <translation>情報メッセージ:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="356"/>
        <source>Active checkers:</source>
        <translation>有効なチェッカー:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="374"/>
        <source>Checkers</source>
        <translation>チェッカー</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="399"/>
        <source>History</source>
        <translation>ヒストリー</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="405"/>
        <source>File:</source>
        <translation>ファイル:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="443"/>
        <source>Copy to Clipboard</source>
        <translation>クリップボードにコピー</translation>
    </message>
    <message>
        <location filename="statsdialog.ui" line="450"/>
        <source>Pdf Export</source>
        <translation>PDF エクスポート</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="163"/>
        <source>1 day</source>
        <translation>一日</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="163"/>
        <source>%1 days</source>
        <translation>%1日</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>1 hour</source>
        <translation>一時間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>%1 hours</source>
        <translation>%1時間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="167"/>
        <source>1 minute</source>
        <translation>一分</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="167"/>
        <source>%1 minutes</source>
        <translation>%1分</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="169"/>
        <source>1 second</source>
        <translation>一秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="169"/>
        <source>%1 seconds</source>
        <translation>%1秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="173"/>
        <source>0.%1 seconds</source>
        <translation>0.%1秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="175"/>
        <source> and </source>
        <translation> と </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="202"/>
        <source>Export PDF</source>
        <translation>PDF エクスポート</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="224"/>
        <source>Project Settings</source>
        <translation>プロジェクトの設定</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="226"/>
        <source>Paths</source>
        <translation>パス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="227"/>
        <source>Include paths</source>
        <translation>インクルードパス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="228"/>
        <source>Defines</source>
        <translation>定義(define)</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="229"/>
        <source>Undefines</source>
        <translation>定義取り消し(Undef)</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="231"/>
        <source>Path selected</source>
        <translation>選択されたパス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="232"/>
        <source>Number of files scanned</source>
        <translation>スキャンしたファイルの数</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="233"/>
        <source>Scan duration</source>
        <translation>スキャン期間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="189"/>
        <location filename="statsdialog.cpp" line="235"/>
        <source>Errors</source>
        <translation>エラー</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="116"/>
        <source>File: </source>
        <translation>ファイル: </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="116"/>
        <source>No cppcheck build dir</source>
        <translation>cppcheckビルドディレクトリがありません</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="191"/>
        <location filename="statsdialog.cpp" line="236"/>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="193"/>
        <location filename="statsdialog.cpp" line="237"/>
        <source>Style warnings</source>
        <translation>スタイル警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="195"/>
        <location filename="statsdialog.cpp" line="238"/>
        <source>Portability warnings</source>
        <translation>移植可能性警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="197"/>
        <location filename="statsdialog.cpp" line="239"/>
        <source>Performance warnings</source>
        <translation>パフォーマンス警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="199"/>
        <location filename="statsdialog.cpp" line="240"/>
        <source>Information messages</source>
        <translation>情報メッセージ</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="45"/>
        <source>%1 of %2 files checked</source>
        <translation>チェック: %1 / %2 (ファイル数)</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="125"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>ユーザーインターフェースの言語 %1 への変更に失敗しました。

そのため言語を 英語にリセットします。設定ダイアログから利用可能な言語を選択してください。</translation>
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
        <translation>結論の出ない</translation>
    </message>
</context>
<context>
    <name>toFilterString</name>
    <message>
        <location filename="common.cpp" line="58"/>
        <source>All supported files (%1)</source>
        <translation>全サポートファイル (%1)</translation>
    </message>
    <message>
        <location filename="common.cpp" line="63"/>
        <source>All files (%1)</source>
        <translation>全ファイル(%1)</translation>
    </message>
</context>
</TS>
