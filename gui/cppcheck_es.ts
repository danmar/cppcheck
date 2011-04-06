<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="es_ES">
<context>
    <name>About</name>
    <message>
        <location filename="about.ui" line="14"/>
        <source>About Cppcheck</source>
        <translation>Sobre Cppcheck</translation>
    </message>
    <message>
        <location filename="about.ui" line="64"/>
        <source>Version %1</source>
        <translation>Version %1</translation>
    </message>
    <message>
        <location filename="about.ui" line="71"/>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - Utilidad para análisis estático de código.</translation>
    </message>
    <message utf8="true">
        <location filename="about.ui" line="81"/>
        <source>Copyright © 2007-2010 Daniel Marjamäki and cppcheck team.</source>
        <translation>Copyright © 2007-2010 Daniel Marjamäki y el equipo de cppcheck.</translation>
    </message>
    <message>
        <location filename="about.ui" line="91"/>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>Estre programa está licenciado bajo los términos de GNU General Public License versión 3</translation>
    </message>
    <message>
        <location filename="about.ui" line="102"/>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Visita Cppcheck en %1</translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <location filename="application.ui" line="17"/>
        <source>Add an application</source>
        <translation>Añade una aplicación</translation>
    </message>
    <message>
        <location filename="application.ui" line="23"/>
        <source>Here you can add applications that can open error files.
Specify a name for the application and the application to execute.

The following texts are replaced with appropriate values when application is executed:
(file) - Filename containing the error
(line) - Line number containing the error
(message) - Error message
(severity) - Error severity

Example opening a file with Kate and make Kate scroll to the correct line:
kate -l(line) (file)</source>
        <translation>Aqui puedes añadir una aplicación que pueda abrir ficheros de error
Especifica el nombre de la aplicación a ejecutar

Los siguientes textos serán reemplazados por los valores adecuados cuando la aplicación sea ejecutada:
(fichero) - Fichero que contiene el error
(línea) - Línea que contiene el error
(mensaje) - Mensaje de error
(importancia) - Importancia del error

Ejemplo sobre como abrir un fichero con kate y mover la barra de desplazamiento a la línea concreta:
kate -l(línea)(fila)</translation>
    </message>
    <message>
        <location filename="application.ui" line="47"/>
        <source>Application&apos;s name:</source>
        <translation>Nombre de la aplicación:</translation>
    </message>
    <message>
        <location filename="application.ui" line="57"/>
        <source>Command to execute:</source>
        <translation>Comando a ejecutar:</translation>
    </message>
    <message>
        <location filename="application.ui" line="96"/>
        <source>Browse</source>
        <translation>Buscar</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="56"/>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Ficheros ejecutables (*.exe);;Todos los ficheros(*.*)</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="59"/>
        <source>Select viewer application</source>
        <translation>Selecciona la aplicación para visualizar</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="96"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="applicationdialog.cpp" line="97"/>
        <source>You must specify a name and a path for the application!</source>
        <translation>¡Tienes que especificar el nombre y la ruta de la aplicación!</translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <location filename="fileviewdialog.cpp" line="43"/>
        <source>Could not find the file: %1</source>
        <translation>No se ha encontrado el fichero: %1</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="47"/>
        <location filename="fileviewdialog.cpp" line="62"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="fileviewdialog.cpp" line="58"/>
        <source>Could not read the file: %1</source>
        <translation>No se ha podido leer el fichero: %1</translation>
    </message>
</context>
<context>
    <name>HelpWindow</name>
    <message>
        <location filename="helpwindow.ui" line="14"/>
        <source>Cppcheck Help</source>
        <translation>Ayuda Cppcheck</translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="22"/>
        <source>Go back</source>
        <translation>Ir atrás</translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="25"/>
        <source>Back</source>
        <translation>Atrás</translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="42"/>
        <source>Go forward</source>
        <translation>Ir adelante</translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="45"/>
        <source>Forward</source>
        <translation>Adelante</translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="62"/>
        <source>Start</source>
        <translation>Empezar</translation>
    </message>
    <message>
        <location filename="helpwindow.ui" line="65"/>
        <source>Home</source>
        <translation>Inicio</translation>
    </message>
</context>
<context>
    <name>LogView</name>
    <message>
        <location filename="logview.ui" line="17"/>
        <source>Checking Log</source>
        <translation>Comprobando log</translation>
    </message>
    <message>
        <location filename="logview.ui" line="48"/>
        <source>Clear</source>
        <translation>Limpiar</translation>
    </message>
    <message>
        <location filename="logview.ui" line="55"/>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="main.ui" line="26"/>
        <location filename="mainwindow.cpp" line="231"/>
        <location filename="mainwindow.cpp" line="261"/>
        <location filename="mainwindow.cpp" line="535"/>
        <location filename="mainwindow.cpp" line="649"/>
        <location filename="mainwindow.cpp" line="667"/>
        <location filename="mainwindow.cpp" line="831"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="main.ui" line="74"/>
        <source>&amp;File</source>
        <translation>&amp;Fichero</translation>
    </message>
    <message>
        <location filename="main.ui" line="88"/>
        <source>&amp;View</source>
        <translation>&amp;Ver</translation>
    </message>
    <message>
        <location filename="main.ui" line="92"/>
        <source>&amp;Toolbars</source>
        <translation>&amp;Herramientas</translation>
    </message>
    <message>
        <location filename="main.ui" line="116"/>
        <source>&amp;Language</source>
        <translation>&amp;Idioma</translation>
    </message>
    <message>
        <location filename="main.ui" line="121"/>
        <source>&amp;Help</source>
        <translation>&amp;Ayuda</translation>
    </message>
    <message>
        <location filename="main.ui" line="131"/>
        <source>&amp;Check</source>
        <translation>&amp;Comprobar</translation>
    </message>
    <message>
        <location filename="main.ui" line="140"/>
        <source>&amp;Edit</source>
        <translation>&amp;Editar</translation>
    </message>
    <message>
        <location filename="main.ui" line="154"/>
        <source>Standard</source>
        <translation>Estándar</translation>
    </message>
    <message>
        <location filename="main.ui" line="173"/>
        <source>Categories</source>
        <translation>Categorías</translation>
    </message>
    <message>
        <location filename="main.ui" line="188"/>
        <source>&amp;License...</source>
        <translation>&amp;licencia...</translation>
    </message>
    <message>
        <location filename="main.ui" line="193"/>
        <source>A&amp;uthors...</source>
        <translation>A&amp;utores...</translation>
    </message>
    <message>
        <location filename="main.ui" line="202"/>
        <source>&amp;About...</source>
        <translation>&amp;sobre...</translation>
    </message>
    <message>
        <location filename="main.ui" line="207"/>
        <source>&amp;Files...</source>
        <translation>&amp;Ficheros...</translation>
    </message>
    <message>
        <location filename="main.ui" line="210"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="main.ui" line="219"/>
        <source>&amp;Directory...</source>
        <translation>&amp;Carpeta...</translation>
    </message>
    <message>
        <location filename="main.ui" line="222"/>
        <source>Ctrl+D</source>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="main.ui" line="231"/>
        <source>&amp;Recheck files</source>
        <translation>&amp;Volver a revisar ficheros</translation>
    </message>
    <message>
        <location filename="main.ui" line="234"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="main.ui" line="243"/>
        <source>&amp;Stop</source>
        <translation>&amp;Parar</translation>
    </message>
    <message>
        <location filename="main.ui" line="246"/>
        <source>Esc</source>
        <translation>Esc</translation>
    </message>
    <message>
        <location filename="main.ui" line="255"/>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Guardar los resultados en el fichero...</translation>
    </message>
    <message>
        <location filename="main.ui" line="258"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="main.ui" line="263"/>
        <source>&amp;Quit</source>
        <translation>&amp;Salir</translation>
    </message>
    <message>
        <location filename="main.ui" line="272"/>
        <source>&amp;Clear results</source>
        <translation>&amp;Limpiar resultados</translation>
    </message>
    <message>
        <location filename="main.ui" line="281"/>
        <source>&amp;Preferences</source>
        <translation>&amp;Preferencias</translation>
    </message>
    <message>
        <location filename="main.ui" line="293"/>
        <source>Style warnings</source>
        <translation>Estilos de advertencias</translation>
    </message>
    <message>
        <location filename="main.ui" line="296"/>
        <location filename="main.ui" line="299"/>
        <source>Show style warnings</source>
        <translation>Mostrar estilos de advertencias</translation>
    </message>
    <message>
        <location filename="main.ui" line="311"/>
        <source>Common errors</source>
        <translation>Errores comunes</translation>
    </message>
    <message>
        <location filename="main.ui" line="314"/>
        <location filename="main.ui" line="317"/>
        <source>Show common errors</source>
        <translation>Mostrar errores comunes</translation>
    </message>
    <message>
        <location filename="main.ui" line="322"/>
        <source>&amp;Check all</source>
        <translation>&amp;Seleccionar todo</translation>
    </message>
    <message>
        <location filename="main.ui" line="327"/>
        <source>&amp;Uncheck all</source>
        <translation>&amp;Deseleccionar todo</translation>
    </message>
    <message>
        <location filename="main.ui" line="332"/>
        <source>Collapse &amp;all</source>
        <translation>Contraer &amp;todo</translation>
    </message>
    <message>
        <location filename="main.ui" line="337"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Expandir todo</translation>
    </message>
    <message>
        <location filename="main.ui" line="345"/>
        <source>&amp;Standard</source>
        <translation>&amp;Estándar</translation>
    </message>
    <message>
        <location filename="main.ui" line="348"/>
        <source>Standard items</source>
        <translation>Elementos estándar</translation>
    </message>
    <message>
        <location filename="main.ui" line="353"/>
        <source>&amp;Contents</source>
        <translation>&amp;Contenidos</translation>
    </message>
    <message>
        <location filename="main.ui" line="356"/>
        <source>Open the help contents</source>
        <translation>Abrir la ayuda de contenidos</translation>
    </message>
    <message>
        <location filename="main.ui" line="359"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="main.ui" line="364"/>
        <source>Toolbar</source>
        <translation>Barra de herramientas</translation>
    </message>
    <message>
        <location filename="main.ui" line="372"/>
        <source>&amp;Categories</source>
        <translation>&amp;Categorías</translation>
    </message>
    <message>
        <location filename="main.ui" line="375"/>
        <source>Error categories</source>
        <translation>Categorías de error</translation>
    </message>
    <message>
        <location filename="main.ui" line="380"/>
        <source>&amp;Open XML...</source>
        <translation>&amp;Abrir XML...</translation>
    </message>
    <message>
        <location filename="main.ui" line="389"/>
        <source>Open P&amp;roject File...</source>
        <translation>Abrir P&amp;royecto...</translation>
    </message>
    <message>
        <location filename="main.ui" line="394"/>
        <source>&amp;New Project File...</source>
        <translation>&amp;Nuevo Proyecto...</translation>
    </message>
    <message>
        <location filename="main.ui" line="399"/>
        <source>&amp;Log View</source>
        <translation>&amp;Vista de log</translation>
    </message>
    <message>
        <location filename="main.ui" line="402"/>
        <source>Log View</source>
        <translation>Vista de log</translation>
    </message>
    <message>
        <location filename="main.ui" line="410"/>
        <source>C&amp;lose Project File</source>
        <translation>C&amp;errar Proyecto</translation>
    </message>
    <message>
        <location filename="main.ui" line="418"/>
        <source>&amp;Edit Project File...</source>
        <translation>&amp;Editar Proyecto...</translation>
    </message>
    <message>
        <location filename="main.ui" line="427"/>
        <source>&amp;Statistics</source>
        <translation>&amp;Estadísticas</translation>
    </message>
    <message>
        <location filename="main.ui" line="439"/>
        <source>Warnings</source>
        <translation>Advertencias</translation>
    </message>
    <message>
        <location filename="main.ui" line="442"/>
        <location filename="main.ui" line="445"/>
        <source>Show warnings</source>
        <translation>Mostrar advertencias</translation>
    </message>
    <message>
        <location filename="main.ui" line="457"/>
        <source>Performance warnings</source>
        <translation>Ajustar advertencias</translation>
    </message>
    <message>
        <location filename="main.ui" line="460"/>
        <location filename="main.ui" line="463"/>
        <source>Show performance warnings</source>
        <translation>Mostrar ajustes de advertencias</translation>
    </message>
    <message>
        <location filename="main.ui" line="471"/>
        <source>Show &amp;hidden</source>
        <translation>Mostrar &amp;ocultas</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="232"/>
        <source>No suitable files found to check!</source>
        <translation>¡No se han encontrado ficheros para comprobar!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="262"/>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>¡Tienes que cerrar el fichero de proyecto antes de seleccionar nuevos ficheros o carpetas!</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="277"/>
        <source>Select files to check</source>
        <translation>Selecciona ficheros para comprobar</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="291"/>
        <source>Select directory to check</source>
        <translation>Selecciona una carpeta para comprobar</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="334"/>
        <location filename="mainwindow.cpp" line="762"/>
        <location filename="mainwindow.cpp" line="808"/>
        <source>Project: </source>
        <translation>Proyecto:</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="464"/>
        <location filename="mainwindow.cpp" line="599"/>
        <source>XML files (*.xml)</source>
        <translation>Ficheros XML(*.xml)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="466"/>
        <source>Open the report file</source>
        <translation>Abrir informe</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="531"/>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?.</source>
        <translation>El proceso de comprobación está en curso.

¿Quieres parar la comprobación y salir del Cppcheck?.</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="574"/>
        <source>License</source>
        <translation>Licencia</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="581"/>
        <source>Authors</source>
        <translation>Autores</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="589"/>
        <source>XML files (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation>Ficheros XML (*.xml);; Ficheros de texto (*.txt);; Ficheros CVS (*.cvs)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="591"/>
        <source>Save the report file</source>
        <translation>Guardar informe</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="605"/>
        <source>Text files (*.txt)</source>
        <translation>Ficheros de texto (*.txt)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="611"/>
        <source>CSV files (*.csv)</source>
        <translation>Ficheros CVS (*.cvs)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="651"/>
        <source>Cppcheck - %1</source>
        <translation>Cppcheck - %1</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="668"/>
        <source>Failed to change the language:

%1

</source>
        <translation>Error al cambiar el idioma:

%1
</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="729"/>
        <location filename="mainwindow.cpp" line="738"/>
        <source>Cppcheck Help</source>
        <translation>Ayuda Cppcheck</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="729"/>
        <source>Failed to load help file (not found)</source>
        <translation>Error al cargar fichero de ayuda (No encontrado)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="738"/>
        <source>Failed to load help file</source>
        <translation>Error al cargar el fichero de ayuda</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="752"/>
        <location filename="mainwindow.cpp" line="797"/>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation>Ficheros de proyecto (*.cppcheck;;Todos los ficheros (*.*)</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="754"/>
        <source>Select Project File</source>
        <translation>Selecciona proyecto</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="799"/>
        <source>Select Project Filename</source>
        <translation>Selecciona el nombre del proyecto</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="832"/>
        <source>No project file loaded</source>
        <translation>No hay ningún proyecto cargado</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="32"/>
        <source>English</source>
        <translation>English</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="33"/>
        <source>Dutch</source>
        <translation>Holandés</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="34"/>
        <source>Finnish</source>
        <translation>Finés</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="35"/>
        <source>Swedish</source>
        <translation>Sueco</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="36"/>
        <source>German</source>
        <translation>Alemán</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="37"/>
        <source>Russian</source>
        <translation>Ruso</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="38"/>
        <source>Polish</source>
        <translation>Polaco</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="39"/>
        <source>Japanese</source>
        <translation>Japonés</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="40"/>
        <source>Serbian</source>
        <translation>Servio</translation>
    </message>
</context>
<context>
    <name>Project</name>
    <message>
        <location filename="project.cpp" line="63"/>
        <location filename="project.cpp" line="103"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="project.cpp" line="64"/>
        <source>Could not read the project file.</source>
        <translation>No se ha podido leer el fichero.</translation>
    </message>
    <message>
        <location filename="project.cpp" line="104"/>
        <source>Could not write the project file.</source>
        <translation>No se ha podido escribir el fichero de proyecto.</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <location filename="projectfile.ui" line="14"/>
        <source>Project File</source>
        <translation>Fichero de proyecto</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="22"/>
        <source>Project:</source>
        <translation>Proyecto:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="39"/>
        <source>Paths:</source>
        <translation>Rutas:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="52"/>
        <location filename="projectfile.ui" line="76"/>
        <source>Browse...</source>
        <translation>Buscar...</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="63"/>
        <source>Include paths:</source>
        <translation>Incluye rutas:</translation>
    </message>
    <message>
        <location filename="projectfile.ui" line="87"/>
        <source>Defines:</source>
        <translation>Definiciones:</translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <location filename="projectfiledialog.cpp" line="36"/>
        <source>Project file: %1</source>
        <translation>Fichero de proyecto: %1</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="153"/>
        <source>Select include directory</source>
        <translation>Selecciona una carpeta para incluir</translation>
    </message>
    <message>
        <location filename="projectfiledialog.cpp" line="165"/>
        <source>Select directory to check</source>
        <translation>Selecciona una carpeta para comprobar</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="translationhandler.cpp" line="97"/>
        <source>Incorrect language specified!</source>
        <translation>¡Lenguaje especificado incorrecto!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="107"/>
        <source>Language file %1 not found!</source>
        <translation>¡Fichero de idioma %1 no encontrado!</translation>
    </message>
    <message>
        <location filename="translationhandler.cpp" line="113"/>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>Fallo al cargar la traducción para el idioma %1 desde el fichero %2</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="962"/>
        <source>File</source>
        <translation>Fichero</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="962"/>
        <source>Severity</source>
        <translation>Importancia</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="962"/>
        <source>Line</source>
        <translation>Línea</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="59"/>
        <location filename="resultstree.cpp" line="962"/>
        <source>Summary</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="108"/>
        <source>Undefined file</source>
        <translation>Fichero no definido</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="504"/>
        <source>Copy filename</source>
        <translation>Copia nombre</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="505"/>
        <source>Copy full path</source>
        <translation>Copia ruta</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="506"/>
        <source>Copy message</source>
        <translation>Copia mensaje</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="507"/>
        <source>Hide</source>
        <translation>Oculta</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="554"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="555"/>
        <source>Configure the text file viewer program in Cppcheck preferences/Applications.</source>
        <translation>Configura el programa para ver ficheros en Cppcheck preferencias/Aplicaciones.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="582"/>
        <source>Could not find the file!</source>
        <translation>¡No se ha encontrado el fichero!</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="620"/>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>No se ha podido ejecutar %1

Por favor comprueba que la ruta a la aplicación y los parámetros son correctos.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="634"/>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>No se ha encontrado el fichero:
%1
Por favor selecciona la carpeta donde se encuentra.</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="641"/>
        <source>Select Directory</source>
        <translation>Selecciona carpeta</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="819"/>
        <source>style</source>
        <translation>estilo</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="823"/>
        <source>error</source>
        <translation>error</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="827"/>
        <source>warning</source>
        <translation>advertencia</translation>
    </message>
    <message>
        <location filename="resultstree.cpp" line="831"/>
        <source>performance</source>
        <translation>ajuste</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <location filename="resultsview.ui" line="26"/>
        <source>Results</source>
        <translation>Resultados</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="117"/>
        <source>No errors found, nothing to save.</source>
        <translation>No se han encontrado errores, nada que guardar.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="144"/>
        <location filename="resultsview.cpp" line="154"/>
        <source>Failed to save the report.</source>
        <translation>Error al guardar el informe.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="190"/>
        <location filename="resultsview.cpp" line="202"/>
        <source>Cppcheck</source>
        <translation>Cppcheck</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="191"/>
        <source>No errors found.</source>
        <translation>No se han encontrado errores.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="199"/>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Se han encontrado errores, pero están configurados para que no se muestren.
Para cambiar el tipo de comportamiento, abrir el menú vista.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="251"/>
        <location filename="resultsview.cpp" line="261"/>
        <source>Failed to read the report.</source>
        <translation>Error al leer el informe.</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="302"/>
        <source>Summary</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <location filename="resultsview.cpp" line="303"/>
        <source>Message</source>
        <translation>Mensaje</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="settings.ui" line="14"/>
        <source>Preferences</source>
        <translation>Preferencias</translation>
    </message>
    <message>
        <location filename="settings.ui" line="24"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="settings.ui" line="32"/>
        <source>Include paths:</source>
        <translation>Incluye rutas:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="45"/>
        <source>Add...</source>
        <translation>Añadir...</translation>
    </message>
    <message>
        <location filename="settings.ui" line="56"/>
        <source>Number of threads: </source>
        <translation>Número de hilos:</translation>
    </message>
    <message>
        <location filename="settings.ui" line="78"/>
        <source>Check all #ifdef configurations</source>
        <translation>Comprobar todas las configuraciones #ifdef</translation>
    </message>
    <message>
        <location filename="settings.ui" line="85"/>
        <source>Show full path of files</source>
        <translation>Mostrar la ruta completa de los ficheros</translation>
    </message>
    <message>
        <location filename="settings.ui" line="92"/>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Mostrar el mensaje &quot;No se han encontrado errores&quot;</translation>
    </message>
    <message>
        <location filename="settings.ui" line="99"/>
        <source>Show internal warnings in log</source>
        <translation>Mostrar advertencias internas en el informe</translation>
    </message>
    <message>
        <location filename="settings.ui" line="120"/>
        <source>Applications</source>
        <translation>Aplicaciones</translation>
    </message>
    <message>
        <location filename="settings.ui" line="129"/>
        <source>Add application</source>
        <translation>Añadir aplicación</translation>
    </message>
    <message>
        <location filename="settings.ui" line="136"/>
        <source>Delete application</source>
        <translation>Borrar aplicación</translation>
    </message>
    <message>
        <location filename="settings.ui" line="143"/>
        <source>Modify application</source>
        <translation>Modificar aplicación</translation>
    </message>
    <message>
        <location filename="settings.ui" line="150"/>
        <source>Set as default application</source>
        <translation>Establecer como aplicación predeterminada</translation>
    </message>
    <message>
        <location filename="settings.ui" line="158"/>
        <source>Reports</source>
        <translation>Informes</translation>
    </message>
    <message>
        <location filename="settings.ui" line="164"/>
        <source>Save all errors when creating report</source>
        <translation>Guardar todos los errores cuando se cree el informe</translation>
    </message>
    <message>
        <location filename="settings.ui" line="171"/>
        <source>Save full path to files in reports</source>
        <translation>Guardar la ruta completa en los ficheros de informes</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="settingsdialog.cpp" line="136"/>
        <source>Add a new application</source>
        <translation>Añade una nueva aplicación</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="169"/>
        <source>Modify an application</source>
        <translation>Modificar aplicación</translation>
    </message>
    <message>
        <location filename="settingsdialog.cpp" line="234"/>
        <source>Select include directory</source>
        <translation>Seleccionar carpeta incluida</translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <location filename="stats.ui" line="14"/>
        <location filename="stats.ui" line="228"/>
        <source>Statistics</source>
        <translation>Estadísticas</translation>
    </message>
    <message>
        <location filename="stats.ui" line="27"/>
        <source>Project</source>
        <translation>Proyecto</translation>
    </message>
    <message>
        <location filename="stats.ui" line="33"/>
        <source>Project:</source>
        <translation>Proyecto:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="53"/>
        <source>Paths:</source>
        <translation>Rutas:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="85"/>
        <source>Include paths:</source>
        <translation>Incluye rutas:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="108"/>
        <source>Defines:</source>
        <translation>Definiciones:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="145"/>
        <source>Previous Scan</source>
        <translation>Revisión anterior</translation>
    </message>
    <message>
        <location filename="stats.ui" line="151"/>
        <source>Path Selected:</source>
        <translation>Ruta seleccionada:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="161"/>
        <source>Number of Files Scanned:</source>
        <translation>Número de ficheros revisados:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="181"/>
        <source>Scan Duration:</source>
        <translation>Tiempo de revisión:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="236"/>
        <source>Errors:</source>
        <translation>Errores:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="254"/>
        <source>Warnings:</source>
        <translation>Advertencias:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="272"/>
        <source>Stylistic warnings:</source>
        <translation>Advertencias de estilo:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="290"/>
        <source>Performance issues:</source>
        <translation>Problemas de rendimiento:</translation>
    </message>
    <message>
        <location filename="stats.ui" line="326"/>
        <source>Copy to Clipboard</source>
        <translation>Copiar al portapapeles</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="69"/>
        <source>%1 secs</source>
        <translation>%1 segundos</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="78"/>
        <source>Project Settings
	Project:	%1
	Paths:	%2
	Include paths:	%3
	Defines:	%4
Previous Scan
	Path selected:	%5
	Number of files scanned:	%6
	Scan duration:	%7
Statistics
	Errors:	%8
	Warnings:	%9
	Style warnings:	%10
	Performance warnings:	%11
</source>
        <translation>Configuración del proyecto
	Proyecto:	%1
	Rutas:	%2
	Rutas incluidas:	%3
	Definiciones:	%4
Revisión anterior
	Ruta seleccionada:	%5
	Número de ficheros revisados:	%6
	Duración de la revisión:	%7
Estadísticas:
	Errores:	%8
	Advertencias:	%9
	Advertencias de estilo:	%10
	Advertencias de rendimiento:	%11
</translation>
    </message>
    <message>
        <location filename="statsdialog.cpp" line="107"/>
        <source>&lt;h3&gt;Project Settings&lt;h3&gt;
&lt;table&gt;
 &lt;tr&gt;&lt;th&gt;Project:&lt;/th&gt;&lt;td&gt;%1&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Paths:&lt;/th&gt;&lt;td&gt;%2&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Include paths:&lt;/th&gt;&lt;td&gt;%3&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Defines:&lt;/th&gt;&lt;td&gt;%4&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;
&lt;h3&gt;Previous Scan&lt;/h3&gt;
&lt;table&gt;
 &lt;tr&gt;&lt;th&gt;Path selected:&lt;/th&gt;&lt;td&gt;%5&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Number of files scanned:&lt;/th&gt;&lt;td&gt;%6&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Scan duration:&lt;/th&gt;&lt;td&gt;%7&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;
&lt;h3&gt;Statistics&lt;/h3&gt;
 &lt;tr&gt;&lt;th&gt;Errors:&lt;/th&gt;&lt;td&gt;%8&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Warnings:&lt;/th&gt;&lt;td&gt;%9&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Style warnings:&lt;/th&gt;&lt;td&gt;%10&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Performance warnings:&lt;/th&gt;&lt;td&gt;%11&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;
</source>
        <translation>&lt;h3&gt;Configuración del proyecto&lt;h3&gt;
&lt;table&gt;
 &lt;tr&gt;&lt;th&gt;Proyecto:&lt;/th&gt;&lt;td&gt;%1&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Rutas:&lt;/th&gt;&lt;td&gt;%2&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Rutas incluidas:&lt;/th&gt;&lt;td&gt;%3&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Definiciones:&lt;/th&gt;&lt;td&gt;%4&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;
&lt;h3&gt;Revisión anterior&lt;/h3&gt;
&lt;table&gt;
 &lt;tr&gt;&lt;th&gt;Ruta seleccionada:&lt;/th&gt;&lt;td&gt;%5&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Número de ficheros revisados:&lt;/th&gt;&lt;td&gt;%6&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Duración de la revisión:&lt;/th&gt;&lt;td&gt;%7&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;
&lt;h3&gt;Estadísticas&lt;/h3&gt;
 &lt;tr&gt;&lt;th&gt;Errores:&lt;/th&gt;&lt;td&gt;%8&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Advertencias:&lt;/th&gt;&lt;td&gt;%9&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Advertencias de estilo:&lt;/th&gt;&lt;td&gt;%10&lt;/td&gt;&lt;/tr&gt;
 &lt;tr&gt;&lt;th&gt;Advertencias de rendimiento:&lt;/th&gt;&lt;td&gt;%11&lt;/td&gt;&lt;/tr&gt;
&lt;/table&gt;
</translation>
    </message>
</context>
</TS>
