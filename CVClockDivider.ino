/***
*       _______    __   ________           __      ____  _       _     __
*      / ____| |  / /  / ____/ ____  _____/ /__   / __ \(__   __(_____/ ___  _____
*     / /    | | / /  / /   / / __ \/ ___/ //_/  / / / / | | / / / __  / _ \/ ___/
*    / /___  | |/ /  / /___/ / /_/ / /__/ ,<    / /_/ / /| |/ / / /_/ /  __/ /
*    \____/  |___/   \____/_/\____/\___/_/|_|  /_____/_/ |___/_/\__,_/\___/_/
*
*
*
*								***THX2112***
*
*							http://syinsi.com
*
*/

//#include <digitalWriteFast.h>

// Pins:

int tempoPot = A1;
int startPin = 4;
int resetPin = 1;
int clockPin = 0;
int out = 3;
int clockPulse;
int tempoValue;
int potMap;
int clockDivMult;
int bounceTimer = 0;
int lastBounceTime = 0;
unsigned long timeoutTimer = 0;		//	microseconds
unsigned long previousPulse = 0;	//	microseconds
unsigned long currentPulse = 0;		//	microseconds
unsigned long periodStartTime = 0;	//	microseconds
unsigned long periodEndTime = 0;	//	microseconds
bool trigState = LOW;
bool lastTrigState = LOW;
bool ResetState = LOW;
bool lastResetState = LOW;
unsigned long duration;	//	microseconds
bool isHit = false;
unsigned long beginTime;
unsigned long now;



///////////////////////////////////////////////////////////////////////////////
//
//	Setup pins.  
//

void setup()
{
	pinMode(startPin, INPUT);
	pinMode(out, OUTPUT);
	pinMode(resetPin, INPUT);
	pinMode(clockPin, INPUT);

}

///////////////////////////////////////////////////////////////////////////////
//
//	Main.
//

void loop()
{
	checkTrigger();

	if (isHit && startPin)		//  Start on rising edge of clock pulse if STARTPIN is high.
	{
		hitIt();
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	Time stuff. See if trigger is being hit...
//

void checkTrigger()
{

	//
	//	Check CLOCK and RESET pins.
	//

	trigState = digitalRead(clockPin);
	resetState = digitalRead(resetPin);

	if (resetState == HIGH && lastResetState == LOW)	//	Reset clock if reset pin pulled high
	{
		clockPulse = 0;
		lastResetState = HIGH;
	}

	if ((trigState == HIGH) && (lastTrigState == LOW))	//	If a new clock is detected...
	{
		currentPulse = millis();

		if ((currentPulse - previousPulse) > 100)		//	Reset clock pulse if clock stops to start on the first clock
		{
			clockPulse = 0;
		}

		previousPulse = currentPulse;

		if (clockPulse > 95)	//	Clocks start at zero.
		{
			clockPulse = 0;
		}

		//
		//	Set division
		//

		tempoValue = analogRead(tempoPot);

		potMap = map(tempoValue, 0, 1023, 8, 0);	// Reverse response of pot and map to X values

		//if (potMap == 9) { clockDivMult = 64; }	//	1.5 Causes uneven beats due to retriggering at end of 96-tick bar. Fix?	
		if (potMap == 8) { clockDivMult = 96; }		//	1	Every bar.
		if (potMap == 7) { clockDivMult = 48; }		//	2	half
		//if (potMap == 7) { clockDivMult = 32; }	//	3	third. Causes uneven beats due to retriggering at end of 96-tick bar. Fix? Could fix by doubling/quadrupling/etc 96-tick counter...
		if (potMap == 6) { clockDivMult = 24; }		//	4	quarter
		if (potMap == 5) { clockDivMult = 12; }		//	8th note -- default setting
		if (potMap == 4) { clockDivMult = 6; }		//	16th notes
		if (potMap == 3) { clockDivMult = 4; }		//	24	fast
		if (potMap == 2) { clockDivMult = 3; }		//	32	really really fast
		if (potMap == 1) { clockDivMult = 2; }		//	64	something is going to break
		if (potMap == 0) { clockDivMult = 1; }		//	96	every 24ppqm clock tick (may fracture spacetime.)

		//
		//	Check if it's time to send a hit
		//

		if ((clockPulse % clockDivMult == 0))
		{
			isHit = true;
		}

		lastTrigState = HIGH;
		clockPulse++;
	}

	if ((trigState == LOW) && (lastTrigState == HIGH))
	{
		lastTrigState = LOW;
	}

	if ((resetState == LOW) && (lastResetState == HIGH))
	{
		lastResetState = LOW;
	}

}

///////////////////////////////////////////////////////////////////////////////
//
//	Send the new clock.
//

void hitIt()
{
	isHit = false;				//	Reset.
	beginTime = micros();
	now = 0;
	duration = 5000;			// pulse duration in microseconds. 5000us = 5ms

	//
	//	Loop until the end of DURATION. 
	//

	while (now < duration)
	{
		digitalWrite(out, HIGH);

		//	Check if timing is changed during a pulse.
		//
		//	Don't reset isHit so that same clock pulse can be used to set up next clock in checkTrigger function.
		//

		checkTrigger();
		if (isHit)
		{
			break;
		}

		now = (micros() - beginTime);	//	Register time for next run through.
	}

	//
	//	Clean up at end of hit. 
	//

	digitalWrite(out, LOW);

}

