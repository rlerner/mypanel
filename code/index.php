<?php
/* (c)2023 Robert Lerner, All Rights Reserved
 *
 * Home Security Monitor -- Reciever Script
 * github.com/rlerner/mypanel/
 * See Also: robert-lerner.com
*/

// Add yours here
$pushoverToken = "??????????????????";
$userToken = "???????????????????";

$zoneMapping = [
	1 => "Garage Door"
	,2 => "Unknown Zone 2"
	,3 => "Back Door"
	,4 => "Unknown Zone 4"
	,5 => "Kitchen Windows"
	,6 => "Bedroom 1 Window"
	,7 => "Bedroom 2 Window"
	,8 => "Bedroom 3 Window"
	,9 => "Bedroom 4 Window"
	,10 => "Dining Room Windows"
	,11 => "Window by Garage"
	,12 => "Bedroom 5 Windows"
	,13 => "Office Windows"
	,14 => "Unknown Zone 14"
	,15 => "Unknown Zone 15"
];

$message = trim(strip_tags($_REQUEST["state"]));
if (substr($message,0,2)=="HB") {
	die(); // Heartbeat message, ignore for right now. The panel will still beep if connectivity is lost, but I'll code something with the heartbeat signature later.
} else {
	$zones = explode("ZONE",$message);
	$message = "The following zones are insecure:\r\n\r\n";
	
	foreach ($zones as $v) {
		if ($v!="") {
			$message .= $zoneMapping[$v] . "\r\n";
		}
	}
	notifyPushover($pushoverToken,$userToken,$message);
}

function notifyPushover($pushoverToken,$userToken,$message) {
	curl_setopt_array($ch = curl_init(), [
		CURLOPT_URL => "https://api.pushover.net/1/messages.json"
		,CURLOPT_POSTFIELDS => [
			"token"		=> $pushoverToken
			,"user"		=> $userToken
			,"title"	=> "Security"
			,"message"	=> $message
		]
		,CURLOPT_SAFE_UPLOAD => true
		,CURLOPT_RETURNTRANSFER => true
	]);
	curl_exec($ch);
	curl_close($ch);
}
