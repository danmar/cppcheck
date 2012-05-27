<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <title>Online Demo Report - Cppcheck</title>
  <link rel="stylesheet" type="text/css" href="http://fonts.googleapis.com/css?family=Orbitron&amp;text=Cppcheck" />
  <link rel="stylesheet" type="text/css" href="/site/css/all.css" />
  <link rel="stylesheet" type="text/css" href="/site/css/geshi.css" />
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
      <li><a href="/">Home</a></li>
      <li><a href="http://sourceforge.net/apps/mediawiki/cppcheck/">Wiki</a></li>
      <li><a href="http://sourceforge.net/apps/phpbb/cppcheck/">Forum</a></li>
      <li><a href="http://sourceforge.net/apps/trac/cppcheck/">Trac</a></li>
      <li><a href="/devinfo/" title="Developer Information">Developer Info</a></li>
      <li><em><a href="/demo/">Online Demo</a></em></li>
      <li><a href="http://sourceforge.net/projects/cppcheck/">Project page</a></li>
    </ul>
  </div> <!-- .wrap -->
</div> <!-- #tabs -->
<div id="content">
  <div class="wrap">
<h2>Online Demo Report</h2>
<?php
  $isCodePosted = isset($_POST['code']) && !empty($_POST['code']);
  
  /**
   * ...
   * @param string $code Source code
   * @return string Output lines
   */
  function get_democlient_outout($code) {
    $postdata = http_build_query(
      array(
        'code' => $code
      )
    );
    
    $opts = array('http' =>
      array(
        'method'  => 'POST',
        'header'  => 'Content-type: application/x-www-form-urlencoded',
        'content' => $postdata
      )
    );
    
    $context = stream_context_create($opts);
    
    return file('http://cppcheck.sourceforge.net/cgi-bin/democlient.cgi', false, $context);
  }
  
  function cut_string($string, $length = 1024) {
    if (strlen($string) > $length) {
      return substr($string, 0, $length);
    }
    return $string;
  }
  
  if ($isCodePosted) { //if code posted...
    include_once '../../site/geshi/geshi.php';
    
    $code = cut_string($_POST['code']);
    
    $geshi = new GeSHi($code, 'cpp');
    $geshi->enable_classes();
    $geshi->set_header_type(GESHI_HEADER_PRE_TABLE);
    $geshi->enable_line_numbers(GESHI_NORMAL_LINE_NUMBERS);
    $geshi->set_overall_class('geshicode');
    
    echo "<h3>Input</h3>\n";
    echo $geshi->parse_code();
    
    echo "<h3>Output</h3>\n";
    
    $lines = get_democlient_outout($code);
    foreach ($lines as $line) { //for each output line...
      $line = trim($line);
      if (empty($line)) { //if empty line...
        continue;
      }
      echo "<p>" . htmlspecialchars($line) . "</p>\n";
    }
  }
  else {
    echo "<p>Use the <a href=\"/demo/\">online demo</a> page to create the report.</p>\n";
  }
?>
  </div> <!-- .wrap -->
</div> <!-- #content -->
</body>
</html>