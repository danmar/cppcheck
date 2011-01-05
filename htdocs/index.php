<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <title>Cppcheck - A tool for static C/C++ code analysis</title>
  <style type="text/css">
body {
  margin: 0;
  padding: 0;
  font-family: Calibri,Verdana,sans-serif;
  background: #eee;
}

/* Default link style */
a:link { color:#036; text-decoration:underline; }
a:visited { color:#036; text-decoration:underline; }
a:focus { color:#369; text-decoration:none; }
a:hover { color:#369; text-decoration:none; }
a:active { color:#369; text-decoration:none; }

/* Header */
#header {
  color: #69c;
  background: #036;
}
#header h1 {
  margin: 0;
  padding: 0;
}
#header p {
  margin: 0;
  padding: 0;
  font-size: larger;
}

/* Tabs */
#tabs {
  color: #eee;
  background: #369;
  border-top: 1px solid black;
  border-bottom: 1px solid black;
}
#tabs ul {
  margin: 0;
  padding: 0;
  list-style-type: none;
  font-size: larger;
}
#tabs ul li {
  display: inline;
  margin: 0;
  padding: 0;
  padding-right: .5em;
}
#tabs a:link { color:#eee; text-decoration:none; }
#tabs a:visited { color:#eee; text-decoration:none; }
#tabs a:focus { color:#fff; text-decoration:underline; }
#tabs a:hover { color:#fff; text-decoration:underline; }
#tabs a:active { color:#fff; text-decoration:underline; }

/* Content */
#content h2 {
  margin-top: 0;
}

/* Wrap */
.wrap {
  width: 50em;
  margin: 0 auto;
  padding: .5em;
}
  </style>
</head>
<body>
<div id="header">
  <div class="wrap">
    <h1>Cppcheck</h1>
    <p>A tool for static C/C++ code analysis</p>
  </div> <!-- .wrap -->
</div> <!-- #header -->
<div id="tabs">
  <div class="wrap">
    <ul>
      <li><strong><a href="/">Home</a></strong></li>
      <li><a href="http://sourceforge.net/apps/mediawiki/cppcheck/">Wiki</a></li>
      <li><a href="http://sourceforge.net/apps/phpbb/cppcheck/">Forum</a></li>
      <li><a href="http://sourceforge.net/apps/trac/cppcheck/">Trac</a></li>
      <li><a href="http://sourceforge.net/projects/cppcheck/">Project page</a></li>
    </ul>
  </div> <!-- .wrap -->
</div> <!-- #tabs -->
<div id="content">
  <div class="wrap">
<p><strong>Cppcheck</strong> is an analysis tool for C/C++ code. Unlike C/C++
compilers and many other analysis tools, it don't detect syntax errors. Cppcheck
only detects the types of bugs that the compilers normally fail to detect. The
goal is no false positives.</p>

<h2>Download</h2>
<p>You can download the standalone tool from our
<a href="http://sourceforge.net/projects/cppcheck/">project page</a> or try it 
as plugin for your favorite IDE:</p>
<ul>
  <li><strong>Code::Blocks</strong> - <em>integrated</em></li>
  <li><strong>CodeLite</strong> - <em>integrated</em></li>
  <li><strong>Eclipse</strong> - <a href="http://cppcheclipse.googlecode.com/">Cppcheclipse</a></li>
  <li><strong>Hudson</strong> - <a href="http://wiki.hudson-ci.org/display/HUDSON/Cppcheck+Plugin">Cppcheck Plugin</a></li>
</ul>
<p>No plugin exists for <strong>Visual Studio</strong>, but it's possible to add
Cppcheck as an external tool.</p>

<h2>Features</h2>
<ul>
  <li>Out of bounds checking</li>
  <li>Check the code for each class</li>
  <li>Checking exception safety</li>
  <li>Memory leaks checking</li>
  <li>Warn if obsolete functions are used</li>
  <li>Check for invalid usage of <acronym title="Standard Template Library">STL</acronym></li>
  <li>Check for uninitialized variables and unused functions</li>
</ul>

<h2>Support</h2>
<ul>
  <li>Use <a href="http://sourceforge.net/apps/trac/cppcheck/">Trac</a> to report
  bugs and feature requests</li>
  <li>Ask questions in the <a href="http://sourceforge.net/apps/phpbb/cppcheck/">forum</a>
  or at the IRC channel <a href="irc://irc.freenode.net/">#cppcheck</a></li>
  <li>For more details look at the <a href="http://sourceforge.net/apps/mediawiki/cppcheck/">wiki</a></li>
</ul>
  </div> <!-- .wrap -->
</div> <!-- #content -->
</body>
</html>