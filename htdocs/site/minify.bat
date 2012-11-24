@echo off
pushd %~dp0

cmd /c cleancss -o css/all.min.css css/all.css
cmd /c cleancss -o css/demo.min.css css/demo.css
cmd /c uglifyjs js/github.js -o js/github.min.js -c -m
cmd /c uglifyjs js/picnet.table.filter.min.js -o js/picnet.table.filter.min.pack.js -c -m
