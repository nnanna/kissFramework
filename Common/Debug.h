#ifndef DEBUG_H
#define DEBUG_H

#if defined _DEBUG
#define KS_ASSERT(expr)		\
   do {						\
      if (!( expr) )		\
         __debugbreak();	\
      } while (0)			\

#else
#define KS_ASSERT
#endif	// _DEBUG

#endif