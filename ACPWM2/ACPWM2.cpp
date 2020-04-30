#include <ACPWM2.h>

ACPWM ACpwm;

ACPWM::ACPWM(){}
ACPWM::~ACPWM(){}

//Initialization procedure
void ACPWM::initialize(int frequency, float adjust, int pwmPin, int scale){	
	ACpwm.scale=scale;
	period=(500000/frequency)/adjust; //(1/2)*(1/f)*1000000
	//period = 4600;
	pwmPn=pwmPin;

	//setLatch(0,0);	//Set latching to zero. Later, the user can change it.
	//pinMode(pwmPn,OUTPUT);
	//digitalWrite(pwmPn,LOW);
	//cli();  //Disable interrupts for setup
	TCCR1A=0;  //Clear registers
	TCCR1B=0;
	TCNT1=0;
	setDutyCycle(0);
	TCCR1B|=(1<<WGM12);  //Sets CTC mode of interrupt (see Atmega specs)
	TCCR1B|=(1<<CS11); //Set prescale to 8 (see Atmega specs)
	TIMSK1|=(1<<OCIE1A); // enable timer compare interrupt
	//sei();  //Enable interrupts
	//If zero control pin not set to 2 or 3, the program will not run.

}

void ACPWM::setLatch(int basicLatch, int onLatch){
	ACpwm.onLatch = onLatch*2;	//(latch*10^-6)*(16*10^6)/8	8:prescaler
	ACpwm.basicLatch = basicLatch*2;
}

void ACPWM::setDutyCycle(int dCycle){
//Serial.print("period    "); Serial.println(period);
	if(dCycle<0) dutyCycle=0;
	else if(dCycle>(scale-1)) dutyCycle=scale;
	else dutyCycle=dCycle;
	offTime = (long) period*(scale-dutyCycle)/(long)scale*2; //[PERIOD*10^-6*(scale-dutyCycle)/scale]*16*10^6/8
                            //[PERIOD*10^-6*(scale-dutyCycle)/scale-latch]: off time. 16*10^6: Clock rate. 8: Prescaler, set below. See ATMega specs
                        
	onTime = (long)period*dutyCycle/(long)scale*2;
	
	//Serial.print("offtme    "); Serial.println(offTime);
	//Serial.print("ontime    "); Serial.println(onTime);
}

void ACPWM::ZeroCrossing(){
	TCCR1B = 0x00;
	if(dutyCycle<scale*0.01){ //Very small value --> turn off    
    digitalWrite(pwmPn,LOW);
  }
  else if(dutyCycle>scale*0.98){  //Very high value --> turn on
	digitalWrite(pwmPn,HIGH);
	
  }
  else{
	  if(basicLatch==0){
		digitalWrite(pwmPn,LOW);  
		OCR1A=offTime+(long)onLatch;
		//Serial.print("OCR1A 1   "); Serial.println(OCR1A);
	  }
	  else if(basicLatch>0){
		digitalWrite(pwmPn,HIGH);
		OCR1A= (long) basicLatch;
		//Serial.print("OCR1A  2  "); Serial.println(OCR1A);
	  }
	  else{
		digitalWrite(pwmPn,LOW);
		OCR1A=offTime+(long)onLatch + (long) basicLatch; 
		//Serial.print("OCR1A 2   "); Serial.println(OCR1A);
		//basicLatch is negative in this case
	 }
     TCCR1B|=(1<<WGM12);  //Re-enable counter by setting again the prescaler
     TCCR1B|=(1<<CS11);
     TCNT1=1;  //Restart counter (setting to zero will cause an interrupt, that's why we set the counter steps of OCR1A to one bit less
  }
}

//Arduino mainInterrupt needs pointer or static function to 2nd argument. But we can't have pointers to member functions,
//and the compiler cannot link static functions with the main program correctly. Solution (after 3 days of swearing the compiler):
//We make a static assignment function that grabs the actual (mainInterrupt) member function.

///}

int ACPWM::getPwmPin(){return pwmPn;}



long ACPWM::getBasicLatch(){return (long) basicLatch;}

long ACPWM::getOnLatch(){return (long) onLatch;}

long ACPWM::getOnTime(){return (long) onTime;}

long ACPWM::getOffTime(){return (long) offTime;}


//Interrupt service routine of Timer1
ISR(TIMER1_COMPA_vect){
	if(ACpwm.getBasicLatch()==0){
		digitalWrite(ACpwm.getPwmPin(),HIGH);
		TCCR1B = 0x00;  //Disable (freeze the counter)
		OCR1A=ACpwm.getOnTime() - ACpwm.getOnLatch() + 10; //10: Dummy period for preventing the pulse from changing.
	//Serial.print("OCR1A3    "); Serial.println(OCR1A);
	}
	else if(ACpwm.getBasicLatch()>0){
		if(digitalRead(ACpwm.getPwmPin())==HIGH){
			digitalWrite(ACpwm.getPwmPin(),LOW);
			TCCR1B = 0x00;  //Disable (freeze the counter)
			OCR1A=ACpwm.getOffTime() + ACpwm.getOnLatch();
			//Serial.print("OCR1A 4   "); Serial.println(OCR1A);
		}
		else{
			digitalWrite(ACpwm.getPwmPin(),HIGH);
			TCCR1B = 0x00;  //Disable (freeze the counter)
			OCR1A=ACpwm.getOnTime() - ACpwm.getOnLatch();
			//Serial.print("OCR1A  5  "); Serial.println(OCR1A);
		}
	}
	else{
		if(digitalRead(ACpwm.getPwmPin())==LOW){
			digitalWrite(ACpwm.getPwmPin(),HIGH);
			TCCR1B = 0x00;  //Disable (freeze the counter)
			OCR1A=ACpwm.getOnTime() - ACpwm.getOnLatch();
			//Serial.print("OCR1A  6  "); Serial.println(OCR1A);
		}
		else{
			digitalWrite(ACpwm.getPwmPin(),LOW);
			TCCR1B = 0x00;  //Disable (freeze the counter)
			OCR1A=-(long)ACpwm.getBasicLatch()+10; //10: Dummy period for preventing the pulse from changing.
			//Serial.print("OCR1A 7   "); Serial.println(OCR1A);
		}
	}
	TCCR1B|=(1<<WGM12);  //Re-enable counter by setting again the prescaler
	TCCR1B|=(1<<CS11);
	TCNT1 = 1;  //Set counter to COMPARE_VALUE-onTime   
}
