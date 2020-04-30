

#ifndef ACPWM_h
#define ACPWM_h


#include "Arduino.h"

//#include <arduino.h>

class ACPWM
{
	private:
		
		int pwmPn;		//Output pin to switching component (TRIAC, SSR, e.t.c.)
		int dutyCycle;	// % ON PWM interval
		int period; //Half AC main period in microseconds
		long offTime; //The off time of the cycle
		long onTime; //The on time of the cycle
		int scale; //The scaling of the duty cycle.
		int onLatch; //The ON latching interval of the switching unit, when pulse goes from off to on.
		int basicLatch; //The basic latching interval of the switching unit, when AC mains crosses zero.

	
	public:
		ACPWM();
		~ACPWM();

		void initialize(int frequency, float adjust,  int pwmPin, int scale);
        void ZeroCrossing();
		void setLatch(int basicLatch, int onLatch);

		//dutyCycle: % ON PWM interval
		void setDutyCycle(int dutyCycle);
		int getPwmPin(); //Not useful. Only for internal use.
		 
		long getBasicLatch(); //Not useful. Only for internal use.
		long getOnLatch(); //Not useful. Only for internal use.
		long getOnTime(); //Not useful. Only for internal use.
		long getOffTime(); //Not useful. Only for internal use.
};
extern ACPWM ACpwm;	//Use this object to access methods from the main program
#endif
