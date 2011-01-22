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
      var githubUrl = 'https://github.com' + this.url;
      var shortMessage = cutLines(this.message);
      var author = this.author.login;
      if (author == '') {
        author = this.author.name;
      }

      list.append('<li><a href="' + githubUrl + '">' + shortMessage + '</a> by <strong>' + author + '</strong></li>');

      if (i == 9) return false;
    });
  });

  function cutLines(message) {
    var lineFeed = message.indexOf("\n");
    if (lineFeed > -1) {
      return message.slice(0, lineFeed);
    }
    return message;
  }
};