#include "wstring.hpp"

using namespace std;

wString::wString(const wchar_t *s) :
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >(s)
{
}

wString::wString(const basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t> > &ws) :
	basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t> >(ws)
{
}

wString::wString(const string &s)
{
	std::string::size_type size;

	size = s.size();
	resize(size);
	for (std::string::size_type i = 0; i < size; ++i)
		(*this)[i] = (unsigned char)s[i];
}

wString &wString::operator=(const string &s)
{
	std::string::size_type size;

	size = s.size();
	this->resize(size);
	for (std::string::size_type i = 0; i < size; ++i)
		(*this)[i] = (unsigned char)s[i];
	return *this;
}

void wString::fromUTF8(const char *s)
{
	size_t len = utf8Len(s);

	clear();
	if (len == 0)
		return;
	reserve(len);
	for (int i = 0; s[i] != 0; )
	{
		if ((s[i] & 0xF8) == 0xF0)
		{
			push_back(((wchar_t)(s[i] & 0x07) << 18) | ((wchar_t)(s[i + 1] & 0x3F) << 12) | ((wchar_t)(s[i + 2] & 0x3F) << 6) | (wchar_t)(s[i + 3] & 0x3F));
			i += 4;
		}
		else if ((s[i] & 0xF0) == 0xE0)
		{
			push_back(((wchar_t)(s[i] & 0x0F) << 12) | ((wchar_t)(s[i + 1] & 0x3F) << 6) | (wchar_t)(s[i + 2] & 0x3F));
			i += 3;
		}
		else if ((s[i] & 0xE0) == 0xC0)
		{
			push_back(((wchar_t)(s[i] & 0x1F) << 6) | (wchar_t)(s[i + 1] & 0x3F));
			i += 2;
		}
		else
		{
			push_back((wchar_t)s[i]);
			++i;
		}
	}
}

string wString::toUTF8(void) const
{
	string s;
	size_t len = 0;
	wchar_t wc;

	for (size_t i = 0; i < size(); ++i)
	{
		wc = operator[](i);
		if (wc < 0x80)
			++len;
		else if (wc < 0x800)
			len += 2;
		else if (wc < 0x10000)
			len += 3;
		else
			len += 4;
	}
	s.reserve(len);
	for (size_t i = 0; i < size(); ++i)
	{
		wc = operator[](i);
		if (wc < 0x80)
			s.push_back((char)wc);
		else if (wc < 0x800)
		{
			s.push_back((char)((wc >> 6) | 0xC0));
			s.push_back((char)((wc & 0x3F) | 0x80));
		}
		else if (wc < 0x10000)
		{
			s.push_back((char)((wc >> 12) | 0xE0));
			s.push_back((char)(((wc >> 6) & 0x3F) | 0x80));
			s.push_back((char)((wc & 0x3F) | 0x80));
		}
		else
		{
			s.push_back((char)(((wc >> 18) & 0x07) | 0xF0));
			s.push_back((char)(((wc >> 12) & 0x3F) | 0x80));
			s.push_back((char)(((wc >> 6) & 0x3F) | 0x80));
			s.push_back((char)((wc & 0x3F) | 0x80));
		}
	}
	return s;
}

size_t utf8Len(const char *s)
{
	size_t len = 0;

	for (int i = 0; s[i] != 0; )
	{
		if ((s[i] & 0xF8) == 0xF0)
		{
			if (((s[i + 1] & 0xC0) != 0x80) || ((s[i + 2] & 0xC0) != 0x80) || ((s[i + 3] & 0xC0) != 0x80))
				return 0;
			++len;
			i += 4;
		}
		else if ((s[i] & 0xF0) == 0xE0)
		{
			if (((s[i + 1] & 0xC0) != 0x80) || ((s[i + 2] & 0xC0) != 0x80))
				return 0;
			++len;
			i += 3;
		}
		else if ((s[i] & 0xE0) == 0xC0)
		{
			if (((s[i + 1] & 0xC0) != 0x80))
				return 0;
			++len;
			i += 2;
		}
		else if ((s[i] & 0x80) == 0x00)
		{
			++len;
			++i;
		}
		else
			return 0;
	}
	return len;
}
