#pragma once
/**
 * String Tokenizer, Copyright (c) 2014 - Jorma Rebane
 */
#include <string>
#include <vector>
#include <iostream>

//// @note Some functions get inlined too aggressively, leading to some serious code bloat
////       Need to hint the compiler to take it easy ^_^'
#define NOINLINE __declspec(noinline) 

/**
* This is a simplified string tokenizer class.
*
* Those who are not familiar with string tokens - these are strings that don't actually
* hold nor control the string data. These strings are read-only and the only operations
* we can do are shifting the start/end pointers.
*
* That is how the token class is built and subsequently operations like trim()
* just shift the start/end pointers towards the middle.
* This appears to be extremely efficient when parsing large file buffers - instead of
* creating thousands of string objects, we tokenize substrings of the file buffer.
*
* The structures below contain methods for efficiently manipulating the token class.
*/




/**
 * C-locale specific, simplified atof that also outputs the end of parsed string
 * @param str Input string, eg "-0.25" / ".25", etc.. '+' is not accepted as part of the number
 * @param end[NULL] (optional) Destination pointer for end of parsed string. Can be NULL.
 * @return Parsed float
 */
float _tofloat(const char* str, const char** end = NULL);


/**
 * Fast locale agnostic atoi
 * @param str Input string, eg "-25" or "25", etc.. '+' is not accepted as part of the number
 * @param end[NULL] (optional) Destination pointer for end of parsed string. Can be NULL.
 * @return Parsed int
 */
int _toint(const char* str, const char** end = NULL);

/**
 * Fast locale agnostic atoi
 * @param str Input string, eg "-25" or "25", etc.. '+' is not accepted as part of the number
 *            HEX syntax is supported: 0xBA or 0BA will parse hex values instead of regular integers
 * @param end[NULL] (optional) Destination pointer for end of parsed string. Can be NULL.
 * @return Parsed int
 */
int _tointhx(const char* str, const char** end = NULL);


/**
 * C-locale specific, simplified ftoa that prints pretty human-readable floats
 * @param buffer Destination buffer assumed to be big enough. 32 bytes is more than enough.
 * @param value Float value to convert to string
 * @return Length of the string
 */
int _tostring(char* buffer, float value);

/**
 * Fast locale agnostic itoa
 * @param buffer Destination buffer assumed to be big enough. 16 bytes is more than enough.
 * @param value Integer value to convert to string
 * @return Length of the string
 */
int _tostring(char* buffer, int value);


struct token_use_hex {};
#define HEX token_use_hex()




struct token_vishelper // VC++ visualization helper
{
	const char* str;
	const char* end;
};




/**
* String token for efficient parsing.
* Represents a 'weak' reference string with Start/End pointers.
* The string can be parsed, manipulated and tokenized through methods like:
*  - trim(), trim_start(), trim_end(), toInt(), toFloat(), next(), skip_until(), skip_after(), ...
*/
struct token
{
	union {
		struct {
			const char* str; // start of string
			const char* end; // end of string
		};
		token_vishelper v;	// VC++ visualization helper
	};


	inline token() : str(""), end(str) {}
	template<int SIZE>
	inline token(const char(&str)[SIZE]) : str(str), end(str + SIZE - 1) {}
	template<int SIZE>
	inline token(char(&str)[SIZE]) : str(str), end(str + strlen(str)) {}
	inline token(const char* str, const char* end) : str(str), end(end) {}
	inline token(const char* str, size_t len) : str(str), end(str + len) {}
	
	// explicitly state you want the std::string overload - this is overload is not very desirable
	inline explicit token(const std::string& s) : str(s.c_str()), end(str + s.length()) {}
	inline char operator[](int index) const { return str[index]; }

	/** Creates a new string from this string-token */
	inline std::string& toString(std::string& out) const { out.assign(str, end - str); return out; }
	inline std::string toString() const { return std::string(str, end - str); }
	/** Parses this string token as an integer */
	inline int toInt() const { return _toint(str); }
	/** Parses this string token as an integer and also auto-detects 0xFF or 0FF as HEX strings */
	inline int toInt(token_use_hex unused) { return _tointhx(str); }
	/** Parses this string token as a long */
	inline long toLong() const { return (long)_toint(str); }
	/** Parses this string token as a float */
	inline float toFloat() const { return _tofloat(str); }
	/** Parses this string token as a double */
	inline double toDouble() const { return (double) _tofloat(str); }
	/** Parses this string token as a bool */
	bool toBool() const;

	/** Clears the string token */
	inline void clear() { str = "", end = str; }
	/** @return Length of the string */
	inline int length() const  { return int(end - str); }
	/** @return TRUE if length of the string is 0 - thus the string is empty */
	inline bool empty() const { return str == end; }
	/** @return TRUE if string is non-empty */
	inline operator bool() const { return str != end; }
	/** @return Pointer to the start of the string */
	inline const char* c_str() const { return str; }
	/** @return TRUE if the string token is only whitespace: " \t\r\n"  */
	NOINLINE bool is_whitespace();

	/** Trims the start of the string from any whitespace */
	NOINLINE token& trim_start();
	/** Trims the end of the string from any whitespace */
	NOINLINE token& trim_end();
	/** Trims start from this char */
	NOINLINE token& trim_start(char ch);
	/** Trims end from this char */
	NOINLINE token& trim_end(char ch);

	NOINLINE token& trim_start(const char* chars, int nchars);
	NOINLINE token& trim_end(const char* chars, int nchars);
	template<int SIZE> inline token& trim_start(const char(&chars)[SIZE])
	{
		return trim_start(chars, (SIZE - 1));
	}
	template<int SIZE> inline token& trim_end(const char(&chars)[SIZE])
	{
		return trim_end(chars, (SIZE - 1));
	}

	/** Trims both start and end with whitespace */
	inline token& trim() { return trim_start().trim_end(); }
	/** Trims both start and end width this char*/
	inline token& trim(char ch) { return trim_start(ch).trim_end(ch); }
	/** Trims both start and end with any of the given chars */
	template<int SIZE> inline token& trim(const char(&chars)[SIZE])
	{
		return trim_start(chars, (SIZE - 1)).trim_end(chars, (SIZE - 1));
	}
	inline token& trim(const char* chars, int len)
	{
		return trim_start(chars, len).trim_end(chars, len);
	}

	/** Consumes the first character in the Token String if possible. */
	inline token& chomp_first() { if (str != end) ++str; return *this; }
	/** Consumes the last character in the Token String if possible. */
	inline token& chomp_last() { if (str != end) --end; return *this; }

	/** Consumes the first COUNT characters in the Token String if possible. */
	inline token& chomp_first(int count) { for (int n = count; n && str != end; --n, ++str); return *this; }
	/** Consumes the last COUNT characters in the Token String if possible. */
	inline token& chomp_last(int count) { for (int n = count; n && str != end; --n, --str); return *this; }

	/** @return TRUE if the string token contains this char */
	inline bool contains(char c) const { return memchr(str, c, end - str) ? true : false; }
	/** @return TRUE if the string token contains any of the chars */
	bool contains(const char* chars, int nchars) const;
	template<int SIZE> inline bool contains(const char(&chars)[SIZE]) const { return contains(chars, SIZE - 1); }

	/** @return Pointer to char if found, NULL otherwise */
	inline const char* find(char c) const { return (const char*)memchr(str, c, end - str); }
	/** @return Pointer to start of substring if found, NULL otherwise */
	const char* find(const char* substr, int len) const;
	template<int SIZE> inline const char* find(const char(&substr)[SIZE]) const { return find(substr, SIZE - 1); }
	inline const char* find(const token& str) const { return find(str.str, str.end - str.str); }

	/** @return TRUE if the string token starts with this string */
	template<int SIZE> inline bool starts_with(const char(&s)[SIZE]) const
	{
		return int(end - str) >= (SIZE - 1) && memcmp(str, s, SIZE - 1) == 0;
	}
	/** @return TRUE if the string token starts with IGNORECASE this string */
	template<int SIZE> inline bool starts_withi(const char(&s)[SIZE]) const
	{
		return int(end - str) >= (SIZE - 1) && _memicmp(str, s, SIZE - 1) == 0;
	}
	/** @return TRUE if the string token starts with this string */
	inline bool starts_with(const char* s, int length) const
	{
		return int(end - str) >= length && memcmp(str, s, length) == 0;
	}
	/** @return TRUE if the string token starts with IGNORECASE this string */
	inline bool starts_withi(const char* s, int length) const
	{
		return int(end - str) >= length && _memicmp(str, s, length) == 0;
	}
	/** @return TRUE if the string token starts with this string */
	inline bool starts_with(const std::string& s) const
	{
		int length = s.length();
		return int(end - str) >= length && memcmp(str, s.c_str(), length) == 0;
	}
	/** @return TRUE if the string token starts with IGNORECASE this string */
	inline bool starts_withi(const std::string& s) const
	{
		int length = s.length();
		return int(end - str) >= length && _memicmp(str, s.c_str(), length) == 0;
	}
	/** @return TRUE if the string token starts with this string */
	inline bool starts_with(const token& s) const
	{
		int length = int(s.end - s.str);
		return int(end - str) >= length && memcmp(str, s.str, length) == 0;
	}
	/** @return TRUE if the string token starts with IGNORECASE this string */
	inline bool starts_withi(const token& s) const
	{
		int length = int(s.end - s.str);
		return int(end - str) >= length && _memicmp(str, s.str, length) == 0;
	}


	/** @return TRUE if the string token ends with this string */
	template<int SIZE> inline bool ends_with(const char(&s)[SIZE]) const
	{
		const int this_len = int(end - str);
		const int str_len = SIZE - 1;
		return this_len >= str_len && memcmp(str + this_len - str_len, s, str_len) == 0;
	}
	/** @return TRUE if the string token ends with IGNORECASE this string */
	template<int SIZE> inline bool ends_withi(const char(&s)[SIZE]) const
	{
		const int this_len = int(end - str);
		const int str_len = SIZE - 1;
		return this_len >= str_len && _memicmp(str + this_len - str_len, s, str_len) == 0;
	}
	/** @return TRUE if the string token ends with this string */
	inline bool ends_with(const char* s, const int str_len) const
	{
		const int this_len = int(end - str);
		return this_len >= str_len && memcmp(str + this_len - str_len, s, str_len) == 0;
	}
	/** @return TRUE if the string token ends with IGNORECASE this string */
	inline bool ends_withi(const char* s, int str_len) const
	{
		const int this_len = int(end - str);
		return this_len >= str_len && _memicmp(str + this_len - str_len, s, str_len) == 0;
	}
	/** @return TRUE if the string token ends with this string */
	inline bool ends_with(const std::string& s) const
	{
		const int this_len = int(end - str);
		const int str_len = s.length();
		return this_len >= str_len && memcmp(str + this_len - str_len, s.c_str(), str_len) == 0;
	}
	/** @return TRUE if the string token ends with IGNORECASE this string */
	inline bool ends_withi(const std::string& s) const
	{
		const int this_len = int(end - str);
		const int str_len = s.length();
		return this_len >= str_len && _memicmp(str + this_len - str_len, s.c_str(), str_len) == 0;
	}



	/** @return TRUE if the string token equals this string */
	template<int SIZE> inline bool equals(const char(&s)[SIZE]) const
	{
		return int(end - str) == (SIZE - 1) && memcmp(str, s, SIZE - 1) == 0;
	}
	/** @return TRUE if the string token equals IGNORECASE this string */
	template<int SIZE> inline bool equalsi(const char(&s)[SIZE]) const
	{
		return int(end - str) == (SIZE - 1) && _memicmp(str, s, SIZE - 1) == 0;
	}
	/** @return TRUE if the string token equals this string */
	inline bool equals(const char* s, int length) const
	{
		return int(end - str) == length && memcmp(str, s, length) == 0;
	}
	/** @return TRUE if the string token equals IGNORECASE this string */
	inline bool equalsi(const char* s, int length) const
	{
		return int(end - str) == length && _memicmp(str, s, length) == 0;
	}
	/** @return TRUE if the string token equals this string */
	inline bool equals(const std::string& s) const
	{
		int length = s.length();
		return int(end - str) == length && memcmp(str, s.c_str(), length) == 0;
	}
	/** @return TRUE if the string token equals IGNORECASE this string */
	inline bool equalsi(const std::string& s) const
	{
		int length = s.length();
		return int(end - str) == length && _memicmp(str, s.c_str(), length) == 0;
	}
	/** @return TRUE if the string token equals this string */
	inline bool equals(const token& s) const
	{
		int length = s.length();
		return int(end - str) == length && memcmp(str, s.c_str(), length) == 0;
	}
	/** @return TRUE if the string token equals IGNORECASE this string */
	inline bool equalsi(const token& s) const
	{
		int length = s.length();
		return int(end - str) == length && _memicmp(str, s.c_str(), length) == 0;
	}



	/** @return TRUE if the string token equals this string */
	template<int SIZE> inline bool operator==(const char(&s)[SIZE]) const
	{
		return int(end - str) == (SIZE - 1) && memcmp(str, s, SIZE - 1) == 0;
	}
	/** @return TRUE if the string token does NOT equal this string */
	template<int SIZE> inline bool operator!=(const char(&s)[SIZE]) const
	{
		return int(end - str) != (SIZE - 1) || memcmp(str, s, SIZE - 1) != 0;
	}
	/** @return TRUE if the string token equals this string */
	template<int SIZE> inline bool operator==(const std::string& s) const
	{
		int length = s.length();
		return int(end - str) == length && memcmp(str, s.c_str(), length) == 0;
	}
	/** @return TRUE if the string token does NOT equal this string */
	template<int SIZE> inline bool operator!=(const std::string& s) const
	{
		int length = s.length();
		return int(end - str) == length && memcmp(str, s.c_str(), length) != 0;
	}
	/** @return TRUE if the string token equals this string */
	template<int SIZE> inline bool operator==(const token& s) const
	{
		int length = s.length();
		return int(end - str) == length && memcmp(str, s.c_str(), length) == 0;
	}
	/** @return TRUE if the string token does NOT equal this string */
	template<int SIZE> inline bool operator!=(const token& s) const
	{
		int length = s.length();
		return int(end - str) == length && memcmp(str, s.c_str(), length) != 0;
	}



	/**
	* Splits the string into TWO and returns token to the first one
	* @param delim Delimiter char to split on
	*/
	NOINLINE token split_first(char delim);
	/**
	* Splits the string into TWO and returns token to the second one
	* @param delim Delimiter char to split on
	*/
	NOINLINE token split_second(char delim);
	/**
	* Splits the string at given delimiter values and trims each split with the specified trim chars.
	* @param out Output split and trimmed strings
	* @param delim Delimiter char to split on [default ' ']
	* @param trimChars Chars to trim on the split strings (optional)
	* @return Number of split strings (at least 1)
	*/
	NOINLINE int split(std::vector<token>& out, char delim = ' ', const char* trimChars = 0);
	/**
	* Splits the string at given delimiter values and trims each split with the specified trim chars.
	* @param out Output split and trimmed strings
	* @param delims Delimiter chars to split on. ex: " \r\t" splits on 3 chars
	* @param trimChars Chars to trim on the split strings (optional)
	* @return Number of split strings (at least 1)
	*/
	NOINLINE int split(std::vector<token>& out, const char* delims, const char* trimChars = 0);

	/**
	* Gets the next string token; also advances the ptr to next token.
	* @param out Resulting string token. Only valid if result is TRUE.
	* @param delim Delimiter char between string tokens
	* @return TRUE if a token was returned, FALSE if no more tokens (no token [out]).
	*/
	NOINLINE bool next(token& out, char delim);
	/**
	* Gets the next string token; also advances the ptr to next token.
	* @param out Resulting string token. Only valid if result is TRUE.
	* @param delims Delimiter characters between string tokens
	* @param ndelims Number of delimiters in the delims string to consider
	* @return TRUE if a token was returned, FALSE if no more tokens (no token [out]).
	*/
	NOINLINE bool next(token& out, const char* delims, int ndelims);
	/**
	* Gets the next string token; also advances the ptr to next token.
	* @param out Resulting string token. Only valid if result is TRUE.
	* @param delims Delimiter characters between string tokens
	* @return TRUE if a token was returned, FALSE if no more tokens (no token [out]).
	*/
	template<int SIZE> inline bool next(token& out, const char(&delims)[SIZE])
	{
		return next(out, delims, (SIZE - 1));
	}
	/**
	 * Same as bool next(token& out, char delim), but returns a token instead
	 */
	inline token next(char delim)
	{
		token out; next(out, delim); return out;
	}
	inline token next(const char* delim, int ndelims)
	{
		token out; next(out, delim, ndelims); return out;
	}
	template<int SIZE> inline token next(const char (&delims)[SIZE])
	{
		token out; next(out, delims, SIZE-1); return out;
	}

	/**
	* Gets the next string token; stops buffer on the identified delimiter.
	* @param out Resulting string token. Only valid if result is TRUE.
	* @param delim Delimiter char between string tokens
	* @return TRUE if a token was returned, FALSE if no more tokens (no token [out]).
	*/
	NOINLINE bool next_notrim(token& out, char delim);
	/**
	* Gets the next string token; stops buffer on the identified delimiter.
	* @param out Resulting string token. Only valid if result is TRUE.
	* @param delims Delimiter characters between string tokens
	* @param ndelims Number of delimiters in the delims string to consider
	* @return TRUE if a token was returned, FALSE if no more tokens (no token [out]).
	*/
	NOINLINE bool next_notrim(token& out, const char* delims, int ndelims);
	/**
	* Gets the next string token; stops buffer on the identified delimiter.
	* @param out Resulting string token. Only valid if result is TRUE.
	* @param delims Delimiter characters between string tokens
	* @return TRUE if a token was returned, FALSE if no more tokens (no token [out]).
	*/
	template<int SIZE> inline bool next_notrim(token& out, const char(&delims)[SIZE])
	{
		return next_notrim(out, delims, (SIZE - 1));
	}
	/**
	 * Same as bool next(token& out, char delim), but returns a token instead
	 */
	inline token next_notrim(char delim)
	{
		token out; next_notrim(out, delim); return out;
	}

	/**
	 * Parses next float from current Token, example: "1.0;sad0.0,'as;2.0" will parse [1.0] [0.0] [2.0]
	 * @return 0.0f if there's nothing to parse or a parsed float
	 */
	NOINLINE float nextFloat();

	/** 
	 * Parses next int from current Token, example: "1,asvc2,x*3" will parse [1] [2] [3]
	 * @return 0 if there's nothing to parse or a parsed int
	 */
	NOINLINE int nextInt();


	/**
	 * Skips start of the string until the specified character is found or end of string is reached.
	 * @param ch Character to skip until
	 */
	NOINLINE void skip_until(char ch);

	/**
	 * Skips start of the string until the specified substring is found or end of string is reached.
	 * @param substr Substring to skip until
	 * @param len Length of the substring
	 */
	NOINLINE void skip_until(const char* substr, int len);

	/**
	 * Skips start of the string until the specified substring is found or end of string is reached.
	 * @param substr Substring to skip until
	 * @param len Length of the substring
	 */
	template<int SIZE> inline void skip_until(const char(&substr)[SIZE])
	{
		skip_until(substr, SIZE - 1);
	}


	/**
	 * Skips start of the string until the specified character is found or end of string is reached.
	 * The specified chracter itself is consumed.
	 * @param ch Character to skip after
	 */
	NOINLINE void skip_after(char ch);

	/**
	 * Skips start of the string until the specified substring is found or end of string is reached.
	 * The specified substring itself is consumed.
	 * @param substr Substring to skip after
	 * @param len Length of the substring
	 */
	NOINLINE void skip_after(const char* substr, int len);

	/**
	 * Skips start of the string until the specified substring is found or end of string is reached.
	 * The specified substring itself is consumed.
	 * @param substr Substring to skip after
	 * @param len Length of the substring
	 */
	template<int SIZE> inline void skip_after(const char(&substr)[SIZE])
	{
		skip_after(substr, SIZE - 1);
	}

	/**
	 * Modifies the target string to lowercase
	 * @warning The const char* will be recasted and modified!
	 */
	NOINLINE token& tolower();

	/**
	 * Creates a copy of this token that is in lowercase
	 */
	NOINLINE std::string aslower() const;

	/**
	 * Creates a copy of this token that is in lowercase
	 */
	NOINLINE char* aslower(char* dst) const;

	/**
	 * Modifies the target string to be UPPERCASE
	 * @warning The const char* will be recasted and modified!
	 */
	NOINLINE token& toupper();

	/**
	 * Creates a copy of this token that is in UPPERCASE
	 */
	NOINLINE std::string asupper() const;

	/**
	 * Creates a copy of this token that is in UPPERCASE
	 */
	NOINLINE char* asupper(char* dst) const;

	/**
	 * Modifies the target string by replacing all chOld
	 * occurrences with chNew
	 * @warning The const char* will be recasted and modified!
	 * @param chOld The old character to replace
	 * @param chNew The new character
	 */
	NOINLINE token& replace(char chOld, char chNew);
};


inline token& operator>>(token& token, float& out)
{
	out = token.nextFloat();
	return token;
}
inline token& operator>>(token& token, int& out)
{
	out = token.nextInt();
	return token;
}
inline token& operator>>(token& token, unsigned& out)
{
	out = token.nextInt();
	return token;
}

inline std::ostream& operator<<(std::ostream& stream, const token& token)
{
	return stream.write(token.c_str(), token.length());
}




/**
 * A POD version of token for use in unions
 */
struct token_
{
	union {
		struct {
			const char* str;
			const char* end;
		};
		token_vishelper v;	// VC++ visualization helper
	};
	inline operator token()				 { return token(str, end); }
	inline operator token&()			 { return *(token*)this;   }
	inline operator const token&() const { return *(token*)this;   }
	inline token* operator->() const	 { return  (token*)this;   }
};






/**
* Parses an input string buffer for individual lines
* The line is returned trimmed of any \r or \n
*
*  This is also an example on how to implement your own custom parsers using the token structure
*/
struct LineParser
{
	token Buffer;
	inline LineParser(const token& buffer)			 : Buffer(buffer)			 {}
	inline LineParser(const char* data, size_t size) : Buffer(data, data + size) {}

	/**
	* Reads next line from the base buffer and advances its pointers.
	* The line is returned trimmed of any \r or \n. Empty lines are not skipped.
	*
	* @param out The output line that is read. Only valid if TRUE is returned.
	* @return Reads the next line. If no more lines, FALSE is returned.
	**/
	NOINLINE bool ReadLine(token& out);

	// same as Readline(token&), but returns a token object instead of a bool
	NOINLINE token ReadLine();
};




/**
* Parses an input string buffer for 'Key=Value' pairs.
* The pairs are returned one by one with 'ReadNext'.
*
* This is also an example on how to implement your own custom parsers using the token structure
*/
class KeyValueParser
{
	token Buffer;
public:
	inline KeyValueParser(const token& buffer) : Buffer(buffer) {}
	inline KeyValueParser(const char* data, size_t size) : Buffer(data, data + size) {}

	/**
	* Reads next line from the base buffer and advances its pointers.
	* The line is returned trimmed of any \r or \n.
	* Empty or whitespace lines are skipped.
	* Comment lines starting with ; are skipped.
	* Comments at the end of a line are trimmed off.
	*
	* @param out The output line that is read. Only valid if TRUE is returned.
	* @return Reads the next line. If no more lines, FALSE is returned.
	*/
	NOINLINE bool ReadLineCleaned(token& out);

	/**
	* Reads the next key-value pair from the buffer and advances its position
	* @param key Resulting key (only valid if return value is TRUE)
	* @param value Resulting value (only valid if return value is TRUE)
	* @return TRUE if a Key-Value pair was parsed
	*/
	NOINLINE bool ReadNext(token& key, token& value);
};




/**
* Parses an input string buffer for balanced-parentheses structures
* The lines are returned one by one with 'ReadLine' together with the corrisponding depth level.
*/
class BracketsParser
{
	token Buffer;
public:
	inline BracketsParser(const token& buffer) : Buffer(buffer) {}
	inline BracketsParser(const char* data, size_t size) : Buffer(data, data + size) {}

	/**
	* Reads the next line from the buffer and advances its position
	* @param key Resulting line key (only valid if return value is TRUE)
	* @param value Resulting line value (only valid if return value is TRUE)
	* @param depth Resulting depth level (only valid if return value is TRUE)
	* @return TRUE if a line was parsed
	*/
	NOINLINE bool ReadLine(token& key, token& value, int& depth);

	/**
	* Skips the buffer lines until it reaches the targeted depth level
	* @param currentdepth Resulting depth level (only valid if return value is TRUE)
	* @param targetdepth Targeted depth level
	*/
	NOINLINE void SkipToNext(int& currentdepth, int targetdepth);
};



/**
 * Converts a string into its lowercase form
 */
char* tolower(char* str, int len);

/**
 * Converts a string into its uppercase form
 */
char* toupper(char* str, int len);

/**
 * Converts an std::string into its lowercase form
 */
std::string& tolower(std::string& str);

/**
 * Converts an std::string into its uppercase form
 */
std::string& toupper(std::string& str);

/**
 * Replaces characters of 'chOld' with 'chNew' inside the specified string
 */
char* replace(char* str, int len, char chOld, char chNew);

/**
 * Replaces characters of 'chOld' with 'chNew' inside this std::string
 */
std::string& replace(std::string& str, char chOld, char chNew);