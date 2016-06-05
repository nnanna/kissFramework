#ifndef DEBUG_H
#define DEBUG_H

#define KS_ENABLE_ASSERTS	defined _DEBUG// | 1

#if KS_ENABLE_ASSERTS
#define KS_ASSERT(expr)		\
   do {						\
      if (!( expr) )		\
         __debugbreak();	\
      } while (0)			\

#else
#define KS_ASSERT
#endif	// KS_ENABLE_ASSERTS

#endif