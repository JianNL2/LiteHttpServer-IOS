#ifndef __LOG_H__
#define __LOG_H__

#include <string.h>

#include <cerrno>
#include <iostream>
#include <system_error>
#include <sys/timeb.h>
#include <chrono>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cinttypes>

inline std::time_t get_time_stamp();
inline std::time_t get_time_stamp()
{
	std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	std::time_t timestamp = tmp.count();
	//std::time_t timestamp = std::chrono::system_clock::to_time_t(tp);
	return timestamp;
}

inline std::string millisecond_to_str(std::int64_t milliseconds);
inline std::string millisecond_to_str(std::int64_t milliseconds)
{
	std::chrono::milliseconds ms(milliseconds);
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> t1(ms);
	std::time_t t = std::chrono::system_clock::to_time_t(t1);

	std::stringstream ss;
	auto const msecs = ms.count() % 1000;
	ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H.%M.%S") << "." << msecs;
	return ss.str();
}

inline std::string log_time()
{
	return millisecond_to_str(get_time_stamp());
}

inline void test();
void test()
{
	std::cout << millisecond_to_str(get_time_stamp()) << std::endl;
}



#define __STR(s) #s
#define WHERE __STR(__FILE__ ":" __LINE__)

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define _E(msg) \
	std::cerr << log_time << "[ERROR] " << __FILENAME__ << ":" << __LINE__ << " " << "errno : " << errno << " " << msg << std::endl;

#define _W(msg) \
	std::cerr << log_time << "[WARNING] " << __FILENAME__ << ":" << __LINE__ << " " << msg << std::endl;

#define _I(msg) \
	std::cerr << log_time << "[INFO] " << __FILENAME__ << ":" << __LINE__ << " " << msg << std::endl;

#define SYSTEM_ERROR() \
do { \
	_E(strerror(errno));\
} while (0)

#define THROW_SYSTEM_ERROR() \
do { \
	SYSTEM_ERROR(); \
	throw std::system_error(std::error_code(errno, std::system_category()));\
} while (0)

#define SYSTEM_WARNING() \
do { \
	_W(strerror(errno)) \
} while (0)

#define THROW(msg) \
do { \
	throw std::runtime_error(msg);\
} while (0)

#define THROW_LOGIC(msg) \
do { \
	_E(msg);\
	throw std::logic_error(msg);\
} while (0)

#endif /* __LOG_H__ */
