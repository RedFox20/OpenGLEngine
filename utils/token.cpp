/**
 * String Tokenizer, Copyright (c) 2014 - Jorma Rebane
 */
#include "token.h"
/**
 * @note Same as strpbrk, except we're not dealing with 0-term strings
 * @note This function is optimized for 4-8 char str and 3-4 char control.
 */
static const char* mempbrk(const char* str, int nstr, const char* control, int ncontrol)
{
	while (nstr) {
		const char strCh = *str;
		for (int i = 0; i < ncontrol; ++i)
		if (strCh == control[i]) // found a match?
			return str; // done
		--nstr;
		++str;
	}
	return 0; // not found
}



///////////// token

bool token::toBool() const
{
	int len = end - str;
	if (len > 4)
		return false; // can't be any of true literals
	return (len == 4 && _memicmp(str, "true", 4) == 0) ||
		(len == 3 && _memicmp(str, "yes", 3) == 0) ||
		(len == 2 && _memicmp(str, "on", 2) == 0) ||
		(len == 1 && _memicmp(str, "1", 1) == 0);
}

bool token::is_whitespace()
{
	const char* s = str;
	while (s < end) {
		char c = *s;
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			return false; // found a non-whitespace char
		++s;
	}
	return true; // this is all whitespace
}

token& token::trim_start()
{
	while (str < end) {
		char c = *str;
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			return *this; // found non-whitespace char
		++str;
	}
	return *this;
}

token& token::trim_end()
{
	while (str < end) {
		char c = *(end - 1);
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			return *this; // found non-whitespace char
		--end;
	}
	return *this;
}

token& token::trim_start(char ch)
{
	while (str < end) {
		if (*str != ch) return *this; // no more ch to trim
		++str;
	}
	return *this;
}

token& token::trim_end(char ch)
{
	while (str < end) {
		if (*(end - 1) != ch) return *this; // no more ch to trim
		--end;
	}
	return *this;
}

token& token::trim_start(const char* chars, int nchars)
{
	while (str < end) {
		const char* s = chars;
		int n = nchars;
		char ch = *str;
		while (n) {
			if (*s == ch)
				break; // we found a trimmable char!
			++s, --n;
		}
		if (!n) // no trimmable chars found?
			return *this; // we're finished
		++str; // skip this char (effectively trimming it)
	}
	return *this;
}

token& token::trim_end(const char* chars, int nchars)
{
	while (str < end) {
		const char* s = chars;
		int n = nchars;
		char ch = *(end - 1);
		while (n) {
			if (*s == ch)
				break; // we found a trimmable char!
			++s, --n;
		}
		if (!n) // no trimmable chars found?
			return *this; // we're finished
		--end;
	}
	return *this;
}

bool token::contains(const char* chars, int nchars) const
{
	return mempbrk(str, end - str, chars, nchars) ? true : false;
}

const char* token::find(const char* substr, int len) const
{
	if (int n = len) //// @note lots of micro-optimization here
	{
		const char* needle = substr;
		const char* haystr = str;
		const char* hayend = end;
		int firstChar = *needle;
		while (haystr < hayend)
		{
			if (!(haystr = (const char*)memchr(haystr, firstChar, hayend - haystr))) 
				return NULL; // definitely not found

			if (memcmp(haystr, needle, n) == 0)
				return haystr; // it's a match
			++haystr; // no match, reset search from next char
		}
	}
	return NULL;
}

token token::split_first(char delim)
{
	if (char* splitend = (char*) memchr(str, delim, end - str))
		return token(str, splitend); // if we find a separator, readjust end of token to that
	return token(str, end);
}

token token::split_second(char delim)
{
	if (char* splitstart = (char*) memchr(str, delim, end - str))
		return token(splitstart + 1, end); // readjust start, also skip the char we split at
	return token(str, end);
}



int token::split(std::vector<token>& out, char delim, const char* trimChars)
{
	int ntrims = trimChars ? (int) strlen(trimChars) : 0;
	int numSplits = 0;
	token tok;
	token splitter(str, end);
	while (splitter.next(tok, delim)) // get next token
	{
		if (ntrims) tok.trim(trimChars, ntrims); // trim if needed
		if (tok.length()) // if we actually have anything after trimming?
		{
			out.push_back(token(tok.str, tok.end)); // push it out
			++numSplits;
		}
	}
	return numSplits;
}
int token::split(std::vector<token>& out, const char* delims, const char* trimChars)
{
	int ntrims = trimChars ? (int) strlen(trimChars) : 0;
	int ndelims = (int) strlen(delims);
	int numSplits = 0;
	token tok;
	token splitter(str, end); // split source token
	while (splitter.next(tok, delims, ndelims)) // get next token
	{
		if (ntrims) tok.trim(trimChars, ntrims); // trim if needed
		if (tok.length()) // if we actually have anything after trimming?
		{
			out.push_back(token(tok.str, tok.end)); // push it out
			++numSplits;
		}
	}
	return numSplits;
}



bool token::next(token& out, char delim)
{
	const char* s = str;
	for (;;) // using a loop to skip empty tokens
	{
		if (s >= end)		// out of bounds?
			return false;	// no more tokens available

		if (const char* e = (const char*) memchr(s, delim, end - s)) // try to find the token in this string
		{
			if (s == e)	// this is an empty token?
			{
				++s;		// increment search string
				continue;	// try again
			}

			out.str = s;	// writeout start/end
			out.end = e;
			str = ++e;		// move past the token for the next parsing call
			return true;	// we got what we needed
		}
		break;
	}
	out.str = s;	// writeout start/end
	out.end = end;
	str = end;		// last token
	return true;
}

bool token::next(token& out, const char* delims, int ndelims)
{
	const char* s = str;
	for (;;) // using a loop to skip empty tokens
	{
		if (s >= end)		// out of bounds?
			return false;	// no more tokens available

		if (const char* e = (const char*) mempbrk(s, end - s, delims, ndelims))
		{
			if (s == e)	// this is an empty token?
			{
				++s;		// increment search string
				continue;	// try again
			}

			out.str = s;	// writeout start/end
			out.end = e;
			str = ++e;		// move past the token for the next parsing call
			return true;	// we got what we needed
		}
		break;
	}
	out.str = s;	// writeout start/end
	out.end = end;
	str = end;		// last token
	return true;
}




bool token::next_notrim(token& out, char delim)
{
	const char* s = str;
	for (;;) // using a loop to skip empty tokens
	{
		if (s >= end)		// out of bounds?
			return false;	// no more tokens available

		if (const char* e = (const char*) memchr(s, delim, end - s)) // try to find the token in this string
		{
			if (s == e)	// this is an empty token?
			{
				++s;		// increment search string
				continue;	// try again
			}

			out.str = s;	// writeout start/end
			out.end = e;
			str = e;		// stop on identified token, NOTE: <-- only difference from next()
			return true;	// we got what we needed
		}
		break;
	}
	out.str = s;	// writeout start/end
	out.end = end;
	str = end;		// last token
	return true;
}

bool token::next_notrim(token& out, const char* delims, int ndelims)
{
	const char* s = str;
	for (;;) // using a loop to skip empty tokens
	{
		if (s >= end)		// out of bounds?
			return false;	// no more tokens available

		if (const char* e = (const char*) mempbrk(s, end - s, delims, ndelims))
		{
			out.str = s;	// writeout start/end (EMPTY tokens are allowed!)
			out.end = e;
			str = e;		// stop on identified token, NOTE: <-- difference from next()
			return true;	// we got what we needed
		}
		break;
	}
	out.str = s;	// writeout start/end
	out.end = end;
	str = end;		// last token
	return true;
}




float token::nextFloat()
{
	const char* s = str;
	while (s < end) {
		char ch = *s;
		// check if we have the start of a number: "-0.25" || ".25" || "25"
		if (ch == '-' || ch == '.' || ('0' <= ch && ch <= '9'))
			return _tofloat(s, &str);
		++s;
	}
	str = s;
	return 0.0f; // no floats parsed :(
}

int token::nextInt()
{
	const char* s = str;
	while (s < end) {
		char ch = *s;
		// check if we have the start of a number: "-25" || "25"
		if (ch == '-' || ('0' <= ch && ch <= '9'))
			return _toint(s, &str);
		++s;
	}
	str = s;
	return 0; // no ints parsed :(
}

void token::skip_until(char ch)
{
	const char* s = str;
	while (s < end) {
		if (ch == *s) break;
		++s;
	}
	str = s;
}

void token::skip_until(const char* substr, int len)
{
	char ch = *substr;		// starting char of the sequence
	const char* s = str;	// modify in register, no immediate writeout
	const char*const substrend = substr + len;
	while (s < end) {
		if (*s == ch) {
			const char* s1 = s,* sub = substr + 1;
			while (sub != substrend) {
				if (*s1++ != *sub) {
					s = s1;
					goto next;
				}
				++sub;
			}
			goto done; // found it!
		}
	next: ++s;
	}
	done: str = s;
}

void token::skip_after(char ch)
{
	const char* s = str;
	while (s < end) if (ch == *s++) break;
	str = s;
}

void token::skip_after(const char* substr, int len)
{
	char ch = *substr;		// starting char of the sequence
	const char* s = str;	// modify in register, no immediate writeout
	const char*const substrend = substr + len;
	while (s < end) {
		if (*s++ == ch) {
			const char* sub = substr + 1;
			while (sub != substrend) {
				if (*s++ != *sub) goto next;
				++sub;
			}
			goto done; // found it!
		}
	next: continue;
	}
	done: str = s;
}

token& token::tolower()
{
	for (char* s = (char*)str; s < end; ++s)
		*s = ::tolower(*s);
	return *this;
}
std::string token::aslower() const
{
	std::string ret;
	ret.reserve(int(end - str));
	for (char* s = (char*)str; s < end; ++s)
		ret.push_back(::tolower(*s));
	return ret;
}
char* token::aslower(char* dst) const
{
	char* p = dst;
	for (const char* s = str; s < end; ++s, ++p)
		*p = ::tolower(*s);
	*p = 0;
	return dst;
}

token& token::toupper()
{
	for (char* s = (char*)str; s < end; ++s)
		*s = ::toupper(*s);
	return *this;
}
std::string token::asupper() const
{
	std::string ret;
	ret.reserve(int(end - str));
	for (char* s = (char*)str; s < end; ++s)
		ret.push_back(::toupper(*s));
	return ret;
}
char* token::asupper(char* dst) const
{
	char* p = dst;
	for (const char* s = str; s < end; ++s, ++p)
		*p = ::toupper(*s);
	*p = 0;
	return dst;
}

token& token::replace(char chOld, char chNew)
{
	for (char* s = (char*)str; s < end; ++s)
		if (*s == chOld) *s = chNew;
	return *this;
}



///////////// KeyValueParser

bool LineParser::ReadLine(token& out)
{
	while (Buffer.next(out, '\n'))
	{
		// trim off trailing CR if it exists
		if (out.str != out.end && out.end[-1] == '\r')
			--out.end;
		return true;
	}
	return false; // no more lines
}

token LineParser::ReadLine()
{
	token out;
	while (Buffer.next(out, '\n'))
	{
		// trim off trailing CR if it exists
		if (out.str != out.end && out.end[-1] == '\r')
			--out.end;
		return out;
	}
	return out;
}



bool KeyValueParser::ReadLineCleaned(token& out)
{
	token line;
	while (Buffer.next(line, '\n'))
	{
		// ignore comment lines or empty newlines
		if (line.str[0] == ';' || line.str[0] == '\n' || line.str[0] == '\r')
			continue; // skip to next line

		if (line.is_whitespace()) // is it all whitespace line?
			continue; // skip to next line

		// now we trim this beast and turn it into a c_str
		line = line.trim_start().split_first(';');
		out = line.trim_end();
		return true;
	}
	return false; // no more lines
}
bool KeyValueParser::ReadNext(token& key, token& value)
{
	token line;
	while (ReadLineCleaned(line))
	{
		if (line.next(key, '='))
		{
			key.trim();
			if (line.next(value, '='))
				value.trim();
			else
				value.clear();
			return true;
		}
		return false;
	}
	return false;
}





///////////// BracketsParser

bool BracketsParser::ReadLine(token& key, token& value, int& depth)
{
	while (Buffer.next(value, "\r\n", 2)) // split by carriage return
	{
		char ch = value.trim_start()[0];
		if (value.empty() || ch == ';') // ignore empty newlines and comments
			continue; // next line plz

		if (ch == '{') // increase depth level on opened bracket
		{ 
			++depth; 
			continue; 
		}
		if (ch == '}')
		{ 
			if (--depth <= 0) 
				return false; // } and depth dropped to <= 0, we're done with parsing this section
			continue; 
		}
		
		value.next(key, " \t;", 3);
		value.trim_start(" \t", 2); // trim any leading whitespace
		return true; // OK a value was read
	}
	return false; // no more lines in Buffer
}
void BracketsParser::SkipToNext(int& currentdepth, int targetdepth)
{
	token line;
	while (Buffer.next(line, "\r\n", 2)) // split by carriage return
	{
		line = line.trim(); // trim the whole line
		if (line.empty()) // ignore empty newlines
			continue;

		char ch = line[0];
		if (ch == ';') // ignore comment lines
			continue;
		if (ch == '{') // increase depth level on opened bracket
		{ 
			if (++currentdepth == targetdepth)
				return; // return if file current depth level is equal to target depth level
		}
		else if (ch == '}')  // increase depth level on closed bracket but keeps reading if depth level is back to 0
		{ 
			if (--currentdepth < 0 || currentdepth == targetdepth)
				return; // return if file current depth level is equal to target depth level
		}
	}
}









/**
* @note Doesn't account for any special cases like +- NAN, E-9 etc. Just a very simple atof.
*		 Some optimizations applied to avoid too many multiplications
*		 This doesn't handle very big floats, max range is:
*		 -2147483648.0f to 2147483647.0f, or -0.0000000001f to 0.0000000001f
*		 Or the worst case:
*		 214748.0001f
*/
float _tofloat(const char* str, const char** end)
{
	int intPart, power = 1;
	bool negative = false;
	char ch = *str;

	if (ch == '-')
		negative = true, ++str; // change sign and skip '-'

	// do value part
	for (intPart = 0; '0' <= (ch = *str) && ch <= '9'; ++str)
		intPart = (intPart << 3) + (intPart << 1) + (ch - '0'); // intPart = intPart*10 + digit

	// @note The '.' is actually the sole reason for this function in the first place. Locale independence.
	if (ch == '.') { /* fraction part follows*/
		for (; '0' <= (ch = *++str) && ch <= '9'; ) {
			intPart = (intPart << 3) + (intPart << 1) + (ch - '0'); // intPart = intPart*10 + digit
			power = (power << 3) + (power << 1); // power *= 10
		}
	}

	if (end) *end = str;

	if (negative) intPart = -intPart; // twiddle sign
	return power == 1 ? float(intPart) : float(intPart) / float(power);
}

// optimized for simplicity and performance
int _toint(const char* str, const char** end)
{
	int intPart   = 0;
	bool negative = false;
	char ch       = *str;

	if (ch == '-')
		negative = true, ch = *++str; // change sign and skip '-'

	// do value part
	do {
		intPart = (intPart << 3) + (intPart << 1) + (ch - '0'); // intPart = intPart*10 + digit
	} while ('0' <= (ch = *++str) && ch <= '9');

	if (end) *end = str; // write end of parsed integer value
	if (negative) intPart = -intPart; // twiddle sign
	return intPart;
}

// optimized for simplicity and performance
// detects HEX integer strings as 0xBA or 0BA. Regular integers also parsed
int _tointhx(const char* str, const char** end)
{
	int intPart   = 0;
	bool negative = false;
	char ch       = *str;

	if (ch == '0') // hex string?
	{
		if (str[1] == 'x') ++str;
		for (;;) {
			int digit;
			if ('0' <= (ch = *++str) && ch <= '9') digit = ch - '0';
			else if ('A' <= ch && ch <= 'F')	 digit = ch - '7'; // hack 'A'-10 == '7'
			else if ('a' <= ch && ch <= 'f')	 digit = ch - 'W'; // hack 'a'-10 == 'W'
			else break; // invalid ch
			intPart = (intPart << 4) + digit; // intPart = intPart*16 + digit
		}
	}
	else // regular int string
	{
		if (ch == '-')
			negative = true, ch = *++str; // change sign and skip '-'
		do {
			intPart = (intPart << 3) + (intPart << 1) + (ch - '0'); // intPart = intPart*10 + digit
		} while ('0' <= (ch = *++str) && ch <= '9');
		if (negative) intPart = -intPart; // twiddle sign
	}

	if (end) *end = str; // write end of parsed integer value
	return intPart;
}


int _tostring(char* buffer, float f)
{
	int value = (int)f;
	f -= value;
	if (*(unsigned int*)&f & 0x80000000)
	if (f < 0.0f) f = -f;
	char* end = buffer + _tostring(buffer, value);

	if (f) // do we have a fraction ?
	{
		double cmp = 0.00001; // 6 decimal places max
		*end++ = '.'; // place the fraction mark
		double x = f; // floats go way imprecise when *=10, perhaps should extract mantissa instead?
		for (;;)
		{
			x *= 10;
			value = (int) x;
			*end++ = '0' + (value % 10);
			x -= value;
			if (x < cmp) // break from 0.750000011 cases
				break;
			cmp *= 10.0f;
		}
	}
	*end = '\0'; // null-terminate
	return (int) (end - buffer); // length of the string
}


int _tostring(char* buffer, int value)
{
	char* start;
	char* rev;
	char* end = buffer;

	if (value < 0) // if neg, abs and writeout '-'
	{
		value = -value;	// flip sign
		*end++ = '-';	// neg
	}
	start = end; // mark start for strrev after:
	
	do // writeout remainder of 10 + '0' while we still have value
	{
		*end++ = '0' + (value % 10);
		value /= 10;
	} while (value != 0);
	
	*end = '\0'; // always null-terminate

	// reverse the string:
	rev = end; // for strrev, we'll need a helper pointer
	while (start < rev)
	{
		char temp = *start;
		*start++ = *--rev;
		*rev = temp;
	}

	return (int)(end - buffer); // length of the string
}







char* tolower(char* str, int len)
{
	char* s = str;
	for (char* e = s + len; s < e; ++s) 
		*s = ::tolower(*s);
	return str;
}
char* toupper(char* str, int len)
{
	char* s = str;
	for (char* e = s + len; s < e; ++s) 
		*s = ::toupper(*s);
	return str;
}
std::string& tolower(std::string& str)
{
	char* s = (char*)str.data();
	for (char* e = s + str.length(); s < e; ++s) 
		*s = ::tolower(*s);
	return str;
}
std::string& toupper(std::string& str)
{
	char* s = (char*)str.data();
	for (char* e = s + str.length(); s < e; ++s) 
		*s = ::toupper(*s);
	return str;
}

char* replace(char* str, int len, char chOld, char chNew)
{
	char* s = (char*)str;
	for (char* e = s + len; s < e; ++s)
		if (*s == chOld) 
			*s = chNew;
	return str;
}

std::string& replace(std::string& str, char chOld, char chNew)
{
	char* s = (char*)str.c_str();
	for (char* e = s + str.length(); s < e; ++s)
		if (*s == chOld)
			*s = chNew;
	return str;
}