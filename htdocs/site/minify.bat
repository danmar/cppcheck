@echo off
rem install node.js and then run:
rem npm install -g clean-css
rem npm install -g uglify-js

pushd %~dp0

type css\all.css css\demo.css | cleancss -o css\pack.css
cmd /c uglifyjs js/github.js js/picnet.table.filter.min.js -o js/pack.js -c -m

popd
