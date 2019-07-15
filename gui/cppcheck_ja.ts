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
        <source>Copyright © 2007-2019 Cppcheck team.</source>
        <translation>Copyright © 2007-2019 Cppcheck team.</translation>
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
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul&gt;
&lt;li&gt;tinyxml2&lt;/li&gt;
&lt;li&gt;picojson&lt;/li&gt;
&lt;li&gt;pcre&lt;/li&gt;
&lt;li&gt;qt&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;私達は以下のライブラリを使用しています。ここで感謝の意を表明します。&lt;/p&gt;&lt;ul&gt;
&lt;li&gt;tinyxml2&lt;/li&gt;
&lt;li&gt;picojson&lt;/li&gt;
&lt;li&gt;pcre&lt;/li&gt;
&lt;li&gt;qt&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="application.ui" line="23"/>
        <source>Add an application</source>
        <translation>アプリケーションの追加</translation>
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
        <location filename="application.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>表示名(&amp;N):</translation>
    </message>
    <message>
        <location filename="application.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>実行ファイルのパス(&amp;E):</translation>
    </message>
    <message>
        <location filename="application.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>パラメータ(&amp;P):</translation>
    </message>
    <message>
        <location filename="application.ui" line="138"/>
        <source>Browse</source>
        <translation>参照</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>実行ファイル (*.exe);;すべてのファイル(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="62"/>
        <source>Select viewer application</source>
        <translation>表示アプリケーションの選択</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="77"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="78"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>アプリケーションの表示名と実行ファイルのパスと(オプションの)引数を指定してください!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="42"/>
        <source>Could not find the file: %1</source>
        <translation>ファイル：%1 が見つかりません</translation>
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
        <translation>ファイル：%1 が読み込めません</translation>
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
        <location filename="librarydialog.cpp" line="82"/>
        <location filename="librarydialog.cpp" line="154"/>
        <source>Library files (*.cfg)</source>
        <translation>ライブラリファイル(*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="84"/>
        <source>Open library file</source>
        <translation>ライブラリファイルを開く</translation>
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
        <translation>ファイルが見つかりません %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="108"/>
        <source>Failed to load %1. %2.</source>
        <translation>読み込みに失敗しました(%1.%2)。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="145"/>
        <source>Can not save file %1.</source>
        <translation>ファイルが保存できません %1。</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="157"/>
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
        <source>constant</source>
        <translation type="obsolete">constant(定数)</translation>
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
    <name>LogView</name>
    <message>
        <source>Checking Log</source>
        <translation type="obsolete">Cppcheck ログ</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="obsolete">消去</translation>
    </message>
    <message>
        <source>Save Log</source>
        <translation type="obsolete">ログ保存</translation>
    </message>
    <message>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation type="obsolete">テキストファイル (*.txt *.log);;すべてのファイル(*.*)</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="obsolete">Cppcheck</translation>
    </message>
    <message>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation type="obsolete">ファイルを書き込みできない</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.ui" line="595"/>
        <location filename="mainwindow.cpp" line="328"/>
        <location filename="mainwindow.cpp" line="479"/>
        <location filename="mainwindow.cpp" line="551"/>
        <location filename="mainwindow.cpp" line="656"/>
        <location filename="mainwindow.cpp" line="678"/>
        <location filename="mainwindow.cpp" line="1105"/>
        <location filename="mainwindow.cpp" line="1230"/>
        <location filename="mainwindow.cpp" line="1351"/>
        <location filename="mainwindow.cpp" line="1491"/>
        <location filename="mainwindow.cpp" line="1514"/>
        <location filename="mainwindow.cpp" line="1585"/>
        <location filename="mainwindow.cpp" line="1655"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="70"/>
        <source>&amp;File</source>
        <translation>ファイル(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="89"/>
        <source>&amp;View</source>
        <translation>表示(&amp;V)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="93"/>
        <source>&amp;Toolbars</source>
        <translation>ツールバー(&amp;T)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="121"/>
        <source>&amp;Help</source>
        <translation>ヘルプ(&amp;H)</translation>
    </message>
    <message>
        <source>&amp;Check</source>
        <translation type="obsolete">解析(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="135"/>
        <source>C++ standard</source>
        <translation>C++標準</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="145"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation>&amp;C標準</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="170"/>
        <source>&amp;Edit</source>
        <translation>編集(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="183"/>
        <source>Standard</source>
        <translation>言語規格</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="201"/>
        <source>Categories</source>
        <translation>カテゴリ</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="231"/>
        <source>&amp;License...</source>
        <translation>ライセンス(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="236"/>
        <source>A&amp;uthors...</source>
        <translation>作者(&amp;u)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="245"/>
        <source>&amp;About...</source>
        <translation>Cppcheckについて(&amp;A)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="250"/>
        <source>&amp;Files...</source>
        <translation>ファイル選択(&amp;F)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="253"/>
        <location filename="mainwindow.ui" line="256"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation>ファイルをチェックする</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="259"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="268"/>
        <source>&amp;Directory...</source>
        <translation>ディレクトリ選択(&amp;D)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="271"/>
        <location filename="mainwindow.ui" line="274"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation>ディレクトリをチェックする</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="277"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>&amp;Recheck files</source>
        <translation type="obsolete">再チェック(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="289"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <source>&amp;Reanalyze all files</source>
        <oldsource>&amp;Recheck all files</oldsource>
        <translation type="obsolete">全ファイル再チェック</translation>
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
        <translation>チェックを停止する</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="316"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="325"/>
        <source>&amp;Save results to file...</source>
        <translation>結果をファイルに保存(&amp;S)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="328"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="333"/>
        <source>&amp;Quit</source>
        <translation>終了(&amp;Q)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="342"/>
        <source>&amp;Clear results</source>
        <translation>結果をクリア(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="351"/>
        <source>&amp;Preferences</source>
        <translation>設定(&amp;P)</translation>
    </message>
    <message>
        <source>Style warnings</source>
        <translation type="obsolete">スタイル警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="366"/>
        <location filename="mainwindow.ui" line="369"/>
        <source>Show style warnings</source>
        <translation>スタイル警告を表示</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation type="obsolete">エラー</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="384"/>
        <location filename="mainwindow.ui" line="387"/>
        <source>Show errors</source>
        <translation>エラーを表示</translation>
    </message>
    <message>
        <source>Show S&amp;cratchpad...</source>
        <translation type="obsolete">スクラッチパッドを表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="767"/>
        <location filename="mainwindow.cpp" line="805"/>
        <source>Information</source>
        <translation>情報</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="568"/>
        <source>Show information messages</source>
        <translation>情報メッセージを表示</translation>
    </message>
    <message>
        <source>Portability</source>
        <translation type="obsolete">移植可能性</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="583"/>
        <source>Show portability warnings</source>
        <translation>移植可能性の問題を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="598"/>
        <source>Show Cppcheck results</source>
        <translation>Cppcheck結果を表示する</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="610"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="613"/>
        <source>Show Clang results</source>
        <translation>Clangの結果を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="621"/>
        <source>&amp;Filter</source>
        <translation>フィルター(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="624"/>
        <source>Filter results</source>
        <translation>フィルタ結果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="640"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSIエンコード</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="648"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="656"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="664"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="672"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <source>Platforms</source>
        <translation type="obsolete">プラットフォーム</translation>
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
        <location filename="mainwindow.ui" line="742"/>
        <source>&amp;Print...</source>
        <translation>印刷(&amp;P)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="745"/>
        <source>Print the Current Report</source>
        <translation>現在のレポートを印刷</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="750"/>
        <source>Print Pre&amp;view...</source>
        <translation>印刷プレビュー(&amp;v)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="753"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>現在のレポートをプレビュー表示</translation>
    </message>
    <message>
        <source>Library Editor...</source>
        <translation type="obsolete">ライブラリの編集</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="761"/>
        <source>Open library editor</source>
        <translation>ライブラリエディタを開く</translation>
    </message>
    <message>
        <source>Auto-detect language</source>
        <translation type="obsolete">言語を自動検出</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="489"/>
        <source>C&amp;lose Project File</source>
        <translation>プロジェクトを閉じる(&amp;l)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="497"/>
        <source>&amp;Edit Project File...</source>
        <translation>プロジェクトの編集(&amp;E)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="509"/>
        <source>&amp;Statistics</source>
        <translation>統計情報(&amp;S)</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation type="obsolete">警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="524"/>
        <location filename="mainwindow.ui" line="527"/>
        <source>Show warnings</source>
        <translation>警告を表示</translation>
    </message>
    <message>
        <source>Performance warnings</source>
        <translation type="obsolete">パフォーマンス警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="542"/>
        <location filename="mainwindow.ui" line="545"/>
        <source>Show performance warnings</source>
        <translation>パフォーマンス警告を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="553"/>
        <source>Show &amp;hidden</source>
        <translation>非表示を表示(&amp;h)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="392"/>
        <source>&amp;Check all</source>
        <translation>すべてのエラーを表示(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="131"/>
        <source>A&amp;nalyze</source>
        <translation>チェック(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="220"/>
        <source>Filter</source>
        <translation>フィルター</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="286"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation>変更ありファイルを再解析(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="298"/>
        <source>Reanal&amp;yze all files</source>
        <translation>全ファイル再解析(&amp;y)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="363"/>
        <source>Style war&amp;nings</source>
        <translation>スタイル警告(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="381"/>
        <source>E&amp;rrors</source>
        <translation>エラー(&amp;r)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="397"/>
        <source>&amp;Uncheck all</source>
        <translation>すべてのエラーを非表示(&amp;U)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="402"/>
        <source>Collapse &amp;all</source>
        <translation>ツリーを折り畳む(&amp;a)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="407"/>
        <source>&amp;Expand all</source>
        <translation>ツリーを展開(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="415"/>
        <source>&amp;Standard</source>
        <translation>言語規格(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="418"/>
        <source>Standard items</source>
        <translation>標準項目</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="423"/>
        <source>&amp;Contents</source>
        <translation>コンテンツ(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="426"/>
        <source>Open the help contents</source>
        <translation>ヘルプファイルを開く</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="429"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="434"/>
        <source>Toolbar</source>
        <translation>ツールバー</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="442"/>
        <source>&amp;Categories</source>
        <translation>カテゴリ(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="445"/>
        <source>Error categories</source>
        <translation>エラーカテゴリ</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="450"/>
        <source>&amp;Open XML...</source>
        <translation>XMLを開く(&amp;O)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="459"/>
        <source>Open P&amp;roject File...</source>
        <translation>プロジェクトを開く(&amp;R)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="468"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation>スクラッチパッドを表示(&amp;o)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="473"/>
        <source>&amp;New Project File...</source>
        <translation>新規プロジェクト(&amp;N)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="478"/>
        <source>&amp;Log View</source>
        <translation>ログを表示(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="481"/>
        <source>Log View</source>
        <translation>ログ表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="521"/>
        <source>&amp;Warnings</source>
        <translation>警告(&amp;W)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="539"/>
        <source>Per&amp;formance warnings</source>
        <translation>パフォーマンス警告(&amp;f)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="565"/>
        <source>&amp;Information</source>
        <translation>情報(&amp;I)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="580"/>
        <source>&amp;Portability</source>
        <translation>移植可能性(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="680"/>
        <source>P&amp;latforms</source>
        <translation>プラットフォーム(&amp;l)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="694"/>
        <source>C++&amp;11</source>
        <translation>C++11(&amp;1)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="705"/>
        <source>C&amp;99</source>
        <translation>C99(&amp;9)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="713"/>
        <source>&amp;Posix</source>
        <translation>Posix(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="721"/>
        <source>C&amp;11</source>
        <translation>C11(&amp;1)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="729"/>
        <source>&amp;C89</source>
        <translation>C89(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="737"/>
        <source>&amp;C++03</source>
        <translation>C++03(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="758"/>
        <source>&amp;Library Editor...</source>
        <translation>ライブラリエディタ(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="769"/>
        <source>&amp;Auto-detect language</source>
        <translation>自動言語検出(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="777"/>
        <source>&amp;Enforce C++</source>
        <translation>C++ 強制(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="785"/>
        <source>E&amp;nforce C</source>
        <translation>C 強制(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="796"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="804"/>
        <source>Reanalyze and check library</source>
        <translation>ライブラリを再チェックする</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="812"/>
        <source>Check configuration (defines, includes)</source>
        <translation>チェックの設定(define、インクルード)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="820"/>
        <source>C++17</source>
        <translation>C++17</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="831"/>
        <source>C++20</source>
        <translation>C++20</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="323"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>エディタアプリの設定の読み込みで問題が発生しました。

Cppcheckの古いバージョンの設定には互換性がありません。エディタアプリケーションの設定を確認して修正してください、そうしないと正しく起動できないかもしれません。</translation>
    </message>
    <message>
        <source>No suitable files found to check!</source>
        <translation type="obsolete">解析可能なファイルではありません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="552"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>新しいファイル／ディレクトリをチェックするには現在のプロジェクトを閉じてください!</translation>
    </message>
    <message>
        <source>C/C++ Source, Compile database, Visual Studio (%1 %2 *.sln *.vcxproj)</source>
        <translation type="obsolete">C/C++ソースコード、プロジェクトファイル、Visual Studioソリューション(%1 %2 *.sln *.vcxproj)</translation>
    </message>
    <message>
        <source>Select directory to check</source>
        <translation type="obsolete">チェック対象のディレクトリを選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="87"/>
        <source>Quick Filter:</source>
        <translation>クイックフィルタ：</translation>
    </message>
    <message>
        <source>Select files to check</source>
        <translation type="obsolete">チェック対象のファイルを選択</translation>
    </message>
    <message>
        <source>C/C++ Source (%1)</source>
        <translation type="obsolete">C/C++ ソース (%1)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="627"/>
        <source>Select configuration</source>
        <translation>コンフィグレーションの選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="657"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>プロジェクトファイルを検出しました: %1

現在のプロジェクトの代わりにこのプロジェクトファイルを読み込んでもかまいませんか？</translation>
    </message>
    <message>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation type="obsolete">ディレクトリからプロジェクトファイルが検出されました。

これらのプロジェクトファイルを使用せずに解析を進めてもかまいませんか？
</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="767"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>このライブラリ &apos;%1&apos; には次の不明な要素が含まれています。
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="776"/>
        <source>File not found</source>
        <translation>ファイルがありません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="779"/>
        <source>Bad XML</source>
        <translation>不正なXML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="782"/>
        <source>Missing attribute</source>
        <translation>属性がありません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="785"/>
        <source>Bad attribute value</source>
        <translation>不正な属性があります</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="788"/>
        <source>Unsupported format</source>
        <translation>サポートされていないフォーマット</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Duplicate platform type</source>
        <translation>プラットフォームの種類が重複しています</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="794"/>
        <source>Platform type redefined</source>
        <translation>プラットフォームの種類が再定義されました</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="797"/>
        <source>Unknown element</source>
        <translation>不明な要素</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="800"/>
        <source>Unknown issue</source>
        <translation>不明な課題</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="805"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>選択したライブラリの読み込みに失敗しました &apos;%1&apos;
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="826"/>
        <source>Error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located.</source>
        <translation type="obsolete">%1の読み込みに失敗しました。CppCheckのインストールに失敗しています。コマンドライン引数に --data-dir=&lt;directory&gt; を指定して、このファイルの場所を指定してください。 </translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="826"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation>%1のロードに失敗しました。あなたの Cppcheck は正しくインストールされていません。あなたは --data-dir=&lt;directory&gt; コマンドラインオプションでロードするファイルの場所を指定できます。ただし、この --data-dir はインストールスクリプトによってサポートされており、GUI版ではサポートされていません。全ての設定は調整済みでなければなりません。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1106"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation>現在の結果を作成します。

新しくXMLファイルを開くと現在の結果が削除されます。実行しますか？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1121"/>
        <location filename="mainwindow.cpp" line="1301"/>
        <source>XML files (*.xml)</source>
        <translation>XML ファイル (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1123"/>
        <source>Open the report file</source>
        <translation>レポートを開く</translation>
    </message>
    <message>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?</source>
        <translation type="obsolete">解析中です.

解析を停止してCppcheckを終了しますか？.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1272"/>
        <source>License</source>
        <translation>ライセンス</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1279"/>
        <source>Authors</source>
        <translation>作者</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation type="obsolete">XML ファイル (*.xml);;テキストファイル (*.txt);;CSV形式ファイル (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1294"/>
        <source>Save the report file</source>
        <translation>レポートを保存</translation>
    </message>
    <message>
        <source>XML files version 1 (*.xml)</source>
        <translation type="obsolete">XMLファイルのバージョン1</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml)</source>
        <translation type="obsolete">XMLファイルのバージョン2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1305"/>
        <source>Text files (*.txt)</source>
        <translation>テキストファイル (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1309"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV形式ファイル (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1353"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1399"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>プロジェクトファイル (*.cppcheck);;すべてのファイル(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1401"/>
        <source>Select Project File</source>
        <translation>プロジェクトファイルを選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="159"/>
        <location filename="mainwindow.cpp" line="1429"/>
        <location filename="mainwindow.cpp" line="1554"/>
        <source>Project:</source>
        <translation>プロジェクト:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="480"/>
        <source>No suitable files found to analyze!</source>
        <translation>チェック対象のファイルがみつかりません!</translation>
    </message>
    <message>
        <source>C/C++ Source</source>
        <translation type="obsolete">C/C++のソースコード</translation>
    </message>
    <message>
        <source>Compile database</source>
        <translation type="obsolete">コンパイルデータベース</translation>
    </message>
    <message>
        <source>Visual Studio</source>
        <translation type="obsolete">Visual Studio</translation>
    </message>
    <message>
        <source>Borland C++ Builder 6</source>
        <translation type="obsolete">Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="572"/>
        <source>Select files to analyze</source>
        <translation>チェック対象のファイルを選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="587"/>
        <source>Select directory to analyze</source>
        <translation>チェックするディレクトリを選択してください</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="627"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>チェックの設定を選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="679"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation>ディレクトリ内にプロジェクトファイルがありました。

みつかったプロジェクトファイルを使用せずにチェックしますか?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1226"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>チェック中です。

チェックを中断して、Cppcheckを終了しますか?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1292"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML ファイル (*.xml);;テキストファイル (*.txt);;CSVファイル (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1492"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>ビルドディレクトリ&apos;%1&apos;がありません。作成しますか?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1515"/>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation>&apos;%1&apos;のインポートに失敗しました。(チェック中断)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1539"/>
        <source>Project files (*.cppcheck)</source>
        <translation>プロジェクトファイル (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1541"/>
        <source>Select Project Filename</source>
        <translation>プロジェクトファイル名を選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1586"/>
        <source>No project file loaded</source>
        <translation>プロジェクトファイルが読み込まれていません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1650"/>
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
        <location filename="newsuppressiondialog.cpp" line="52"/>
        <source>Edit suppression</source>
        <translation>抑制の編集</translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <source>Built-in</source>
        <translation type="obsolete">ビルトイン</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="37"/>
        <source>Native</source>
        <translation>ネイティブ</translation>
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
        <translation>Windows 32-bit ANSIエンコード</translation>
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
        <translation type="obsolete">Cppcheck</translation>
    </message>
    <message>
        <source>Could not read the project file.</source>
        <translation type="obsolete">プロジェクトファイルが読み込めませんでした</translation>
    </message>
    <message>
        <source>Could not write the project file.</source>
        <translation type="obsolete">プロジェクトファイルが保存できませんでした</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfiledialog.ui" line="14"/>
        <source>Project File</source>
        <translation>プロジェクトファイル</translation>
    </message>
    <message>
        <source>Project</source>
        <translation type="obsolete">プロジェクト</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation>パスと定義</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="30"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <oldsource>Import Project (Visual studio / compile database)</oldsource>
        <translation>プロジェクトのインポート(Visual studio / compile database Borland C++ Builder 6))</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="163"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <oldsource>Defines must be separated by a semicolon &apos;;&apos;</oldsource>
        <translation>定義(Define)はセミコロン&apos;;&apos;で区切る必要があります。 例: DEF1;DEF2=5;DEF3=int</translation>
    </message>
    <message>
        <source>&amp;Root:</source>
        <oldsource>Root:</oldsource>
        <translation type="obsolete">ルート：</translation>
    </message>
    <message>
        <source>Libraries:</source>
        <translation type="obsolete">ライブラリ</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="393"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>カスタマイズした cfgファイルを同じフォルダにプロジェクトファイルとして保存してください。ここに表示できるようになります。</translation>
    </message>
    <message>
        <source>Exclude paths</source>
        <translation type="obsolete">除外するパス</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="597"/>
        <source>MISRA C 2012</source>
        <translation>MISRA C 2012</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="606"/>
        <source>Misra rule texts</source>
        <translation>Misra ルールテキスト</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="613"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;MISRA C 2012 pdfのAppendix A &amp;quot;Summary of guidelines&amp;quot; からテキストをコピーペーストしてください。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="620"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="69"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;選択済み:&lt;/p&gt;&lt;p&gt; * 全Debug と Release設定をチェックする&lt;/p&gt;&lt;p&gt; * 最初にマッチした Debug 設定のみチェックする&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="60"/>
        <location filename="projectfiledialog.ui" line="309"/>
        <source>Browse...</source>
        <translation>参照...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="72"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation>Visual Studioの全ての設定をチェックする</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="86"/>
        <source>Paths:</source>
        <translation>パス:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="113"/>
        <location filename="projectfiledialog.ui" line="228"/>
        <location filename="projectfiledialog.ui" line="461"/>
        <source>Add...</source>
        <translation>追加...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="120"/>
        <location filename="projectfiledialog.ui" line="235"/>
        <location filename="projectfiledialog.ui" line="468"/>
        <source>Edit</source>
        <translation>編集</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="127"/>
        <location filename="projectfiledialog.ui" line="242"/>
        <location filename="projectfiledialog.ui" line="475"/>
        <location filename="projectfiledialog.ui" line="518"/>
        <source>Remove</source>
        <translation>取り除く</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="174"/>
        <source>Undefines:</source>
        <translation>定義取り消し(Undefines):</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="184"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation>定義の取り消しはセミコロンで区切ります。例: UNDEF1;UNDEF2;UNDEF3</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="197"/>
        <source>Include Paths:</source>
        <translation>インクルードパス:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="331"/>
        <source>Analysis</source>
        <translation>チェック</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="337"/>
        <source>Check code in headers  (slower analysis, more results)</source>
        <translation>ヘッダファイルのコードもチェック(解析に時間がかかりますが結果は増えます)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="347"/>
        <source>Check code in unused templates  (slower and less accurate analysis)</source>
        <translation>未使用テンプレートのコードもチェック (解析に時間がかかり、また正確性は低い)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="356"/>
        <source>Max CTU depth</source>
        <translation>CTUの最大深さ</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="450"/>
        <source>Exclude source files in paths</source>
        <translation>除外するソースファイルのPATH</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="632"/>
        <source>External tools</source>
        <translation>外部ツール</translation>
    </message>
    <message>
        <source>Includes</source>
        <translation type="obsolete">インクルード</translation>
    </message>
    <message>
        <source>Include directories:</source>
        <translation type="obsolete">インクルードディレクトリ:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="262"/>
        <source>Up</source>
        <translation>上</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="269"/>
        <source>Down</source>
        <translation>下</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="294"/>
        <source>Checking</source>
        <translation>チェック</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="319"/>
        <source>Platform</source>
        <translation>プラットフォーム</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="420"/>
        <source>Warning options</source>
        <translation>警告オプション</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="426"/>
        <source>Root path:</source>
        <translation>ルートパス:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="438"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>警告タグ(セミコロン区切り)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="300"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation>Cppcheck ビルドディレクトリ (全プログラムチェック, 差分チェック, 統計等)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="384"/>
        <source>Libraries</source>
        <translation>ライブラリ</translation>
    </message>
    <message>
        <source>Exclude</source>
        <translation type="obsolete">除外する</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="500"/>
        <source>Suppressions</source>
        <translation>指摘の抑制</translation>
    </message>
    <message>
        <source>Suppression list:</source>
        <translation type="obsolete">抑制リスト</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="511"/>
        <source>Add</source>
        <translation>追加</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="557"/>
        <source>Addons and tools</source>
        <translation>アドオンとツール</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="563"/>
        <source>Addons</source>
        <translation>アドオン</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="569"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="576"/>
        <source>Thread safety</source>
        <translation>スレッドセーフ</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="583"/>
        <source>Coding standards</source>
        <translation>コーディング標準</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="590"/>
        <source>Cert</source>
        <translation>CERT</translation>
    </message>
    <message>
        <source>Extra Tools</source>
        <translation type="obsolete">エクストラツール</translation>
    </message>
    <message>
        <source>It is common best practice to use several tools.</source>
        <translation type="obsolete">複数ツールの併用はよい結果を生みます。</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="645"/>
        <source>Clang analyzer</source>
        <translation>Clang Analyzer</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="638"/>
        <source>Clang-tidy</source>
        <translation>Clang-tidy</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="153"/>
        <source>Defines:</source>
        <translation>定義(Defines):</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="71"/>
        <source>Project file: %1</source>
        <translation>プロジェクトファイル:%1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="378"/>
        <source>Select Cppcheck build dir</source>
        <translation>Cppcheckビルドディレクトリ</translation>
    </message>
    <message>
        <source>Visual Studio (*.sln *.vcxproj);;Compile database (compile_commands.json)</source>
        <translation type="obsolete">Visual Studio (*.sln *.vcxproj);;コンパイルデータベース (compile_commands.json)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="606"/>
        <source>Select include directory</source>
        <translation>includeディレクトリを選択</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="586"/>
        <source>Select a directory to check</source>
        <translation>チェックするディレクトリを選択してください</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="291"/>
        <source>(no rule texts file)</source>
        <translation>(ルールテキストファイルがない)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="297"/>
        <source>Clang-tidy (not found)</source>
        <translation>Clang-tidy (みつかりません)</translation>
    </message>
    <message>
        <source>Visual Studio</source>
        <translation type="obsolete">Visual Studio</translation>
    </message>
    <message>
        <source>Compile database</source>
        <translation type="obsolete">コンパイルデータベース</translation>
    </message>
    <message>
        <source>Borland C++ Builder 6</source>
        <translation type="obsolete">Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="417"/>
        <source>Import Project</source>
        <translation>プロジェクトのインポート</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="626"/>
        <source>Select directory to ignore</source>
        <translation>除外するディレクトリを選択してください</translation>
    </message>
    <message>
        <source>Add Suppression</source>
        <translation type="obsolete">抑制する指摘を追加</translation>
    </message>
    <message>
        <source>Select error id suppress:</source>
        <translation type="obsolete">抑制するエラーID(error id)を選択してください</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="711"/>
        <source>Select MISRA rule texts file</source>
        <translation>MISRAルールテキストファイルを選択</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="711"/>
        <source>Misra rule texts file (%1)</source>
        <translation>Misraルールテキストファイル (%1)</translation>
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
        <translation>キャンセル</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>Save</source>
        <translation>保存する</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="103"/>
        <source>Unknown language specified!</source>
        <translation>未知の言語が指定されました!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="132"/>
        <source>Language file %1 not found!</source>
        <translation>言語ファイル %1 が見つかりません!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="138"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>言語 %2 から %1 への翻訳ファイルの読み込みに失敗</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="33"/>
        <source>line %1: Unhandled element %2</source>
        <translation>行 %1: 扱われていない要素(Unhandled element) %2</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="229"/>
        <source> (Not found)</source>
        <translation> (見つかりません)</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="69"/>
        <source>Thin</source>
        <translation>シン(細)</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="71"/>
        <source>ExtraLight</source>
        <translation>エクストラライト</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="73"/>
        <source>Light</source>
        <translation>ライト</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="75"/>
        <source>Normal</source>
        <translation>ノーマル</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="77"/>
        <source>Medium</source>
        <translation>メディウム</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="79"/>
        <source>DemiBold</source>
        <translation>デミボールト</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="81"/>
        <source>Bold</source>
        <translation>ボールド</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="83"/>
        <source>ExtraBold</source>
        <translation>エクストラボールド</translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="85"/>
        <source>Black</source>
        <translation>黒</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="69"/>
        <source>Editor Foreground Color</source>
        <translation>エディタの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="72"/>
        <source>Editor Background Color</source>
        <translation>エディタの背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="75"/>
        <source>Highlight Background Color</source>
        <translation>ハイライトの背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="78"/>
        <source>Line Number Foreground Color</source>
        <translation>行番号の前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="81"/>
        <source>Line Number Background Color</source>
        <translation>行番号の背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="84"/>
        <source>Keyword Foreground Color</source>
        <translation>キーワードの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="87"/>
        <source>Keyword Font Weight</source>
        <translation>キーワードのフォントのウェイト</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="90"/>
        <source>Class ForegroundColor</source>
        <translation>クラスは前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="93"/>
        <source>Class Font Weight</source>
        <translation>クラスフォントのウェイト</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="96"/>
        <source>Quote Foreground Color</source>
        <translation>クォートの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="99"/>
        <source>Quote Font Weight</source>
        <translation>クォートのフォントウェイト</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="102"/>
        <source>Comment Foreground Color</source>
        <translation>コメントの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="105"/>
        <source>Comment Font Weight</source>
        <translation>コメントフォントのウェイト</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="108"/>
        <source>Symbol Foreground Color</source>
        <translation>シンボルの前景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="111"/>
        <source>Symbol Background Color</source>
        <translation>シンボルの背景色</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="114"/>
        <source>Symbol Font Weight</source>
        <translation>シンボルのフォントウェイト</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="130"/>
        <source>Set to Default Light</source>
        <translation>デフォルトをライトに設定</translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="132"/>
        <source>Set to Default Dark</source>
        <translation>デフォルトをダークに設定</translation>
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
        <translation>キャンセル</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>Save</source>
        <translation>保存する</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>File</source>
        <translation>ファイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Severity</source>
        <translation>警告の種別</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Line</source>
        <translation>行</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Summary</source>
        <translation>要約</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="136"/>
        <source>Undefined file</source>
        <translation>未定義ファイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="614"/>
        <source>Copy</source>
        <translation>コピー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="797"/>
        <source>Could not find file:</source>
        <translation>ファイルが見つかりません:</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="801"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation>フォルダ &apos;%1&apos; を選択してください</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="802"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation>ディレクトリ &apos;%1&apos; 選択</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="804"/>
        <source>Please select the directory where file is located.</source>
        <translation>ファイルのあるディレクトリを選択してください。</translation>
    </message>
    <message>
        <source>[Inconclusive]</source>
        <translation type="obsolete">[結論の出ない]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="312"/>
        <source>debug</source>
        <translation>デバッグ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="249"/>
        <source>note</source>
        <translation>注意</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="613"/>
        <source>Recheck</source>
        <translation>再チェック</translation>
    </message>
    <message>
        <source>Copy filename</source>
        <translation type="obsolete">ファイル名をコピー</translation>
    </message>
    <message>
        <source>Copy full path</source>
        <translation type="obsolete">フルパスをコピー</translation>
    </message>
    <message>
        <source>Copy message</source>
        <translation type="obsolete">メッセージをコピー</translation>
    </message>
    <message>
        <source>Copy message id</source>
        <translation type="obsolete">メッセージidをコピー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="615"/>
        <source>Hide</source>
        <translation>非表示</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="616"/>
        <source>Hide all with id</source>
        <translation>IDで非表示を指定</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="617"/>
        <source>Suppress selected id(s)</source>
        <translation>選択したidを抑制</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="618"/>
        <source>Open containing folder</source>
        <translation>含まれるフォルダを開く</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="648"/>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Tag</source>
        <translation>タグ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="650"/>
        <source>No tag</source>
        <translation>タグなし</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="693"/>
        <location filename="resultstree.cpp" line="707"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="694"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>エディタアプリが設定されていません。

Cppcheckの「設定」からテキストファイルを編集するアプリケーションを設定してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="708"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>デフォルトのエディタアプリケーションが指定されていません。

設定からデフォルトのエディタアプリケーションを設定してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="737"/>
        <source>Could not find the file!</source>
        <translation>ファイルが見つかりません!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="783"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>%1 が実行できません。

実行ファイルパスや引数の設定を確認してください。</translation>
    </message>
    <message>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation type="obsolete">ファイルが見つかりません:
%1
ディレクトリにファイルが存在するか確認してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="805"/>
        <source>Select Directory</source>
        <translation>ディレクトリを選択</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Inconclusive</source>
        <translation>結論のでない</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1341"/>
        <source>Since date</source>
        <translation>日付</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="294"/>
        <source>style</source>
        <translation>スタイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="297"/>
        <source>error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="300"/>
        <source>warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="303"/>
        <source>performance</source>
        <translation>パフォーマンス</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="306"/>
        <source>portability</source>
        <translation>移植可能性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="309"/>
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
        <location filename="resultsview.ui" line="82"/>
        <source>Analysis Log</source>
        <translation>チェックログ</translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="104"/>
        <source>Warning Details</source>
        <translation>警告の詳細</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="159"/>
        <source>No errors found, nothing to save.</source>
        <translation>警告/エラーが見つからなかったため、保存しません。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="183"/>
        <location filename="resultsview.cpp" line="191"/>
        <source>Failed to save the report.</source>
        <translation>レポートの保存に失敗しました。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="201"/>
        <source>Print Report</source>
        <translation>レポートの印刷</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="220"/>
        <source>No errors found, nothing to print.</source>
        <translation>指摘がないため、印刷するものがありません。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="264"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 / %2 :ファイル数)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="277"/>
        <location filename="resultsview.cpp" line="288"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="278"/>
        <source>No errors found.</source>
        <translation>警告/エラーは見つかりませんでした。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="285"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>警告/エラーが見つかりましたが、非表示設定になっています。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="331"/>
        <location filename="resultsview.cpp" line="350"/>
        <source>Failed to read the report.</source>
        <translation>レポートの読み込みに失敗.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="338"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>XML フォーマットバージョン 1 はもうサポートされていません。</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation type="obsolete">内容</translation>
    </message>
    <message>
        <source>Message</source>
        <translation type="obsolete">メッセージ</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="399"/>
        <source>First included by</source>
        <translation>は次のものが最初にインクルードしました</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="402"/>
        <source>Id</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="471"/>
        <source>Clear Log</source>
        <translation>ログの消去</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="472"/>
        <source>Copy this Log entry</source>
        <translation>このログ項目をコピー</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="473"/>
        <source>Copy complete Log</source>
        <translation>ログ全体をコピー</translation>
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
        <source>Include paths:</source>
        <translation type="obsolete">Include ディレクトリ:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="195"/>
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
        <location filename="settings.ui" line="176"/>
        <source>Show internal warnings in log</source>
        <translation>ログの内部警告も表示する</translation>
    </message>
    <message>
        <location filename="settings.ui" line="287"/>
        <source>Addons</source>
        <translation>アドオン</translation>
    </message>
    <message>
        <location filename="settings.ui" line="293"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation>Pythonインタプリタの場所(空白の場合システムのPATHから検索)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="302"/>
        <location filename="settings.ui" line="334"/>
        <location filename="settings.ui" line="379"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="312"/>
        <source>Misra addon</source>
        <translation>Misraアドオン</translation>
    </message>
    <message>
        <location filename="settings.ui" line="320"/>
        <source>Misra rule texts file</source>
        <translation>Misra ルールテキストファイル</translation>
    </message>
    <message>
        <location filename="settings.ui" line="327"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdfのテキストをテキストファイルにコピー&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="360"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="settings.ui" line="366"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation>Clangの場所(空白の場合システムのPATHから検索)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="389"/>
        <source>Visual Studio headers</source>
        <translation>Visual Studioのヘッダ</translation>
    </message>
    <message>
        <location filename="settings.ui" line="395"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Visual Studioのヘッダーファイル(セミコロン区切り&apos;;&apos;)。&lt;/p&gt;&lt;p&gt;Visual Studio コマンドプロンプトを開き、 &amp;quot;SET INCLUDE&amp;quot;. と入力後、そのパスをコピーペーストしてください。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="425"/>
        <source>Code Editor</source>
        <translation>コードエディタ</translation>
    </message>
    <message>
        <location filename="settings.ui" line="431"/>
        <source>Code Editor Style</source>
        <translation>コードエディタスタイル</translation>
    </message>
    <message>
        <location filename="settings.ui" line="437"/>
        <source>Default Light Style</source>
        <translation>ライトスタイルをデフォルトに</translation>
    </message>
    <message>
        <location filename="settings.ui" line="444"/>
        <source>Default Dark Style</source>
        <translation>ダークスタイルをデフォルトに</translation>
    </message>
    <message>
        <location filename="settings.ui" line="453"/>
        <source>Custom</source>
        <translation>カスタム</translation>
    </message>
    <message>
        <source>Paths</source>
        <translation type="obsolete">パス</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="obsolete">編集</translation>
    </message>
    <message>
        <location filename="settings.ui" line="209"/>
        <source>Remove</source>
        <translation>削除</translation>
    </message>
    <message>
        <location filename="settings.ui" line="184"/>
        <source>Applications</source>
        <translation>アプリケーション</translation>
    </message>
    <message>
        <location filename="settings.ui" line="202"/>
        <location filename="settings.ui" line="460"/>
        <source>Edit...</source>
        <translation>編集...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="216"/>
        <source>Set as default</source>
        <translation>デフォルトとして設定</translation>
    </message>
    <message>
        <location filename="settings.ui" line="239"/>
        <source>Reports</source>
        <translation>レポート</translation>
    </message>
    <message>
        <location filename="settings.ui" line="245"/>
        <source>Save all errors when creating report</source>
        <translation>レポート作成時にすべての警告/エラーを保存</translation>
    </message>
    <message>
        <location filename="settings.ui" line="252"/>
        <source>Save full path to files in reports</source>
        <translation>レポートにファイルのフルパスを保存</translation>
    </message>
    <message>
        <location filename="settings.ui" line="273"/>
        <source>Language</source>
        <translation>言語</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="obsolete">高度</translation>
    </message>
    <message>
        <source>&amp;Show inconclusive errors</source>
        <translation type="obsolete">結論の出ないのエラーを表示</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="99"/>
        <source>N/A</source>
        <translation>N/A</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="200"/>
        <source>Add a new application</source>
        <translation>新しいアプリケーションの追加</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="233"/>
        <source>Modify an application</source>
        <translation>アプリケーションの変更</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="238"/>
        <source> [Default]</source>
        <translation> [デフォルト]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="263"/>
        <source>[Default]</source>
        <translation>[デフォルト]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="318"/>
        <source>Select python binary</source>
        <translation>pythonの場所の選択</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="325"/>
        <source>Select MISRA File</source>
        <translation>MISRAファイルの選択</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="352"/>
        <source>Select clang path</source>
        <translation>clangのパスの選択</translation>
    </message>
    <message>
        <source>Select include directory</source>
        <translation type="obsolete">include ディレクトリを選択</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="248"/>
        <location filename="statsdialog.cpp" line="137"/>
        <location filename="statsdialog.cpp" line="184"/>
        <source>Statistics</source>
        <translation>統計情報</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="175"/>
        <source>Project</source>
        <translation>プロジェクト</translation>
    </message>
    <message>
        <location filename="stats.ui" line="33"/>
        <source>Project:</source>
        <translation>プロジェクト:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="53"/>
        <source>Paths:</source>
        <translation>パス:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="85"/>
        <source>Include paths:</source>
        <translation>インクルードパス:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation>定義(define):</translation>
    </message>
    <message>
        <location filename="stats.ui" line="131"/>
        <source>Undefines:</source>
        <translation>定義取り消し(undef):</translation>
    </message>
    <message>
        <location filename="stats.ui" line="165"/>
        <location filename="statsdialog.cpp" line="180"/>
        <source>Previous Scan</source>
        <translation>前回の解析</translation>
    </message>
    <message>
        <location filename="stats.ui" line="171"/>
        <source>Path Selected:</source>
        <translation>ディレクトリ選択:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Number of Files Scanned:</source>
        <translation>解析済みファイル数:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="201"/>
        <source>Scan Duration:</source>
        <translation>解析にかかった時間:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="256"/>
        <source>Errors:</source>
        <translation>エラー:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="274"/>
        <source>Warnings:</source>
        <translation>警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="292"/>
        <source>Stylistic warnings:</source>
        <translation>スタイル警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="310"/>
        <source>Portability warnings:</source>
        <translation>移植可能性の警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="328"/>
        <source>Performance issues:</source>
        <translation>パフォーマンス警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="346"/>
        <source>Information messages:</source>
        <translation>情報メッセージ:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="363"/>
        <source>History</source>
        <translation>ヒストリー</translation>
    </message>
    <message>
        <location filename="stats.ui" line="369"/>
        <source>File:</source>
        <translation>ファイル:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="407"/>
        <source>Copy to Clipboard</source>
        <translation>クリップボードにコピー</translation>
    </message>
    <message>
        <location filename="stats.ui" line="414"/>
        <source>Pdf Export</source>
        <translation>PDF エクスポート</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>1 day</source>
        <translation>一日</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>%1 days</source>
        <translation>%1日</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>1 hour</source>
        <translation>一時間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>%1 hours</source>
        <translation>%1時間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>1 minute</source>
        <translation>一分</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>%1 minutes</source>
        <translation>%1分</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="119"/>
        <source>1 second</source>
        <translation>一秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="119"/>
        <source>%1 seconds</source>
        <translation>%1秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="123"/>
        <source>0.%1 seconds</source>
        <translation>0.%1秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="125"/>
        <source> and </source>
        <translation></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="152"/>
        <source>Export PDF</source>
        <translation>PDF エクスポート</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="174"/>
        <source>Project Settings</source>
        <translation>プロジェクトの設定</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="176"/>
        <source>Paths</source>
        <translation>パス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="177"/>
        <source>Include paths</source>
        <translation>インクルードパス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="178"/>
        <source>Defines</source>
        <translation>定義(define)</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="179"/>
        <source>Undefines</source>
        <translation>定義取り消し(Undef)</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="181"/>
        <source>Path selected</source>
        <translation>選択されたパス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="182"/>
        <source>Number of files scanned</source>
        <translation>スキャンしたファイルの数</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="183"/>
        <source>Scan duration</source>
        <translation>スキャン期間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="139"/>
        <location filename="statsdialog.cpp" line="185"/>
        <source>Errors</source>
        <translation>エラー</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="65"/>
        <source>File: </source>
        <translation>ファイル: </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="65"/>
        <source>No cppcheck build dir</source>
        <translation>cppcheckビルドディレクトリがありません</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="141"/>
        <location filename="statsdialog.cpp" line="186"/>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="143"/>
        <location filename="statsdialog.cpp" line="187"/>
        <source>Style warnings</source>
        <translation>スタイル警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="145"/>
        <location filename="statsdialog.cpp" line="188"/>
        <source>Portability warnings</source>
        <translation>移植可能性警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="147"/>
        <location filename="statsdialog.cpp" line="189"/>
        <source>Performance warnings</source>
        <translation>パフォーマンス警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="149"/>
        <location filename="statsdialog.cpp" line="190"/>
        <source>Information messages</source>
        <translation>情報メッセージ</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="54"/>
        <source>%1 of %2 files checked</source>
        <translation>チェック: %1 / %2 (ファイル数)</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="144"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>ユーザーインターフェースの言語 %1 への変更に失敗しました。

そのため言語を 英語にリセットします。設定ダイアログから利用可能な言語を選択してください。</translation>
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
        <translation>結論の出ない</translation>
    </message>
</context>
<context>
    <name>toFilterString</name>
    <message>
        <location filename="common.cpp" line="52"/>
        <source>All supported files (%1)</source>
        <translation>全サポートファイル (%1)</translation>
    </message>
    <message>
        <location filename="common.cpp" line="57"/>
        <source>All files (%1)</source>
        <translation>全ファイル(%1)</translation>
    </message>
</context>
</TS>
