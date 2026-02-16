#pragma once

class StatusCallback
{
public:
	virtual void Status(const std::string& msg) = 0;
};