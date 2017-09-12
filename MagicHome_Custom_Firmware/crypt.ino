
String encryptDecrypt(String data)
{
	MD5Builder md5;
	md5.begin();
	md5.add((getMacString() + ESP.getChipId()));
	md5.calculate();
	String key = md5.toString();
	key.toLowerCase();

	String output = data;

	for (int i = 0; i < data.length(); i++)
		output[i] = data[i] ^ key.charAt(i % (key.length()));

	return output;
}

String string_to_hex(String input)
{
	static const char* const lut = "0123456789ABCDEF";
	size_t len = input.length();

	String output;
	output.reserve(2 * len);
	for (size_t i = 0; i < len; ++i)
	{
		const unsigned char c = input[i];
		output.concat(lut[c >> 4]);
		output.concat(lut[c & 15]);
	}
	return output;
}

String hex_to_string(String input)
{
	static const char* const lut = "0123456789ABCDEF";
	size_t len = input.length();
	if (len & 1) return "";

	String output;
	output.reserve(len / 2);
	for (size_t i = 0; i < len; i += 2)
	{
		char a = input[i];
		const char* p = std::lower_bound(lut, lut + 16, a);
		if (*p != a) return "";

		char b = input[i + 1];
		const char* q = std::lower_bound(lut, lut + 16, b);
		if (*q != b) return "";

		output.concat((char)(((p - lut) << 4) | (q - lut)));
	}
	return output;
}