@echo off
cleancss -o css/all.min.css css/all.css && ^
cleancss -o css/geshi.min.css css/geshi.css && ^
uglifyjs --no-copyright -o js/github.min.js js/github.js && ^
uglifyjs --no-copyright -o js/picnet.table.filter.min.pack.js js/picnet.table.filter.min.js
