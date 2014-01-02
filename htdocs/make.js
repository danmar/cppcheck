/**!
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2013 XhmikosR and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

(function () {
    "use strict";

    require("shelljs/make");
    var fs = require("fs"),
        CleanCSS = require("clean-css"),
        UglifyJS = require("uglify-js"),
        rootDir = __dirname + "/";      // absolute path to project's root

    //
    // make minify
    //
    target.minify = function () {
        cd(rootDir);
        echo();
        echo("### Minifying css files...");

        // pack.css
        var inCss = cat(["site/css/normalize.css",
                         "site/css/all.css",
                         "site/css/demo.css"
        ]);

        var minifier = new CleanCSS({
                keepSpecialComments: 0,
                selectorsMergeMode: "ie8"
            });

        fs.writeFileSync("site/css/pack.css", minifier.minify(inCss), "utf8");

        echo();
        echo("### Finished site/css/pack.css.");

        echo();
        echo("### Minifying js files...");

        var inJs = cat(["site/js/github.js",
                        "site/js/picnet.table.filter.min.js"]);

        var minifiedJs = UglifyJS.minify(inJs, {
            compress: true,
            fromString: true, // this is needed to pass JS source code instead of filenames
            mangle: true,
            warnings: false
        });

        fs.writeFileSync("site/js/pack.js", minifiedJs.code, "utf8");

        echo();
        echo("### Finished site/js/pack.js.");
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
