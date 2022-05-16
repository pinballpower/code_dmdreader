#include "serviceregistry.hpp"

#include <boost/log/trivial.hpp>

ServiceRegistry serviceRegistry;

ServiceRegistry::ServiceRegistry()
{
}

void ServiceRegistry::registerService(std::shared_ptr<Service> service)
{
	services.push_back(service);
}

void ServiceRegistry::clear()
{
	for (auto service : services) {
		service->stop();
	}
	services.clear();
}

vector<std::pair<ServiceResponse, string>> ServiceRegistry::command(const string serviceName, const string& cmd, int instanceId)
{
	vector<std::pair<ServiceResponse, string>> res;

	for (auto& service : services) {
		if (service->name() == serviceName) {
			res.push_back(service->command(cmd));
		}
	}

	return res;
}
