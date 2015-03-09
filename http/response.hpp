#pragma once

#include <sstream>
#include <map>
#include <uvpp.hpp>
#include "text.hpp"

namespace uvpp
{
	namespace http {

		class Context;

		typedef std::shared_ptr<Context> http_client_ptr;

		const std::map<int,const char *> statuses = {
		        {100,"Continue"},
		        {101,"Switching Protocols"},
		        {200, "OK"},
		        {201, "Created"},
		        {202, "Accepted"},
		        {203,"Non-Authoritative Information"},
		        {204,"No Content"},
		        {205,"Reset Content"},
		        {206,"Partial Content"},
		        {300,"Multiple Choices"},
		        {301,"Moved Permanently"},
		        {302,"Found"},
		        {303,"See Other"},
		        {304,"Not Modified"},
		        {305,"Use Proxy"},
		        //306,"(reserved)";
		        {307,"Temporary Redirect"},
		        {400,"Bad Request"},
		        {401,"Unauthorized"},
		        {402,"Payment Required"},
		        {403,"Forbidden"},
		        {404,"Not Found"},
		        {405,"Method Not Allowed"},
		        {406,"Not Acceptable"},
		        {407,"Proxy Authentication Required"},
		        {408,"Request Timeout"},
		        {409,"Conflict"},
		        {410,"Gone"},
		        {411,"Length Required"},
		        {412,"Precondition Failed"},
		        {413,"Request Entity Too Large"},
		        {414,"Request-URI Too Long"},
		        {415,"Unsupported Media Type"},
		        {416,"Requested Range Not Satisfiable"},
		        {417,"Expectation Failed"},
		        {500,"Internal Server Error"},
		        {501,"Not Implemented"},
		        {502,"Bad Gateway"},
		        {503,"Service Unavailable"},
		        {504,"Gateway Timeout"},
		        {505,"HTTP Version Not Supported"}
		};	

		class response_exception : public uvpp::exception
        {
        public:
            response_exception(const std::string& message="HTTP respsonse error.")
                : uvpp::exception(message)
            {}
        };	

		class Response {

			friend class Context;

			private:
				Response(Context* client, uvpp::Tcp* socket)
				    : client_(client)
				    , socket_(socket)
				    , headers_()
				    , status_(200)
				{
				    headers_["Content-Type"] = "text/html";
				}

				~Response()
				{}

			public:
			    bool end(const std::string& body)
			    {
			        // Content-Length
			        if(headers_.find("Content-Length") == headers_.end())
			        {
			            std::stringstream ss;
			            ss << body.length();
			            headers_["Content-Length"] = ss.str();
			        }

			        std::stringstream response_text;
			        response_text << "HTTP/1.1 ";
			        response_text << status_ << " " << get_status_text(status_) << "\r\n";
			        for(auto h : headers_)
			        {
			            response_text << h.first << ": " << h.second << "\r\n";
			        }
			        response_text << "\r\n";
			        response_text << body;

			        auto str = response_text.str();
			        return socket_->write(str.c_str(), static_cast<int>(str.length()), [=](error e) {
			            if(e)
			            {
			                // TODO: handle error
			            }
			            // clean up
			            client_.reset();
			        });
			    }

			    void set_status(int status_code)
			    {
			        status_ = status_code;
			    }

			    void set_header(const std::string& key, const std::string& value)
			    {
			        headers_[key] = value;
			    }

			    static std::string get_status_text(int status)
			    {
			        auto status_iter = statuses.find(status);
			        if (status_iter!=statuses.end()) {
			        	return status_iter->second;
			        } else
			        	throw response_exception("Not supported status code.");
			    }

			private:
			    http_client_ptr client_;
			    uvpp::Tcp* socket_;
			    std::map<std::string, std::string, uvpp::text::ci_less> headers_;
			    int status_;				
		};
	}
}