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
        <source>Copyright © 2007-2018 Cppcheck team.</source>
        <oldsource>Copyright © 2007-2017 Cppcheck team.</oldsource>
        <translation>Copyright © 2007-2018 Cppcheck team.</translation>
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
        <translation>ここにエラー指摘のあるファイルを開くアプリケーションを追加できます。そのアプリケーションの名前、実行ファイル名、コマンドラインパラメータを指定してください。

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
        <translation>名称(&amp;N):</translation>
    </message>
    <message>
        <location filename="application.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>実行バイナリ(&amp;E):</translation>
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
        <translation>アプリケーションの名称とパスと(オプションの)引数を指定してください!</translation>
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
        <translation type="vanished">constant(定数)</translation>
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
        <translation type="vanished">Cppcheck ログ</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">消去</translation>
    </message>
    <message>
        <source>Save Log</source>
        <translation type="vanished">ログ保存</translation>
    </message>
    <message>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation type="vanished">テキストファイル (*.txt *.log);;すべてのファイル(*.*)</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="vanished">Cppcheck</translation>
    </message>
    <message>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation type="vanished">ファイルを書き込みできない</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.ui" line="594"/>
        <location filename="mainwindow.cpp" line="326"/>
        <location filename="mainwindow.cpp" line="491"/>
        <location filename="mainwindow.cpp" line="563"/>
        <location filename="mainwindow.cpp" line="668"/>
        <location filename="mainwindow.cpp" line="690"/>
        <location filename="mainwindow.cpp" line="1116"/>
        <location filename="mainwindow.cpp" line="1241"/>
        <location filename="mainwindow.cpp" line="1362"/>
        <location filename="mainwindow.cpp" line="1501"/>
        <location filename="mainwindow.cpp" line="1524"/>
        <location filename="mainwindow.cpp" line="1604"/>
        <location filename="mainwindow.cpp" line="1672"/>
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
        <location filename="mainwindow.ui" line="143"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation>&amp;C標準</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="169"/>
        <source>&amp;Edit</source>
        <translation>編集(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="182"/>
        <source>Standard</source>
        <translation>言語規格</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="200"/>
        <source>Categories</source>
        <translation>カテゴリ</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="230"/>
        <source>&amp;License...</source>
        <translation>ライセンス(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="235"/>
        <source>A&amp;uthors...</source>
        <translation>作者(&amp;u)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="244"/>
        <source>&amp;About...</source>
        <translation>Cppcheckについて(&amp;A)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="249"/>
        <source>&amp;Files...</source>
        <translation>ファイル選択(&amp;F)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="252"/>
        <location filename="mainwindow.ui" line="255"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation>ファイルをチェックする</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="258"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="267"/>
        <source>&amp;Directory...</source>
        <translation>ディレクトリ選択(&amp;D)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="270"/>
        <location filename="mainwindow.ui" line="273"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation>ディレクトリをチェックする</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="276"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>&amp;Recheck files</source>
        <translation type="obsolete">再チェック(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="288"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <source>&amp;Reanalyze all files</source>
        <oldsource>&amp;Recheck all files</oldsource>
        <translation type="obsolete">全ファイル再チェック</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="306"/>
        <source>&amp;Stop</source>
        <translation>停止(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="309"/>
        <location filename="mainwindow.ui" line="312"/>
        <source>Stop analysis</source>
        <oldsource>Stop checking</oldsource>
        <translation>チェックを停止する</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="315"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="324"/>
        <source>&amp;Save results to file...</source>
        <translation>結果をファイルに保存(&amp;S)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="327"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="332"/>
        <source>&amp;Quit</source>
        <translation>終了(&amp;Q)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="341"/>
        <source>&amp;Clear results</source>
        <translation>結果をクリア(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="350"/>
        <source>&amp;Preferences</source>
        <translation>設定(&amp;P)</translation>
    </message>
    <message>
        <source>Style warnings</source>
        <translation type="vanished">スタイル警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="365"/>
        <location filename="mainwindow.ui" line="368"/>
        <source>Show style warnings</source>
        <translation>スタイル警告を表示</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation type="vanished">エラー</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="383"/>
        <location filename="mainwindow.ui" line="386"/>
        <source>Show errors</source>
        <translation>エラーを表示</translation>
    </message>
    <message>
        <source>Show S&amp;cratchpad...</source>
        <translation type="vanished">スクラッチパッドを表示</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="779"/>
        <location filename="mainwindow.cpp" line="817"/>
        <source>Information</source>
        <translation>情報</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="567"/>
        <source>Show information messages</source>
        <translation>情報メッセージを表示</translation>
    </message>
    <message>
        <source>Portability</source>
        <translation type="vanished">移植可能性</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="582"/>
        <source>Show portability warnings</source>
        <translation>移植可能性の問題を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="597"/>
        <source>Show Cppcheck results</source>
        <translation>Cppcheck結果を表示する</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="609"/>
        <source>Clang</source>
        <translation>Clang</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="612"/>
        <source>Show Clang results</source>
        <translation>Clangの結果を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="620"/>
        <source>&amp;Filter</source>
        <translation>フィルター(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="623"/>
        <source>Filter results</source>
        <translation>フィルタ結果</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="639"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSIエンコード</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="647"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="655"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="663"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="671"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <source>Platforms</source>
        <translation type="vanished">プラットフォーム</translation>
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
        <location filename="mainwindow.ui" line="741"/>
        <source>&amp;Print...</source>
        <translation>印刷(&amp;P)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="744"/>
        <source>Print the Current Report</source>
        <translation>現在のレポートを印刷</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="749"/>
        <source>Print Pre&amp;view...</source>
        <translation>印刷プレビュー(&amp;v)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="752"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>現在のレポートをプレビュー表示</translation>
    </message>
    <message>
        <source>Library Editor...</source>
        <translation type="vanished">ライブラリの編集</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="760"/>
        <source>Open library editor</source>
        <translation>ライブラリエディタを開く</translation>
    </message>
    <message>
        <source>Auto-detect language</source>
        <translation type="vanished">言語を自動検出</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="488"/>
        <source>C&amp;lose Project File</source>
        <translation>プロジェクトを閉じる(&amp;l)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="496"/>
        <source>&amp;Edit Project File...</source>
        <translation>プロジェクトの編集(&amp;E)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="508"/>
        <source>&amp;Statistics</source>
        <translation>統計情報(&amp;S)</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation type="vanished">警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="523"/>
        <location filename="mainwindow.ui" line="526"/>
        <source>Show warnings</source>
        <translation>警告を表示</translation>
    </message>
    <message>
        <source>Performance warnings</source>
        <translation type="vanished">パフォーマンス警告</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="541"/>
        <location filename="mainwindow.ui" line="544"/>
        <source>Show performance warnings</source>
        <translation>パフォーマンス警告を表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="552"/>
        <source>Show &amp;hidden</source>
        <translation>非表示を表示(&amp;h)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="391"/>
        <source>&amp;Check all</source>
        <translation>すべてのエラーを表示(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="131"/>
        <source>A&amp;nalyze</source>
        <translation>解析(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="219"/>
        <source>Filter</source>
        <translation>フィルター</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="285"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation>変更ありファイルを再解析(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="297"/>
        <source>Reanal&amp;yze all files</source>
        <translation>全ファイル再解析(&amp;y)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="362"/>
        <source>Style war&amp;nings</source>
        <translation>スタイル警告(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="380"/>
        <source>E&amp;rrors</source>
        <translation>エラー(&amp;r)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="396"/>
        <source>&amp;Uncheck all</source>
        <translation>すべてのエラーを非表示(&amp;U)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="401"/>
        <source>Collapse &amp;all</source>
        <translation>ツリーを折り畳む(&amp;a)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="406"/>
        <source>&amp;Expand all</source>
        <translation>ツリーを展開(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="414"/>
        <source>&amp;Standard</source>
        <translation>言語規格(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="417"/>
        <source>Standard items</source>
        <translation>標準項目</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="422"/>
        <source>&amp;Contents</source>
        <translation>コンテンツ(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="425"/>
        <source>Open the help contents</source>
        <translation>ヘルプファイルを開く</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="428"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="433"/>
        <source>Toolbar</source>
        <translation>ツールバー</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="441"/>
        <source>&amp;Categories</source>
        <translation>カテゴリ(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="444"/>
        <source>Error categories</source>
        <translation>エラーカテゴリ</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="449"/>
        <source>&amp;Open XML...</source>
        <translation>XMLを開く(&amp;O)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="458"/>
        <source>Open P&amp;roject File...</source>
        <translation>プロジェクトを開く(&amp;R)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="467"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation>スクラッチパッドを表示(&amp;o)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="472"/>
        <source>&amp;New Project File...</source>
        <translation>新規プロジェクト(&amp;N)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="477"/>
        <source>&amp;Log View</source>
        <translation>ログを表示(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="480"/>
        <source>Log View</source>
        <translation>ログ表示</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="520"/>
        <source>&amp;Warnings</source>
        <translation>警告(&amp;W)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="538"/>
        <source>Per&amp;formance warnings</source>
        <translation>パフォーマンス警告(&amp;f)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="564"/>
        <source>&amp;Information</source>
        <translation>情報(&amp;I)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="579"/>
        <source>&amp;Portability</source>
        <translation>移植可能性(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="679"/>
        <source>P&amp;latforms</source>
        <translation>プラットフォーム(&amp;l)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="693"/>
        <source>C++&amp;11</source>
        <translation>C++11(&amp;1)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="704"/>
        <source>C&amp;99</source>
        <translation>C99(&amp;9)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="712"/>
        <source>&amp;Posix</source>
        <translation>Posix(&amp;P)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="720"/>
        <source>C&amp;11</source>
        <translation>C11(&amp;1)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="728"/>
        <source>&amp;C89</source>
        <translation>C89(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="736"/>
        <source>&amp;C++03</source>
        <translation>C++03(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="757"/>
        <source>&amp;Library Editor...</source>
        <translation>ライブラリエディタ(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="768"/>
        <source>&amp;Auto-detect language</source>
        <translation>自動言語検出(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="776"/>
        <source>&amp;Enforce C++</source>
        <translation>C++ 強制(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="784"/>
        <source>E&amp;nforce C</source>
        <translation>C 強制(&amp;n)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="795"/>
        <source>C++14</source>
        <translation>C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="803"/>
        <source>Reanalyze and check library</source>
        <translation>ライブラリを再解析する</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="811"/>
        <source>Check configuration (defines, includes)</source>
        <translation>解析の設定(define、インクルード)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="321"/>
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
        <location filename="mainwindow.cpp" line="564"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>新しいファイル／ディレクトリを解析するには現在のプロジェクトを閉じてください!</translation>
    </message>
    <message>
        <source>C/C++ Source, Compile database, Visual Studio (%1 %2 *.sln *.vcxproj)</source>
        <translation type="vanished">C/C++ソースコード、プロジェクトファイル、Visual Studioソリューション(%1 %2 *.sln *.vcxproj)</translation>
    </message>
    <message>
        <source>Select directory to check</source>
        <translation type="obsolete">チェック対象のディレクトリを選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="85"/>
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
        <location filename="mainwindow.cpp" line="639"/>
        <source>Select configuration</source>
        <translation>コンフィグレーションの選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="669"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>プロジェクトファイルが検出されました: %1

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
        <location filename="mainwindow.cpp" line="779"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>このライブラリ &apos;%1&apos; には次の不明な要素が含まれています。
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="788"/>
        <source>File not found</source>
        <translation>ファイルがありません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Bad XML</source>
        <translation>不正なXML</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="794"/>
        <source>Missing attribute</source>
        <translation>属性がありません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="797"/>
        <source>Bad attribute value</source>
        <translation>不正な属性があります</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="800"/>
        <source>Unsupported format</source>
        <translation>サポートされていないフォーマット</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="803"/>
        <source>Duplicate platform type</source>
        <translation>プラットフォームの種類が重複しています</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="806"/>
        <source>Platform type redefined</source>
        <translation>プラットフォームの種類が再定義されました</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="809"/>
        <source>Unknown element</source>
        <translation>不明な要素</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="812"/>
        <source>Unknown issue</source>
        <translation>不明な課題</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="817"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>選択したライブラリの読み込みに失敗しました &apos;%1&apos;
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="929"/>
        <source>Error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located.</source>
        <translation type="obsolete">%1の読み込みに失敗しました。CppCheckのインストールに失敗しています。コマンドライン引数に --data-dir=&lt;directory&gt; を指定して、このファイルの場所を指定してください。 </translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="929"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation>%1のロードに失敗しました。あなたの Cppcheck は正しくインストールされていません。あなたは --data-dir=&lt;directory&gt; コマンドラインオプションでこのファイルの場所を指定してできます。ただし、この --data-dir はインストールスクリプトによってサポートされており、GUI版ではサポートされていません。全ての設定は調整済みでなければなりません。</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1117"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation>現在の結果を作成します。

新しくXMLファイルを開くと現在の結果が削除されます。実行しますか？</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1132"/>
        <location filename="mainwindow.cpp" line="1312"/>
        <source>XML files (*.xml)</source>
        <translation>XML ファイル (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1134"/>
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
        <location filename="mainwindow.cpp" line="1283"/>
        <source>License</source>
        <translation>ライセンス</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1290"/>
        <source>Authors</source>
        <translation>作者</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <oldsource>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</oldsource>
        <translation type="obsolete">XML ファイル (*.xml);;テキストファイル (*.txt);;CSV形式ファイル (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1305"/>
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
        <location filename="mainwindow.cpp" line="1316"/>
        <source>Text files (*.txt)</source>
        <translation>テキストファイル (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1320"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV形式ファイル (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1364"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1411"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>プロジェクトファイル (*.cppcheck);;すべてのファイル(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1413"/>
        <source>Select Project File</source>
        <translation>プロジェクトファイルを選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="156"/>
        <location filename="mainwindow.cpp" line="1441"/>
        <location filename="mainwindow.cpp" line="1573"/>
        <source>Project:</source>
        <translation>プロジェクト:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="492"/>
        <source>No suitable files found to analyze!</source>
        <translation>チェック対象のファイルがみつかりません!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="578"/>
        <source>C/C++ Source</source>
        <translation>C/C++のソースコード</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="579"/>
        <source>Compile database</source>
        <translation>コンパイルデータベース</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="580"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="581"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="584"/>
        <source>Select files to analyze</source>
        <translation>チェック対象のファイルを選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="599"/>
        <source>Select directory to analyze</source>
        <translation>チェックするディレクトリを選択してください</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="639"/>
        <source>Select the configuration that will be analyzed</source>
        <translation>チェックの設定を選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="691"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation>ディレクトリ内にプロジェクトファイルがありました。

みつかったプロジェクトファイルを使用せずにチェックしますか?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1237"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation>チェック中です。

チェックを中断して、Cppcheckを終了しますか?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1303"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>XML ファイル (*.xml);;テキストファイル (*.txt);;CSVファイル (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1502"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation>ビルドディレクトリ&apos;%1&apos;がありません。作成しますか?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1525"/>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation>&apos;%1&apos;のインポートに失敗しました。(チェック中断)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1558"/>
        <source>Project files (*.cppcheck)</source>
        <translation>プロジェクトファイル (*.cppcheck)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1560"/>
        <source>Select Project Filename</source>
        <translation>プロジェクトファイル名を選択</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1605"/>
        <source>No project file loaded</source>
        <translation>プロジェクトファイルが読み込まれていません</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1667"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>このプロジェクトファイル %1 が見つかりません。
最近使用したプロジェクトのリストからこのファイルを取り除きますか？</translation>
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
        <location filename="main.cpp" line="120"/>
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
        <translation type="vanished">Cppcheck</translation>
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
        <translation type="vanished">プロジェクト</translation>
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
        <translation type="vanished">ライブラリ</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="319"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>カスタマイズした cfgファイルを同じフォルダにプロジェクトファイルとして保存してください。ここに表示できるようになります。</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="376"/>
        <source>Exclude paths</source>
        <translation>除外するパス</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="523"/>
        <source>MISRA C 2012</source>
        <translation>MISRA C 2012</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="532"/>
        <source>Misra rule texts</source>
        <translation>Misra ルールテキスト</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="539"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012からテキストをコピーペーストしてください。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="546"/>
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
        <location filename="projectfiledialog.ui" line="288"/>
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
        <location filename="projectfiledialog.ui" line="207"/>
        <location filename="projectfiledialog.ui" line="387"/>
        <source>Add...</source>
        <translation>追加...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="120"/>
        <location filename="projectfiledialog.ui" line="214"/>
        <location filename="projectfiledialog.ui" line="394"/>
        <source>Edit</source>
        <translation>編集</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="127"/>
        <location filename="projectfiledialog.ui" line="221"/>
        <location filename="projectfiledialog.ui" line="401"/>
        <location filename="projectfiledialog.ui" line="444"/>
        <source>Remove</source>
        <translation>取り除く</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="176"/>
        <source>Include Paths:</source>
        <translation>インクルードパス:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="558"/>
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
        <location filename="projectfiledialog.ui" line="241"/>
        <source>Up</source>
        <translation>上</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="248"/>
        <source>Down</source>
        <translation>下</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="273"/>
        <source>Checking</source>
        <translation>解析</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="298"/>
        <source>Platform</source>
        <translation>プラットフォーム</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="346"/>
        <source>Warning options</source>
        <translation>警告オプション</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="352"/>
        <source>Root path:</source>
        <translation>ルートパス:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="364"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation>警告タグ(セミコロン区切り)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="279"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation>Cppcheck ビルドディレクトリ (全プログラムチェック, 差分チェック, 統計等)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="310"/>
        <source>Libraries</source>
        <translation>ライブラリ</translation>
    </message>
    <message>
        <source>Exclude</source>
        <translation type="vanished">除外する</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="426"/>
        <source>Suppressions</source>
        <translation>指摘の抑制</translation>
    </message>
    <message>
        <source>Suppression list:</source>
        <translation type="vanished">抑制リスト</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="437"/>
        <source>Add</source>
        <translation>追加</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="483"/>
        <source>Addons and tools</source>
        <translation>アドオンとツール</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="489"/>
        <source>Addons</source>
        <translation>アドオン</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="495"/>
        <source>Y2038</source>
        <translation>Y2038</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="502"/>
        <source>Thread safety</source>
        <translation>スレッドセーフ</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="509"/>
        <source>Coding standards</source>
        <translation>コーディング標準</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="516"/>
        <source>Cert</source>
        <translation>CERT</translation>
    </message>
    <message>
        <source>Extra Tools</source>
        <translation type="vanished">エクストラツール</translation>
    </message>
    <message>
        <source>It is common best practice to use several tools.</source>
        <translation type="vanished">複数ツールの併用はよい結果を生みます。</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="571"/>
        <source>Clang analyzer</source>
        <translation>Clang Analyzer</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="564"/>
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
        <location filename="projectfiledialog.cpp" line="59"/>
        <source>Project file: %1</source>
        <translation>プロジェクトファイル:%1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="334"/>
        <source>Select Cppcheck build dir</source>
        <translation>Cppcheckビルドディレクトリ</translation>
    </message>
    <message>
        <source>Visual Studio (*.sln *.vcxproj);;Compile database (compile_commands.json)</source>
        <translation type="vanished">Visual Studio (*.sln *.vcxproj);;コンパイルデータベース (compile_commands.json)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="584"/>
        <source>Select include directory</source>
        <translation>includeディレクトリを選択</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="564"/>
        <source>Select a directory to check</source>
        <translation>チェックするディレクトリを選択してください</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="242"/>
        <source>(no rule texts file)</source>
        <translation>(ルールテキストファイルがない)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="248"/>
        <source>Clang-tidy (not found)</source>
        <translation>Clang-tidy (みつかりません)</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="369"/>
        <source>Visual Studio</source>
        <translation>Visual Studio</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="370"/>
        <source>Compile database</source>
        <translation>コンパイルデータベース</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="371"/>
        <source>Borland C++ Builder 6</source>
        <translation>Borland C++ Builder 6</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="372"/>
        <source>Import Project</source>
        <translation>プロジェクトのインポート</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="604"/>
        <source>Select directory to ignore</source>
        <translation>除外するディレクトリを選択してください</translation>
    </message>
    <message>
        <source>Add Suppression</source>
        <translation type="vanished">抑制する指摘を追加</translation>
    </message>
    <message>
        <source>Select error id suppress:</source>
        <translation type="vanished">抑制するエラーID(error id)を選択してください</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="665"/>
        <source>Select MISRA rule texts file</source>
        <translation>MISRAルールテキストファイルを選択</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="665"/>
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
        <location filename="projectfiledialog.cpp" line="184"/>
        <source> (Not found)</source>
        <translation> (みつかりません)</translation>
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
        <location filename="resultstree.cpp" line="1347"/>
        <source>File</source>
        <translation>ファイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1347"/>
        <source>Severity</source>
        <translation>警告種別</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1347"/>
        <source>Line</source>
        <translation>行</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1347"/>
        <source>Summary</source>
        <translation>内容</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="137"/>
        <source>Undefined file</source>
        <translation>未定義ファイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="615"/>
        <source>Copy</source>
        <translation>コピー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="798"/>
        <source>Could not find file:</source>
        <translation>ファイルが見つかりません:</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="802"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation>フォルダ &apos;%1&apos; を選択してください</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="803"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation>ディレクトリ &apos;%1&apos; 選択</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="805"/>
        <source>Please select the directory where file is located.</source>
        <translation>ファイルのあるディレクトリを選択してください。</translation>
    </message>
    <message>
        <source>[Inconclusive]</source>
        <translation type="vanished">[結論の出ない]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="313"/>
        <source>debug</source>
        <translation>デバッグ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="250"/>
        <source>note</source>
        <translation>注意</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="614"/>
        <source>Recheck</source>
        <translation>再チェック</translation>
    </message>
    <message>
        <source>Copy filename</source>
        <translation type="vanished">ファイル名をコピー</translation>
    </message>
    <message>
        <source>Copy full path</source>
        <translation type="vanished">フルパスをコピー</translation>
    </message>
    <message>
        <source>Copy message</source>
        <translation type="vanished">メッセージをコピー</translation>
    </message>
    <message>
        <source>Copy message id</source>
        <translation type="vanished">メッセージidをコピー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="616"/>
        <source>Hide</source>
        <translation>非表示</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="617"/>
        <source>Hide all with id</source>
        <translation>IDを表示しない</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="618"/>
        <source>Suppress selected id(s)</source>
        <translation>選択したidを抑制</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="619"/>
        <source>Open containing folder</source>
        <translation>含まれるフォルダを開く</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="649"/>
        <location filename="resultstree.cpp" line="1347"/>
        <source>Tag</source>
        <translation>タグ</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="651"/>
        <source>No tag</source>
        <translation>タグなし</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="694"/>
        <location filename="resultstree.cpp" line="708"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="695"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>エディタアプリが設定されていません。

Cppcheckの「設定」からテキストファイルを編集するアプリケーションを設定してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="709"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>デフォルトのエディタアプリケーションが指定されていません。

設定からデフォルトのエディタアプリケーションを設定してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="738"/>
        <source>Could not find the file!</source>
        <translation>ファイルが見つかりません!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="784"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>%1 が実行できません。

実行ファイルパスや引数の設定を確認してください。</translation>
    </message>
    <message>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation type="vanished">ファイルが見つかりません:
%1
ディレクトリにファイルが存在するか確認してください。</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="806"/>
        <source>Select Directory</source>
        <translation>ディレクトリを選択</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1347"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1347"/>
        <source>Inconclusive</source>
        <translation>結論のでない</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1347"/>
        <source>Since date</source>
        <translation>日付</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="295"/>
        <source>style</source>
        <translation>スタイル</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="298"/>
        <source>error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="301"/>
        <source>warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="304"/>
        <source>performance</source>
        <translation>パフォーマンス</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="307"/>
        <source>portability</source>
        <translation>移植可能性</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="310"/>
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
        <location filename="resultsview.cpp" line="156"/>
        <source>No errors found, nothing to save.</source>
        <translation>警告/エラーが見つからなかったため、保存しません。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="180"/>
        <location filename="resultsview.cpp" line="188"/>
        <source>Failed to save the report.</source>
        <translation>レポートの保存に失敗しました。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="198"/>
        <source>Print Report</source>
        <translation>レポートの印刷</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="217"/>
        <source>No errors found, nothing to print.</source>
        <translation>指摘がないため、印刷するものがありません。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="255"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 / %2 :ファイル数)</translation>
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
        <translation>警告/エラーは見つかりませんでした。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="276"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>警告/エラーが見つかりましたが、非表示設定になっています。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="322"/>
        <location filename="resultsview.cpp" line="343"/>
        <source>Failed to read the report.</source>
        <translation>レポートの読み込みに失敗.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="329"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation>XML フォーマットバージョン 1 はもうサポートされていません。</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="393"/>
        <source>Summary</source>
        <translation>内容</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="394"/>
        <source>Message</source>
        <translation>メッセージ</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="398"/>
        <source>First included by</source>
        <translation>は次のものが最初にインクルードしました</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="401"/>
        <source>Id</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="470"/>
        <source>Clear Log</source>
        <translation>ログの消去</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="471"/>
        <source>Copy this Log entry</source>
        <translation>このログ項目をコピー</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="472"/>
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
        <translation>オプション: ファイル名を入力(言語は自動判定)して&quot;解析&quot;をクリックしてください:</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="71"/>
        <source>filename</source>
        <translation>ファイル名</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="78"/>
        <source>Check</source>
        <translation>解析</translation>
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
        <translation type="vanished">Include ディレクトリ:</translation>
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
        <translation>inline抑制を有効</translation>
    </message>
    <message>
        <location filename="settings.ui" line="149"/>
        <source>Check for inconclusive errors also</source>
        <translation>結論のでない指摘も解析する</translation>
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
        <source>Paths</source>
        <translation type="vanished">パス</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="vanished">編集</translation>
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
        <translation>すべての警告/エラーを保存</translation>
    </message>
    <message>
        <location filename="settings.ui" line="252"/>
        <source>Save full path to files in reports</source>
        <translation>ファイルのフルパスを保存</translation>
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
        <location filename="settingsdialog.cpp" line="90"/>
        <source>N/A</source>
        <translation>N/A</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="189"/>
        <source>Add a new application</source>
        <translation>新しいアプリケーションの追加</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="222"/>
        <source>Modify an application</source>
        <translation>アプリケーションの変更</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="227"/>
        <source> [Default]</source>
        <translation> [デフォルト]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="252"/>
        <source>[Default]</source>
        <translation>[デフォルト]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="307"/>
        <source>Select python binary</source>
        <translation>pythonの場所の選択</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="314"/>
        <source>Select MISRA File</source>
        <translation>MISRAファイルの選択</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="322"/>
        <source>Select clang path</source>
        <translation>clangのパスの選択</translation>
    </message>
    <message>
        <source>Select include directory</source>
        <translation type="vanished">include ディレクトリを選択</translation>
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
        <translation>統計情報</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="173"/>
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
        <translation>Include ディレクトリ:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation>Defines:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="145"/>
        <location filename="statsdialog.cpp" line="177"/>
        <source>Previous Scan</source>
        <translation>前回の解析</translation>
    </message>
    <message>
        <location filename="stats.ui" line="151"/>
        <source>Path Selected:</source>
        <translation>ディレクトリ選択:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="161"/>
        <source>Number of Files Scanned:</source>
        <translation>解析済みファイル数:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Scan Duration:</source>
        <translation>解析にかかった時間:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="236"/>
        <source>Errors:</source>
        <translation>エラー:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="254"/>
        <source>Warnings:</source>
        <translation>警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="272"/>
        <source>Stylistic warnings:</source>
        <translation>スタイル警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="290"/>
        <source>Portability warnings:</source>
        <translation>移植可能性の警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="308"/>
        <source>Performance issues:</source>
        <translation>パフォーマンス警告:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="326"/>
        <source>Information messages:</source>
        <translation>情報メッセージ:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="343"/>
        <source>History</source>
        <translation>ヒストリー</translation>
    </message>
    <message>
        <location filename="stats.ui" line="349"/>
        <source>File:</source>
        <translation>ファイル:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="387"/>
        <source>Copy to Clipboard</source>
        <translation>クリップボードにコピー</translation>
    </message>
    <message>
        <location filename="stats.ui" line="394"/>
        <source>Pdf Export</source>
        <translation>PDF エクスポート</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>1 day</source>
        <translation>一日</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="111"/>
        <source>%1 days</source>
        <translation>%1日</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>1 hour</source>
        <translation>一時間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>%1 hours</source>
        <translation>%1時間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>1 minute</source>
        <translation>一分</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>%1 minutes</source>
        <translation>%1分</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>1 second</source>
        <translation>一秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>%1 seconds</source>
        <translation>%1秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="121"/>
        <source>0.%1 seconds</source>
        <translation>0.%1秒</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="123"/>
        <source> and </source>
        <translation></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="150"/>
        <source>Export PDF</source>
        <translation>PDF エクスポート</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="172"/>
        <source>Project Settings</source>
        <translation>プロジェクトの設定</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="174"/>
        <source>Paths</source>
        <translation>パス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="175"/>
        <source>Include paths</source>
        <translation>インクルードパス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="176"/>
        <source>Defines</source>
        <translation>定義(define)</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="178"/>
        <source>Path selected</source>
        <translation>選択されたパス</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="179"/>
        <source>Number of files scanned</source>
        <translation>スキャンしたファイルの数</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="180"/>
        <source>Scan duration</source>
        <translation>スキャン期間</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="137"/>
        <location filename="statsdialog.cpp" line="182"/>
        <source>Errors</source>
        <translation>エラー</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="64"/>
        <source>File: </source>
        <translation>ファイル: </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="64"/>
        <source>No cppcheck build dir</source>
        <translation>cppcheckビルドディレクトリがありません</translation>
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
        <translation>スタイル警告</translation>
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
        <translation>パフォーマンス警告</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="147"/>
        <location filename="statsdialog.cpp" line="187"/>
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
