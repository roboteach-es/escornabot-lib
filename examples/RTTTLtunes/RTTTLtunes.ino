/**
 * Escornabot-lib RTTTL tunes example: sounds and music
 */

#include <Escornabot-lib.h>
Escornabot luci;

// NOTE: on-line player at https://adamonsoon.github.io/rtttl-play/

const char *songs[] = 
{
	"startup:d=16,o=6,b=140:c,p,e,p,g,",
	"fido:d=16,o=6,b=800:f,4p,f,4p,f,4p,f,4p,c,4p,c,4p,c,4p,c,",
	"dtmf:d=8,o=4,b=320:d#6,e6,f#6,d#6,",
	"hang:d=4,o=5,b=160:b,f6,a6,",
	"Mario:d=4,o=5,b=100:32p,16e6,16e6,16p,16e6,16p,16c6,16e6,16p,16g6,8p,16p,16g,8p,32p,16c6,8p,16g,8p,16e,8p,16a,16p,16b,16p,16a#,16a,16p,16g,16e6,16g6,16a6,16p,16f6,16g6,16p,16e6,16p,16c6,16d6,16b,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,8d#6,16p,16d6,8p,8c6",
	"intel:d=8,o=5,b=320:d,p,d,p,d,p,g,p,g,p,g,p,d,p,d,p,d,p,a,p,a,p,",
	"mosaic:d=8,o=6,b=400:c,e,g,e,c,g,e,g,c,g,c,e,c,g,e,g,e,c,",
	"furelisa:d=8,o=6,b=125:e,d#,e,d#,e,b5,d,c,a,",
	"fanfare:d=16,o=6,b=180:g,8p,g,p,g,p,4a#,",
	"backtothefuture:d=16,o=5,b=200:4g.,p,4c.,p,2f#.,p,g.,p,a.,p,8g,p,8e,p,8c,p,4f#,p,g.,p,a.,p,8g.,p,8d.,p,8g.,p,8d.6,p,4d.6,p,4c#6,p,b.,p,c#.6,p,2d.6,"
};

//"scale:d=16,o=4,b=200:c,c#,d,d#,e,f,f#,g,g#,a,a#,b,c5,c#5,d5,d#5,e5,f5,f#5,g5,g#5,a5,a#5,b5,c6,c#6,d6,d#6,e6,f6,f#6,g6,g#6,a6,a#6,b6,c7,c#7,d7,d#7,e7,f7,f#7,g7,g#7,a7,a#7,b7,"

void setup() {
	// banner
	Serial.begin(9600);
	Serial.println("Escornalib RTTTL test for Luci\n");
	// setup luci
	luci.init();
	// start-up sequence: beep + Luci color
	luci.beep(EB_BEEP_DEFAULT, 100);
	luci.showColor(50, 0, 20); // purple
	delay(1000);
}

void loop() {
	for (int i = 0; i < 10; i ++)
	{
		Serial.println(songs[i]);
		luci.playRTTTL(songs[i]);
		delay(1000);  
	}
}
