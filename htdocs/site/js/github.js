// Inspired by:
// http://aboutcode.net/2010/11/11/list-github-projects-using-javascript.html
jQuery.fn.listCommits = function(username, repository, branch) {
  this.html('<span>Querying GitHub for recent commits&hellip;</span>');

  var target = this;
  $.getJSON('http://github.com/api/v2/json/commits/list/' + username + '/' + repository + '/' + branch + '?callback=?', function(data) {
    var repos = data.commits;

    var list = $('<ul/>');
    target.empty().append(list);
    $(repos).each(function(i) {
      list.append('<li><a href="' + this.url + '">' + this.message + '</a> by <strong>' + this.author.login + '</strong></li>');

      if (i == 9) return false;
    });
  });
};