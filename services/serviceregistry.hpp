#include "service.hpp"

class ServiceRegistry {

public:

	ServiceRegistry();
	bool registerService(std::shared_ptr<Service> service);
	void clear();

	std::pair<ServiceResponse, string> command(const string serviceName, const string& cmd);

private:
	map<string, std::shared_ptr<Service>> services;

	ServiceRegistry(const ServiceRegistry&) = delete;
	ServiceRegistry& operator=(const ServiceRegistry&) = delete;
};

extern ServiceRegistry serviceRegistry;


