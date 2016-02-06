#include <avr/interrupt.h>
#include <IRReceiver.h>
#include <Button.h>

// ATMEL ATTINY45 / ARDUINO
//
//                  +-\/-+
// Ain0 (D 5) PB5  1|    |8  Vcc
// Ain3 (D 3) PB3  2|    |7  PB2 (D 2)  Ain1
// Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
//            GND  4|    |5  PB0 (D 0) pwm0
//                  +----+

#define MAX_RANDOM_JUMPS_FOLDER 	50
#define MIN_RANDOM_JUMPS_FOLDER 	0
#define MAX_RANDOM_JUMPS_TRACK	 	6
#define MIN_RANDOM_JUMPS_TRACK	 	0

#define MUTE			0x00	// Currently not implemented
#define NEXT_FOLDER		0x01
#define PREVIOUS_FOLDER	0x02
#define NEXT_TRACK		0x03
#define FAST_FORWARD	0x03
#define PREVIOUS_TRACK	0x10
#define VOLUME_UP		0x11
#define VOLUME_DOWN		0x12
#define CHANGE_SOURCE	0x13

#define VOLUME_UP_REPEAT 	3
#define VOLUME_DOWN_REPEAT 	3

#define ACTIVE_DURATION		57
#define INACTIVE_DURATION	57

#define MULTIPLEXER_A	0x00
#define MULTIPLEXER_B	0x01
#define MULTIPLEXER_C	0x04

#define BUTTON_TIMEOUT_THRESHOLD 70

IRReceiver irReceiver(2);
Button buttonRight;
Button buttonLeft;
Button buttonUp;
Button buttonDown;
Button buttonMiddle;
Button buttonVolumeUp;
Button buttonVolumeDown;
static volatile unsigned long curTime;
static volatile unsigned long pressTime = 0;

void setup(){
	Button::SetPressTimeout(350);

	buttonRight.OnClick = buttonRight_OnClick;
	buttonRight.OnLongPress = buttonRight_OnLongPress;
	buttonRight.OnLongRelease = buttonRight_OnLongRelease;

	buttonLeft.OnPress = buttonLeft_OnPress;
	buttonLeft.OnRelease = buttonLeft_OnRelease;
	buttonLeft.OnLongRelease = buttonLeft_OnRelease;

	buttonUp.OnPress = buttonUp_OnPress;
	buttonUp.OnRelease = buttonUp_OnRelease;
	buttonUp.OnLongRelease = buttonUp_OnRelease;

	buttonDown.OnPress = buttonDown_OnPress;
	buttonDown.OnRelease = buttonDown_OnRelease;
	buttonDown.OnLongRelease = buttonDown_OnRelease;

	buttonMiddle.OnClick = buttonMiddle_OnClick;
	
	buttonVolumeUp.OnPress = buttonVolumeUp_OnPress;
	buttonVolumeUp.OnRelease = buttonVolumeUp_OnRelease;
	buttonVolumeUp.OnLongRelease = buttonVolumeUp_OnRelease;
	
	buttonVolumeDown.OnPress = buttonVolumeDown_OnPress;
	buttonVolumeDown.OnRelease = buttonVolumeDown_OnRelease;
	buttonVolumeDown.OnLongRelease = buttonVolumeDown_OnRelease;
	
	pinMode(MULTIPLEXER_A, OUTPUT);
	pinMode(MULTIPLEXER_B, OUTPUT);
	pinMode(MULTIPLEXER_C, OUTPUT);

	randomSeed(analogRead(3));
	
	attachInterrupt(0, SIGNAL_IR, FALLING);
}

void loop(){
	curTime = millis();
	
	buttonRight.Refresh();
	buttonLeft.Refresh();
	buttonUp.Refresh();
	buttonDown.Refresh();
	buttonMiddle.Refresh();
	buttonVolumeUp.Refresh();
	buttonVolumeDown.Refresh();
	
	if(curTime - pressTime >= BUTTON_TIMEOUT_THRESHOLD)
	{
		// Disable interrupt since this block might generate a button event
		detachInterrupt(0);

		buttonRight.SetState(Button::low);
		buttonLeft.SetState(Button::low);
		buttonUp.SetState(Button::low);
		buttonDown.SetState(Button::low);
		buttonMiddle.SetState(Button::low);
		buttonVolumeUp.SetState(Button::low);
		buttonVolumeDown.SetState(Button::low);

		// Re-enable the interrupt
		attachInterrupt(0, SIGNAL_IR, FALLING);
	}
}

void SIGNAL_IR(){
	detachInterrupt(0); // Do not allow an interrupt within an interrupt
	handleIRInterrupt();
	attachInterrupt(0, SIGNAL_IR, FALLING);
}

void handleIRInterrupt(){
	unsigned long irCode;
	
	irCode = irReceiver.GetCode();
	
	if (irCode > 0)
	{
		switch(irCode){
			case 0x1000405: // Volume Up
				buttonVolumeUp.SetState(Button::high);
				pressTime = millis();
				break;
				
			case 0x1008485: // Volume Down
				buttonVolumeDown.SetState(Button::high);
				pressTime = millis();
				break;
				
			case 0x100F2F3: // Right
				buttonRight.SetState(Button::high);
				pressTime = millis();
				break;
				
			case 0x1007273: // Left
				buttonLeft.SetState(Button::high);
				pressTime = millis();
				break;
				
			case 0x1005253: // Up
				buttonUp.SetState(Button::high);
				pressTime = millis();
				break;
				
			case 0x100D2D3: // Down
				buttonDown.SetState(Button::high);
				pressTime = millis();
				break;
				
			case 0x1009C9D: // Middle
				buttonMiddle.SetState(Button::high);
				pressTime = millis();
				break;
		}
	}
}

inline void setMuxValue(byte i_Val){
	/*
	if((i_Val & MULTIPLEXER_A) > 0)
	{
		digitalWrite(MULTIPLEXER_A, HIGH);
	}
	
	if((i_Val & MULTIPLEXER_B) > 0)
	{
		digitalWrite(MULTIPLEXER_B, HIGH);
	}
	
	if((i_Val & MULTIPLEXER_C) > 0)
	{
		digitalWrite(MULTIPLEXER_C, HIGH);
	}
	*/
	PORTB |= i_Val;
}

inline void clearMuxValue(byte i_Val){
	/*
	digitalWrite(MULTIPLEXER_A, LOW);
	digitalWrite(MULTIPLEXER_B, LOW);
	digitalWrite(MULTIPLEXER_C, LOW);
	*/
	PORTB &= (~i_Val);
}

inline void toggleMuxValue(byte i_Val){
	setMuxValue(i_Val);
	delay(ACTIVE_DURATION);
	clearMuxValue(i_Val);
	delay(INACTIVE_DURATION);
}

// Button Right
void buttonRight_OnClick(){
	toggleMuxValue(NEXT_TRACK);
}
void buttonRight_OnLongPress(){
	setMuxValue(FAST_FORWARD);
}
void buttonRight_OnLongRelease(){
	clearMuxValue(FAST_FORWARD);
}

// Button Left
void buttonLeft_OnPress(){
	setMuxValue(PREVIOUS_TRACK);
}
void buttonLeft_OnRelease(){
	clearMuxValue(PREVIOUS_TRACK);
}

// Button UP
void buttonUp_OnPress(){
	setMuxValue(NEXT_FOLDER);
}
void buttonUp_OnRelease(){
	clearMuxValue(NEXT_FOLDER);
}

// Button Down
void buttonDown_OnPress(){
	setMuxValue(PREVIOUS_FOLDER);
}
void buttonDown_OnRelease(){
	clearMuxValue(PREVIOUS_FOLDER);
}

// Button Middle
void buttonMiddle_OnClick(){
	byte folder, n;
	n = random(MIN_RANDOM_JUMPS_FOLDER, MAX_RANDOM_JUMPS_FOLDER);
	folder = random(0, 2) ? NEXT_FOLDER : PREVIOUS_FOLDER;
	while(n>0)
	{
		toggleMuxValue(folder);
		n--;
	}
	n = random(MIN_RANDOM_JUMPS_TRACK, MAX_RANDOM_JUMPS_TRACK);
	while(n>0)
	{
		toggleMuxValue(NEXT_TRACK);
		n--;
	}
}

// Button Volume Up
void buttonVolumeUp_OnPress(){
	uint8_t n = VOLUME_UP_REPEAT;
	while(n>0)
	{
		toggleMuxValue(VOLUME_UP);
		n--;
	}
	setMuxValue(VOLUME_UP);
}
void buttonVolumeUp_OnRelease(){
	clearMuxValue(VOLUME_UP);
}

// Button Volume Down
void buttonVolumeDown_OnPress(){
	uint8_t n = VOLUME_DOWN_REPEAT;
	while(n>0)
	{
		toggleMuxValue(VOLUME_DOWN);
		n--;
	}
	setMuxValue(VOLUME_DOWN);
}
void buttonVolumeDown_OnRelease(){
	clearMuxValue(VOLUME_DOWN);
}