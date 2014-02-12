<?php
  $isCodePosted = isset($_POST['code']) && !empty($_POST['code']);
  $isXmlOutput = isset($_POST['xml']) && $_POST['xml'] == '1';

  /**
   * ...
   * @param string $code Source code
   * @return string Output lines
   */
  function get_democlient_output($code) {
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

    return @file_get_contents('http://cppcheck.sourceforge.net/cgi-bin/democlient.cgi', false, $context);
  }

  function cut_string($string, $length = 1024) {
    if (strlen($string) > $length) {
      return substr($string, 0, $length);
    }
    return $string;
  }

  //--------------------------------------------------------------------------------
  // XML output...
  //--------------------------------------------------------------------------------
  if ($isXmlOutput) { //if XML output...
    header('Content-Type: text/xml');

    if (!$isCodePosted) { //if NO code posted...
      echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<results></results>\n";
      exit;
    }

    $output = get_democlient_output(cut_string($_POST['code']));

    if ($output === false) { //if NO demo client output...
      echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<results></results>\n";
      exit;
    }

    echo $output;
    exit;
  }
  //--------------------------------------------------------------------------------
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <title>Online Demo Report - Cppcheck</title>
  <link rel="stylesheet" type="text/css" href="//fonts.googleapis.com/css?family=Orbitron&amp;text=Cppcheck" />
  <link rel="stylesheet" type="text/css" href="/site/css/pack.css" />
  <link rel="shortcut icon" type="image/x-icon" href="/favicon.ico" />
  <script src="//ajax.googleapis.com/ajax/libs/jquery/1.11.0/jquery.min.js" type="text/javascript"></script>
  <script type="text/javascript">
    //<![CDATA[
    window.jQuery || document.write('<script type="text/javascript" src="/site/js/jquery-1.11.0.min.js"><\/script>')
    //]]>
  </script>
  <script src="/site/js/pack.js" type="text/javascript"></script>
  <script type="text/javascript">
    $(document).ready(function() {
      $("#resultsTable").tableFilter();
    });
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
      <li><em><a href="/demo/">Online Demo</a></em></li>
      <li><a href="http://sourceforge.net/projects/cppcheck/">Project page</a></li>
    </ul>
  </div> <!-- .wrap -->
</div> <!-- #tabs -->
<div id="content">
  <div class="wrap">
<h2>Online Demo Report</h2>
<?php
  /**
   * ...
   * @param string $output Output lines
   * @return array Parsed output
   */
  function parse_democlient_output($output) {
    try {
      $parsed = array();
      $xml = simplexml_load_string($output);
      foreach ($xml->errors->error as $error) { //for all errors...
        $severity = (string)$error->attributes()->severity;
        $msg = (string)$error->attributes()->msg;
        $line = (string)$error->location->attributes()->line;
        if (!empty($severity) && !empty($msg) && !empty($line)) { //if complete error...
          $parsed[] = array('severity' => $severity, 'msg' => $msg, 'line' => $line);
        }
      }
      return $parsed;
    }
    catch (Exception $ex) {
      return array();
    }
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

    $output = get_democlient_output($code);

    if (!$output === false) {
      $results = parse_democlient_output($output);

      if (!empty($results)) {
        echo "<table id=\"resultsTable\">\n";
        echo "<thead>\n";
        echo "  <tr><th class=\"center\" filter-type=\"ddl\">Line</th><th class=\"center\" filter-type=\"ddl\">Severity</th><th>Message</th></tr>\n";
        echo "</thead>\n";
        echo "<tbody>\n";
        foreach ($results as $result) { //for each result...
          echo "  <tr><td class=\"center\">" . htmlspecialchars($result['line']) . "</td><td class=\"center\">" . htmlspecialchars($result['severity']) . "</td><td>" . htmlspecialchars($result['msg']) . "</td></tr>\n";
        }
        echo "</tbody>\n";
        echo "</table>\n";
      }
      else {
        echo "<p>No errors found.</p>\n";
      }
    }
    else {
      echo "<p>Problem with demo client. Please try again.</p>\n";
    }
  }
  else { //if NO code posted...
    echo "<p>Use the <a href=\"/demo/\">online demo</a> page to create the report.</p>\n";
  }
?>
  </div> <!-- .wrap -->
</div> <!-- #content -->
<?php include_once("../../analyticstracking.php") ?>
</body>
</html>