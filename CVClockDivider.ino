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
int i = 0;
unsigned long timeoutTimer = 0;		//	microseconds
unsigned long previousPulse = 0;	//	microseconds
unsigned long currentPulse = 0;		//	microseconds
unsigned long periodStartTime = 0;	//	microseconds
unsigned long periodEndTime = 0;	//	microseconds
bool trigState = LOW;
bool lastTrigState = LOW;
bool resetState = LOW;
bool lastResetState = LOW;
bool startState = LOW;
bool lastStartState = LOW;
unsigned long duration;				//	microseconds
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

	// Flash LED to show we're alive.
	digitalWrite(out, HIGH);
	delay(100);
	digitalWrite(out, LOW);

	getTempo(); // Prime the pump.

}

///////////////////////////////////////////////////////////////////////////////
//
//	Main.
//

void loop()
{
	checkTrigger();
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
	startState = digitalRead(startPin);

	if (resetState == HIGH && lastResetState == LOW)	//	Reset clock if reset pin pulled high
	{
		clockPulse = 0;
		lastResetState = HIGH;
		getTempo();
		isHit = true;	//	All hits start on downbeat. Prevents skipping the first hit after a start or reset
	}

	if (startState == HIGH && lastStartState == LOW)	//	Reset clock if this is first new START
	{
		clockPulse = 0;
		lastStartState = HIGH;
		getTempo();
		isHit = true;	//	All hits start on downbeat. Prevents skipping the first hit after a start or reset
	}

	if ((trigState == HIGH) && (lastTrigState == LOW))	//	If a new clock is detected...
	{
		clockPulse++;

		if (clockPulse > 96)	//	Clocks start at one. Sync 24 = 24ppqn = 96 pulses per bar.
		{
			clockPulse = 1;
		}

		//
		//	Send the hit if it's time and START is held high. This is done here to minimize latency.
		//

		if (isHit)
		{
			if (startState)
			{
				hitIt();
			}
		}

		lastTrigState = HIGH;
	}

	//
	//	Reset state toggles.
	//

	if ((trigState == LOW) && (lastTrigState == HIGH))	//	This is a good place for time consuming code like maths.
		getTempo();		//	This is slow so do this here while nothing important is being done.
	{

		lastTrigState = LOW;
		checkNextHit();				//	Check if next clock will be a hit here to reduce latency.

	}

	if ((resetState == LOW) && (lastResetState == HIGH))
	{
		lastResetState = LOW;
	}

	if ((startState == LOW) && (lastStartState == HIGH))
	{
		lastStartState = LOW;
	}

}

///////////////////////////////////////////////////////////////////////////////
//
//	Send the new clock.
//

void hitIt()
{
	digitalWrite(out, HIGH);	//	Rising edge of new output clock.
	isHit = false;				//	Reset.
	beginTime = micros();
	now = 0;
	duration = 5000;			//	Pulse duration in microseconds. 5000us = 5ms

	clockPulse = 0;				//	Each pulse restarts clock.

	//
	//	Loop until the end of DURATION.
	//

	while (now < duration)
	{
		//
		//	Set division here for reduced latency
		//

		getTempo();


		//
		//	Don't reset isHit so that same clock pulse can be used to set up next clock in checkTrigger function.
		//


		now = (micros() - beginTime);	//	Register time for next run through.
	}

	//
	//	Clean up at end of pulse.
	//

	digitalWrite(out, LOW);	//	Falling edge of new clock.

}


//
//	Set division
//

int getTempo()
{

	tempoValue = analogRead(tempoPot);		//	Slow.

	potMap = map(tempoValue, 0, 1023, 11, 0);	// Reverse response of pot and map to X values

	if (potMap == 11) { clockDivMult = 96; }	//	1	Every bar.
	if (potMap == 10) { clockDivMult = 64; }	//	1.5 Causes uneven beats due to retriggering at end of 96-tick bar. (FIXED)
	if (potMap == 9) { clockDivMult = 48; }		//	2	half
	if (potMap == 8) { clockDivMult = 32; }		//	3	third. Causes uneven beats due to retriggering at end of 96-tick bar. (FIXED)
	if (potMap == 7) { clockDivMult = 24; }		//	4	quarter
	if (potMap == 6) { clockDivMult = 16; }		//	6
	if (potMap == 5) { clockDivMult = 12; }		//	8th note -- default setting
	if (potMap == 4) { clockDivMult = 6; }		//	16th notes
	if (potMap == 3) { clockDivMult = 4; }		//	24	fast
	if (potMap == 2) { clockDivMult = 3; }		//	32	really really fast
	if (potMap == 1) { clockDivMult = 2; }		//	64	something is going to break
	if (potMap == 0) { clockDivMult = 1; }		//	96	every 24ppqm clock tick (may fracture spacetime.)

	//checkHit();

	return clockDivMult; // Return isHit?

}

void checkHit()
{

	if ((clockPulse % clockDivMult == 0))	//	Slow.
	{
		isHit = true;
	}

}

void checkNextHit()
{

	if (((clockPulse + 1) % clockDivMult == 0))	//	Slow.
	{
		isHit = true;
	}

}

