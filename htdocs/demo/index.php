<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <title>Online Demo - Cppcheck</title>
  <link rel="stylesheet" type="text/css" href="//fonts.googleapis.com/css?family=Orbitron&amp;text=Cppcheck" />
  <link rel="stylesheet" type="text/css" href="/site/css/pack.css" />
  <link rel="shortcut icon" type="image/x-icon" href="/favicon.ico" />
  <script src="//ajax.googleapis.com/ajax/libs/jquery/1.10.2/jquery.min.js" type="text/javascript"></script>
  <script type="text/javascript">
    //<![CDATA[
    window.jQuery || document.write('<script type="text/javascript" src="/site/js/jquery-1.10.2.min.js"><\/script>')
    //]]>
  </script>
  <script src="/site/js/pack.js" type="text/javascript"></script>
  <script type="text/javascript">
    function checkCodeLength() {
      if (document.f.code.value.length > 1024) {
        alert("code length exceeded");
        return false;
      }
      return true;
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
      <li><strong><a href="/demo/">Online Demo</a></strong></li>
      <li><a href="http://sourceforge.net/projects/cppcheck/">Project page</a></li>
    </ul>
  </div> <!-- .wrap -->
</div> <!-- #tabs -->
<div id="content">
  <div class="wrap">
<h2>Online Demo</h2>
<form action="/demo/report/" name="f" onsubmit="return checkCodeLength();" method="post" target="_blank">
<p><label for="code">Enter code:</label> <span class="maxChars">(max 1024 characters)</span><br />
<textarea id="code" name="code" rows="20" cols="80">
void f()
{
    char *p;
    *p = 0;
}
</textarea><br />
<input type="submit" value="Check" />
<label><input type="checkbox" name="xml" value="1" />XML output</label></p>
</form>
<h3>Examples</h3>
<p>This code can be copied and pasted in the edit box above.</p>
<h4>NULL pointers</h4>
<pre class="cpp geshicode"><span class="kw4">void</span> f1<span class="br0">(</span><span class="kw4">struct</span> fred_t <span class="sy2">*</span>p<span class="br0">)</span>
<span class="br0">{</span>
    <span class="co1">// dereference p and then check if it's NULL</span>
    <span class="kw4">int</span> x <span class="sy1">=</span> p<span class="sy2">-</span><span class="sy1">&gt;</span>x<span class="sy4">;</span>
    <span class="kw1">if</span> <span class="br0">(</span>p<span class="br0">)</span>
        do_something<span class="br0">(</span>x<span class="br0">)</span><span class="sy4">;</span>
<span class="br0">}</span>

<span class="kw4">void</span> f2<span class="br0">(</span><span class="br0">)</span>
<span class="br0">{</span>
    <span class="kw4">const</span> <span class="kw4">char</span> <span class="sy2">*</span>p <span class="sy1">=</span> <span class="kw2">NULL</span><span class="sy4">;</span>
    <span class="kw1">for</span> <span class="br0">(</span><span class="kw4">int</span> i <span class="sy1">=</span> <span class="nu0">0</span><span class="sy4">;</span> str<span class="br0">[</span>i<span class="br0">]</span> <span class="sy3">!</span><span class="sy1">=</span> <span class="st0">'<span class="es5">\0</span>'</span><span class="sy4">;</span> i<span class="sy2">++</span><span class="br0">)</span>
    <span class="br0">{</span>
        <span class="kw1">if</span> <span class="br0">(</span>str<span class="br0">[</span>i<span class="br0">]</span> <span class="sy1">==</span> <span class="st0">' '</span><span class="br0">)</span>
        <span class="br0">{</span>
            p <span class="sy1">=</span> str <span class="sy2">+</span> i<span class="sy4">;</span>
            <span class="kw1">break</span><span class="sy4">;</span>
        <span class="br0">}</span>
    <span class="br0">}</span>

    <span class="co1">// p is NULL if str doesn't have a space. If str always has a</span>
    <span class="co1">// a space then the condition (str[i] != '\0') would be redundant</span>
    <span class="kw1">return</span> p<span class="br0">[</span><span class="nu0">1</span><span class="br0">]</span><span class="sy4">;</span>
<span class="br0">}</span>

<span class="kw4">void</span> f3<span class="br0">(</span><span class="kw4">int</span> a<span class="br0">)</span>
<span class="br0">{</span>
    <span class="kw4">struct</span> fred_t <span class="sy2">*</span>p <span class="sy1">=</span> <span class="kw2">NULL</span><span class="sy4">;</span>
    <span class="kw1">if</span> <span class="br0">(</span>a <span class="sy1">==</span> <span class="nu0">1</span><span class="br0">)</span>
        p <span class="sy1">=</span> fred1<span class="sy4">;</span>

    <span class="co1">// if a is not 1 then p is NULL</span>
    p<span class="sy2">-</span><span class="sy1">&gt;</span>x <span class="sy1">=</span> <span class="nu0">0</span><span class="sy4">;</span>
<span class="br0">}</span></pre>
  </div> <!-- .wrap -->
</div> <!-- #content -->
<?php include_once("../analyticstracking.php") ?>
</body>
</html>