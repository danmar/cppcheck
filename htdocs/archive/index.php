<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <meta name="description" content="Cppcheck is an analysis tool for C/C++ code.
It detects the types of bugs that the compilers normally fail to detect. The
goal is no false positives." />
  <meta name="keywords" content="Cppcheck, open source, analysis tool, C/C++,
code, errors, bugs, compilers, bounds checking, memory leaks, obsolete functions,
uninitialized variables, unused functions" />
  <title>Cppcheck - Archive</title>
  <link rel="stylesheet" type="text/css" href="//fonts.googleapis.com/css?family=Orbitron&amp;text=Cppcheck" />
  <link rel="stylesheet" type="text/css" href="/site/css/pack.css" />
  <link rel="shortcut icon" type="image/x-icon" href="/favicon.ico" />
  <link rel="alternate" type="application/rss+xml" title="Project News"
        href="http://sourceforge.net/p/cppcheck/news/feed" />
  <script type="text/javascript">
    function addFile() {
        var name = prompt("Name of library/platform/etc", "");
        if (name != null)
            window.location = "http://cppcheck.sourceforge.net/cgi-bin/addfile.cgi?name=" + name;
    }

    function editFile(name,version) {
        window.location = "http://cppcheck.sourceforge.net/cgi-bin/edit.cgi?name=" + name + "&amp;version=" + version;
    }

    function renameFile(name1,version) {
        var name2 = prompt("Name", name1);
        if (name2 != null)
            window.location = "http://cppcheck.sourceforge.net/cgi-bin/renamefile.cgi?name1=" + name1 + "&amp;name2=" + name2;
    }

    function deleteFile(name,version) {
        window.location = "http://cppcheck.sourceforge.net/cgi-bin/deletefile.cgi?name=" + name + "&version=" + version;
    }
  </script>
</head>
<body>
<div id="header">
  <div class="wrap">
    <h1><a href="/">Cppcheck</a></h1>
    <p>A tool for static C/C++ code analysis</p>
  </div> <!-- .wrap -->
</div> <!-- #header -->
<div id="tabs">
  <div class="wrap">
    <ul>
      <li><a href="/">Home</a></li>
      <li><a href="http://sourceforge.net/apps/mediawiki/cppcheck/">Wiki</a></li>
      <li><a href="http://sourceforge.net/apps/phpbb/cppcheck/">Forum</a></li>
      <li><a href="http://sourceforge.net/apps/trac/cppcheck/">Issues</a></li>
      <li><a href="/devinfo/" title="Developer Information">Developer Info</a></li>
      <li><a href="/demo/">Online Demo</a></li>
      <li><strong><a href="/archive/">Archive</a></strong></li>
      <li><a href="http://sourceforge.net/projects/cppcheck/">Project page</a></li>
    </ul>
  </div> <!-- .wrap -->
</div> <!-- #tabs -->
<div id="content">
  <div class="wrap">
    <h1>Cppcheck - Archive</h1>
    <p>This archive is for useful Cppcheck rules and library configuration files.</p>

    <p> If you have a rule or library configuration that you want to share, use this
    archive. Feel free to add any library or rule here.</p>

    <?php virtual('../cgi-bin/report.cgi') ?>

    <p><a href="/cgi-bin/archive.zip">Download all</a></p>
  </div> <!-- .wrap -->
</div> <!-- #content -->
</body>
</html>
