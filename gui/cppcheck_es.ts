<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="es_ES">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui"/>
        <source>About Cppcheck</source>
        <translation>Acerca de Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui"/>
        <source>Version %1</source>
        <translation>Versión %1</translation>
    </message>
    <message>
        <location filename="about.ui"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - Una utilidad para el análisis estático de código C/C++.</translation>
    </message>
    <message>
        <location filename="about.ui"/>
        <source>Copyright © 2007-%1 Cppcheck team.</source>
        <oldsource>Copyright © 2007-2021 Cppcheck team.</oldsource>
        <translation type="unfinished">Copyright © 2007-2021 el equipo de cppcheck.</translation>
    </message>
    <message>
        <location filename="about.ui"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>Este programa está licenciado bajo los términos de GNU General Public License versión 3</translation>
    </message>
    <message>
        <location filename="about.ui"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Visita el sitio de Cppcheck en %1</translation>
    </message>
    <message>
        <location filename="about.ui"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PCRE&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;PicoJSON&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;TinyXML2&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Boost&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <oldsource>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Many thanks to these libraries that we use:&lt;/p&gt;&lt;ul style=&quot;margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;&quot;&gt;&lt;li style=&quot; margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;pcre&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;picojson&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;qt&lt;/li&gt;&lt;li style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;tinyxml2&lt;/li&gt;&lt;/ul&gt;&lt;/body&gt;&lt;/html&gt;</oldsource>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="applicationdialog.ui"/>
        <source>Add an application</source>
        <translation>Añade una aplicación</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui"/>
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
        <location filename="applicationdialog.ui"/>
        <source>&amp;Name:</source>
        <translation>&amp;Nombre:</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui"/>
        <source>&amp;Executable:</source>
        <translation>&amp;Ejecutable:</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui"/>
        <source>&amp;Parameters:</source>
        <translation>&amp;Parámetros:</translation>
    </message>
    <message>
        <location filename="applicationdialog.ui"/>
        <source>Browse</source>
        <translation>Buscar</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="65"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Archivos ejecutables (*.exe);;Todos los archivos(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="68"/>
        <source>Select viewer application</source>
        <translation>Selecciona la aplicación para visualizar</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="83"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="84"/>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation>¡Debes especificar el nombre, la ruta y opcionalmente los parámetros para la aplicación!</translation>
    </message>
</context>
<context>
    <name>ComplianceReportDialog</name>
    <message>
        <location filename="compliancereportdialog.ui"/>
        <source>Compliance Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui"/>
        <source>Project name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui"/>
        <source>Project version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui"/>
        <source>Coding Standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.ui"/>
        <source>List of files with md5 checksums</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="133"/>
        <source>Compliance report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="135"/>
        <source>HTML files (*.html)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="176"/>
        <location filename="compliancereportdialog.cpp" line="239"/>
        <source>Save compliance report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="compliancereportdialog.cpp" line="177"/>
        <source>Failed to import &apos;%1&apos; (%2), can not show files in compliance report</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="53"/>
        <source>Could not find the file: %1</source>
        <translation>No se ha encontrado el fichero: %1</translation>
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
        <translation>No se ha podido leer el fichero: %1</translation>
    </message>
</context>
<context>
    <name>HelpDialog</name>
    <message>
        <location filename="helpdialog.ui"/>
        <source>Cppcheck GUI help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.ui"/>
        <source>Contents</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.ui"/>
        <source>Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="84"/>
        <source>Helpfile &apos;%1&apos; was not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="helpdialog.cpp" line="86"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
</context>
<context>
    <name>LibraryAddFunctionDialog</name>
    <message>
        <location filename="libraryaddfunctiondialog.ui"/>
        <source>Add function</source>
        <translation>Añadir función</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui"/>
        <source>Function name(s)</source>
        <translation>Nombre(s) de la función</translation>
    </message>
    <message>
        <location filename="libraryaddfunctiondialog.ui"/>
        <source>Number of arguments</source>
        <translation>Número de argumentos</translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Library Editor</source>
        <translation>Editor de bibliotecas</translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Open</source>
        <translation>Abrir</translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Save</source>
        <translation>Guardar</translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Save as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Functions</source>
        <translation>Funciones</translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Sort</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Comments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>noreturn</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>False</source>
        <translation>Falso</translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>True</source>
        <translation>Verdadero</translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Unknown</source>
        <translation>Desconocido</translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>return value must be used</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>ignore function in leaks checking</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Arguments</source>
        <translation>Argumentos</translation>
    </message>
    <message>
        <location filename="librarydialog.ui"/>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="98"/>
        <location filename="librarydialog.cpp" line="170"/>
        <source>Library files (*.cfg)</source>
        <translation>Archivos de biblioteca (*.cfg)</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="100"/>
        <source>Open library file</source>
        <translation>Abrir archivo de biblioteca</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="111"/>
        <location filename="librarydialog.cpp" line="123"/>
        <location filename="librarydialog.cpp" line="160"/>
        <source>Cppcheck</source>
        <translation type="unfinished">Cppcheck</translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="112"/>
        <source>Cannot open file %1.</source>
        <oldsource>Can not open file %1.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="124"/>
        <source>Failed to load %1. %2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="161"/>
        <source>Cannot save file %1.</source>
        <oldsource>Can not save file %1.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="librarydialog.cpp" line="173"/>
        <source>Save the library as</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>Edit argument</source>
        <translation>Editar argumento</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is bool value allowed? For instance result from comparison or from &apos;!&apos; operator.&lt;/p&gt;
&lt;p&gt;Typically, set this if the argument is a pointer, size, etc.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // last argument should not have a bool value&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>Not bool</source>
        <translation>No bool</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is a null parameter value allowed?&lt;/p&gt;
&lt;p&gt;Typically this should be used on any pointer parameter that does not allow null.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // neither x or y is allowed to be null.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>Not null</source>
        <translation>No null</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>Not uninit</source>
        <translation>No uninit</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>String</source>
        <translation>String</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>Format string</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>Min size of buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>None</source>
        <translation>Ninguno</translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>argvalue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>mul</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>strlen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>Arg</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>Arg2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>and</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="libraryeditargdialog.ui"/>
        <source>Valid values</source>
        <translation>Valores válidos</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.ui"/>
        <location filename="mainwindow.cpp" line="448"/>
        <location filename="mainwindow.cpp" line="635"/>
        <location filename="mainwindow.cpp" line="715"/>
        <location filename="mainwindow.cpp" line="820"/>
        <location filename="mainwindow.cpp" line="842"/>
        <location filename="mainwindow.cpp" line="1422"/>
        <location filename="mainwindow.cpp" line="1551"/>
        <location filename="mainwindow.cpp" line="1883"/>
        <location filename="mainwindow.cpp" line="1891"/>
        <location filename="mainwindow.cpp" line="1939"/>
        <location filename="mainwindow.cpp" line="1948"/>
        <location filename="mainwindow.cpp" line="2020"/>
        <location filename="mainwindow.cpp" line="2094"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;File</source>
        <translation>&amp;Archivo</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;View</source>
        <translation>&amp;Ver</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Toolbars</source>
        <translation>&amp;Herramientas</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Help</source>
        <translation>&amp;Ayuda</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>C++ standard</source>
        <translation>C++ estándar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;C standard</source>
        <oldsource>C standard</oldsource>
        <translation type="unfinished">C estándar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Edit</source>
        <translation>&amp;Editar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Standard</source>
        <translation>Estándar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Categories</source>
        <translation>Categorías</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;License...</source>
        <translation>&amp;Licencia...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>A&amp;uthors...</source>
        <translation>A&amp;utores...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;About...</source>
        <translation>&amp;Acerca de...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Files...</source>
        <translation>&amp;Ficheros...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Analyze files</source>
        <oldsource>Check files</oldsource>
        <translation type="unfinished">Comprobar archivos</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Carpeta...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Analyze directory</source>
        <oldsource>Check directory</oldsource>
        <translation type="unfinished">Comprobar carpeta</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Stop</source>
        <translation>&amp;Detener</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Stop analysis</source>
        <oldsource>Stop checking</oldsource>
        <translation type="unfinished">Detener comprobación</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Guardar los resultados en el fichero...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Quit</source>
        <translation>&amp;Salir</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Clear results</source>
        <translation>&amp;Limpiar resultados</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Preferencias</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Show style warnings</source>
        <translation>Mostrar advertencias de estilo</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <location filename="mainwindow.cpp" line="2318"/>
        <source>Show errors</source>
        <translation>Mostrar errores</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="946"/>
        <location filename="mainwindow.cpp" line="987"/>
        <source>Information</source>
        <translation>Información</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Show information messages</source>
        <translation>Mostrar mensajes de información</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Show portability warnings</source>
        <translation>Mostrar advertencias de portabilidad</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Show Cppcheck results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Clang</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Show Clang results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Filter</source>
        <translation>&amp;Filtro</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Filter results</source>
        <translation>Resultados del filtro</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Windows 32-bit ANSI</source>
        <translation>Windows 32-bit ANSI</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Windows 32-bit Unicode</source>
        <translation>Windows 32-bit Unicode</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Unix 32-bit</source>
        <translation>Unix 32-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Unix 64-bit</source>
        <translation>Unix 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Windows 64-bit</source>
        <translation>Windows 64-bit</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Print...</source>
        <translation>Im&amp;primir...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Print the Current Report</source>
        <translation>Imprimir el informe actual</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Print Pre&amp;view...</source>
        <translation>Pre&amp;visualización de impresión...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation>Abre el diálogo de previsualización de impresión para el informe actual</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Open library editor</source>
        <translation>Abrir el editor de bibliotecas</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Check all</source>
        <translation>&amp;Seleccionar todo</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Checking for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Hide</source>
        <translation type="unfinished">Ocultar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Report</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>A&amp;nalyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Filter</source>
        <translation>Filtro</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Reanalyze modified files</source>
        <oldsource>&amp;Recheck modified files</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Reanal&amp;yze all files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Ctrl+Q</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Style war&amp;nings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>E&amp;rrors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Uncheck all</source>
        <translation>&amp;Deseleccionar todo</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Collapse &amp;all</source>
        <translation>Contraer &amp;todo</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Expandir todo</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Standard</source>
        <translation>&amp;Estándar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Standard items</source>
        <translation>Elementos estándar</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Contents</source>
        <translation>&amp;Contenidos</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Open the help contents</source>
        <translation>Abrir la ayuda de contenidos</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Toolbar</source>
        <translation>Barra de herramientas</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Categories</source>
        <translation>&amp;Categorías</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Error categories</source>
        <translation>Categorías de error</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Open XML...</source>
        <translation>&amp;Abrir XML...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Open P&amp;roject File...</source>
        <translation>Abrir P&amp;royecto...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Ctrl+Shift+O</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Sh&amp;ow Scratchpad...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;New Project File...</source>
        <translation>&amp;Nuevo Proyecto...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Ctrl+Shift+N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Log View</source>
        <translation>&amp;Visor del log</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Log View</source>
        <translation>Visor del log</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>C&amp;lose Project File</source>
        <translation>C&amp;errar Proyecto</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Edit Project File...</source>
        <translation>&amp;Editar Proyecto...</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Statistics</source>
        <translation>&amp;Estadísticas</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Per&amp;formance warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>P&amp;latforms</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>C++&amp;11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>C&amp;99</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Posix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>C&amp;11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;C89</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;C++03</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Library Editor...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Auto-detect language</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>&amp;Enforce C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>E&amp;nforce C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>C++14</source>
        <translation type="unfinished">C++14</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Reanalyze and check library</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Check configuration (defines, includes)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>C++17</source>
        <translation type="unfinished">C++17</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>C++20</source>
        <translation type="unfinished">C++20</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Compliance report...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Normal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Misra C++ 2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Misra C++ 2023</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <location filename="mainwindow.cpp" line="2319"/>
        <source>Show warnings</source>
        <translation>Mostrar advertencias</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Show performance warnings</source>
        <translation>Mostrar advertencias de rendimiento</translation>
    </message>
    <message>
        <location filename="mainwindow.ui"/>
        <source>Show &amp;hidden</source>
        <translation>Mostrar &amp;ocultos</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="443"/>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="716"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>¡Tienes que cerrar el proyecto antes de seleccionar nuevos ficheros o carpetas!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Select configuration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="955"/>
        <source>File not found</source>
        <translation>Archivo no encontrado</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="958"/>
        <source>Bad XML</source>
        <translation type="unfinished">XML malformado</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="961"/>
        <source>Missing attribute</source>
        <translation type="unfinished">Falta el atributo</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="964"/>
        <source>Bad attribute value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="967"/>
        <source>Unsupported format</source>
        <translation>Formato no soportado</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="976"/>
        <source>Duplicate define</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="987"/>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="998"/>
        <source>File not found: &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1024"/>
        <source>Failed to load/setup addon %1: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1046"/>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located. Please note that --data-dir is supposed to be used by installation scripts and therefore the GUI does not start when it is used, all that happens is that the setting is configured.

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1056"/>
        <source>Failed to load %1 - %2

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1066"/>
        <location filename="mainwindow.cpp" line="1153"/>
        <source>%1

Analysis is aborted.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1438"/>
        <location filename="mainwindow.cpp" line="1632"/>
        <source>XML files (*.xml)</source>
        <translation>Archivos XML (*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1440"/>
        <source>Open the report file</source>
        <translation>Abrir informe</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1603"/>
        <source>License</source>
        <translation>Licencia</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1610"/>
        <source>Authors</source>
        <translation>Autores</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1625"/>
        <source>Save the report file</source>
        <translation>Guardar informe</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="145"/>
        <location filename="mainwindow.cpp" line="1729"/>
        <source>Quick Filter:</source>
        <translation>Filtro rápido:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="821"/>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation>Se encontró el fichero de proyecto: %1

¿Quiere cargar este fichero de proyecto en su lugar?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="946"/>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>La biblioteca &apos;%1&apos; contiene elementos deconocidos:
%2</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="970"/>
        <source>Duplicate platform type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="973"/>
        <source>Platform type redefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="979"/>
        <source>Unknown element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="982"/>
        <source>Unknown issue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1046"/>
        <location filename="mainwindow.cpp" line="1056"/>
        <location filename="mainwindow.cpp" line="1066"/>
        <location filename="mainwindow.cpp" line="1153"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1636"/>
        <source>Text files (*.txt)</source>
        <translation>Ficheros de texto (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1640"/>
        <source>CSV files (*.csv)</source>
        <translation>Ficheros CVS (*.cvs)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1768"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Ficheros de proyecto (*.cppcheck;;Todos los ficheros (*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1770"/>
        <source>Select Project File</source>
        <translation>Selecciona el archivo de proyecto</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="226"/>
        <location filename="mainwindow.cpp" line="1731"/>
        <location filename="mainwindow.cpp" line="1798"/>
        <location filename="mainwindow.cpp" line="1988"/>
        <source>Project:</source>
        <translation>Proyecto:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="636"/>
        <source>No suitable files found to analyze!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="730"/>
        <source>C/C++ Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="731"/>
        <source>Compile database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="732"/>
        <source>Visual Studio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="733"/>
        <source>Borland C++ Builder 6</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="736"/>
        <source>Select files to analyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="751"/>
        <source>Select directory to analyze</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="791"/>
        <source>Select the configuration that will be analyzed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="843"/>
        <source>Found project files from the directory.

Do you want to proceed analysis without using any of these project files?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1423"/>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.
Do you want to proceed?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1547"/>
        <source>Analyzer is running.

Do you want to stop the analysis and exit Cppcheck?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1589"/>
        <source>About</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1623"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1663"/>
        <source>Cannot generate a compliance report right now, an analysis must finish successfully. Try to reanalyze the code and ensure there are no critical errors.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1884"/>
        <source>Build dir &apos;%1&apos; does not exist, create it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1892"/>
        <source>To check the project using addons, you need a build directory.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1924"/>
        <source>Failed to open file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1927"/>
        <source>Unknown project file format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1930"/>
        <source>Failed to import project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1940"/>
        <source>Failed to import &apos;%1&apos;: %2

Analysis is stopped.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1949"/>
        <source>Failed to import &apos;%1&apos; (%2), analysis is stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1973"/>
        <source>Project files (*.cppcheck)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="1975"/>
        <source>Select Project Filename</source>
        <translation>Selecciona el nombre del proyecto</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2021"/>
        <source>No project file loaded</source>
        <translation>No hay ningún proyecto cargado</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2089"/>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation>¡El fichero de proyecto

%1

 no puede ser encontrado!

¿Quiere eliminar el fichero de la lista de proyectos recientes?</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2249"/>
        <source>Install</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2253"/>
        <source>New version available: %1. %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2291"/>
        <source>Show Mandatory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2292"/>
        <source>Show Required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2293"/>
        <source>Show Advisory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2294"/>
        <source>Show Document</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2311"/>
        <source>Show L1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2312"/>
        <source>Show L2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2313"/>
        <source>Show L3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2320"/>
        <source>Show style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2321"/>
        <source>Show portability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2322"/>
        <source>Show performance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="2323"/>
        <source>Show information</source>
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
        <location filename="main.cpp" line="121"/>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>NewSuppressionDialog</name>
    <message>
        <location filename="newsuppressiondialog.ui"/>
        <source>New suppression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui"/>
        <source>Error ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui"/>
        <source>File name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui"/>
        <source>Line number</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.ui"/>
        <source>Symbol name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="newsuppressiondialog.cpp" line="82"/>
        <source>Edit suppression</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <location filename="platforms.cpp" line="38"/>
        <source>Native</source>
        <translation type="unfinished"></translation>
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
        <translation>Windows 32-bit ANSI</translation>
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
        <location filename="projectfile.ui"/>
        <source>Project File</source>
        <translation>Archivo de proyecto</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Paths and Defines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Import Project (Visual studio / compile database/ Borland C++ Builder 6)</source>
        <oldsource>Import Project (Visual studio / compile database)</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Defines must be separated by a semicolon. Example: DEF1;DEF2=5;DEF3=int</source>
        <oldsource>Defines must be separated by a semicolon &apos;;&apos;</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation>Nota: Ponga sus propios archivos .cfg en la misma carpeta que el proyecto. Debería verlos arriba.</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>If tags are added, you will be able to right click on warnings and set one of these tags. You can manually categorize warnings.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Exclude source files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Exclude folder...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Exclude file...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Misra C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>2012</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>2023</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>MISRA rule texts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;You have a choice:&lt;/p&gt;&lt;p&gt; * Analyze all Debug and Release configurations&lt;/p&gt;&lt;p&gt; * Only analyze the first matching Debug configuration&lt;/p&gt;&lt;p&gt;&lt;br/&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Analyze all Visual Studio configurations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Selected VS Configurations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Paths:</source>
        <translation>Rutas:</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Add...</source>
        <translation>Añadir...</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Edit</source>
        <translation>Editar</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Undefines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Undefines must be separated by a semicolon. Example: UNDEF1;UNDEF2;UNDEF3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Include Paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Types and Functions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>This is a workfolder that Cppcheck will use for various purposes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Parser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Cppcheck (built in)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Check level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Normal -- meant for normal analysis in CI. Analysis should finish in reasonable time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Exhaustive -- meant for nightly builds etc. Analysis time can be longer (10x slower than compilation is OK).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Check that each class has a safe public interface</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Limit analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Check code in unused templates (should be ON normally, however in theory you can safely ignore warnings in unused templates)</source>
        <oldsource>Check code in unused templates  (slower and less accurate analysis)</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Max CTU depth</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Premium License</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Enable inline suppressions</source>
        <translation type="unfinished">Habilitar supresiones inline</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Misra C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Cert C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>CERT-INT35-C:  int precision (if size equals precision, you can leave empty)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Autosar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Bug hunting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>External tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Up</source>
        <translation>Subir</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Down</source>
        <translation>Bajar</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Platform</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Clang (experimental)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>If you want to design your classes to be as flexible and robust as possible then the public interface must be very robust. Cppcheck will asumme that arguments can take *any* value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Check code in headers  (should be ON normally. if you want a limited quick analysis then turn this OFF)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Max recursion in template instantiation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Warning options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Root path:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Filepaths in warnings will be relative to this path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Warning tags (separated by semicolon)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Cppcheck build dir (whole program analysis, incremental analysis, statistics, etc)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Libraries</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Suppressions</source>
        <translation type="unfinished">Supresiones</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Addons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Note: Addons require &lt;a href=&quot;https://www.python.org/&quot;&gt;Python&lt;/a&gt; being installed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Y2038</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Thread safety</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Coding standards</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Cert C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Bug hunting (Premium)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Clang analyzer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Clang-tidy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfile.ui"/>
        <source>Defines:</source>
        <translation>Definiciones:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="118"/>
        <source>Project file: %1</source>
        <translation>Archivo de proyecto: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="562"/>
        <source>Select Cppcheck build dir</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="831"/>
        <source>Select include directory</source>
        <translation>Selecciona una carpeta para incluir</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="811"/>
        <source>Select a directory to check</source>
        <translation>Selecciona la carpeta a comprobar</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="437"/>
        <source>Clang-tidy (not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="601"/>
        <source>Visual Studio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="602"/>
        <source>Compile database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="603"/>
        <source>Borland C++ Builder 6</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="604"/>
        <source>Import Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="851"/>
        <source>Select directory to ignore</source>
        <translation>Selecciona la carpeta a ignorar</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="859"/>
        <source>Source files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="860"/>
        <source>All files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="861"/>
        <source>Exclude file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="945"/>
        <source>Select MISRA rule texts file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="947"/>
        <source>MISRA rule texts file (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="964"/>
        <source>Select license file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="964"/>
        <source>License file (%1)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="87"/>
        <source>Unknown language specified!</source>
        <translation>¡Idioma especificado desconocido!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="113"/>
        <source>Language file %1 not found!</source>
        <translation>¡Fichero de idioma %1 no encontrado!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="118"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>Fallo al cargar la traducción para el idioma %1 desde el fichero %2</translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="42"/>
        <source>line %1: Unhandled element %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="cppchecklibrarydata.cpp" line="47"/>
        <source>line %1: Mandatory attribute &apos;%2&apos; missing in &apos;%3&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="280"/>
        <source> (Not found)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="76"/>
        <source>Thin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="78"/>
        <source>ExtraLight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="80"/>
        <source>Light</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="82"/>
        <source>Normal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="84"/>
        <source>Medium</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="86"/>
        <source>DemiBold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="88"/>
        <source>Bold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="90"/>
        <source>ExtraBold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstylecontrols.cpp" line="92"/>
        <source>Black</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="81"/>
        <source>Editor Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="84"/>
        <source>Editor Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="87"/>
        <source>Highlight Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="90"/>
        <source>Line Number Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="93"/>
        <source>Line Number Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="96"/>
        <source>Keyword Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="99"/>
        <source>Keyword Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="102"/>
        <source>Class Foreground Color</source>
        <oldsource>Class ForegroundColor</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="105"/>
        <source>Class Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="108"/>
        <source>Quote Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="111"/>
        <source>Quote Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="114"/>
        <source>Comment Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="117"/>
        <source>Comment Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="120"/>
        <source>Symbol Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="123"/>
        <source>Symbol Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="126"/>
        <source>Symbol Font Weight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="146"/>
        <source>Set to Default Light</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="codeeditstyledialog.cpp" line="148"/>
        <source>Set to Default Dark</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="227"/>
        <source>File</source>
        <translation type="unfinished">Archivo</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="228"/>
        <source>Line</source>
        <translation type="unfinished">Línea</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="229"/>
        <source>Severity</source>
        <translation type="unfinished">Severidad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="230"/>
        <source>Classification</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="231"/>
        <source>Level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="232"/>
        <source>Inconclusive</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="233"/>
        <source>Summary</source>
        <translation type="unfinished">Resumen</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="234"/>
        <source>Id</source>
        <translation type="unfinished">Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="235"/>
        <source>Guideline</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="236"/>
        <source>Rule</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="237"/>
        <source>Since date</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="238"/>
        <source>Tags</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="239"/>
        <source>CWE</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Save</source>
        <translation>Guardar</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <source>File</source>
        <translation type="vanished">Archivo</translation>
    </message>
    <message>
        <source>Severity</source>
        <translation type="vanished">Severidad</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="vanished">Línea</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation type="vanished">Resumen</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="374"/>
        <source>Undefined file</source>
        <translation>Fichero no definido</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="865"/>
        <source>Copy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1058"/>
        <source>Could not find file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1062"/>
        <source>Please select the folder &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1063"/>
        <source>Select Directory &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1065"/>
        <source>Please select the directory where file is located.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="568"/>
        <source>portability</source>
        <translation>portabilidad</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="500"/>
        <source>note</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="571"/>
        <source>information</source>
        <translation>información</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="574"/>
        <source>debug</source>
        <translation>depuración</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="864"/>
        <source>Recheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="866"/>
        <source>Hide</source>
        <translation>Ocultar</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="867"/>
        <source>Hide all with id</source>
        <translation>Ocultar todos con el mismo id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="886"/>
        <source>Suppress selected id(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="868"/>
        <source>Open containing folder</source>
        <translation>Abrir carpeta contenedora</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="577"/>
        <source>internal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="907"/>
        <source>Tag</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="909"/>
        <source>No tag</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="940"/>
        <location filename="resultstree.cpp" line="954"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="941"/>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <oldsource>Configure the text file viewer program in Cppcheck preferences/Applications.</oldsource>
        <translation>No se ha configurado una aplicación para editar.

Configura el programa para editar en Preferencias/Aplicaciones.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="955"/>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation>No se ha definido una aplicación para editar prefeterminada.

Configura el programa para editar por defecto en Preferencias/Aplicaciones.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="981"/>
        <source>Could not find the file!</source>
        <translation>¡No se ha encontrado el fichero!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1044"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>No se ha podido ejecutar %1

Por favor comprueba que la ruta a la aplicación y los parámetros son correctos.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="1066"/>
        <source>Select Directory</source>
        <translation>Selecciona carpeta</translation>
    </message>
    <message>
        <source>Id</source>
        <translation type="vanished">Id</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="556"/>
        <source>style</source>
        <translation>estilo</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="559"/>
        <source>error</source>
        <translation>error</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="562"/>
        <source>warning</source>
        <translation>advertencia</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="565"/>
        <source>performance</source>
        <translation>ajuste</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.ui"/>
        <source>Results</source>
        <translation>Resultados</translation>
    </message>
    <message>
        <location filename="resultsview.ui"/>
        <source>Critical errors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui"/>
        <source>Analysis Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.ui"/>
        <source>Warning Details</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="232"/>
        <location filename="resultsview.cpp" line="240"/>
        <source>Failed to save the report.</source>
        <translation>Error al guardar el informe.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="250"/>
        <source>Print Report</source>
        <translation>Imprimir informe</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="269"/>
        <source>No errors found, nothing to print.</source>
        <translation>No se encontraron errores, nada que imprimir.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="321"/>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 of %2 archivos comprobados)</translation>
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
        <translation>No se han encontrado errores.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="353"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Se han encontrado errores, pero están configurados para que no se muestren.
Para cambiar el tipo de comportamiento, abra el menú Ver.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="402"/>
        <location filename="resultsview.cpp" line="421"/>
        <source>Failed to read the report.</source>
        <translation>Error al leer el informe.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="409"/>
        <source>XML format version 1 is no longer supported.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="470"/>
        <source>First included by</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="475"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="477"/>
        <source>Bug hunting analysis is incomplete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="551"/>
        <source>Clear Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="552"/>
        <source>Copy this Log entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="553"/>
        <source>Copy complete Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="561"/>
        <source>Analysis was stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="575"/>
        <source>There was a critical error with id &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="577"/>
        <source>when checking %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="579"/>
        <source>when checking a file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="580"/>
        <source>Analysis was aborted.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <location filename="scratchpad.ui"/>
        <source>Scratchpad</source>
        <translation type="unfinished">Scratchpad</translation>
    </message>
    <message>
        <location filename="scratchpad.ui"/>
        <source>Copy or write some C/C++ code here:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui"/>
        <source>Optionally enter a filename (mainly for automatic language detection) and click on &quot;Check&quot;:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="scratchpad.ui"/>
        <source>filename</source>
        <translation>nombre de archivo</translation>
    </message>
    <message>
        <location filename="scratchpad.ui"/>
        <source>Check</source>
        <translation>Comprobar</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui"/>
        <source>Preferences</source>
        <translation>Preferencias</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Ideal count:</source>
        <translation>Cantidad ideal:</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Force checking all #ifdef configurations</source>
        <translation>Forzar comprobación de todas las configuraciones #ifdef</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>Mostrar el Id del error en la columna &quot;Id&quot;</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Enable inline suppressions</source>
        <translation>Habilitar supresiones inline</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Check for inconclusive errors also</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Show statistics on check completion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Check for updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Show internal warnings in log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Addons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Python binary (leave this empty to use python in the PATH)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>MISRA addon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>MISRA rule texts file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Copy/paste the text from Appendix A &amp;quot;Summary of guidelines&amp;quot; from the MISRA C 2012 pdf to a text file.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Clang</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Clang path (leave empty to use system PATH)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Visual Studio headers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Paths to Visual Studio headers, separated by semicolon &apos;;&apos;.&lt;/p&gt;&lt;p&gt;You can open a Visual Studio command prompt, write &amp;quot;SET INCLUDE&amp;quot;. Then copy/paste the paths.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Code Editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Code Editor Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>System Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Default Light Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Default Dark Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Custom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Add...</source>
        <translation>Añadir...</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Edit...</source>
        <translation>Editar...</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Set as default</source>
        <translation>Definir por defecto</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Language</source>
        <translation>Idioma</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Number of threads: </source>
        <translation>Número de hilos:</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Show full path of files</source>
        <translation>Mostrar la ruta completa de los ficheros</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Mostrar el mensaje &quot;No se han encontrado errores&quot;</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Applications</source>
        <translation>Aplicaciones</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Reports</source>
        <translation>Informes</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Save all errors when creating report</source>
        <translation>Guardar todos los errores cuando se cree el informe</translation>
    </message>
    <message>
        <location filename="settings.ui"/>
        <source>Save full path to files in reports</source>
        <translation>Guardar la ruta completa en los ficheros de informes</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="248"/>
        <source>Add a new application</source>
        <translation>Añadir una nueva aplicación</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="278"/>
        <source>Modify an application</source>
        <translation>Modificar  una aplicación</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="283"/>
        <source> [Default]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="308"/>
        <source>[Default]</source>
        <translation>[Predeterminada]</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="363"/>
        <source>Select python binary</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="370"/>
        <source>Select MISRA File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="401"/>
        <source>Select clang path</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="statsdialog.ui"/>
        <location filename="statsdialog.cpp" line="187"/>
        <location filename="statsdialog.cpp" line="234"/>
        <source>Statistics</source>
        <translation>Estadísticas</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <location filename="statsdialog.cpp" line="225"/>
        <source>Project</source>
        <translation>Proyecto</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Project:</source>
        <translation>Proyecto:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Paths:</source>
        <translation>Rutas:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Include paths:</source>
        <translation type="unfinished">Incluye las rutas:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Defines:</source>
        <translation type="unfinished">Definiciones:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Undefines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <location filename="statsdialog.cpp" line="230"/>
        <source>Previous Scan</source>
        <translation type="unfinished">Análisis anterior</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Path Selected:</source>
        <translation>Ruta seleccionada:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Number of Files Scanned:</source>
        <translation type="unfinished">Número de archivos analizados:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Scan Duration:</source>
        <translation type="unfinished">Duración del análisis:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Errors:</source>
        <translation>Errores:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Warnings:</source>
        <translation>Advertencias:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Stylistic warnings:</source>
        <translation>Advertencias de estilo:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Portability warnings:</source>
        <translation>Advertencias de portabilidad:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Performance issues:</source>
        <translation>Problemas de rendimiento:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Information messages:</source>
        <translation>Mensajes de información:</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Active checkers:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Checkers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>History</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>File:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Copy to Clipboard</source>
        <translation>Copiar al portapapeles</translation>
    </message>
    <message>
        <location filename="statsdialog.ui"/>
        <source>Pdf Export</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="163"/>
        <source>1 day</source>
        <translation>1 día</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="163"/>
        <source>%1 days</source>
        <translation>%1 días</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>1 hour</source>
        <translation>1 hora</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="165"/>
        <source>%1 hours</source>
        <translation>%1 horas</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="167"/>
        <source>1 minute</source>
        <translation>1 minuto</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="167"/>
        <source>%1 minutes</source>
        <translation>%1 minutos</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="169"/>
        <source>1 second</source>
        <translation>1 segundo</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="169"/>
        <source>%1 seconds</source>
        <translation>%1 segundos</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="173"/>
        <source>0.%1 seconds</source>
        <translation>0.%1 segundos</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="175"/>
        <source> and </source>
        <translation> y </translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="202"/>
        <source>Export PDF</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="224"/>
        <source>Project Settings</source>
        <translation>Preferencias del proyecto</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="226"/>
        <source>Paths</source>
        <translation>Rutas</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="227"/>
        <source>Include paths</source>
        <translation type="unfinished">Incluye las rutas</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="228"/>
        <source>Defines</source>
        <translation type="unfinished">Definiciones</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="229"/>
        <source>Undefines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="231"/>
        <source>Path selected</source>
        <translation>Ruta seleccionada</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="232"/>
        <source>Number of files scanned</source>
        <translation type="unfinished">Número de archivos analizados</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="233"/>
        <source>Scan duration</source>
        <translation type="unfinished">Duración del análisis</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="189"/>
        <location filename="statsdialog.cpp" line="235"/>
        <source>Errors</source>
        <translation>Errores</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="116"/>
        <source>File: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="116"/>
        <source>No cppcheck build dir</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="191"/>
        <location filename="statsdialog.cpp" line="236"/>
        <source>Warnings</source>
        <translation>Advertencias</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="193"/>
        <location filename="statsdialog.cpp" line="237"/>
        <source>Style warnings</source>
        <translation>Advertencias de estilo</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="195"/>
        <location filename="statsdialog.cpp" line="238"/>
        <source>Portability warnings</source>
        <translation>Advertencias de portabilidad</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="197"/>
        <location filename="statsdialog.cpp" line="239"/>
        <source>Performance warnings</source>
        <translation>Advertencias de rendimiento</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="199"/>
        <location filename="statsdialog.cpp" line="240"/>
        <source>Information messages</source>
        <translation>Mensajes de información</translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <location filename="threadresult.cpp" line="45"/>
        <source>%1 of %2 files checked</source>
        <translation>%1 de %2 archivos comprobados</translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <location filename="translationhandler.cpp" line="126"/>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation>Ocurrió un error al cambiar el idioma de la interfaz gráfica:

%1

El idioma de la interfaz gráfica ha sido cambiado a Inglés. Abra la ventana de Preferencias para seleccionar alguno de los idiomas disponibles.</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="132"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <location filename="txtreport.cpp" line="74"/>
        <source>inconclusive</source>
        <translation>no concluyente</translation>
    </message>
</context>
<context>
    <name>toFilterString</name>
    <message>
        <location filename="common.cpp" line="58"/>
        <source>All supported files (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="common.cpp" line="63"/>
        <source>All files (%1)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
