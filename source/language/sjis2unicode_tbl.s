# SJIS(CP932) -> UNICODE 変換テーブル指定	by Rudolph

	.rodata

	.globl sjis2unicode_tbl

	.balign 32

sjis2unicode_tbl:
	.incbin "../source/language/sjis2unicode.tbl"

