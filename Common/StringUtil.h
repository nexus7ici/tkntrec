#ifndef INCLUDE_STRING_UTIL_H
#define INCLUDE_STRING_UTIL_H

void Format(string& strBuff, PRINTF_FORMAT_SZ const char *format, ...);

void Format(wstring& strBuff, PRINTF_FORMAT_SZ const WCHAR *format, ...);

void Replace(string& strBuff, const string& strOld, const string& strNew); //alias-safe

void Replace(wstring& strBuff, const wstring& strOld, const wstring& strNew); //alias-safe

//convert to CP932
void WtoA(const wstring& strIn, string& strOut);
//convert to CP932
size_t WtoA(const WCHAR *in, size_t inLenHint, vector<char>& out);

void WtoUTF8(const wstring& strIn, string& strOut);
size_t WtoUTF8(const WCHAR *in, size_t inLenHint, vector<char>& out);

//convert from CP932
void AtoW(const string& strIn, wstring& strOut);
//convert from CP932
size_t AtoW(const char *in, size_t inLenHint, vector<WCHAR>& out);

void UTF8toW(const string& strIn, wstring& strOut);
size_t UTF8toW(const char *in, size_t inLenHint, vector<WCHAR>& out);

BOOL Separate(const string& strIn, const char *sep, string& strLeft, string& strRight); //alias-safe
inline BOOL Separate(const string& strIn, const string& strSep, string& strLeft, string& strRight){ return Separate(strIn, strSep.c_str(), strLeft, strRight); }

BOOL Separate(const wstring& strIn, const WCHAR *sep, wstring& strLeft, wstring& strRight); //alias-safe
inline BOOL Separate(const wstring& strIn, const wstring& strSep, wstring& strLeft, wstring& strRight){ return Separate(strIn, strSep.c_str(), strLeft, strRight); }

int CompareNoCase(const string& str1, const char *str2);
inline int CompareNoCase(const string& str1, const string& str2) { return CompareNoCase(str1, str2.c_str()); }

int CompareNoCase(const wstring& str1, const WCHAR *str2);
inline int CompareNoCase(const wstring& str1, const wstring& str2) { return CompareNoCase(str1, str2.c_str()); }

#endif
