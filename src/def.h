#ifndef QST_DEF_H
#define QST_DEF_H
#if defined(_WIN32) || defined(_WIN64)
#include <tchar.h>
namespace qst {
  typedef TCHAR env_char;
}
#elif defined(__linux)
namespace qst {
  typedef char env_char;
}
#endif
#endif