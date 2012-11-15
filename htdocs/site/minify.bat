@echo off
pushd %~dp0

cmd /c cleancss -o css/all.min.css css/all.css
cmd /c cleancss -o css/demo.min.css css/demo.css
cmd /c uglifyjs -o js/github.min.js js/github.js
cmd /c uglifyjs -o js/picnet.table.filter.min.pack.js js/picnet.table.filter.min.js
