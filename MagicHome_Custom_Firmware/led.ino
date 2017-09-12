
// RPin, GPin, BPin, WPin = Output Pins (already set up)

bool ledSet(int r, int g, int b, int w, int mode, int time) // -1 = do not change // will be called on boot with last set values from EEPROM
{
	/* Ranges:
	 * r, g, b, w = 0-255
	 * mode = 0 (off), 1 (on), 2 (fade)
	 * time = 0-10000 (transition or fade time)
	 */

	if (r >= 0) global_r = r;
	if (g >= 0) global_g = g;
	if (b >= 0) global_b = b;
	if (w >= 0) global_w = w;
	if (mode >= 0) global_mode = mode;
	if (time >= 0) global_time = time;


	memSetString(String(global_r), memoryAddresses[6]); //R
	memSetString(String(global_g), memoryAddresses[7]); //G
	memSetString(String(global_b), memoryAddresses[8]); //B
	memSetString(String(global_w), memoryAddresses[9]); //W
	memSetString(String(global_mode), memoryAddresses[10]); //MODE
	memSetString(String(global_time), memoryAddresses[11]); //TIME


  //Serial.println("R:"+String(global_r)+" G:"+String(global_g)+" B:"+String(global_b)+" W:"+String(global_w)+" T:"+String(global_time)+" M:"+String(global_mode));
	switch (global_mode) {
	case 0: //Off
		if (global_time == 0) {
			analogWrite(RPin, 0);
			analogWrite(GPin, 0);
			analogWrite(BPin, 0);
			analogWrite(WPin, 0);
		}
		else fader.fadeTo(0, 0, 0, 0, global_time);
		rgb_fader = false;
		break;
	case 1: //On
		if (global_time == 0) {
			analogWrite(RPin, global_r);
			analogWrite(GPin, global_g);
			analogWrite(BPin, global_b);
			analogWrite(WPin, global_w);
		}
		else fader.fadeTo(global_r, global_g, global_b, global_w, global_time);
		rgb_fader = false;
		break;
	case 2: //RGB Fade
		rgb_fade_color = 0;
		rgb_fader = true;
		break;
	default:
		//fader.fadeTo(0, 0, 0, 0, global_time);
		//rgb_fader = false;
		return false;
		break;
	}

	return true;
}


void ledUpdate(void) //Loop function
{
	fader.update();

	if (rgb_fader) {
		if (!fader.isFading()) {
			if (eeprom_autosave && eeprom_changed) {
				eeprom_changed = false;
				eepromCommit();
			}
			int _time = global_time;
			if (_time < 1) _time = 1;
			switch (rgb_fade_color) {
			case 0:
				fader.fadeTo(255, 0, 0, 0, _time);
				rgb_fade_color++;
				break;
			case 1:
				fader.fadeTo(255, 255, 0, 0, _time);
				rgb_fade_color++;
				break;
			case 2:
				fader.fadeTo(0, 255, 0, 0, _time);
				rgb_fade_color++;
				break;
			case 3:
				fader.fadeTo(0, 255, 255, 0, _time);
				rgb_fade_color++;
				break;
			case 4:
				fader.fadeTo(0, 0, 255, 0, _time);
				rgb_fade_color++;
				break;
			case 5:
				fader.fadeTo(255, 0, 255, 0, _time);
				rgb_fade_color++;
				break;
			default:
				rgb_fade_color = 0;
				break;
			}
		}
	}

}

