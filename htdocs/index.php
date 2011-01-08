<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <meta name="description" content="Cppcheck is an analysis tool for C/C++ code.
It detects the types of bugs that the compilers normally fail to detect. The
goal is no false positives." />
  <meta name="keywords" content="Cppcheck, open source, analysis tool, C/C++,
code, errors, bugs, compilers, bounds checking, memory leaks, obsolete functions,
uninitialized variables, unused functions" />
  <title>Cppcheck - A tool for static C/C++ code analysis</title>
  <link rel="stylesheet" type="text/css" href="/site/css/all.css" />
  <link rel="shortcut icon" type="image/x-icon" href="/favicon.ico" />
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
      <li><a href="/devinfo/" title="Developer Information">Developer Info</a></li>
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