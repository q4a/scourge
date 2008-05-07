<?php

	function connectToDb() {
		#echo "Connecting to the db...";
	        $conn = mysql_pconnect( "mysql4-s", "s98006rw", "Sc0urgerw" );
	        if( !$conn ) {
	                echo "Could not connect to db.";
			header( "HTTP/1.1 501 Can't connect to db." );
			return 0;
	        } else {
	                #echo "Connected to db.";
	                $db = mysql_select_db( "s98006_highscore", $conn );
	                if( !$db ) {
	                        echo "Could not select db.";
				header( "HTTP/1.1 502 Can't select db." );
				return 0;
	                } else {
				return 1;
			}
		}
	}

        function displayRow( $row, $line ) {
		echo "<a name=\"id", $row['id'], "\"></a>";
		echo "<img src=\"tomb.gif\">";
		if( $line == 1 ) echo "<img src=\"3stars.gif\"><span class='usernameBlock1'>";
		else if( $line == 2 ) echo "<img src=\"2stars.gif\"><span class='usernameBlock2'>";
		else if( $line == 3 ) echo "<img src=\"1star.gif\"><span class='usernameBlock3'>";
		else echo "&nbsp;<span class='usernameBlock'>";
		echo $row['username'], "</span><br>";
		echo "<span class='scoreBlock'>", $row['score'], "</span> points.<br>";
		echo $row['description'], "<br>";
		echo "<span class='dateBlock'>", $row['date_created'], " - ", $row['soft_ver'], "</span><br>";
        }

	function displayList() {
		echo "<b><span class=usernameBlock>Ranks of the Faithful:</span> S.c.o.u.r.g.e. High Scores</b><br>";
		echo "(This page is still under construction. Feedback and feature wishes are appreciated.)<p>";
		$result = mysql_query( "select * from score order by score desc" );
		if( !$result ) {
			header( "HTTP/1.1 503 Can't execute select." );
			echo "Unable to select data.";
		} else {
			#$numRows = mysql_num_rows( $result );
	                #echo "Data selected: $numRows rows.";
			$line = 1;
	                while( $row = mysql_fetch_assoc( $result ) ) {
	                	displayRow( $row, $line );
				$line = $line + 1;
				echo "<br>";
	                }
			mysql_free_result( $result );
		}
	}

	function addScore( $username, $score, $desc ) {
		$ver = $_SERVER['HTTP_USER_AGENT'];
		if( mysql_query( "insert into score( date_created, username, score, description, soft_ver ) values( NOW(), '$username', $score, '$desc', '$ver' )" ) ) {
			# this version of mysql doesn't support limit and subqueries.
			# if( mysql_query( "delete from score where id not in ( select id from score order by score desc limit 10 )" ) ) {
				return "add-success";
			# } else {
				# echo "Unable to limit table.";
				# return "add-failed";
			# }
		} else {
			header( "HTTP/1.1 504 Unable to insert into db." );
			echo "Unable to insert into db.";
			return "add-failed";
		}
	}

	function echoRSS() {

		$rssDateFormat = 'D, d M Y H:i:s T';

		# when was this table last updated?
		#$info = mysql_fetch_array( mysql_query( "show table status from 98006_highscore like 'score'" ) );
		$lastUpdate = '';
		$info = mysql_query( "select max( unix_timestamp( date_created ) ) as d from score" );
		if( $info ) {
			if( $inforow = mysql_fetch_assoc( $info ) ) {
				$lastUpdate = date( $rssDateFormat, $inforow['d'] );
			}
			mysql_free_result( $info );
		}

		echo "<?xml version=\"1.0\"?><rss version=\"2.0\"><channel><title>Scourge! High Scores</title>";
		echo "<link>http://scourge.sourceforge.net/highscore/score.php</link>";
		echo "<description>Info posted by the game Scourge! ordered by score</description>";
		echo "<language>en-us</language>";
		echo "<pubDate>", $lastUpdate, "</pubDate>";
		echo "<lastBuildDate>", $lastUpdate, "</lastBuildDate>";
		echo "<image>";
		echo "<url>http://scourge.sourceforge.net/highscore/tomb.gif</url>";
		echo "<title>Scourge! High Scores</title>";
		echo "<link>http://scourge.sourceforge.net/highscore/score.php</link>";
		echo "</image>";		

		$result = mysql_query( "select id, username, description, score, unix_timestamp(date_created) as d from score order by score desc" );
                if( !$result ) {
			echo "<item><title>No scores found.</title>";
			echo "<link>http://scourge.sourceforge.net/highscore/score.php</link>";
			echo "<description>No Scourge! high scores were found in the db.</description>";
			echo "<pubDate></pubDate>";
			echo "<guid>http://scourge.sourceforge.net/highscore/score.php</guid>";
			echo "</item>";
                } else {
                        #$numRows = mysql_num_rows( $result );
                        #echo "Data selected: $numRows rows.";
                        $line = 1;
                        while( $row = mysql_fetch_assoc( $result ) ) {
				echo "<item><title>", $row['username'], ": ", $row['score'], "</title>";
        	                echo "<link>http://scourge.sourceforge.net/highscore/score.php#id", $row['id'], "</link>";
	                        echo "<description>", $row['description'], "</description>";
	                        echo "<pubDate>", date( $rssDateFormat, $row['d'] ), "</pubDate>";
	                        echo "<guid>http://scourge.sourceforge.net/highscore/score.php#id", $row['id'], "</guid>";
				echo "</item>";

                                $line = $line + 1;
                        }
                        mysql_free_result( $result );
                }
		echo "</channel></rss>";	
	}

	#echo "------------------------------------\n";
	#echo "POST size=", count($_POST), "\n";
	#foreach ($_POST as $i => $value) {
		#echo $_POST[$i], "\n";
	#}
	#echo "GET size=", count($_GET), "\n";
        #foreach ($_GET as $i => $value) {
                #echo $_GET[$i], "\n";
        #}
	#echo "------------------------------------\n";
	if( connectToDb() == 1 ) {
		if( $_POST['mode'] == "test" ) {
			echo "Test successful. User=", $_POST['user'];
		} else if( $_GET['mode'] == "add" ) {
			echo addScore( $_GET['user'], $_GET['score'], $_GET['desc'] );
		} elseif( $_POST['mode'] == "add" ) {
			echo addScore( $_POST['user'], $_POST['score'], $_POST['desc'] );
		} elseif( $_GET['mode'] == "rss" ) {
			echoRSS();
		} else {
?>
<html>
   <head>
       <title>Scourge High Scores</title>
       <link rel=StyleSheet href="scourge.css" type="text/css">
   </head>
   <body>
<?php
			# save the version of scourge sent:
			# echo $_SERVER['HTTP_USER_AGENT'];
			displayList();
?>
   </body>
</html>
<?php
		}
	}
?>

