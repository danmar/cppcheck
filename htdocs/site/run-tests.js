#!/usr/bin/env node

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

require('shelljs/global');

cd(__dirname);

//
// JSHint
//
JSHINT_BIN = './node_modules/jshint/bin/jshint';

if (!test('-f', JSHINT_BIN)) {
    echo('JSHint not found. Run `npm install` in the root dir first.');
    exit(1);
}

if (exec('node' +' ' + JSHINT_BIN +' ' + 'make.js run-tests.js').code !== 0) {
    echo('*** JSHint failed! (return code != 0)');
    echo();
} else {
    echo('JSHint completed successfully');
    echo();
}


//
// csslint
//
CSSLINT_BIN = './node_modules/csslint/cli.js';

if (!test('-f', CSSLINT_BIN)) {
    echo('csslint not found. Run `npm install` in the root dir first.');
    exit(1);
}

if (exec('node' +' ' + CSSLINT_BIN +' ' + 'css/all.css').code !== 0) {
    echo('*** csslint failed! (return code != 0)');
    echo();
}
