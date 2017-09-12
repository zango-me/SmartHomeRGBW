bool handleCommad(String command)
{
	command.trim();
	if (command.length() > 0)
	{
		/* Examples:
		 * r=12|b=38
		 * w=255|t=10000
		 * m=1
		 * m=0|t=500
		 */

		 //Serial.println("Command: " + command);

		int r = getCommandValue('r', command, 255);
		int g = getCommandValue('g', command, 255);
		int b = getCommandValue('b', command, 255);
		int w = getCommandValue('w', command, 255);
		int mode = getCommandValue('m', command, 2);
		int time = getCommandValue('t', command, 10000);

		//Serial.println("Parsed values: r=" + String(r) + "g=" + String(g) + "b=" + String(b) + "w=" + String(w) + "m=" + String(mode) + "t=" + String(time));

		return ledSet(r, g, b, w, mode, time);

	}
	return false;
}

int getCommandValue(char key, String command, int _max)
{
	int indexOfKey = command.indexOf(key);
	if (indexOfKey == -1 || command.length() < (indexOfKey + 2)) //Key nicht vorhanden, oder Value kaputt
	{
		return -1;
	}

	String value = command.substring(indexOfKey + 2);

	int indexOfPipe = value.indexOf("|");
	if (indexOfPipe != -1)
	{
		value = value.substring(0, indexOfPipe);
	}
	int v = value.toInt();
	if (v > _max) v = _max;
	if (v < -1) v = -1;
	return v;
}


bool ledSetString(String _r, String _g, String _b, String _w, String _mode, String _time) // Empty String = do not change
{
	int r = _r == "" ? -1 : _r.toInt();
	int g = _g == "" ? -1 : _g.toInt();
	int b = _b == "" ? -1 : _b.toInt();
	int w = _w == "" ? -1 : _w.toInt();
	int mode = _mode == "" ? -1 : _mode.toInt();
	int time = _time == "" ? -1 : _time.toInt();

	return ledSet(r, g, b, w, mode, time);
}


