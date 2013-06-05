/**!
 * make.js, script to build the website for cppcheck
 * Released under the terms of MIT license.
 *
 * https://github.com/danmar/cppcheck
 *
 * Copyright (C) 2013 XhmikosR and Cppcheck team.
 */

(function () {
    'use strict';

    require('shelljs/make');
    var fs = require('fs'),
        cleanCSS = require('clean-css'),
        UglifyJS = require('uglify-js'),
        ROOT_DIR = __dirname + '/';     // absolute path to project's root

    //
    // make minify
    //
    target.minify = function () {
        cd(ROOT_DIR);
        echo();
        echo("### Minifying css files...");

        // pack.css

        var inCss = cat(['css/all.css',
                         'css/demo.css',
                         'css/normalize.css'
        ]);

        var packCss = cleanCSS.process(inCss, {
            removeEmpty: true,
            keepSpecialComments: 0
        });

        fs.writeFileSync('css/pack.css', packCss, 'utf8');

        echo();
        echo('### Finished' + ' ' + 'css/pack.css' + '.');

        echo();
        echo("### Minifying js files...");

        var inJs = cat(['js/github.js',
                        'js/picnet.table.filter.min.js']);

        var minifiedJs = UglifyJS.minify(inJs, {
            compress: true,
            fromString: true, // this is needed to pass JS source code instead of filenames
            mangle: true,
            warnings: false
        });

        fs.writeFileSync('js/pack.js', minifiedJs.code, 'utf8');

        echo();
        echo('### Finished' + ' ' + 'js/pack.js' + '.');
    };


    //
    // make all
    //
    target.all = function () {
        target.minify();
    };

    //
    // make help
    //
    target.help = function () {
        echo("Available targets:");
        echo("  minify  Creates the minified CSS and JS");
        echo("  help    shows this help message");
    };

}());
