/*
	Nnanna Kama
	Simple class for basic exception handling
*/


#ifndef KS_ERRORNOTIFY_H
#define KS_ERRORNOTIFY_H

namespace ks {

	class ErrorNotify
	{
	public:

		ErrorNotify();

		ErrorNotify(const char* error);

		~ErrorNotify();

		void	PrintError();


	private:

		char	mErrString[512];
	};

}	// namespace ks

#endif