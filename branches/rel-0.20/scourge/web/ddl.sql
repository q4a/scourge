CREATE TABLE `score` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `date_created` timestamp NOT NULL default '0000-00-00 00:00:00' on update CURRENT_TIMESTAMP,
  `username` varchar(64) NOT NULL default '',
  `score` int(11) NOT NULL default '0',
  `description` text NOT NULL,
  `soft_ver` varchar(40) default NULL,
  PRIMARY KEY  (`id`),
  KEY `score` (`score`)
) ENGINE=MyISAM AUTO_INCREMENT=15 DEFAULT CHARSET=latin1 AUTO_INCREMENT=15 ;


