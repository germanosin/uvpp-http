#pragma once

#include "tcp.hpp"
#include "url.hpp"
#include "text.hpp"
#include <map>

namespace uvpp
{
	namespace http {
		
		class Context;

		class Request {

            friend class Context;

			private:
			    Request()
			        : url_()
			        , headers_()
			        , body_("")
			    {
			    }

			    ~Request()
			    {
			        //printf("~request() %x\n", this);
			    }

			public:
			    const Url& url() const { return url_; }

			    const std::string& get_header(const std::string& key) const
			    {
			        auto it = headers_.find(key);
			        if(it != headers_.end()) return it->second;
			        return default_value_;
			    }

			    bool get_header(const std::string& key, std::string& value) const
			    {
			        auto it = headers_.find(key);
			        if(it != headers_.end())
			        {
			            value = it->second;
			            return true;
			        }
			        return false;
			    }

			    std::string get_body (void)
			    {
			        return body_;
			    }

			private:
			    Url url_;
			    std::map<std::string, std::string, uvpp::text::ci_less> headers_;
			    std::string body_;
			    std::string default_value_;
			};
	}
}