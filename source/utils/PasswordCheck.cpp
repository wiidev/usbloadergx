#include "prompts/PromptWindows.h"

int PasswordCheck(const char * password)
{
	if(!password || strcmp(password, "") == 0 || strcmp(password, "not set") == 0)
		return 2;

	char entered[100];
	memset(entered, 0, sizeof(entered));

	int result = OnScreenKeyboard(entered, 20, 0, true);
	if (result == 1)
	{
		if (strcmp(entered, password) == 0) //if password correct
			return 1;
		else
			return -1;
	}

	return 0;
}
