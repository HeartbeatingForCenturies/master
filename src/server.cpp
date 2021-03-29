#include "std_include.hpp"
#include "server.hpp"
#include "service.hpp"
#include "console.hpp"

#include "services/getservers_command.hpp"
#include "services/heartbeat_command.hpp"
#include "services/info_response_command.hpp"
#include "services/ping_handler.hpp"
#include "services/elimination_handler.hpp"

server::server(const network::address& bind_addr)
	: server_base(bind_addr)
{
	this->register_service<getservers_command>();
	this->register_service<heartbeat_command>();
}

server_list& server::get_server_list()
{
	return server_list_;
}

const server_list& server::get_server_list() const
{
	return server_list_;
}

void server::run_frame()
{
	for(auto& service : services_)
	{
		try
		{
			service->run_frame();
		}
		catch(const service::execution_exception& e)
		{
			console::warn("Execption in service: %s", e.what());
		}
		catch(const std::exception& e)
		{
			console::error("Fatal execption in service: %s", e.what());
		}
	}
}

void server::handle_command(const network::address& target, const std::string_view& command,
                            const std::string_view& data)
{
	const auto handler = this->command_services_.find(std::string{command});
	if (handler == this->command_services_.end())
	{
		console::warn("Unhandled command (%s): %.*s", target.to_string().data(), command.size(), command.data());
		return;
	}

#ifdef DEBUG
	console::log("Handling command (%s): %.*s", target.to_string().data(), command.size(), command.data());
#endif

	try
	{
		handler->second->handle_command(target, data);
	}
	catch(const service::execution_exception& e)
	{
		console::warn("Execption in command %.*s (%s): %s", command.size(), command.data(), target.to_string().data(), e.what());
	}
	catch(const std::exception& e)
	{
		console::error("Fatal execption in command %.*s (%s): %s", command.size(), command.data(), target.to_string().data(), e.what());
	}
}
