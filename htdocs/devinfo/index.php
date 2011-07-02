<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <title>Developer Information - Cppcheck</title>
  <link rel="stylesheet" type="text/css" href="http://fonts.googleapis.com/css?family=Orbitron&amp;text=Cppcheck" />
  <link rel="stylesheet" type="text/css" href="/site/css/all.css" />
  <link rel="shortcut icon" type="image/x-icon" href="/favicon.ico" />
  <link rel="alternate" type="application/atom+xml" title="Recent Commits to cppcheck:master"
        href="https://github.com/danmar/cppcheck/commits/master.atom" />
  <link rel="alternate" type="application/atom+xml" title="Trac Timeline"
        href="http://sourceforge.net/apps/trac/cppcheck/timeline?changeset=on&amp;ticket=on&amp;milestone=on&amp;wiki=on&amp;max=50&amp;daysback=90&amp;format=rss" />
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.4.4/jquery.min.js" type="text/javascript"></script>
  <script src="/site/js/github.js" type="text/javascript"></script>
  <script type="text/javascript">
    $(function() {
      $("#github-commits").listCommits("danmar", "cppcheck", "master");
    });
  </script>
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
      <li><a href="/">Home</a></li>
      <li><a href="http://sourceforge.net/apps/mediawiki/cppcheck/">Wiki</a></li>
      <li><a href="http://sourceforge.net/apps/phpbb/cppcheck/">Forum</a></li>
      <li><a href="http://sourceforge.net/apps/trac/cppcheck/">Trac</a></li>
      <li><strong><a href="/devinfo/" title="Developer Information">Developer Info</a></strong></li>
      <li><a href="http://sourceforge.net/projects/cppcheck/">Project page</a></li>
    </ul>
  </div> <!-- .wrap -->
</div> <!-- #tabs -->
<div id="content">
  <div class="wrap">
<h2>Source Code</h2>
<p>Latest version can be found in the <a href="https://github.com/danmar/cppcheck/">
cppcheck git repository</a>. To download it, run the following command:</p>
<pre>git clone git://github.com/danmar/cppcheck.git</pre>
<p>You can also <a href="https://github.com/danmar/cppcheck/downloads">download
the latest sources in a zip or tgz archive</a> from the github website.</p>
<h3>Recent Commits</h3>
<div id="github-commits"><a href="https://github.com/danmar/cppcheck/commits/master">View recent commits&hellip;</a></div>
<p><a href="https://github.com/danmar/cppcheck/commits/master">View all commits&hellip;</a></p>
<h2>Trac Timeline</h2>
<?php
  require '../site/simplepie/simplepie.inc';

  $feed = new SimplePie();
  $feed->set_feed_url('http://sourceforge.net/apps/trac/cppcheck/timeline?changeset=on&ticket=on&milestone=on&wiki=on&max=10&daysback=90&format=rss');
  $feed->set_cache_location('./site/simplepie/cache');
  $feed->init();
  print("<ul class=\"rssfeeditems\">\n");
  foreach ($feed->get_items() as $item) { //for the last timeline items...
    $author = $item->get_author();
    print("  <li><a href=\"".$item->get_link()."\">".$item->get_title()."</a> <em>by <strong>".$author->get_name()."</strong> on ".$item->get_date('Y-m-d')."</em></li>\n");
  }
  print("</ul>\n");
?>
<p><a href="http://sourceforge.net/apps/trac/cppcheck/timeline">View complete Trac timeline&hellip;</a></p>
<h2>Active Forum Topics</h2>
<?php
  require '../site/activetopics.php';

  $activetopics = new Forum_ActiveTopics('http://sourceforge.net/apps/phpbb/cppcheck/');
  print("<ul class=\"rssfeeditems\">\n");
  foreach ($activetopics->getTopics(0, 10) as $topic) { //for all active topics...
    $lastPostLine = '';
    if ($topic->getLastPost() != null) {
        $lastPost = $topic->getLastPost();
        $lastPostLine = sprintf('last post by <strong>%1$s</strong> at %2$s', $lastPost->getUser(), $lastPost->getDate('Y-m-d H:i'));
    }
    print("  <li><a href=\"".$topic->getLink()."\">".$topic->getTitle()."</a> <em>".$lastPostLine."</em></li>\n");
  }
  print("</ul>\n");
?>
<p><a href="http://sourceforge.net/apps/phpbb/cppcheck/search.php?st=0&amp;search_id=active_topics">View all active topics&hellip;</a></p>
<h2>Doxygen</h2>
<ul>
  <li><a href="doxyoutput/">Output</a></li>
  <li><a href="doxygen-errors.txt">Errors</a></li>
</ul>
<h2>Other</h2>
<ul>
  <li><a href="coverage_report/">Coverage report</a></li>
  <li><a href="cpd.txt">CPD report (duplicate code)</a></li>
</ul>
  </div> <!-- .wrap -->
</div> <!-- #content -->
</body>
</html>