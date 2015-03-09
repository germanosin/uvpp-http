#pragma once

#include "tcp.hpp"
#include <http_parser.h>

namespace uvpp
{
 namespace http {

  class Context {
   
   friend class Server;

   private:
    Context(uvpp::Tcp *server) :
    socket_(nullptr)
              , parser_()
              , was_header_value_(true)
              , last_header_field_()
              , last_header_value_()
              , parser_settings_()
              , request_(nullptr)
              , response_(nullptr)
              , callback_lut_(new uvpp::callbacks(1)) {

               assert(server);

               socket_ = std::shared_ptr<uvpp::Tcp> (new uvpp::Tcp);
               server->accept(*socket_.get());

    }

   public:
    ~Context()
    {
        if(request_)
        {
            delete request_;
            request_ = nullptr;
        }

        if(response_)
        {
            delete response_;
            response_ = nullptr;
        }

        if(callback_lut_)
        {
            delete callback_lut_;
            callback_lut_ = nullptr;
        }

        if(socket_.use_count())
        {
            socket_->close([=](){});
        }
    }

   private:

    bool parse(std::function<void(Request&, Response&)> callback)
    {
        request_ = new Request;
        response_ = new Response(this, socket_.get());

        http_parser_init(&parser_, HTTP_REQUEST);
        parser_.data = this;

        // store callback object
        callbacks::store(callback_lut_, 0, callback);

        parser_settings_.on_url = [](http_parser* parser, const char *at, size_t len) {
            auto client = reinterpret_cast<Context*>(parser->data);

            //  TODO: from_buf() can throw an exception: check
            client->request_->url_.from_buf(at, len);

            return 0;
        };

        parser_settings_.on_header_field = [](http_parser* parser, const char* at, size_t len) {
            auto client = reinterpret_cast<Context*>(parser->data);

            if(client->was_header_value_)
            {
                // new field started
                if(!client->last_header_field_.empty())
                {
                    // add new entry
                    client->request_->headers_[client->last_header_field_] = client->last_header_value_;
                    client->last_header_value_.clear();
                }

                client->last_header_field_ = std::string(at, len);
                client->was_header_value_ = false;
            }
            else
            {
                // appending
                client->last_header_field_ += std::string(at, len);
            }
            return 0;
        };
        parser_settings_.on_header_value = [](http_parser* parser, const char* at, size_t len) {
            auto client = reinterpret_cast<Context*>(parser->data);

            if(!client->was_header_value_)
            {
                client->last_header_value_ = std::string(at, len);
                client->was_header_value_ = true;
            }
            else
            {
                // appending
                client->last_header_value_ += std::string(at, len);
            }
            return 0;
        };
        parser_settings_.on_headers_complete = [](http_parser* parser) {
            auto client = reinterpret_cast<Context*>(parser->data);

            // add last entry if any
            if(!client->last_header_field_.empty()) {
                // add new entry
                client->request_->headers_[client->last_header_field_] = client->last_header_value_;
            }

            return 0; // 1 to prevent reading of message body.
        };
        parser_settings_.on_body = [](http_parser* parser, const char* at, size_t len) {
            //printf("on_body, len of 'char* at' is %d\n", len);
            auto client = reinterpret_cast<Context*>(parser->data);
            client->request_->body_ = std::string(at, len);
            return 0;
        };
        parser_settings_.on_message_complete = [](http_parser* parser) {
            //printf("on_message_complete, so invoke the callback.\n");
            auto client = reinterpret_cast<Context*>(parser->data);
            // invoke stored callback object
            callbacks::invoke<decltype(callback)>(client->callback_lut_, 0, *client->request_, *client->response_);
            return 1; // 0 or 1?
        };

        socket_->read_start([=](const char* buf, int len){
            if (buf == 0x00 && len == -1) {
                response_->set_status(500);
            } else {
                http_parser_execute(&parser_, &parser_settings_, buf, len);
            }
        });

        return true;
    }    


   private:

    http_parser parser_;
    http_parser_settings parser_settings_;
    bool was_header_value_;
    std::string last_header_field_;
    std::string last_header_value_;

    Request* request_;
    Response* response_;

    std::shared_ptr<uvpp::Tcp> socket_;
    uvpp::callbacks* callback_lut_;
  };

 }
}