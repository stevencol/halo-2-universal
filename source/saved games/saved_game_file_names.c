/*
SAVED_GAME_FILE_NAMES.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The name that a file of each type is given when the player gives none, in
each language of the game. The game has a function for each type, which is
a switch of the language: the languages that are not there get the English
name. The ninth language of the game's text, Portuguese, is in all the
switches, and in all it gets the English name: it is not told apart here. The names of the file and of the functions are guesses.
*/

#include "saved_game_files.h"

#include <stddef.h>

/* ---------- constants */

enum
{
	SAVED_GAME_FILE_DEFAULT_NAME_LENGTH= 256,
	SAVED_GAME_FILE_NAME_MAXIMUM_LENGTH= 255,

	/* a text that is none of the types' */
	_saved_game_file_unknown_string_id= 0x1B0006E8
};

/* ---------- prototypes of functions of the C library */

/* 0x003207ed: of characters of 16 bits */
wchar *wcsncpy(wchar *destination, const wchar *source, size_t count);

/* ---------- code */

/* 0x00217620 */
const wchar *saved_game_file_default_name_profile(
	long language)
{
	switch (language)
	{
	case _language_japanese:
		return u"\u30d7\u30ec\u30a4\u30e4\u30fc";
	case _language_german:
		return u"Profil";
	case _language_french:
		return u"Profil";
	case _language_spanish:
		return u"Perfil";
	case _language_italian:
		return u"Profilo";
	case _language_korean:
		return u"\ud504\ub85c\ud544";
	case _language_chinese:
		return u"\u8a2d\u5b9a\u6a94";
	}

	return u"Profile";
}

/* 0x00217680 */
const wchar *saved_game_file_default_name_slayer(
	long language)
{
	switch (language)
	{
	case _language_german:
		return u"Showdown";
	case _language_french:
		return u"Assassin";
	case _language_spanish:
		return u"Asesino";
	case _language_italian:
		return u"Massacro";
	case _language_chinese:
		return u"\u6bba\u622e\u4e4b\u738b";
	}

	return u"Slayer";
}

/* 0x002176e0 */
const wchar *saved_game_file_default_name_koth(
	long language)
{
	switch (language)
	{
	case _language_german:
		return u"H\u00fcgelk\u00f6nig";
	case _language_french:
		return u"Roi de la colline";
	case _language_spanish:
		return u"Rey de la colina";
	case _language_italian:
		return u"Re della collina";
	case _language_chinese:
		return u"\u5c71\u4e18\u4e4b\u738b";
	}

	return u"King of the Hill";
}

/* 0x00217740 */
const wchar *saved_game_file_default_name_race(
	long language)
{
	switch (language)
	{
	case _language_german:
		return u"Rennen";
	case _language_french:
		return u"Course";
	case _language_spanish:
		return u"Carrera";
	case _language_italian:
		return u"Corsa";
	case _language_chinese:
		return u"\u7af6\u901f\u4e4b\u738b";
	}

	return u"Race";
}

/* 0x002177a0 */
const wchar *saved_game_file_default_name_oddball(
	long language)
{
	switch (language)
	{
	case _language_french:
		return u"Cr\u00e2ne";
	case _language_spanish:
		return u"Bola loca";
	case _language_italian:
		return u"Teschio";
	case _language_chinese:
		return u"\u4fdd\u885b\u4e4b\u738b";
	}

	return u"Oddball";
}

/* 0x002177f0 */
const wchar *saved_game_file_default_name_juggernaut(
	long language)
{
	switch (language)
	{
	case _language_german:
		return u"Moloch";
	case _language_french:
		return u"Mastodonte";
	case _language_chinese:
		return u"\u6bc0\u6ec5\u4e4b\u738b";
	}

	return u"Juggernaut";
}

/* 0x00217840 */
const wchar *saved_game_file_default_name_headhunter(
	long language)
{
	switch (language)
	{
	case _language_german:
		return u"Kopfj\u00e4ger";
	case _language_french:
		return u"Chasseur de t\u00eates";
	case _language_spanish:
		return u"Cazarrecompensas";
	case _language_italian:
		return u"Cacciatore di teste";
	case _language_chinese:
		return u"\u7375\u982d\u4e4b\u738b";
	}

	return u"Headhunter";
}

/* 0x002178a0 */
const wchar *saved_game_file_default_name_ctf(
	long language)
{
	switch (language)
	{
	case _language_german:
		return u"Kampf um die Flagge";
	case _language_french:
		return u"Capture du drapeau";
	case _language_spanish:
		return u"Capturar la bandera";
	case _language_italian:
		return u"Conquista la bandiera";
	case _language_chinese:
		return u"\u6436\u65d7\u4e4b\u738b";
	}

	return u"Capture the Flag";
}

/* 0x00217900 */
const wchar *saved_game_file_default_name_assault(
	long language)
{
	switch (language)
	{
	case _language_german:
		return u"Angriff";
	case _language_french:
		return u"Assaut";
	case _language_spanish:
		return u"Asalto";
	case _language_italian:
		return u"Assalto";
	case _language_chinese:
		return u"\u7a81\u64ca\u4e4b\u738b";
	}

	return u"Assault";
}

/* 0x00217960 */
const wchar *saved_game_file_default_name_territories(
	long language)
{
	switch (language)
	{
	case _language_german:
		return u"Territorien";
	case _language_french:
		return u"Territoires";
	case _language_spanish:
		return u"Territorios";
	case _language_italian:
		return u"Territori";
	case _language_chinese:
		return u"\u9818\u571f\u4e4b\u738b";
	}

	return u"Territories";
}

/* 0x002179c0 */
const wchar *saved_game_file_default_name_playlist(
	long language)
{
	switch (language)
	{
	case _language_japanese:
		return u"\u30d7\u30ec\u30a4\u30ea\u30b9\u30c8";
	case _language_german:
		return u"Spielliste";
	case _language_french:
		return u"Liste";
	case _language_spanish:
		return u"Lista de juego";
	case _language_italian:
		return u"Elenco partite";
	case _language_korean:
		return u"\uac8c\uc784 \ubaa9\ub85d";
	case _language_chinese:
		return u"\u904a\u6232\u6e05\u55ae";
	}

	return u"Playlist";
}

/* 0x00217a20 */
const wchar *saved_game_file_default_name_unknown(
	long language)
{
	switch (language)
	{
	case _language_japanese:
		return u"Player";
	case _language_german:
		return u"Standard";
	case _language_french:
		return u"Par d\u00e9faut";
	case _language_spanish:
		return u"Predeterminada";
	case _language_italian:
		return u"Predefinito";
	case _language_chinese:
		return u"\u9810\u8a2d";
	}

	return u"Default";
}

/* 0x00217a80: the name that a file is given when the player gives none: by
the text that names its type, in a language. Empty if the text is of no
type */
void saved_game_file_get_default_name(
	unsigned long string_id,
	long language,
	wchar *name)
{
	const wchar *text= u"";

	switch (string_id)
	{
	case 0x1B0006E9:
		text= saved_game_file_default_name_profile(language);
		break;
	case 0x130006EA:
		text= saved_game_file_default_name_slayer(language);
		break;
	case 0x110006EB:
		text= saved_game_file_default_name_koth(language);
		break;
	case 0x110006EC:
		text= saved_game_file_default_name_race(language);
		break;
	case 0x140006ED:
		text= saved_game_file_default_name_oddball(language);
		break;
	case 0x170006EE:
		text= saved_game_file_default_name_juggernaut(language);
		break;
	case 0x170006EF:
		text= saved_game_file_default_name_headhunter(language);
		break;
	case 0x100006F0:
		text= saved_game_file_default_name_ctf(language);
		break;
	case 0x140006F1:
		text= saved_game_file_default_name_assault(language);
		break;
	case 0x180006F2:
		text= saved_game_file_default_name_territories(language);
		break;
	case 0x150006F3:
		text= saved_game_file_default_name_playlist(language);
		break;
	case _saved_game_file_unknown_string_id:
		text= saved_game_file_default_name_unknown(language);
		break;
	}
	wcsncpy(name, text, SAVED_GAME_FILE_DEFAULT_NAME_LENGTH-1);
	name[SAVED_GAME_FILE_DEFAULT_NAME_LENGTH-1]= 0;
}

/* 0x00217b70: how many characters a name has, up to 255 */
long saved_game_file_name_length(
	const wchar *name)
{
	long length= 0;

	while (length<SAVED_GAME_FILE_NAME_MAXIMUM_LENGTH && name[length])
	{
		length++;
	}

	return length;
}
