<?php
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

class Forum_ActiveTopics {
    /**
     * ...
     * @var string ...
     */
    private $_forumHome;

    /**
     * ...
     * @var array ...
     */
    private $_topics;

    /**
     * ...
     * @param string $forumHome ...
     */
    public function __construct($forumHome) {
        $this->_forumHome = $forumHome;
        $this->_topics = array();

        $this->update();
    }

    /**
     * ...
     * @param int $start ...
     * @param int $end ...
     * @return array ...
     */
    public function getTopics($start = 0, $end = 0) {
        if ($end == 0) {
            return array_slice($this->_topics, $start);
        }
        else {
            return array_slice($this->_topics, $start, $end);
        }
        //return $this->_topics;
    }

    /**
     * ...
     * @param int $days ...
     */
    public function update($days = 30) {
        $this->_topics = array();
        $html = file_get_contents($this->_forumHome . 'search.php?st=' . $days . '&search_id=active_topics');
        $html = strip_tags($html, '<a><dd>');
        $html = preg_replace(array('#\t#', '#&amp;sid=[a-z0-9]+#'), '', $html);
        if (preg_match_all('#<a href="\./(.*?)" class="topictitle">(.*?)</a>#im', $html, $matches)) {
            $lastPosts = array();
            if (preg_match_all('#<dd class="lastpost">\nby <a href="\./(.*?)">(.*?)</a><a href=".*?"></a> ([A-Za-z0-9,: ]+) \n</dd>#i', $html, $lastPostMatches)) {
                $lastPostUserLinks = $lastPostMatches[1];
                $lastPostUserNames = $lastPostMatches[2];
                $lastPostTimes = $lastPostMatches[3];
                for ($i = 0; $i < count($lastPostUserLinks); $i++) { //for all users...
                    $link = $this->_forumHome . $lastPostUserLinks[$i];
                    $name = $lastPostUserNames[$i];
                    $timestamp = strtotime($lastPostTimes[$i]);
                    if ($timestamp === false || $timestamp === -1) {
                        $timestamp = 0;
                    }

                    $lastPosts[] = new Forum_LastPost(new Forum_User($link, $name), $timestamp);
                }
            }
            $links = $matches[1];
            $titles = $matches[2];
            for ($i = 0; $i < count($links); $i++) { //for all topics...
                $link = $this->_forumHome . $links[$i];
                $title = $titles[$i];
                $lastPost = $lastPosts[$i];

                $this->_topics[] = new Forum_Topic($link, $title, $lastPost);
            }
        }
    }
}

class Forum_Topic {
    /**
     * ...
     * @var string ...
     */
    private $_link;

    /**
     * ...
     * @var string ...
     */
    private $_title;

    /**
     * ...
     * @var Forum_LastPost ...
     */
    private $_lastPost;

    /**
     * ...
     * @param string $link ...
     * @param string $title ...
     * @param Forum_LastPost $lastPost ...
     */
    public function __construct($link, $title, $lastPost = null) {
        $this->_link = $link;
        $this->_title = $title;
        $this->_lastPost = $lastPost;
    }

    /**
     * ...
     * @return string ...
     */
    public function getLink() {
        return $this->_link;
    }

    /**
     * ...
     * @return string ...
     */
    public function getTitle() {
        return $this->_title;
    }

    /**
     * ...
     * @return Forum_LastPost ...
     */
    public function getLastPost() {
        return $this->_lastPost;
    }

    /**
     * ...
     * @return Forum_User ...
     * @deprecated
     */
    public function getLastPostUser() {
        if (!empty($this->_lastPost)) {
            return $this->_lastPost->getUser();
        }
        return null;
    }

    /**
     * ...
     * @return integer ...
     * @deprecated
     */
    public function getLastPostTimestamp() {
        if (!empty($this->_lastPost)) {
            return $this->_lastPost->getTimestamp();
        }
        return 0;
    }
}

class Forum_User {
    /**
     * ...
     * @var string ...
     */
    private $_link;

    /**
     * ...
     * @var string ...
     */
    private $_name;

    /**
     * ...
     * @param string $link ...
     * @param string $name ...
     */
    public function __construct($link, $name) {
        $this->_link = $link;
        $this->_name = $name;
    }

    /**
     * ...
     * @return string ...
     */
    public function getLink() {
        return $this->_link;
    }

    /**
     * ...
     * @return string ...
     */
    public function getName() {
        return $this->_name;
    }

    /**
     * ...
     */
    public function __toString() {
        return $this->_name;
    }
}

class Forum_LastPost {
    /**
     * ...
     * @var Forum_User ...
     */
    private $_user;

    /**
     * ...
     * @var integer ...
     */
    private $_timestamp;

    /**
     * ...
     * @param Forum_User $user ...
     * @param integer $timestamp ...
     */
    public function __construct($user, $timestamp) {
        $this->_user = $user;
        $this->_timestamp = $timestamp;
    }

    /**
     * ...
     * @return Forum_User ...
     */
    public function getUser() {
        return $this->_user;
    }

    /**
     * ...
     * @return integer ...
     */
    public function getTimestamp() {
        return $this->_timestamp;
    }

    /**
     * ...
     * @return string ...
     */
    public function getDate($format) {
        return date($format, $this->_timestamp);
    }
}
?>
