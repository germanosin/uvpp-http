#pragma once
#include "response.hpp"
#include "request.hpp"
#include "context.hpp"

namespace uvpp
{
	namespace http {

        class Server
        {
            public:
                Server()
                    : socket_(new uvpp::Tcp)
                {
                }

                virtual ~Server()
                {
                    if(socket_)
                    {
                        socket_->close([](){});
                    }
                }

            public:
                static std::shared_ptr<Server> create(const std::string& ip, int port, std::function<void(Request&, Response&)> callback)
                {
                    auto server = std::shared_ptr<Server>(new Server);
                    if(server->listen(ip, port, callback)) return server;
                    return nullptr;
                }

                bool listen(const std::string& ip, int port, std::function<void(Request&, Response&)> callback)
                {
                    if(!socket_->bind(ip, port)) return false;

                    if(!socket_->listen([=](error e) {
                        if(e)
                        {
                            // TODO: handle client connection error
                        }
                        else
                        {
                            auto client = new Context(socket_.get());
                            client->parse(callback);
                        }
                    })) return false;

                    return true;
                }

            private:
                std::shared_ptr<uvpp::Tcp> socket_;
            };

        }
}