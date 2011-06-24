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
            $lastPostUsers = array();
            if (preg_match_all('#<dd class="lastpost">\nby <a href="\./(.*?)">(.*?)</a>#i', $html, $lastPosts)) {
                $lastPostUserLinks = $lastPosts[1];
                $lastPostUserNames = $lastPosts[2];
                for ($i = 0; $i < count($lastPostUserLinks); $i++) { //for all users...
                    $link = $this->_forumHome . $lastPostUserLinks[$i];
                    $name = $lastPostUserNames[$i];

                    $lastPostUsers[] = new Forum_User($link, $name);
                }
            }
            $links = $matches[1];
            $titles = $matches[2];
            for ($i = 0; $i < count($links); $i++) { //for all topics...
                $link = $this->_forumHome . $links[$i];
                $title = $titles[$i];
                $lastPostUser = $lastPostUsers[$i];

                $this->_topics[] = new Forum_Topic($link, $title, $lastPostUser);
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
     * @var Forum_User ...
     */
    private $_lastPostUser;

    /**
     * ...
     * @param string $link ...
     * @param string $title ...
     * @param Forum_User $lastPostUser ...
     */
    public function __construct($link, $title, $lastPostUser = null) {
        $this->_link = $link;
        $this->_title = $title;
        $this->_lastPostUser = $lastPostUser;
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
     * @return Forum_User ...
     */
    public function getLastPostUser() {
        return $this->_lastPostUser;
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
?>
