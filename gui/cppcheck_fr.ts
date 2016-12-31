<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="fr_FR" sourcelanguage="en_GB">
<context>
    <name>About</name>
    <message>
        <source>About Cppcheck</source>
        <translation>A propos</translation>
    </message>
    <message>
        <source>Version %1</source>
        <translation>Version %1</translation>
    </message>
    <message>
        <source>Cppcheck - A tool for static C/C++ code analysis.</source>
        <translation>Cppcheck - Un outil d&apos;analyse statique de code C/C++.</translation>
    </message>
    <message>
        <source>This program is licensed under the terms
of the GNU General Public License version 3</source>
        <translation>Ce programme est sous licence GNU
General Public License version 3</translation>
    </message>
    <message>
        <source>Visit Cppcheck homepage at %1</source>
        <translation>Visitez le site Cppcheck : %1</translation>
    </message>
    <message utf8="true">
        <source>Copyright © 2007-2016 Cppcheck team.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ApplicationDialog</name>
    <message>
        <source>Add an application</source>
        <translation>Ajouter une application</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Parcourir</translation>
    </message>
    <message>
        <source>Executable files (*.exe);;All files(*.*)</source>
        <translation>Fichier exécutable (*.exe);;Tous les fichiers(*.*)</translation>
    </message>
    <message>
        <source>Select viewer application</source>
        <translation>Sélection de l&apos;application</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;Executable:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Parameters:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
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
        <source>&amp;Name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You must specify a name, a path and optionally parameters for the application!</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FileViewDialog</name>
    <message>
        <source>Could not find the file: %1</source>
        <translation>Ne trouve pas le fichier : %1</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not read the file: %1</source>
        <translation>Ne peut pas lire le fichier : %1</translation>
    </message>
</context>
<context>
    <name>LibraryAddFunctionDialog</name>
    <message>
        <source>Add function</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Function name(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Number of arguments</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LibraryDialog</name>
    <message>
        <source>Library Editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="unfinished">Sauvegarder</translation>
    </message>
    <message>
        <source>Functions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add</source>
        <translation type="unfinished">Ajouter</translation>
    </message>
    <message>
        <source>noreturn</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>False</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>True</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>return value must be used</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ignore function in leaks checking</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Arguments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="unfinished">Editer</translation>
    </message>
    <message>
        <source>Library files (*.cfg)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open library file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sort</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Comments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Can not open file %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Can not save file %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save the library as</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LibraryEditArgDialog</name>
    <message>
        <source>Edit argument</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is bool value allowed? For instance result from comparison or from &apos;!&apos; operator.&lt;/p&gt;
&lt;p&gt;Typically, set this if the argument is a pointer, size, etc.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    memcmp(x, y, i == 123);   // last argument should not have a bool value&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not bool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;p&gt;Is a null parameter value allowed?&lt;/p&gt;
&lt;p&gt;Typically this should be used on any pointer parameter that does not allow null.&lt;/p&gt;
&lt;p&gt;Example:&lt;/p&gt;
&lt;pre&gt;    strcpy(x,y); // neither x or y is allowed to be null.&lt;/pre&gt;
&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not null</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not uninit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>String</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Format string</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Min size of buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>None</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>argvalue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>constant</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>mul</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>strlen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Arg</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Arg2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>and</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Valid values</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LogView</name>
    <message>
        <source>Checking Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Text files (*.txt *.log);;All files (*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not open file for writing: &quot;%1&quot;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <source>Cppcheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>&amp;Fichier</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Affichage</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Aide</translation>
    </message>
    <message>
        <source>&amp;Check</source>
        <translation>&amp;Vérifier</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Édition</translation>
    </message>
    <message>
        <source>Standard</source>
        <translation>Standard</translation>
    </message>
    <message>
        <source>&amp;License...</source>
        <translation>&amp;Licence...</translation>
    </message>
    <message>
        <source>A&amp;uthors...</source>
        <translation>A&amp;uteurs...</translation>
    </message>
    <message>
        <source>&amp;About...</source>
        <translation>À &amp;Propos...</translation>
    </message>
    <message>
        <source>&amp;Files...</source>
        <translation>&amp;Fichiers...</translation>
    </message>
    <message>
        <source>Ctrl+F</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Directory...</source>
        <translation>&amp;Répertoires...</translation>
    </message>
    <message>
        <source>Ctrl+D</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Recheck files</source>
        <translation type="obsolete">&amp;Revérifier les fichiers</translation>
    </message>
    <message>
        <source>Ctrl+R</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Stop</source>
        <translation>&amp;Arrêter</translation>
    </message>
    <message>
        <source>Esc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Save results to file...</source>
        <translation>&amp;Sauvegarder les résultats dans un fichier...</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Quit</source>
        <translation>&amp;Quitter</translation>
    </message>
    <message>
        <source>&amp;Clear results</source>
        <translation>&amp;Effacer les résultats</translation>
    </message>
    <message>
        <source>&amp;Preferences</source>
        <translation>&amp;Préférences</translation>
    </message>
    <message>
        <source>&amp;Check all</source>
        <translation>&amp;Tout cocher</translation>
    </message>
    <message>
        <source>&amp;Uncheck all</source>
        <translation>&amp;Tout décocher</translation>
    </message>
    <message>
        <source>Collapse &amp;all</source>
        <translation>&amp;Tout réduire</translation>
    </message>
    <message>
        <source>&amp;Expand all</source>
        <translation>&amp;Tout dérouler</translation>
    </message>
    <message>
        <source>&amp;Contents</source>
        <translation>&amp;Contenus</translation>
    </message>
    <message>
        <source>Open the help contents</source>
        <translation>Ouvir l&apos;aide</translation>
    </message>
    <message>
        <source>F1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No suitable files found to check!</source>
        <translation>Pas de fichiers trouvés à vérifier !</translation>
    </message>
    <message>
        <source>Select directory to check</source>
        <translation>Sélectionner le répertoire à vérifier</translation>
    </message>
    <message>
        <source>License</source>
        <translation>Licence</translation>
    </message>
    <message>
        <source>Authors</source>
        <translation>Auteurs</translation>
    </message>
    <message>
        <source>Save the report file</source>
        <translation>Sauvegarder le rapport</translation>
    </message>
    <message>
        <source>XML files (*.xml)</source>
        <translation>Fichiers XML (*.xml)</translation>
    </message>
    <message>
        <source>Text files (*.txt)</source>
        <translation>Fichiers Texte (*.txt)</translation>
    </message>
    <message>
        <source>CSV files (*.csv)</source>
        <translation>Fichiers CSV (*.csv)</translation>
    </message>
    <message>
        <source>Cppcheck - %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Toolbars</source>
        <translation>&amp;Boite à outils</translation>
    </message>
    <message>
        <source>Categories</source>
        <translation>Catégories</translation>
    </message>
    <message>
        <source>Check files</source>
        <translation>Vérifier les fichiers</translation>
    </message>
    <message>
        <source>Check directory</source>
        <translation>Vérifier un répertoire</translation>
    </message>
    <message>
        <source>Stop checking</source>
        <translation>Arrêter la vérification</translation>
    </message>
    <message>
        <source>Style warnings</source>
        <translation>Avertissement de style</translation>
    </message>
    <message>
        <source>Show style warnings</source>
        <translation>Afficher les avertissements de style</translation>
    </message>
    <message>
        <source>Errors</source>
        <translation>Erreurs</translation>
    </message>
    <message>
        <source>Show errors</source>
        <translation>Afficher les erreurs</translation>
    </message>
    <message>
        <source>&amp;Standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Standard items</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Toolbar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Categories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error categories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Open XML...</source>
        <translation>&amp;Ouvrir un fichier XML...</translation>
    </message>
    <message>
        <source>Open P&amp;roject File...</source>
        <translation>Ouvrir un P&amp;rojet...</translation>
    </message>
    <message>
        <source>&amp;New Project File...</source>
        <translation>&amp;Nouveau Projet...</translation>
    </message>
    <message>
        <source>&amp;Log View</source>
        <translation>&amp;Journal</translation>
    </message>
    <message>
        <source>Log View</source>
        <translation>Journal</translation>
    </message>
    <message>
        <source>C&amp;lose Project File</source>
        <translation>F&amp;ermer le projet</translation>
    </message>
    <message>
        <source>&amp;Edit Project File...</source>
        <translation>&amp;Editer le projet</translation>
    </message>
    <message>
        <source>&amp;Statistics</source>
        <translation>Statistiques</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation>Avertissements</translation>
    </message>
    <message>
        <source>Show warnings</source>
        <translation>Afficher les avertissements</translation>
    </message>
    <message>
        <source>Performance warnings</source>
        <translation>Avertissements de performance</translation>
    </message>
    <message>
        <source>Show performance warnings</source>
        <translation>Afficher les avertissements de performance</translation>
    </message>
    <message>
        <source>Show &amp;hidden</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Information</source>
        <translation>Information</translation>
    </message>
    <message>
        <source>Show information messages</source>
        <translation>Afficher les messages d&apos;information</translation>
    </message>
    <message>
        <source>Portability</source>
        <translation>Portabilité</translation>
    </message>
    <message>
        <source>Show portability warnings</source>
        <translation>Afficher les problèmes de portabilité</translation>
    </message>
    <message>
        <source>You must close the project file before selecting new files or directories!</source>
        <translation>Vous devez d&apos;abord fermer le projet avant de choisir des fichiers/répertoires</translation>
    </message>
    <message>
        <source>Open the report file</source>
        <translation>Ouvrir le rapport</translation>
    </message>
    <message>
        <source>Checking is running.

Do you want to stop the checking and exit Cppcheck?</source>
        <translation>Vérification en cours.
		
Voulez-vous arrêter la vérification et quitter CppCheck ?</translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml);;XML files version 1 (*.xml);;Text files (*.txt);;CSV files (*.csv)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>XML files version 1 (*.xml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>XML files version 2 (*.xml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Project files (*.cppcheck);;All files(*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select Project File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select Project Filename</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No project file loaded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>There was a problem with loading the editor application settings.

This is probably because the settings were changed between the Cppcheck versions. Please check (and fix) the editor application settings, otherwise the editor program might not start correctly.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Filter</source>
        <translation>&amp;Filtre</translation>
    </message>
    <message>
        <source>Filter results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Quick Filter:</source>
        <translation>Filtre rapide : </translation>
    </message>
    <message>
        <source>Found project file: %1

Do you want to load this project file instead?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Found project files from the directory.

Do you want to proceed checking without using any of these project files?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Project:</source>
        <translation>Projet : </translation>
    </message>
    <message>
        <source>The project file

%1

 could not be found!

Do you want to remove the file from the recently used projects -list?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Filter</source>
        <translation>Filtre</translation>
    </message>
    <message>
        <source>Windows 32-bit ANSI</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Windows 32-bit Unicode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unix 32-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unix 64-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Windows 64-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Platforms</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>C++11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>C99</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Posix</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Current results will be cleared.

Opening a new XML file will clear current results.Do you want to proceed?</source>
        <translation>Les résultats courant seront effacés.
		
L&apos;ouverture d&apos;un nouveau fichier XML effacera les resultats. Voulez-vous confirmar l&apos;opération ?</translation>
    </message>
    <message>
        <source>Show S&amp;cratchpad...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select files to check</source>
        <translation>Sélectionner les fichiers à vérifier</translation>
    </message>
    <message>
        <source>Cppcheck GUI - Command line parameters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>C++ standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>C standard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>C11</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>C89</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>C++03</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Erreur</translation>
    </message>
    <message>
        <source>Cppcheck GUI.

Syntax:
    cppcheck-gui [OPTIONS] [files or paths]

Options:
    -h, --help              Print this help
    -p &lt;file&gt;               Open given project file and start checking it
    -l &lt;file&gt;               Open given results xml file
    -d &lt;directory&gt;          Specify the directory that was checked to generate the results xml specified with -l
    -v, --version           Show program version
    --data-dir=&lt;directory&gt;  Specify directory where GUI datafiles are located (translations, cfg)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to load %1. Your Cppcheck installation is broken. You can use --data-dir=&lt;directory&gt; at the command line to specify where this file is located.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File not found</source>
        <translation>Fichier introuvable</translation>
    </message>
    <message>
        <source>Bad XML</source>
        <translation>Mauvais fichier XML</translation>
    </message>
    <message>
        <source>Missing attribute</source>
        <translation>Attribut manquant</translation>
    </message>
    <message>
        <source>Bad attribute value</source>
        <translation>Mauvaise valeur d&apos;attribut</translation>
    </message>
    <message>
        <source>Failed to load the selected library &apos;%1&apos;.
%2</source>
        <translation>Echec lors du chargement de la bibliothèque &apos;%1&apos;.
%2</translation>
    </message>
    <message>
        <source>Unsupported format</source>
        <translation>Format non supporté</translation>
    </message>
    <message>
        <source>The library &apos;%1&apos; contains unknown elements:
%2</source>
        <translation>La bibliothèque &apos;%1&apos; contient des éléments inconnus:
%2</translation>
    </message>
    <message>
        <source>Duplicate platform type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Platform type redefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation> &amp;Imprimer...</translation>
    </message>
    <message>
        <source>Print the Current Report</source>
        <translation>Imprimer le rapport</translation>
    </message>
    <message>
        <source>Print Pre&amp;view...</source>
        <translation>Apercu d&apos;impression...</translation>
    </message>
    <message>
        <source>Open a Print Preview Dialog for the Current Results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Library Editor...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open library editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Auto-detect language</source>
        <translation>Auto-detection du langage</translation>
    </message>
    <message>
        <source>Enforce C++</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enforce C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Recheck modified files</source>
        <translation>&amp;Revérifier les fichiers modifiés</translation>
    </message>
    <message>
        <source>&amp;Recheck all files</source>
        <translation>&amp;Revérifier tous les fichiers</translation>
    </message>
    <message>
        <source>Unknown element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unknown issue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>C/C++ Source, Compile database, Visual Studio (%1 %2 *.sln *.vcxproj)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select configuration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select the configuration that will be checked</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Deprecated XML format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>XML format 1 is deprecated and will be removed in cppcheck 1.81.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Platforms</name>
    <message>
        <source>Unix 32-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unix 64-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Windows 32-bit ANSI</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Windows 32-bit Unicode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Windows 64-bit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Project</name>
    <message>
        <source>Cppcheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not read the project file.</source>
        <translation>Impossible de lire le fichier projet.</translation>
    </message>
    <message>
        <source>Could not write the project file.</source>
        <translation>Impossible d&apos;écrire dans le fichier projet.</translation>
    </message>
</context>
<context>
    <name>ProjectFile</name>
    <message>
        <source>Project File</source>
        <translation>Fichier Projet</translation>
    </message>
    <message>
        <source>Paths:</source>
        <translation>Chemins : </translation>
    </message>
    <message>
        <source>Defines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Project</source>
        <translation>Projet</translation>
    </message>
    <message>
        <source>Add...</source>
        <translation>Ajouter...</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editer</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Supprimer</translation>
    </message>
    <message>
        <source>Includes</source>
        <translation type="obsolete">Inclusions</translation>
    </message>
    <message>
        <source>Include directories:</source>
        <translation type="obsolete">Inclure les répertoires</translation>
    </message>
    <message>
        <source>Root:</source>
        <translation>Répertoire racine</translation>
    </message>
    <message>
        <source>Up</source>
        <translation>Monter</translation>
    </message>
    <message>
        <source>Down</source>
        <translation>Descendre</translation>
    </message>
    <message>
        <source>Exclude</source>
        <translation>Exclure</translation>
    </message>
    <message>
        <source>Libraries:</source>
        <translation>Bibliothèques</translation>
    </message>
    <message>
        <source>Suppressions</source>
        <translation>Suppressions</translation>
    </message>
    <message>
        <source>Suppression list:</source>
        <translation>Liste de suppressions</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Ajouter</translation>
    </message>
    <message>
        <source>Note: Put your own custom .cfg files in the same folder as the project file. You should see them above.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Defines must be separated by a semicolon &apos;;&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Visual Studio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Visual Studio

Cppcheck can import visual studio solutions and projects.

Files to check, include paths, configurations, defines, platform settings are imported.

Library settings are not imported.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>CMake</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Compile database

Cppcheck can import files to analyse, include paths, defines from the compile database.

Platform settings are not provided in compile database and must be configured.

Library settings are not provided in compile database, be careful about this configuration also.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Other</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Include Paths:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cppcheck build dir (optional)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProjectFileDialog</name>
    <message>
        <source>Project file: %1</source>
        <translation>Fichier projet : %1</translation>
    </message>
    <message>
        <source>Select include directory</source>
        <translation>Selectionner un répertoire à inclure</translation>
    </message>
    <message>
        <source>Select directory to ignore</source>
        <translation>Selectionner un répertoire à ignorer</translation>
    </message>
    <message>
        <source>Select a directory to check</source>
        <translation>Selectionner un répertoire à vérifier</translation>
    </message>
    <message>
        <source>Add Suppression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select error id suppress:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Compile Database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Compile database (compile_database.json)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Visual Studio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Visual Studio Solution/Project (*.sln *.vcxproj)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select Cppcheck build dir</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QDialogButtonBox</name>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annuler</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fermer</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Sauvegarder</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>Language file %1 not found!</source>
        <translation>Fichier de langue %1 non trouvé !</translation>
    </message>
    <message>
        <source>Failed to load translation for language %1 from file %2</source>
        <translation>Erreur lors du chargement de la langue %1 depuis le fichier %2</translation>
    </message>
    <message>
        <source>Unknown language specified!</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QPlatformTheme</name>
    <message>
        <source>OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="unfinished">Annuler</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Fermer</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="unfinished">Sauvegarder</translation>
    </message>
</context>
<context>
    <name>ResultsTree</name>
    <message>
        <source>File</source>
        <translation type="unfinished">Fichier</translation>
    </message>
    <message>
        <source>Severity</source>
        <translation type="unfinished">Sévérité</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="unfinished">Ligne</translation>
    </message>
    <message>
        <source>Undefined file</source>
        <translation>Fichier indéterminé</translation>
    </message>
    <message>
        <source>Copy filename</source>
        <translation>Copier le nom du fichier</translation>
    </message>
    <message>
        <source>Copy full path</source>
        <translation>Copier le chemin complet</translation>
    </message>
    <message>
        <source>Copy message</source>
        <translation>Copier le message</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not start %1

Please check the application path and parameters are correct.</source>
        <translation>Ne peut pas démarrer %1

Merci de vérifier que le chemin de l&apos;application et que les paramètres sont corrects.</translation>
    </message>
    <message>
        <source>style</source>
        <translation>erreur de style</translation>
    </message>
    <message>
        <source>error</source>
        <translation>erreur</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Résumé</translation>
    </message>
    <message>
        <source>Hide</source>
        <translation>Cacher</translation>
    </message>
    <message>
        <source>Could not find the file!</source>
        <translation>Fichier introuvable !</translation>
    </message>
    <message>
        <source>Could not find file:
%1
Please select the directory where file is located.</source>
        <translation>Fichier introuvable:
%1
Veuillez sélectionner le répertoire où est situé le fichier.</translation>
    </message>
    <message>
        <source>Select Directory</source>
        <translation>Selectionner dossier</translation>
    </message>
    <message>
        <source>warning</source>
        <translation>avertissement</translation>
    </message>
    <message>
        <source>performance</source>
        <translation>performance</translation>
    </message>
    <message>
        <source>portability</source>
        <translation>portabilité</translation>
    </message>
    <message>
        <source>information</source>
        <translation>information</translation>
    </message>
    <message>
        <source>debug</source>
        <translation>débogage</translation>
    </message>
    <message>
        <source>[Inconclusive]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No editor application configured.

Configure the editor application for Cppcheck in preferences/Applications.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No default editor application selected.

Please select the default editor application in preferences/Applications.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <source>Copy message id</source>
        <translation>Copier l&apos;identifiant du message</translation>
    </message>
    <message>
        <source>Hide all with id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open containing folder</source>
        <translation>Ouvrir l&apos;emplacement du fichier</translation>
    </message>
    <message>
        <source>Inconclusive</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Recheck</source>
        <translation>Revérifier</translation>
    </message>
</context>
<context>
    <name>ResultsView</name>
    <message>
        <source>Results</source>
        <translation>Résultats</translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No errors found.</source>
        <translation>Pas d&apos;erreurs trouvées.</translation>
    </message>
    <message>
        <source>Errors were found, but they are configured to be hidden.
To toggle what kind of errors are shown, open view menu.</source>
        <translation>Des erreurs ont été trouvées mais sont configurées pour rester cachées
Pour configurer les erreurs affichées, ouvrez le menu d&apos;affichage.</translation>
    </message>
    <message>
        <source>No errors found, nothing to save.</source>
        <translation>Pas d&apos;erreurs trouvées, rien à sauvegarder.</translation>
    </message>
    <message>
        <source>Failed to save the report.</source>
        <translation>Erreur lors de la sauvegarde du rapport.</translation>
    </message>
    <message>
        <source>Failed to read the report.</source>
        <translation>Erreur lors de la lecture du rapport</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Résumé</translation>
    </message>
    <message>
        <source>Message</source>
        <translation>Message</translation>
    </message>
    <message>
        <source>%p% (%1 of %2 files checked)</source>
        <translation>%p% (%1 fichiers sur %2 vérifiés)</translation>
    </message>
    <message>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <source>Print Report</source>
        <translation>Imprimer le rapport</translation>
    </message>
    <message>
        <source>No errors found, nothing to print.</source>
        <translation>Aucune erreur trouvée. Il n&apos;y a rien à imprimer</translation>
    </message>
    <message>
        <source>First included by</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ScratchPad</name>
    <message>
        <source>Scratchpad</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>filename</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Check</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <source>Preferences</source>
        <translation>Préférences</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Général</translation>
    </message>
    <message>
        <source>Number of threads: </source>
        <translation>Nombre de fils : </translation>
    </message>
    <message>
        <source>Show full path of files</source>
        <translation>Montrer le chemin complet des fichiers</translation>
    </message>
    <message>
        <source>Show &quot;No errors found&quot; message when no errors found</source>
        <translation>Afficher un message &quot;Pas d&apos;erreur trouvée&quot; lorsque aucune erreur est trouvée</translation>
    </message>
    <message>
        <source>Applications</source>
        <translation>Applications</translation>
    </message>
    <message>
        <source>Reports</source>
        <translation>Rapports</translation>
    </message>
    <message>
        <source>Save all errors when creating report</source>
        <translation>Sauvegarder toutes les erreurs lorsqu&apos;un rapport est créé</translation>
    </message>
    <message>
        <source>Save full path to files in reports</source>
        <translation>Sauvegarder le chemin complet des fichiers dans les rapports</translation>
    </message>
    <message>
        <source>Include paths:</source>
        <translation>Inclure les chemins</translation>
    </message>
    <message>
        <source>Add...</source>
        <translation>Ajouter...</translation>
    </message>
    <message>
        <source>Ideal count:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Force checking all #ifdef configurations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enable inline suppressions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Language</source>
        <translation>Langue</translation>
    </message>
    <message>
        <source>Paths</source>
        <translation>Chemins</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Editer</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Supprimer</translation>
    </message>
    <message>
        <source>Edit...</source>
        <translation>Editer...</translation>
    </message>
    <message>
        <source>Set as default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Display error Id in column &quot;Id&quot;</source>
        <translation>Afficher l&apos;identifiant d&apos;erreur Id dans la colonne &quot;Id&quot;</translation>
    </message>
    <message>
        <source>Check for inconclusive errors also</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show internal warnings in log</source>
        <translation>Montrer les avertissements internes dans le journal</translation>
    </message>
    <message>
        <source>Show statistics on check completion</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <source>Add a new application</source>
        <translation>Ajouter une nouvelle application</translation>
    </message>
    <message>
        <source>Modify an application</source>
        <translation>Modifier une application</translation>
    </message>
    <message>
        <source>N/A</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select include directory</source>
        <translation type="unfinished">Selectionner un répertoire à inclure</translation>
    </message>
    <message>
        <source>[Default]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> [Default]</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>StatsDialog</name>
    <message>
        <source>Statistics</source>
        <translation>Statistiques</translation>
    </message>
    <message>
        <source>Project</source>
        <translation>Projet</translation>
    </message>
    <message>
        <source>Project:</source>
        <translation>Projet :</translation>
    </message>
    <message>
        <source>Paths:</source>
        <translation>Chemins :</translation>
    </message>
    <message>
        <source>Include paths:</source>
        <translation>Inclure les chemins :</translation>
    </message>
    <message>
        <source>Defines:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Previous Scan</source>
        <translation>Analyse précédente</translation>
    </message>
    <message>
        <source>Path Selected:</source>
        <translation>Chemin sélectionné :</translation>
    </message>
    <message>
        <source>Number of Files Scanned:</source>
        <translation>Nombre de fichiers analysés :</translation>
    </message>
    <message>
        <source>Scan Duration:</source>
        <translation>Durée de l&apos;analyse :</translation>
    </message>
    <message>
        <source>Errors:</source>
        <translation>Erreurs :</translation>
    </message>
    <message>
        <source>Warnings:</source>
        <translation>Avertissements</translation>
    </message>
    <message>
        <source>Stylistic warnings:</source>
        <translation>Avertissements de style</translation>
    </message>
    <message>
        <source>Portability warnings:</source>
        <translation>Avertissements de portabilité</translation>
    </message>
    <message>
        <source>Performance issues:</source>
        <translation>Problème de performance</translation>
    </message>
    <message>
        <source>Information messages:</source>
        <translation>Messages d&apos;information :</translation>
    </message>
    <message>
        <source>Copy to Clipboard</source>
        <translation>Copier vers le presse-papier</translation>
    </message>
    <message>
        <source>1 day</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%1 days</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>1 hour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%1 hours</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>1 minute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%1 minutes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>1 second</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%1 seconds</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>0.%1 seconds</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> and </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Project Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Paths</source>
        <translation type="unfinished">Chemins</translation>
    </message>
    <message>
        <source>Include paths</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Defines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Path selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Number of files scanned</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Scan duration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Errors</source>
        <translation type="unfinished">Erreurs</translation>
    </message>
    <message>
        <source>Warnings</source>
        <translation type="unfinished">Avertissements</translation>
    </message>
    <message>
        <source>Style warnings</source>
        <translation type="unfinished">Avertissement de style</translation>
    </message>
    <message>
        <source>Portability warnings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Performance warnings</source>
        <translation type="unfinished">Avertissements de performance</translation>
    </message>
    <message>
        <source>Information messages</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ThreadResult</name>
    <message>
        <source>%1 of %2 files checked</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>TranslationHandler</name>
    <message>
        <source>Failed to change the user interface language:

%1

The user interface language has been reset to English. Open the Preferences-dialog to select any of the available languages.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cppcheck</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>TxtReport</name>
    <message>
        <source>inconclusive</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
