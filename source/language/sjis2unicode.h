////////////////////////////////////////////
// SJIS(CP932)/UTF-8 -> UNICODE
//	2009/05/02	by Rudolph
////////////////////////////////////////////

extern const u8 sjis2unicode_tbl[];

static bool _isKanji1(wchar_t ch)
{
	if((ch >= (wchar_t)0x81) && (ch <= (wchar_t)0x9F))
		return true;
	if((ch >= (wchar_t)0xE0) && (ch <= (wchar_t)0xEF))
		return true;
	if((ch >= (wchar_t)0xFA) && (ch <= (wchar_t)0xFB))	// JIS X 0218‘¼ IBMŠg’£•¶Žš (0xFA40-0xFC4B)
		return true;
	return false;
}

static void _sjis2unicode(char *src, wchar_t *dest)
{
	wchar_t lc = 0;
	int	bt;


	bt = mbstowcs(dest, src, strlen(src));	// UTF-8 to UTF-16
	if(bt > 0) {
		dest[bt] = (wchar_t)'\0';
		return;
	}

	while(*src) {
		lc = (wchar_t)*src;
		src++;

		if(_isKanji1(lc) && *src) {
			lc = (lc << 8) + ((wchar_t)*src);
			src++;
		}

		if(lc < ((wchar_t)(sjis2unicode_tbl[3]) << 8) + (wchar_t)(sjis2unicode_tbl[2])) {
			*dest = (((wchar_t)(sjis2unicode_tbl[lc*2+5]) << 8)) + (wchar_t)(sjis2unicode_tbl[lc*2+4]);
			if(*dest == (wchar_t)'\0') 
				*dest = (wchar_t)'?';
		} else	*dest = (wchar_t)'?';
		dest++;
	}
	*dest = (wchar_t)'\0';
	return;
}
