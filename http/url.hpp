#pragma once

#include <http_parser.h>
#include <uvpp.hpp>

namespace uvpp
{

 namespace http {

  class url_parse_exception : public uvpp::exception
    {
     public:
         url_parse_exception(const std::string& message="Failed to parse URL.")
            : uvpp::exception(message)
        {}
    };


  class Url {
    friend class Context;
    
    public:
        Url()
            : handle_(), buf_()
        {
            //printf("Url() %x\n", this);
        }

        Url(const Url& c)
            : handle_(c.handle_), buf_(c.buf_)
        {
            //printf("Url(const Url&) %x\n", this);
        }

        Url& operator =(const Url& c)
        {
            //printf("Url::operator =(const Url&) %x\n", this);
            handle_ = c.handle_;
            buf_ = c.buf_;
            return *this;
        }

        ~Url()
        {
            //printf("~Url() %x\n", this);
        }

    public:
        std::string schema() const
        {
            if(has_schema()) return buf_.substr(handle_.field_data[UF_SCHEMA].off, handle_.field_data[UF_SCHEMA].len);
            return "HTTP";
        }

        std::string host() const
        {
            // TODO: if not specified, use host name
            if(has_schema()) return buf_.substr(handle_.field_data[UF_HOST].off, handle_.field_data[UF_HOST].len);
            return std::string("localhost");
        }

        int port() const
        {
            if(has_port()) return static_cast<int>(handle_.port);
            return (schema() == "HTTP" ? 80 : 443);
        }

        std::string path() const
        {
            if(has_path()) return buf_.substr(handle_.field_data[UF_PATH].off, handle_.field_data[UF_PATH].len);
            return std::string("/");
        }

        std::string query() const
        {
            if(has_query()) return buf_.substr(handle_.field_data[UF_QUERY].off, handle_.field_data[UF_QUERY].len);
            return std::string();
        }

        std::string fragment() const
        {
            if(has_query()) return buf_.substr(handle_.field_data[UF_FRAGMENT].off, handle_.field_data[UF_FRAGMENT].len);
            return std::string();
        }

    private:

        void from_buf(const char* buf, std::size_t len, bool is_connect=false)
        {
            // TODO: validate input parameters

            buf_ = std::string(buf, len);
            if(http_parser_parse_url(buf, len, is_connect, &handle_) != 0)
            {
                // failed for some reason
                // TODO: let the caller know the error code (or error message)
                throw url_parse_exception();
            }
        }

        bool has_schema() const { return handle_.field_set & (1<<UF_SCHEMA); }
        bool has_host() const { return handle_.field_set & (1<<UF_HOST); }
        bool has_port() const { return handle_.field_set & (1<<UF_PORT); }
        bool has_path() const { return handle_.field_set & (1<<UF_PATH); }
        bool has_query() const { return handle_.field_set & (1<<UF_QUERY); }
        bool has_fragment() const { return handle_.field_set & (1<<UF_FRAGMENT); }

    private:
        http_parser_url handle_;
        std::string buf_;
   };
  }
}