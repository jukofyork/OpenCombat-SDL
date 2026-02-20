#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <cstdlib>
#include <string>

namespace Error
{
	inline void LogMessage(const char* severity, const char* file, int line, const std::string& message)
	{
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
		
		char timeStr[32];
		strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&time));
		
		std::ostringstream oss;
		oss << "[" << timeStr << "] "
		    << "[" << severity << "] "
		    << file << ":" << line << ": "
		    << message;
		
		std::string output = oss.str();
		
		std::cerr << output << std::endl;
		
		std::ofstream logFile("error.log", std::ios::app);
		if (logFile.is_open())
		{
			logFile << output << std::endl;
			logFile.close();
		}
	}
	
	[[noreturn]] inline void FatalError(const char* file, int line, const std::string& message)
	{
		LogMessage("FATAL", file, line, message);
		std::cerr.flush();
		
		// Intentionally cause a segfault by writing to address 0 (null pointer dereference)
		// This creates a useful stack trace in GDB for debugging, as abort() may not
		// provide as much detail about where the error originated
		#ifdef DEBUG
		*((volatile int*)0) = 0;
		#endif
		
		std::abort();
	}
}

#define WARNING(msg) \
	do { \
		Error::LogMessage("WARNING", __FILE__, __LINE__, (msg)); \
	} while (0)

#define ERROR(msg) \
	do { \
		Error::FatalError(__FILE__, __LINE__, (msg)); \
	} while (0)


