<?
	// Some variables for the stats.
	$getfirstdate = 0;
	$tempadd = 0;
	$temphigh = 0;
	$templow = 99;
	$humadd = 0;
	$humhigh = 0;
	$humlow = 99;
	$rowcnt = 0;
	
	// Read from MySQL Database table.
	$result = mysqli_query($conn, "SELECT date, time, temp, hum, lum FROM data ORDER BY date DESC");
	while($row = mysqli_fetch_assoc($result)) {
		// Get last date, which would be the first date from the table that isn't today.
		if($row['date'] != $curdate = date('m/d/Y')){
			if($getfirstdate == 0){
				$lastdate = $row['date'];
				$getfirstdate = 1;
			}
		}
		
		// Use the data from the last date which was captured in the code just above.
		if($row['date'] == $lastdate){
			$tempadd += $row['temp'];	// Temperature total
			$humadd += $row['hum'];		// Humidity total
			$rowcnt++;		// Keep track of number of values for average calculation.
			
			// Get temperature/humidity highs and lows.
			if($row['temp'] > $temphigh){
				$temphigh = $row['temp'];
			}
			if($row['temp'] < $templow){
				$templow = $row['temp'];
			}
			if($row['hum'] > $humhigh){
				$humhigh = $row['hum'];
			}
			if($row['hum'] < $humlow){
				$humlow = $row['hum'];
			}
		}
		
	}
	echo "Last day with recordings: " . $lastdate;
	echo "<br/><br/>Average temperature on this date: " . number_format($tempadd/$rowcnt, 2, '.', '') . "&deg;C";
	echo "<br/>Highest temperature on this date: " . number_format($temphigh, 2, '.', '') . "&deg;C"; 
	echo "<br/>Lowest temperature on this date: " . number_format($templow, 2, '.', '') . "&deg;C"; 
	echo "<br/><br/>Average humidity on this date: " . number_format($humadd/$rowcnt, 2, '.', '') . "%";
	echo "<br/>Highest humidity on this date: " . number_format($humhigh, 2, '.', '') . "%"; 
	echo "<br/>Lowest humidity on this date: " . number_format($humlow, 2, '.', '') . "%"; 
?>