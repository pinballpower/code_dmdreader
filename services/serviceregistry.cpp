#include "serviceregistry.hpp"

#include <boost/log/trivial.hpp>

ServiceRegistry serviceRegistry;

ServiceRegistry::ServiceRegistry()
{
}

bool ServiceRegistry::registerService(std::shared_ptr<Service> service)
{
	if (services.contains(service->name()))
	{
		BOOST_LOG_TRIVIAL(debug) << "[serviceregistry] couldn't add service with name " << service->name() << " to registry, already exists";
		return false;
	}

	services[service->name()] = service;
	return true;
}

void ServiceRegistry::clear()
{
	for (auto service : services) {
		service.second->stop();
	}
	services.clear();
}

std::pair<ServiceResponse, string> ServiceRegistry::command(const string serviceName, const string& cmd)
{
	if (! services.contains(serviceName)) {
		return make_pair(ServiceResponse::SERVICE_NOT_FOUND, "");
	}
	return services[serviceName]->command(cmd);
}
