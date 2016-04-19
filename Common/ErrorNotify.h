/*
	Nnanna Kama
	Simple class for basic exception handling
*/


#ifndef ERRORNOTIFY_H
#define ERRORNOTIFY_H


class ErrorNotify
{
public:

	ErrorNotify();

	ErrorNotify( const char* error );

	~ErrorNotify();

	void	PrintError();


private:

	char	mErrString[512];


};

#endif