<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <title>Online Demo - Cppcheck</title>
  <link rel="stylesheet" type="text/css" href="http://fonts.googleapis.com/css?family=Orbitron&amp;text=Cppcheck" />
  <link rel="stylesheet" type="text/css" href="/site/css/all.min.css" />
  <link rel="shortcut icon" type="image/x-icon" href="/favicon.ico" />
  <script type="text/javascript">
    function checkCodeLength() {
      if (document.f.code.value.length > 1024) {
        alert('code length exceeded');
        return false;
      }
      return true;
    }
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
<p><label for="code">Enter code:</label> <i class="maxChars">(max 1024 characters)</i><br />
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
<p>This code can be copy and pasted in the edit box above.</p>
<h4>NULL pointers</h4>
<pre class="code">void f1(struct fred_t *p)
{
    <em>// dereference p and then check if it's NULL</em>
    int x = p-&gt;x;
    if (p)
        do_something(x);
}

void f2()
{
    const char *p = NULL;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == ' ')
        {
            p = str + i;
            break;
        }
    }

    <em>// p is NULL if str doesn't have a space. If str always has a</em>
    <em>// a space then the condition (str[i] != '\0') would be redundant</em>
    return p[1];
}

void f3(int a)
{
    struct fred_t *p = NULL;
    if (a == 1)
        p = fred1;

    <em>// if a is not 1 then p is NULL</em>
    p-&gt;x = 0;
}</pre>
  </div> <!-- .wrap -->
</div> <!-- #content -->
</body>
</html>