#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <map>
#include <string>
#include <vector>
#include <sstream>

class HttpRequest
{
public:

	HttpRequest()
			: versionMajor(0), versionMinor(0), keepAlive(false)
	{}

	struct HeaderItem
	{
		std::string name;
		std::string value;
	};

	std::string method;
	std::string uri;
	int versionMajor;
	int versionMinor;
	std::vector<HeaderItem> headers;
	std::vector<char> content;
	bool keepAlive;

	std::string inspect() const
	{
		std::stringstream stream;
		stream << method << " " << uri << " HTTP/"
			   << versionMajor << "." << versionMinor << "\n";

		for(std::vector<HttpRequest::HeaderItem>::const_iterator it = headers.begin();
			it != headers.end(); ++it)
		{
			stream << it->name << ": " << it->value << "\n";
		}

		std::string data(content.begin(), content.end());
		stream << data << "\n";
		stream << "+ keep-alive: " << keepAlive << "\n";;
		return stream.str();
	}

};

typedef std::shared_ptr<HttpRequest> HttpRequestPtr;

#endif /* __HTTP_REQUEST_H__ */
