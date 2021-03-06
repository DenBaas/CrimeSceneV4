#pragma once

#include <VrLib/Singleton.h>
#include <string>
#include <vector>
#include <json/value.h>

class RestApi : public Singleton<RestApi>
{
	const std::string apiHost = "145.48.6.64";
	std::string myHostname;


public:
	RestApi();
	~RestApi();


	void registerAsEnvironment();
	
private:
	enum Method { POST, GET, PUT };

	Json::Value callApi(Method method, const std::string &url, const std::vector<std::string> &headers, const Json::Value &postData = Json::Value::null);
	Json::Value buildJson(const std::string &data);

};

