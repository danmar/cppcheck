/*! Inspired by: http://aboutcode.net/2010/11/11/list-github-projects-using-javascript.html */

jQuery.fn.listCommits = function(username, repository, branch) {
  this.html('<span>Querying GitHub for recent commits&hellip;</span>');

  var target = this;
  $.getJSON('https://api.github.com/repos/' + username + '/' + repository + '/commits?sha=' + branch + '&callback=?', function(response) {
    var commits = response.data;

    var list = $('<ul class="rssfeeditems"/>');
    target.empty().append(list);
    $(commits).each(function(i) {
      var githubUrl = 'https://github.com/' + username + '/' + repository + '/commit/' + this.sha;
      var shortMessage = cutLines(this.commit.message);
      var author = this.author.login;
      if (author === '') {
        author = this.author.name;
      }

      list.append('<li><a href="' + githubUrl + '">' + shortMessage + '</a> <em>by <strong>' + author + '</strong></em></li>');

      if (i === 9) {
        return false;
      }
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