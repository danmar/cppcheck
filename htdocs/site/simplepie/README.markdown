SimplePie
=========

SimplePie is a very fast and easy-to-use class, written in PHP, that puts the
'simple' back into 'really simple syndication'.  Flexible enough to suit
beginners and veterans alike, SimplePie is focused on [speed, ease of use,
compatibility and standards compliance][what_is].

[what_is]: http://simplepie.org/wiki/faq/what_is_simplepie


Requirements
------------
* PHP 5.2.0 or newer
* libxml2 (certain 2.7.x releases are too buggy for words, and will crash)
* Either the iconv or mbstring extension
* cURL or fsockopen()
* PCRE support

If you're looking for PHP 4.x support, pull the "one-dot-two" branch, as that's
the last version to support PHP 4.x.


What comes in the package?
--------------------------
1. `library/` - SimplePie classes for use with the autoloader
2. `autoloader.php` - The SimplePie Autoloader if you want to use the separate
   file version.
3. `README.markdown` - This document.
4. `LICENSE.txt` - A copy of the BSD license.
5. `compatibility_test/` - The SimplePie compatibility test that checks your
   server for required settings.
6. `demo/` - A basic feed reader demo that shows off some of SimplePie's more
   noticeable features.
7. `idn/` - A third-party library that SimplePie can optionally use to
   understand Internationalized Domain Names (IDNs).
8. `build/` - Scripts related to generating pieces of SimplePie
9. `test/` - SimplePie's unit test suite.

### Where's `simplepie.inc`?
For SimplePie 1.3, we've split the classes into separate files to make it easier
to maintain and use.

If you'd like a single monolithic file, you can run `php build/compile.php` to
generate `SimplePie.compiled.php`, or grab a copy from
[dev.simplepie.org][dev_compiled] (this is kept up-to-date with the latest
code from Git).

[dev_compiled]: http://dev.simplepie.org/SimplePie.compiled.php


To start the demo
-----------------
1. Upload this package to your webserver.
2. Make sure that the cache folder inside of the demo folder is server-writable.
3. Navigate your browser to the demo folder.


Need support?
-------------
For further setup and install documentation, function references, etc., visit
[the wiki][wiki]. If you're using the latest version off GitHub, you can also
check out the [API documentation][].

If you can't find an answer to your question in the documentation, head on over
to one of our [support channels][]. For bug reports and feature requests, visit
the [issue tracker][].

[API documentation]: http://dev.simplepie.org/api/
[wiki]: http://simplepie.org/wiki/
[support channels]: http://simplepie.org/support/
[issue tracker]: http://github.com/simplepie/simplepie/issues


Project status
--------------
SimplePie is currently maintained by Ryan McCue.

As an open source project, SimplePie is maintained on a somewhat sporadic basis.
This means that feature requests may not be fulfilled straight away, as time has
to be prioritized.

If you'd like to contribute to SimplePie, the best way to get started is to fork
the project on GitHub and send pull requests for patches. When doing so, please
be aware of our [coding standards][].

[coding standards]: http://simplepie.org/wiki/misc/coding_standards


Authors and contributors
------------------------
### Current
* [Ryan McCue][] (Maintainer, support)

### Alumni
* [Ryan Parman][] (Creator, developer, evangelism, support)
* [Geoffrey Sneddon][] (Lead developer)
* [Michael Shipley][] (Submitter of patches, support)
* [Steve Minutillo][] (Submitter of patches)

[Ryan McCue]: http://ryanmccue.info
[Ryan Parman]: http://ryanparman.com
[Geoffrey Sneddon]: http://gsnedders.com
[Michael Shipley]: http://michaelpshipley.com
[Steve Minutillo]: http://minutillo.com/steve/


### Contributors
For a complete list of contributors:

1. Pull down the latest SimplePie code
2. In the `simplepie` directory, run `git shortlog -ns`


License
-------
[New BSD license](http://www.opensource.org/licenses/BSD-3-Clause)
