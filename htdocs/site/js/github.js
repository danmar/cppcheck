/*! Inspired by: http://aboutcode.net/2010/11/11/list-github-projects-using-javascript.html */

jQuery.fn.listCommits = function(username, repository, branch) {
  this.html('<span>Querying GitHub for recent commits&hellip;</span>');

  var target = this;
  $.getJSON('https://api.github.com/repos/' + username + '/' + repository + '/commits?sha=' + branch + '&callback=?', function(response) {
    var commits = response.data;

    var list = $('<ul class="rssfeeditems"/>');
    target.empty().append(list);

    $(commits).each(function(i) {
      var githubUrl = 'https://github.com/' + username + '/' + repository + '/commit/' + this.sha,
          shortMessage = cutLines(this.commit.message),
          commitAuthor;

      if (this.author !== null) {
        commitAuthor = this.author.login;
      }

      if (commitAuthor) {
        list.append('<li><a href="' + githubUrl + '">' + shortMessage + '</a> <em>by <strong>' + commitAuthor + '</strong></em></li>');
      } else {
        list.append('<li><a href="' + githubUrl + '">' + shortMessage + '</a></li>');
      }

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
