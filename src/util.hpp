#ifndef __GHOST_UTIL_HEADER__
#define __GHOST_UTIL_HEADER__
#include <string>

#define GDNS_RESOLVE_NONE 0
#define GDNS_RESOLVE_ALL 1
#define GDNS_RESOLVE_TRANSLATED 2
#define GDNS_RESOLVE_LOCALHOST 3
#define GDNS_RESOLVE_NO_TRANSLATION 4
/** For our trim functions (stolen from: https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring ) */
#include <algorithm>

#define GDNS_BUFFER_SIZE 512

namespace gdns {
	namespace util {
		static inline void ltrim(std::string& s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
			}));
		}

		// trim from end (in place)
		static inline void rtrim(std::string& s) {
			s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(), s.end());
		}

		// trim from both ends (in place)
		static inline void trim(std::string& s) {
			ltrim(s);
			rtrim(s);
		}

		// trim from start (copying)
		static inline std::string ltrim_copy(std::string s) {
			ltrim(s);
			return s;
		}

		// trim from end (copying)
		static inline std::string rtrim_copy(std::string s) {
			rtrim(s);
			return s;
		}

	}; //end namespace util
}; //end namespace gdns
#endif
