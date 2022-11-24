void encSetup() {
	encoder = new ClickEncoder(BTN_EN1, BTN_EN2, BTN_ENC, STEPS_PER_NOTCH);
	Timer1.initialize(1000);
	Timer1.attachInterrupt(timerIsr);
	last = -1;
	encoder->setDoubleClickEnabled(1);
}
void timerIsr() {
	encoder->service();
}
//Call this service when encoder input required
int encService()
{
	value += encoder->getValue();

	if (value != last)
	{
		last = value;
	}

	//here you can define what happens if encoder button is clicked/held(and then released)/doubleclicked.
	//Put this wherewer you want, but not here! (or OK, here, but it`ll always be the same, so not recommended)


	/*
	ClickEncoder::Button b = encoder->getButton();
	if (b != ClickEncoder::Open) {
		switch (b) {
		case ClickEncoder::DoubleClicked:
			return 2;
			break;
		case ClickEncoder::Held:
			return 3;
			break;
		case ClickEncoder::Released:
			return 4;
			break;
		case ClickEncoder::Clicked:
			return 1;
			break;
		default:
			return -1;
			break;
		}
	}
	*/
}