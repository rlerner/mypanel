# Files

| File | Purpose |
| :--- | :------ |
| index.php | The script that recieves the zone information, translates it to a useful name, and sends a Pushover notification |
| arduino.ino | The code that reads the zones, sets off alarms, and calls the index.php above |

It's ideal not to host the index.php public facing as it is very "Version 1" right now. I did not set up a way to lock it down and it can easily be abused if it were public. Making it more secure is a bit of a moot point considering the Ethernet shield can't handle SSL/TLS.
