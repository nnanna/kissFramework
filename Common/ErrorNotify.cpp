

#include "ErrorNotify.h"
#include <string>

ErrorNotify::ErrorNotify()
{
	strcpy_s(mErrString, sizeof(mErrString), "Run for your life yo!");
}


ErrorNotify::ErrorNotify(const char *error)
{
	strcpy_s(mErrString, sizeof(mErrString), error);
}


ErrorNotify::~ErrorNotify()
{
	PrintError();
}


void ErrorNotify::PrintError()
{
	printf("\n%s", mErrString);
}
