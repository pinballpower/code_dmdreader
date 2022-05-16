#include "service.hpp"

class ServiceRegistry {

public:

	ServiceRegistry();
	void registerService(std::shared_ptr<Service> service);
	void clear();

	vector<std::pair<ServiceResponse, string>> command(const string serviceName, const string& cmd, int instanceId = -1);

private:
	vector<std::shared_ptr<Service>> services;

	ServiceRegistry(const ServiceRegistry&) = delete;
	ServiceRegistry& operator=(const ServiceRegistry&) = delete;
};

extern ServiceRegistry serviceRegistry;


