/* jshint unused:false, jquery:true, browser:true, devel:true */

"use strict";

function addFile() {
    var name = prompt("Name of library/platform/etc", "");
    if (name !== null) {
        window.location = "http://cppcheck.sourceforge.net/cgi-bin/addfile.cgi?name=" + name;
    }
}

function editFile(name, version) {
    window.location = "http://cppcheck.sourceforge.net/cgi-bin/edit.cgi?name=" + name + "&version=" + version;
}

function renameFile(name1, version) {
    var name2 = prompt("Name", name1);
    if (name2 !== null) {
        window.location = "http://cppcheck.sourceforge.net/cgi-bin/renamefile.cgi?name1=" + name1 + "&name2=" + name2;
    }
}

function deleteFile(name, version) {
    window.location = "http://cppcheck.sourceforge.net/cgi-bin/deletefile.cgi?name=" + name + "&version=" + version;
}

function checkCodeLength() {
    if (document.f.code.value.length > 1024) {
        alert("Code length exceeded.");
        return false;
    }
    return true;
}

$(document).ready(function() {
    $("#resultsTable").tableFilter();
});

$(function() {
    $("#github-commits").listCommits("danmar", "cppcheck", "master");
});
