<?php
	// Current date and time.
	$curdate = date('m/d/Y');
	$curtime = date('H:i:s');
	
	// Get values.
	$password = $_GET['code'];
	$lum = $_GET['l'];
	$temp = $_GET['t'];
	$hum = $_GET['h'];
	
	// Set password. Must be consistent with PASSCODE in Arduino code.
	$passcode = "";
	
	// Check if password is right.
	if(isset($password) && ($password == $passcode)){
		// If all three values are present, insert it into the MySQL database.
		if(isset($lum)&&isset($lum)&&isset($lum)){
			// Database credentials
			$servername = "";
			$username = "";
			$dbname = "";
			$password = "";

			// Create connection.
			$conn = mysqli_connect($servername, $username, $password, $dbname);
			if (!$conn) {
				die("Connection failed: " . mysqli_connect_error());
			}

			// Insert values into table.
			$sql = "INSERT INTO data (date, time, temp, hum, lum)
			VALUES ('$curdate', '$curtime', $temp, $hum, $lum)";

			if (mysqli_query($conn, $sql)) {
				echo "OK";
			} else {
				echo "Fail: " . $sql . "<br>" . mysqli_error($conn);
			}
			
			// Close connection.
			mysqli_close($conn);
		}
	}
?>