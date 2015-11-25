#ifndef DEBUG_H
#define DEBUG_H

#if defined _DEBUG
#define KISS_ASSERT(expr)	\
   do {						\
      if (!expr)			\
         __debugbreak();	\
      } while (0)			\

#else
#define KISS_ASSERT
#endif	// _DEBUG

#endif