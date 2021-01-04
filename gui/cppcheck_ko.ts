<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ko-KR">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>Cppcheck 정보</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>버전 %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - 정적 C/C++ 코드 분석 도구.</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>이 프로그램은 GNU General Public License version 3을
준수합니다</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Cppcheck 홈페이지(%1)를 방문해보세요</translation>
    </message>
    <message>
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-%1 Cppcheck team.</source>
        <oldsource>Copyright © 2007-2019 Cppcheck team.</oldsource>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="application.ui" line="23"/>
        <source>Add an application</source>
        <translation>응용 프로그램 추가</translation>
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
        <translation>에러 파일을 열 응용 프로그램을 추가할 수 있습니다. 응용 프로그램의 이름, 실행 파일, 명령행 인자를 입력하세요.

인자 중에서 아래와 같은 텍스트는 응용 프로그램 실행 시 해당 값으로 대치됩니다:
(file) - 에러를 포함한 파일이름
(line) - 에러를 포함한 행번호
(message) - 에러 메시지
(severity) - 에러 종류

Kate로 파일을 열고, 해당 행으로 이동하는 예제:
실행파일: kate
인자: -l(line) (file)</translation>
    </message>
    <message>
        <location filename="application.ui" line="76"/>
        <source>&amp;Name:</source>
        <translation>이름(&amp;N):</translation>
    </message>
    <message>
        <location filename="application.ui" line="86"/>
        <source>&amp;Executable:</source>
        <translation>실행 파일(&amp;E):</translation>
    </message>
    <message>
        <location filename="application.ui" line="96"/>
        <source>&amp;Parameters:</source>
        <translation>명령행 인자(&amp;P):</translation>
    </message>
    <message>
        <location filename="application.ui" line="138"/>
        <source>Browse</source>
        <translation>찾기</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>실행 파일(*.exe);;모든 파일(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="62"/>
        <source>Select viewer application</source>
        <translation>뷰어 프로그램 선택</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="77"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <source>You must specify a name, a path and parameters for the application!</source>
        <translation type="obsolete">응용 프로그램의 이름, 경로 및 인자를 명시해야 합니다!</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="78"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="42"/>
        <source>Could not find the file: %1</source>
        <translation>파일 찾기 실패: %1</translation>
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
        <translation>파일 읽기 실패: %1</translation>
    </message>
</context>
<context>
    <name>FunctionContractDialog</name>
    <message>
        <location filename="functioncontractdialog.ui" line="14"/>
        <source>Function contract</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="functioncontractdialog.ui" line="20"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="functioncontractdialog.ui" line="27"/>
        <source>Requirements for parameters</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>HelpDialog</name>
    <message>
        <location filename="helpdialog.ui" line="14"/>
        <source>Cppcheck GUI help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="34"/>
        <source>Contents</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.ui" line="44"/>
        <source>Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="57"/>
        <source>Helpfile &apos;%1&apos; was not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="59"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
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
        <location filename="librarydialog.ui" line="62"/>
        <source>Functions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="111"/>
        <source>Add</source>
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
        <translation type="unfinished">편집</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="81"/>
        <location filename="librarydialog.cpp" line="153"/>
        <source>Library files (*.cfg)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="83"/>
        <source>Open library file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui" line="71"/>
        <source>Sort</source>
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
        <location filename="librarydialog.ui" line="36"/>
        <source>Save as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="94"/>
        <location filename="librarydialog.cpp" line="106"/>
        <location filename="librarydialog.cpp" line="143"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="156"/>
        <source>Save the library as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="107"/>
        <source>Failed to load %1. %2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="95"/>
        <source>Cannot open file %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="144"/>
        <source>Cannot save file %1.</source>
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
        <translation type="obsolete">로그 확인</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="obsolete">지우기</translation>
    </message>
    <message>
        <source>Save Log</source>
        <translation type="obsolete">로그 저장</translation>
    </message>
    <message>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation type="obsolete">텍스트 파일 (*.txt *.log);;모든 파일 (*.*)</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="obsolete">Cppcheck</translation>
    </message>
    <message>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation type="obsolete">기록할 파일 열기 실패: &quot;%1&quot;</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui" line="26"/>
        <location filename="mainwindow.ui" line="604"/>
        <location filename="mainwindow.cpp" line="328"/>
        <location filename="mainwindow.cpp" line="484"/>
        <location filename="mainwindow.cpp" line="557"/>
        <location filename="mainwindow.cpp" line="689"/>
        <location filename="mainwindow.cpp" line="711"/>
        <location filename="mainwindow.cpp" line="1190"/>
        <location filename="mainwindow.cpp" line="1315"/>
        <location filename="mainwindow.cpp" line="1585"/>
        <location filename="mainwindow.cpp" line="1608"/>
        <location filename="mainwindow.cpp" line="1685"/>
        <location filename="mainwindow.cpp" line="1760"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="70"/>
        <source>&amp;File</source>
        <translation>파일(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="89"/>
        <source>&amp;View</source>
        <translation>보기(&amp;V)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="93"/>
        <source>&amp;Toolbars</source>
        <translation>도구바(&amp;T)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="121"/>
        <source>&amp;Help</source>
        <translation>도움말(&amp;H)</translation>
    </message>
    <message>
        <source>&amp;Check</source>
        <translation type="obsolete">검사(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="170"/>
        <source>&amp;Edit</source>
        <translation>편집(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="183"/>
        <source>Standard</source>
        <translation>표준 도구</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="201"/>
        <source>Categories</source>
        <translation>분류 도구</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="220"/>
        <source>Filter</source>
        <translation>필터 도구</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="231"/>
        <source>&amp;License...</source>
        <translation>저작권(&amp;L)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="236"/>
        <source>A&amp;uthors...</source>
        <translation>제작자(&amp;u)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="245"/>
        <source>&amp;About...</source>
        <translation>정보(&amp;A)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="250"/>
        <source>&amp;Files...</source>
        <translation>파일(&amp;F)...</translation>
    </message>
    <message>
        <source>Check files</source>
        <translation type="obsolete">파일 검사</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="259"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="268"/>
        <source>&amp;Directory...</source>
        <translation>디렉토리(&amp;D)...</translation>
    </message>
    <message>
        <source>Check directory</source>
        <translation type="obsolete">디렉토리 검사</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="277"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>&amp;Recheck files</source>
        <translation type="obsolete">파일 재검사(&amp;R)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="289"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="307"/>
        <source>&amp;Stop</source>
        <translation>중지(&amp;S)</translation>
    </message>
    <message>
        <source>Stop checking</source>
        <translation type="obsolete">검사 중지</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="316"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="325"/>
        <source>&amp;Save results to file...</source>
        <translation>결과를 파일에 저장(&amp;S)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="328"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="333"/>
        <source>&amp;Quit</source>
        <translation>종료(&amp;Q)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="345"/>
        <source>&amp;Clear results</source>
        <translation>결과 지우기(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="354"/>
        <source>&amp;Preferences</source>
        <translation>설정(&amp;P)</translation>
    </message>
    <message>
        <source>Style warnings</source>
        <translation type="obsolete">스타일 경고</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="369"/>
        <location filename="mainwindow.ui" line="372"/>
        <source>Show style warnings</source>
        <translation>스타일 경고 표시</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation type="obsolete">에러</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="387"/>
        <location filename="mainwindow.ui" line="390"/>
        <source>Show errors</source>
        <translation>애러 표시</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="395"/>
        <source>&amp;Check all</source>
        <translation>전체 선택(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="400"/>
        <source>&amp;Uncheck all</source>
        <translation>전체 해제(&amp;U)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="405"/>
        <source>Collapse &amp;all</source>
        <translation>전체 접기(&amp;A)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="410"/>
        <source>&amp;Expand all</source>
        <translation>전체 펼치기(&amp;E)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="418"/>
        <source>&amp;Standard</source>
        <translation>표준 도구(&amp;S)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="421"/>
        <source>Standard items</source>
        <translation>표준 아이템</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="426"/>
        <source>&amp;Contents</source>
        <translation>내용(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="429"/>
        <source>Open the help contents</source>
        <translation>도움말을 엽니다</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="432"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="437"/>
        <source>Toolbar</source>
        <translation>도구바</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="445"/>
        <source>&amp;Categories</source>
        <translation>분류 도구(&amp;C)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="448"/>
        <source>Error categories</source>
        <translation>에러 종류</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="453"/>
        <source>&amp;Open XML...</source>
        <translation>XML 열기(&amp;O)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="462"/>
        <source>Open P&amp;roject File...</source>
        <translation>프로젝트 파일 열기(&amp;R)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="479"/>
        <source>&amp;New Project File...</source>
        <translation>새 프로젝트 파일(&amp;N)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="487"/>
        <source>&amp;Log View</source>
        <translation>로그 보기(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="490"/>
        <source>Log View</source>
        <translation>로그 보기</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="498"/>
        <source>C&amp;lose Project File</source>
        <translation>프로젝트 파일 닫기(&amp;L)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="506"/>
        <source>&amp;Edit Project File...</source>
        <translation>프로젝트 파일 편집(&amp;E)...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="518"/>
        <source>&amp;Statistics</source>
        <translation>통계 보기(&amp;S)</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation type="obsolete">경고</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="533"/>
        <location filename="mainwindow.ui" line="536"/>
        <source>Show warnings</source>
        <translation>경고 표시</translation>
    </message>
    <message>
        <source>Performance warnings</source>
        <translation type="obsolete">성능 경고</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="551"/>
        <location filename="mainwindow.ui" line="554"/>
        <source>Show performance warnings</source>
        <translation>성능 경고 표시</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="562"/>
        <source>Show &amp;hidden</source>
        <translation>숨기기 보기(&amp;H)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="800"/>
        <location filename="mainwindow.cpp" line="838"/>
        <source>Information</source>
        <translation>정보</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="577"/>
        <source>Show information messages</source>
        <translation>정보 표시</translation>
    </message>
    <message>
        <source>Portability</source>
        <translation type="obsolete">이식성 경고</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="592"/>
        <source>Show portability warnings</source>
        <translation>이식성 경고 표시</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="630"/>
        <source>&amp;Filter</source>
        <translation>필터 도구(&amp;F)</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="633"/>
        <source>Filter results</source>
        <translation>필터링 결과</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="649"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSI</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="657"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="665"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="673"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="681"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <source>Platforms</source>
        <translation type="obsolete">플랫폼</translation>
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
        <location filename="mainwindow.cpp" line="83"/>
        <source>Quick Filter:</source>
        <translation>빠른 필터:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="323"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation>편집기 설정을 불러오는데 문제가 있습니다.

Cppcheck 버전간 설정 방법 차이때문인 것으로 보입니다. 편집기 설정을 검사(및 수정)해주세요, 그렇지 않으면 편집기가 제대로 시작하지 않습니다.</translation>
    </message>
    <message>
        <source>No suitable files found to check!</source>
        <translation type="obsolete">검사할 수 있는 파일이 없습니다!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="558"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>새로운 파일이나 디렉토리를 선택하기 전에 프로젝트 파일을 닫으세요!</translation>
    </message>
    <message>
        <source>Select directory to check</source>
        <translation type="obsolete">검사할 디렉토리 선택</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="690"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>프로젝트 파일 존재: %1

이 프로젝트 파일을 불러오겠습니까?</translation>
    </message>
    <message>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation type="obsolete">디렉토리에 프로젝트 파일 존재.

이 프로젝트 파일을 사용하지 않고 검사를 계속하시겠습니까?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1206"/>
        <location filename="mainwindow.cpp" line="1386"/>
        <source>XML files (*.xml)</source>
        <translation>XML 파일 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1208"/>
        <source>Open the report file</source>
        <translation>보고서 파일 열기</translation>
    </message>
    <message>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?</source>
        <translation type="obsolete">검사 중.

검사를 중지하고 Cppcheck을 종료하시겠습니까?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1357"/>
        <source>License</source>
        <translation>저작권</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1364"/>
        <source>Authors</source>
        <translation>제작자</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation type="obsolete">XML 파일 버전 2 (*.xml);;XML 파일 버전 1 (*.xml);;텍스트 파일 (*.txt);;CSV 파일 (*.csv)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1379"/>
        <source>Save the report file</source>
        <translation>보고서 파일 저장</translation>
    </message>
    <message>
        <source>XML files version 1 (*.xml)</source>
        <translation type="obsolete">XML 파일 버전 1 (*.xml)</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml)</source>
        <translation type="obsolete">XML 파일 버전 2 (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1390"/>
        <source>Text files (*.txt)</source>
        <translation>텍스트 파일 (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1394"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 파일 (*.csv)</translation>
    </message>
    <message>
        <source>Cppcheck - %1</source>
        <translation type="vanished">Cppcheck - %1</translation>
    </message>
    <message>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation type="obsolete">언어 변경 실패:

%1

언어가 영어로 초기화 됐습니다. 설정창을 열어서 설정 가능한 언어를 선택하세요.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1492"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>프로젝트 파일 (*.cppcheck);;모든 파일(*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1494"/>
        <source>Select Project File</source>
        <translation>프로젝트 파일 선택</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="159"/>
        <location filename="mainwindow.cpp" line="1522"/>
        <location filename="mainwindow.cpp" line="1648"/>
        <source>Project:</source>
        <translation>프로젝트:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1635"/>
        <source>Select Project Filename</source>
        <translation>프로젝트 파일이름 선택</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1686"/>
        <source>No project file loaded</source>
        <translation>프로젝트 파일 불러오기 실패</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1755"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>프로젝트 파일

%1

이 존재하지 않습니다!

최근 프로젝트 목록에서 파일을 제거하시겠습니까?</translation>
    </message>
    <message>
        <source>Select files to check</source>
        <translation type="obsolete">검사할 파일 선택</translation>
    </message>
    <message>
        <location filename="main.cpp" line="121"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="135"/>
        <source>C++ standard</source>
        <translation type="unfinished"></translation>
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
        <location filename="mainwindow.cpp" line="859"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="809"/>
        <source>File not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="812"/>
        <source>Bad XML</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="815"/>
        <source>Missing attribute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="818"/>
        <source>Bad attribute value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="838"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="821"/>
        <source>Unsupported format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="800"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="824"/>
        <source>Duplicate platform type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="827"/>
        <source>Platform type redefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="751"/>
        <source>&amp;Print...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="754"/>
        <source>Print the Current Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="759"/>
        <source>Print Pre&amp;view...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="762"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="770"/>
        <source>Open library editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="830"/>
        <source>Unknown element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="833"/>
        <source>Unknown issue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="660"/>
        <source>Select configuration</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="859"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1586"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="253"/>
        <location filename="mainwindow.ui" line="256"/>
        <source>Analyze files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="271"/>
        <location filename="mainwindow.ui" line="274"/>
        <source>Analyze directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="286"/>
        <source>&amp;Reanalyze modified files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="310"/>
        <location filename="mainwindow.ui" line="313"/>
        <source>Stop analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1377"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="485"/>
        <source>No suitable files found to analyze!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="578"/>
        <source>Select files to analyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="593"/>
        <source>Select directory to analyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="660"/>
        <source>Select the configuration that will be analyzed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="712"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1311"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="131"/>
        <source>A&amp;nalyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="145"/>
        <source>&amp;C standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="298"/>
        <source>Reanal&amp;yze all files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="336"/>
        <source>Ctrl+Q</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="366"/>
        <source>Style war&amp;nings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="384"/>
        <source>E&amp;rrors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="465"/>
        <source>Ctrl+Shift+O</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="474"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="482"/>
        <source>Ctrl+Shift+N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="530"/>
        <source>&amp;Warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="548"/>
        <source>Per&amp;formance warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="574"/>
        <source>&amp;Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="589"/>
        <source>&amp;Portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="607"/>
        <source>Show Cppcheck results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="619"/>
        <source>Clang</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="622"/>
        <source>Show Clang results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="689"/>
        <source>P&amp;latforms</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="703"/>
        <source>C++&amp;11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="714"/>
        <source>C&amp;99</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="722"/>
        <source>&amp;Posix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="730"/>
        <source>C&amp;11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="738"/>
        <source>&amp;C89</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="746"/>
        <source>&amp;C++03</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="767"/>
        <source>&amp;Library Editor...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="778"/>
        <source>&amp;Auto-detect language</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="786"/>
        <source>&amp;Enforce C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="794"/>
        <source>E&amp;nforce C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="805"/>
        <source>C++14</source>
        <translation type="unfinished">C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1609"/>
        <source>Failed to import &apos;%1&apos;, analysis is stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1633"/>
        <source>Project files (*.cppcheck)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="813"/>
        <source>Reanalyze and check library</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="821"/>
        <source>Check configuration (defines, includes)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="829"/>
        <source>C++17</source>
        <translation type="unfinished">C++17</translation>
    </message>
    <message>
        <location filename="mainwindow.ui" line="840"/>
        <source>C++20</source>
        <translation type="unfinished">C++20</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="572"/>
        <source>C/C++ Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="573"/>
        <source>Compile database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="574"/>
        <source>Visual Studio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="575"/>
        <source>Borland C++ Builder 6</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1191"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.
Do you want to proceed?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>NewSuppressionDialog</name>
    <message>
        <location filename="newsuppressiondialog.ui" line="17"/>
        <source>New suppression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="25"/>
        <source>Error ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="32"/>
        <source>File name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="42"/>
        <source>Line number</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui" line="52"/>
        <source>Symbol name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.cpp" line="53"/>
        <source>Edit suppression</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Platforms</name>
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
    <message>
        <source>Built-in</source>
        <translation type="obsolete">내장 방식</translation>
    </message>
    <message>
        <location filename="platforms.cpp" line="37"/>
        <source>Native</source>
        <translation type="unfinished"></translation>
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
        <translation type="obsolete">프로젝트 파일을 읽을 수 없습니다.</translation>
    </message>
    <message>
        <source>Could not write the project file.</source>
        <translation type="obsolete">프로젝트 파일에 쓸 수 없습니다.</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfiledialog.ui" line="14"/>
        <source>Project File</source>
        <translation>프로젝트 파일</translation>
    </message>
    <message>
        <source>Project</source>
        <translation type="obsolete">프로젝트</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="221"/>
        <source>Defines:</source>
        <translation>Defines:</translation>
    </message>
    <message>
        <source>Root:</source>
        <translation type="obsolete">Root:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="147"/>
        <source>Paths:</source>
        <translation>경로:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="181"/>
        <location filename="projectfiledialog.ui" line="296"/>
        <source>Add...</source>
        <translation>추가...</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="188"/>
        <location filename="projectfiledialog.ui" line="303"/>
        <location filename="projectfiledialog.ui" line="671"/>
        <source>Edit</source>
        <translation>편집</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="195"/>
        <location filename="projectfiledialog.ui" line="310"/>
        <location filename="projectfiledialog.ui" line="678"/>
        <location filename="projectfiledialog.ui" line="721"/>
        <source>Remove</source>
        <translation>제거</translation>
    </message>
    <message>
        <source>Includes</source>
        <translation type="obsolete">Includes</translation>
    </message>
    <message>
        <source>Include directories:</source>
        <translation type="obsolete">Include 디렉토리:</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="330"/>
        <source>Up</source>
        <translation>위로</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="337"/>
        <source>Down</source>
        <translation>아래로</translation>
    </message>
    <message>
        <source>Exclude</source>
        <translation type="obsolete">Exclude</translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="703"/>
        <source>Suppressions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="714"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="389"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="833"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="265"/>
        <source>Include Paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="24"/>
        <source>Paths and Defines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="73"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="76"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="614"/>
        <source>Root path:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="630"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="409"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="113"/>
        <source>Selected VS Configurations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="362"/>
        <source>Types and Functions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="380"/>
        <source>Libraries</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="432"/>
        <source>Parser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="438"/>
        <source>Cppcheck (built in)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="484"/>
        <source>Check that each class has a safe public interface</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="500"/>
        <source>Limit analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="760"/>
        <location filename="projectfiledialog.ui" line="766"/>
        <source>Addons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="772"/>
        <source>Note: Addons require &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt; being installed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="782"/>
        <source>Y2038</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="789"/>
        <source>Thread safety</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="796"/>
        <source>Coding standards</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="803"/>
        <source>Cert</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="858"/>
        <source>Clang analyzer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="851"/>
        <source>Clang-tidy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="60"/>
        <location filename="projectfiledialog.ui" line="422"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="231"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="368"/>
        <source>Platform</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="415"/>
        <source>This is a workfolder that Cppcheck will use for various purposes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="448"/>
        <source>Clang (experimental)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="464"/>
        <source>Normal analysis -- Avoid false positives.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="474"/>
        <source>Bug hunting -- Generates mostly noise. The goal is to be &quot;soundy&quot; and detect most bugs.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="481"/>
        <source>If you want to design your classes to be as flexible and robust as possible then the public interface must be very robust. Cppcheck will asumme that arguments can take *any* value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="506"/>
        <source>Check code in headers  (should be ON normally. if you want a limited quick analysis then turn this OFF)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="559"/>
        <source>Max recursion in template instantiation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="608"/>
        <source>Warning options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="620"/>
        <source>Filepaths in warnings will be relative to this path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="636"/>
        <source>If tags are added, you will be able to right click on warnings and set one of these tags. You can manually categorize warnings.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="646"/>
        <source>Exclude source files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="657"/>
        <source>Exclude folder...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="664"/>
        <source>Exclude file...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="810"/>
        <source>MISRA C 2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="819"/>
        <source>Misra rule texts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="826"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="845"/>
        <source>External tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="30"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="242"/>
        <source>Undefines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="252"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="403"/>
        <location filename="projectfiledialog.ui" line="458"/>
        <source>Analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="516"/>
        <source>Check code in unused templates (should be ON normally, however in theory you can safely ignore warnings in unused templates)</source>
        <oldsource>Check code in unused templates  (slower and less accurate analysis)</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.ui" line="528"/>
        <source>Max CTU depth</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="86"/>
        <source>Project file: %1</source>
        <translation>프로젝트 파일: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="715"/>
        <source>Select include directory</source>
        <translation>Include 디렉토리 선택</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="695"/>
        <source>Select a directory to check</source>
        <translation>검사할 디렉토리 선택</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="735"/>
        <source>Select directory to ignore</source>
        <translation>무시할 디렉토리 선택</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="456"/>
        <source>Select Cppcheck build dir</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="499"/>
        <source>Import Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="353"/>
        <source>Clang-tidy (not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="347"/>
        <source>(no rule texts file)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="743"/>
        <source>Source files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="744"/>
        <source>All files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="745"/>
        <source>Exclude file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="829"/>
        <source>Select MISRA rule texts file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="831"/>
        <source>Misra rule texts file (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="496"/>
        <source>Visual Studio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="497"/>
        <source>Compile database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="498"/>
        <source>Borland C++ Builder 6</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QDialogButtonBox</name>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>Close</source>
        <translation type="unfinished">닫기</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="104"/>
        <source>Unknown language specified!</source>
        <translation>알 수 없는 언어입니다!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="129"/>
        <source>Language file %1 not found!</source>
        <translation>언어 파일(%1)이 없습니다!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="135"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>파일(%2)로부터 언어(%1) 불러오기 실패</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="35"/>
        <source>line %1: Unhandled element %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="242"/>
        <source> (Not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="69"/>
        <source>Thin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="71"/>
        <source>ExtraLight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="73"/>
        <source>Light</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="75"/>
        <source>Normal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="77"/>
        <source>Medium</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="79"/>
        <source>DemiBold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="81"/>
        <source>Bold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="83"/>
        <source>ExtraBold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="85"/>
        <source>Black</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="71"/>
        <source>Editor Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="74"/>
        <source>Editor Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="77"/>
        <source>Highlight Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="80"/>
        <source>Line Number Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="83"/>
        <source>Line Number Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="86"/>
        <source>Keyword Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="89"/>
        <source>Keyword Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="95"/>
        <source>Class Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="98"/>
        <source>Quote Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="101"/>
        <source>Quote Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="104"/>
        <source>Comment Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="107"/>
        <source>Comment Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="110"/>
        <source>Symbol Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="113"/>
        <source>Symbol Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="116"/>
        <source>Symbol Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="132"/>
        <source>Set to Default Light</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="134"/>
        <source>Set to Default Dark</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="92"/>
        <source>Class Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="41"/>
        <source>Close</source>
        <translation type="unfinished">닫기</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="42"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="1450"/>
        <source>File</source>
        <translation>파일</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1450"/>
        <source>Severity</source>
        <translation>분류</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1450"/>
        <source>Line</source>
        <translation>행</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1450"/>
        <source>Summary</source>
        <translation>요약</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="160"/>
        <source>Undefined file</source>
        <translation>미정의된 파일</translation>
    </message>
    <message>
        <source>[Inconclusive]</source>
        <translation type="obsolete">[불확실]</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="331"/>
        <source>style</source>
        <translation>스타일</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="334"/>
        <source>error</source>
        <translation>에러</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="337"/>
        <source>warning</source>
        <translation>경고</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="340"/>
        <source>performance</source>
        <translation>성능</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="343"/>
        <source>portability</source>
        <translation>이식성</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="346"/>
        <source>information</source>
        <translation>정보</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="349"/>
        <source>debug</source>
        <translation>디버그</translation>
    </message>
    <message>
        <source>Copy filename</source>
        <translation type="obsolete">파일이름 복사</translation>
    </message>
    <message>
        <source>Copy full path</source>
        <translation type="obsolete">전체 경로 복사</translation>
    </message>
    <message>
        <source>Copy message</source>
        <translation type="obsolete">메시지 복사</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="662"/>
        <source>Hide</source>
        <translation>숨기기</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="747"/>
        <location filename="resultstree.cpp" line="761"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="748"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <translation>편집기 미설정.

[설정 - 응용 프로그램]에서 편집기를 설정하세요.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="762"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>기본 편집기 미선택.

[설정 - 응용 프로그램]에서 기본 편집기를 선택하세요.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="791"/>
        <source>Could not find the file!</source>
        <translation>파일을 찾을 수 없습니다!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="837"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>%1을 시잘할 수 없습니다

경로와 인자가 정확한지 확인하세요.</translation>
    </message>
    <message>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation type="obsolete">파일 찾기 실패:
%1
파일이 위치한 디렉토리를 선택하세요.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="859"/>
        <source>Select Directory</source>
        <translation>디렉토리 선택</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1450"/>
        <source>Id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="663"/>
        <source>Hide all with id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="664"/>
        <source>Open containing folder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1450"/>
        <source>Inconclusive</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="660"/>
        <source>Recheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="286"/>
        <source>note</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="653"/>
        <source>Edit contract..</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="683"/>
        <source>Suppress selected id(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="687"/>
        <source>Suppress</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="703"/>
        <location filename="resultstree.cpp" line="1450"/>
        <source>Tag</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="705"/>
        <source>No tag</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1450"/>
        <source>Since date</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="851"/>
        <source>Could not find file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="855"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="856"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="858"/>
        <source>Please select the directory where file is located.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="661"/>
        <source>Copy</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>결과</translation>
    </message>
    <message>
        <source>No errors found, nothing to save.</source>
        <translation type="vanished">에러가 발견되지 않았고, 저장할 내용이 없습니다.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="228"/>
        <location filename="resultsview.cpp" line="236"/>
        <source>Failed to save the report.</source>
        <translation>결과 저장 실패.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="309"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%2 중 %1 파일 검사됨)</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="326"/>
        <location filename="resultsview.cpp" line="337"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="327"/>
        <source>No errors found.</source>
        <translation>에러가 발견되지 않았습니다.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="334"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>에러가 발견되었지만, 감추도록 설정되어 있습니다.
에러 종류를 표시하도록 설정하려면, 보기 메뉴를 선택하세요.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="380"/>
        <location filename="resultsview.cpp" line="399"/>
        <source>Failed to read the report.</source>
        <translation>결과 불러오기 실패.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="455"/>
        <source>Bug hunting analysis is incomplete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Summary</source>
        <translation type="obsolete">요약</translation>
    </message>
    <message>
        <source>Message</source>
        <translation type="obsolete">내용</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="453"/>
        <source>Id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="246"/>
        <source>Print Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="265"/>
        <source>No errors found, nothing to print.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="448"/>
        <source>First included by</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="387"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation type="unfinished"></translation>
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
        <location filename="resultsview.ui" line="158"/>
        <source>Functions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="196"/>
        <source>Variables</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="217"/>
        <source>Only show variable names that contain text:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="164"/>
        <location filename="resultsview.ui" line="229"/>
        <source>Configured contracts:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui" line="178"/>
        <location filename="resultsview.ui" line="243"/>
        <source>Missing contracts:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="568"/>
        <source>Clear Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="569"/>
        <source>Copy this Log entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="570"/>
        <source>Copy complete Log</source>
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
        <location filename="scratchpad.ui" line="71"/>
        <source>filename</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="78"/>
        <source>Check</source>
        <translation type="unfinished">검사</translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="20"/>
        <source>Copy or write some C/C++ code here:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui" line="37"/>
        <source>Optionally enter a filename (mainly for automatic language detection) and click on &quot;Check&quot;:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>설정</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>일반</translation>
    </message>
    <message>
        <location filename="settings.ui" line="41"/>
        <source>Number of threads: </source>
        <translation>쓰레드 수: </translation>
    </message>
    <message>
        <location filename="settings.ui" line="85"/>
        <source>Ideal count:</source>
        <translation>최적 값:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="114"/>
        <source>Force checking all #ifdef configurations</source>
        <translation>모든 #ifdef 설정을 강제로 검사</translation>
    </message>
    <message>
        <location filename="settings.ui" line="121"/>
        <source>Show full path of files</source>
        <translation>파일의 전체 경로 표시</translation>
    </message>
    <message>
        <location filename="settings.ui" line="128"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>에러가 발견되지 않는 경우 &quot;에러가 없습니다.&quot; 메시지 표시</translation>
    </message>
    <message>
        <location filename="settings.ui" line="142"/>
        <source>Enable inline suppressions</source>
        <translation>Inline suppression 사용</translation>
    </message>
    <message>
        <source>Paths</source>
        <translation type="obsolete">경로</translation>
    </message>
    <message>
        <source>Include paths:</source>
        <translation type="obsolete">Include 경로:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="195"/>
        <source>Add...</source>
        <translation>추가...</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="obsolete">편집</translation>
    </message>
    <message>
        <location filename="settings.ui" line="209"/>
        <source>Remove</source>
        <translation>제거</translation>
    </message>
    <message>
        <location filename="settings.ui" line="184"/>
        <source>Applications</source>
        <translation>응용 프로그램</translation>
    </message>
    <message>
        <location filename="settings.ui" line="202"/>
        <location filename="settings.ui" line="478"/>
        <source>Edit...</source>
        <translation>편집...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="216"/>
        <source>Set as default</source>
        <translation>기본으로 지정</translation>
    </message>
    <message>
        <location filename="settings.ui" line="239"/>
        <source>Reports</source>
        <translation>보고서</translation>
    </message>
    <message>
        <location filename="settings.ui" line="245"/>
        <source>Save all errors when creating report</source>
        <translation>보고서 생성 시 모든 에러 저장</translation>
    </message>
    <message>
        <location filename="settings.ui" line="252"/>
        <source>Save full path to files in reports</source>
        <translation>보고서에 파일의 전체 경로 저장</translation>
    </message>
    <message>
        <location filename="settings.ui" line="273"/>
        <source>Language</source>
        <translation>언어</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="obsolete">고급</translation>
    </message>
    <message>
        <source>&amp;Show inconclusive errors</source>
        <translation type="obsolete">불확실한 에러 표시(&amp;S)</translation>
    </message>
    <message>
        <source>S&amp;how internal warnings in log</source>
        <translation type="obsolete">로그에 내부 경고 표시(&amp;H)</translation>
    </message>
    <message>
        <location filename="settings.ui" line="135"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="149"/>
        <source>Check for inconclusive errors also</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="176"/>
        <source>Show internal warnings in log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="156"/>
        <source>Show statistics on check completion</source>
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
        <location filename="settings.ui" line="304"/>
        <location filename="settings.ui" line="345"/>
        <location filename="settings.ui" line="390"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="371"/>
        <source>Clang</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="377"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="400"/>
        <source>Visual Studio headers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="406"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="323"/>
        <source>Misra addon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="331"/>
        <source>Misra rule texts file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="338"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="436"/>
        <source>Code Editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="442"/>
        <source>Code Editor Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="455"/>
        <source>Default Light Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="462"/>
        <source>Default Dark Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="471"/>
        <source>Custom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui" line="448"/>
        <source>System Style</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="106"/>
        <source>N/A</source>
        <translation>N/A</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="217"/>
        <source>The executable file &quot;%1&quot; is not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="228"/>
        <source>Add a new application</source>
        <translation>새 응용 프로그램 추가</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="261"/>
        <source>Modify an application</source>
        <translation>응용 프로그램 편집</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="291"/>
        <source>[Default]</source>
        <translation>[기본]</translation>
    </message>
    <message>
        <source>Select include directory</source>
        <translation type="obsolete">Include 디렉토리 선택</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="266"/>
        <source> [Default]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="346"/>
        <source>Select python binary</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="384"/>
        <source>Select clang path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="353"/>
        <source>Select MISRA File</source>
        <translation type="unfinished"></translation>
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
        <translation>통계</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <location filename="statsdialog.cpp" line="175"/>
        <source>Project</source>
        <translation>프로젝트</translation>
    </message>
    <message>
        <location filename="stats.ui" line="33"/>
        <source>Project:</source>
        <translation>프로젝트:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="53"/>
        <source>Paths:</source>
        <translation>경로:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="85"/>
        <source>Include paths:</source>
        <translation>Include 경로:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation>Defines:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="165"/>
        <location filename="statsdialog.cpp" line="180"/>
        <source>Previous Scan</source>
        <translation>직전 검사</translation>
    </message>
    <message>
        <location filename="stats.ui" line="171"/>
        <source>Path Selected:</source>
        <translation>선택된 경로:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Number of Files Scanned:</source>
        <translation>검사된 파일 수:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="201"/>
        <source>Scan Duration:</source>
        <translation>검사 시간:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="256"/>
        <source>Errors:</source>
        <translation>에러:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="274"/>
        <source>Warnings:</source>
        <translation>경고:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="292"/>
        <source>Stylistic warnings:</source>
        <translation>스타일 경고:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="310"/>
        <source>Portability warnings:</source>
        <translation>이식성 경고:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="328"/>
        <source>Performance issues:</source>
        <translation>성능 경고:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="346"/>
        <source>Information messages:</source>
        <translation>정보 메시지:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="407"/>
        <source>Copy to Clipboard</source>
        <translation>클립보드에 복사</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>1 day</source>
        <translation>1일</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="113"/>
        <source>%1 days</source>
        <translation>%1일</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>1 hour</source>
        <translation>1시간</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="115"/>
        <source>%1 hours</source>
        <translation>%1시간</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>1 minute</source>
        <translation>1분</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="117"/>
        <source>%1 minutes</source>
        <translation>%1분</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="119"/>
        <source>1 second</source>
        <translation>1초</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="119"/>
        <source>%1 seconds</source>
        <translation>%1초</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="123"/>
        <source>0.%1 seconds</source>
        <translation>0.%1초</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="125"/>
        <source> and </source>
        <translation> 및 </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="174"/>
        <source>Project Settings</source>
        <translation>프로젝트 설정</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="176"/>
        <source>Paths</source>
        <translation>경로</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="177"/>
        <source>Include paths</source>
        <translation>Include 경로</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="178"/>
        <source>Defines</source>
        <translation>Defines</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="181"/>
        <source>Path selected</source>
        <translation>선택된 경로</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="182"/>
        <source>Number of files scanned</source>
        <translation>검사된 파일 수</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="183"/>
        <source>Scan duration</source>
        <translation>검사 시간</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="139"/>
        <location filename="statsdialog.cpp" line="185"/>
        <source>Errors</source>
        <translation>에러</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="141"/>
        <location filename="statsdialog.cpp" line="186"/>
        <source>Warnings</source>
        <translation>경고</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="143"/>
        <location filename="statsdialog.cpp" line="187"/>
        <source>Style warnings</source>
        <translation>스타일 경고</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="145"/>
        <location filename="statsdialog.cpp" line="188"/>
        <source>Portability warnings</source>
        <translation>이식성 경고</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="147"/>
        <location filename="statsdialog.cpp" line="189"/>
        <source>Performance warnings</source>
        <translation>성능 경고</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="149"/>
        <location filename="statsdialog.cpp" line="190"/>
        <source>Information messages</source>
        <translation>정보 메시지</translation>
    </message>
    <message>
        <location filename="stats.ui" line="414"/>
        <source>Pdf Export</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="152"/>
        <source>Export PDF</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="363"/>
        <source>History</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="369"/>
        <source>File:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="65"/>
        <source>File: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="65"/>
        <source>No cppcheck build dir</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="stats.ui" line="131"/>
        <source>Undefines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="179"/>
        <source>Undefines</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="54"/>
        <source>%1 of %2 files checked</source>
        <translation>%2 중 %1 파일 검사됨</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="142"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation type="unfinished">언어 변경 실패:

%1

언어가 영어로 초기화 됐습니다. 설정창을 열어서 설정 가능한 언어를 선택하세요.</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="148"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="74"/>
        <source>inconclusive</source>
        <translation>불확실</translation>
    </message>
</context>
<context>
    <name>VariableContractsDialog</name>
    <message>
        <location filename="variablecontractsdialog.ui" line="14"/>
        <source>Dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="variablecontractsdialog.ui" line="20"/>
        <source>You can specify min and max value for the variable here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="variablecontractsdialog.ui" line="29"/>
        <source>Min</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="variablecontractsdialog.ui" line="39"/>
        <source>Max</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>toFilterString</name>
    <message>
        <location filename="common.cpp" line="52"/>
        <source>All supported files (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="common.cpp" line="57"/>
        <source>All files (%1)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
