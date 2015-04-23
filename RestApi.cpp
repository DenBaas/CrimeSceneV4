#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "RestApi.h"

#include <VrLib/Log.h>
#include <json/reader.h>
#include <json/writer.h>
#include <Windows.h>
#include <WinSock2.h>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")


RestApi::RestApi()
{
	char hostname[1024];
	gethostname(hostname, 1024);
	myHostname = hostname;
	
	logger << "Rest API: hostname: " << myHostname << Log::newline;
}


RestApi::~RestApi()
{
}

void RestApi::registerAsEnvironment()
{
	std::vector<std::string> headers;
	headers.push_back("Content-Type: application/json");
	Json::Value postData;
	postData["name"] = myHostname;
	postData["platform"] = "cave";
	postData["scenarios"].append("Presentatie1.json");
	postData["scenarios"].append("Presentatie2.json");
	postData["scenarios"].append("Presentatie3.json");
	Json::Value ret = callApi(POST, "environment", headers, postData);


	int id = ret["success"]["id"].asInt();

	Json::StyledWriter w;
	logger << w.write(ret) << Log::newline;

	int sessionId = 0;

	while (sessionId == 0)
	{
		Json::Value ret = callApi(PUT, "environment/" + std::to_string(id), headers, Json::Value());

		sessionId = ret["success"]["sessionid"].asInt();
		if (sessionId != 0)
			break;
		Sleep(5000);
	}

	Json::Value session = callApi(GET, "session:" + std::to_string(sessionId), headers, Json::Value());

}



std::vector<std::string> split(std::string str, const std::string &sep)
{
	std::vector<std::string> ret;
	std::size_t index;
	while (true)
	{
		index = str.find(sep);
		if (index == std::string::npos)
			break;
		ret.push_back(str.substr(0, index));
		str = str.substr(index + sep.size());
	}
	ret.push_back(str);
	return ret;
}

Json::Value RestApi::callApi(Method method, const std::string &url, const std::vector<std::string> &headers, const Json::Value &postData /*= Json::nullValue*/)
{
	std::string request;
	if (method == POST)
		request += "POST";
	if (method == GET)
		request += "GET";
	if (method == PUT)
		request += "PUT";

	logger << "REST " << request << " call to " << apiHost << "/" << url << Log::newline;


	struct sockaddr_in addr;
	struct hostent* host;

	host = gethostbyname(apiHost.c_str());
	if (host == NULL)
	{
		logger << "Could not look up host " << apiHost << "', are you connected to the internet?";
		return NULL;
	}
	addr.sin_family = host->h_addrtype;
	memcpy((char*)&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	addr.sin_port = htons(8080);
	memset(addr.sin_zero, 0, 8);
	
	SOCKET s;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		logger << "Cannot create socket, try a reboot" << Log::newline;
		closesocket(s);
		return NULL;
	}

	int rc;
	int siz = sizeof(addr);
	rc = ::connect(s, (struct sockaddr*) &addr, siz);
	if (rc < 0)
	{
		logger << "Could not connect to REST api host: " << apiHost << Log::newline;
		closesocket(s);
		return NULL;
	}
	
	Json::StyledWriter writer;
	std::string postDataStr = writer.write(postData);

	request += " /" + url + " HTTP/1.1\r\n";

	request += "Host: " + apiHost + ":8080\r\n";

	if (method == POST)
	{
		request += "Content-Length: " + Log::format("%i\r\n", postDataStr.size());
		request += "Content-Type: application/json\r\n";
	}

	for (auto h : headers)
		request += h + "\r\n";
	request += "\r\n";
	if (method == POST)
	{
		request += postDataStr;
	}

	size_t sent = 0;
	while (sent < request.size())
	{
		int rc = send(s, request.c_str() + sent, min(request.size()-sent, 1024), 0);
		if (rc <= 0)
		{
			logger << "Error sending to socket" << Log::newline;
			closesocket(s);
			return NULL;
		}
		sent += rc;
	}

	std::string buffer;
	bool headersFound = false;
	int len = -1;
	while (true)
	{
		char buf[1024];
		int rc = recv(s, buf, 1024, 0);
		if (rc <= 0)
		{
			closesocket(s);
			break;
		}
		buffer += std::string(buf, rc);
		if (!headersFound && buffer.find("\r\n\r\n") != std::string::npos)
		{
			headersFound = true;
			std::vector<std::string> headers = split(buffer.substr(0, buffer.find("\r\n\r\n")), "\r\n");
			for (auto h : headers)
			{
				std::string lower = h;
				std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
				if (lower.find(":"))
					lower = lower.substr(0, lower.find(":"));
				if (lower == "content-length")
					len = atoi(h.substr(lower.size() + 1).c_str());
			}
			buffer = buffer.substr(buffer.find("\r\n\r\n") + 4);
		}
		if (headersFound && buffer.size() >= len && len != -1)
			break;
	}

	Json::Reader reader;
	Json::Value ret;
	reader.parse(buffer, ret);

	closesocket(s);




	return ret;
}

Json::Value RestApi::buildJson(const std::string &data)
{
	Json::Value ret;
	Json::Reader reader;
	reader.parse(data.c_str(), ret);
	logger << reader.getFormattedErrorMessages() << Log::newline;
	return ret;
}
