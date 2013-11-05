/*! Inspired by: http://aboutcode.net/2010/11/11/list-github-projects-using-javascript.html */

/* jshint quotmark:true, jquery:true */

'use strict';

// htmlEntities taken from http://css-tricks.com/snippets/javascript/htmlentities-for-javascript/
function htmlEntities(str) {
    return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

jQuery.fn.listCommits = function(username, repository, branch) {
    this.html('<span>Querying GitHub for recent commits&hellip;</span>');

    var target = this;
    $.getJSON('https://api.github.com/repos/' + username + '/' + repository + '/commits?sha=' + branch + '&callback=?', function(response) {
        var commits = response.data;
        var list = $('<ul class="rssfeeditems"/>');

        target.empty().append(list);

        $(commits).each(function(i) {
            var githubUrl = 'https://github.com/' + username + '/' + repository + '/commit/' + this.sha;
            var shortMessage = htmlEntities(cutLines(this.commit.message));

            if (this.author !== null) {
                list.append('<li><a href="' + githubUrl + '">' + shortMessage + '</a> <em>by <strong><a class="author" href="' + 'https://github.com/' + this.author.login + '">' + this.author.login + '</a></strong></em></li>');
            } else {
                list.append('<li><a href="' + githubUrl + '">' + shortMessage + '</a> <em>by <strong>' + this.commit.author.name + '</strong></em></li>');
            }

            if (i === 9) {
                return false;
            }
        });
    });

    function cutLines(message) {
        var lineFeed = message.indexOf('\n');

        if (lineFeed > -1) {
            return message.slice(0, lineFeed);
        }
        return message;
    }
};
