//
// Created by 袁健 on 2017/12/11.
//

#include "LiteHttpServer.h"
#include <sys/poll.h>
#include "httprequestparser.h"
#include "HttpRequest.h"
#include "Log.h"


void LiteHttpServer::start(bool is_ipv4)
{
    //phase init
    _listen_fd = ::socket(AF_INET,SOCK_STREAM,0);
    if (_listen_fd == -1)
    {
        _E("socket init error");
        return;
    }
    int one = 1;
    int ret = setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
    if (ret != 0)
    {
        _E("setsockopt error " << ret);
        return;
    }
    //phase bind
    struct hostent *host = gethostbyname(_address.c_str());

    if (!host)
    {
        const char *errMsg = hstrerror(h_errno);
        _E(errMsg);
        return;
    }
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(_port);
    //TODO: use correct host
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int bind_ret = ::bind(_listen_fd, reinterpret_cast<struct sockaddr*>(&sockaddr),
                     sizeof(sockaddr));
    if (bind_ret != 0)
    {
        _E("bind error");
        return;
    }

    //phase listen
    int listen_ret = ::listen(_listen_fd, 5);

    if (listen_ret != 0)
    {
        _E("listen error");
        return;
    }
//    long listen_one = 1;
//    int ioctl_ret = ioctl(_listen_fd, FIONBIO, &listen_one);
//    if (ioctl_ret == -1)
//    {
//        _E("make none block error");
//        return;
//    }

    //phase add listen to poll
    struct pollfd pfd;

    pfd.fd = _listen_fd;

    //TODO : consider the out event
    pfd.events = POLL_IN/* | POLL_OUT*/;

    _pollfds.push_back(pfd);
    _handlers[_listen_fd] = std::bind(&LiteHttpServer::accept_handler,
                                      this,
                                      std::placeholders::_1);

    struct pollfd pfd_wakeup;

    pfd_wakeup.fd = _pipe_fd[0];

    //TODO : consider the out event
    pfd_wakeup.events = POLL_IN/* | POLL_OUT*/;

    _pollfds.push_back(pfd_wakeup);
    _handlers[_pipe_fd[0]] = std::bind(&LiteHttpServer::wakeup_handler,
                                       this,
                                       std::placeholders::_1);

    //phase start event loop
    loop();
}


void LiteHttpServer::stop()
{
    _stop = true;
    wake_up();
}

void LiteHttpServer::loop()
{
    _I("loop start");
    while (!_stop)
    {
        auto pollfds_local = _pollfds;
        int nfds = ::poll(pollfds_local.data(),pollfds_local.size(),10);

        if (nfds == -1 && errno != EINTR)
        {
            _E("poll error");
            return;
        }
        _polling = true;
        for (int i = 0; i < pollfds_local.size() && nfds > 0; ++i)
        {
            int fd = pollfds_local[i].fd;
            if (_handlers.count(fd) == 0)
                continue;

            auto &handler = _handlers[fd];

            if (pollfds_local[i].revents & POLL_IN)
            {
                handler(fd);
            }
            if (pollfds_local[i].revents & POLL_OUT)
            {
                handler(fd);
            }
        }
        _polling = false;
        if (!_functors.empty()) {
            std::vector<Functor> funcs;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                funcs.swap(_functors);
            }
            for (const auto &func : funcs) {
                func();
            }
        }
    }
    _I("loop end");

}

void LiteHttpServer::dispatch(Functor func)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _functors.push_back(func);
    if (!_polling)
        wake_up();
}

void LiteHttpServer::wake_up()
{
    char tmp[1];
    ::write(_pipe_fd[1],&tmp,1);
}

void LiteHttpServer::accept_handler(int fd)
{
    struct sockaddr_in sockaddr;
    struct pollfd pfd;
    socklen_t size = sizeof(sockaddr);
    int acc_ret = ::accept(_listen_fd, reinterpret_cast<struct sockaddr*>(&sockaddr), &size);

    if (acc_ret == -1)
    {
        _E("accept error");
        return;
    }

    //make noblock io
    long one = 1;
    int ioctl_ret = ioctl(acc_ret, FIONBIO, &one);
    if (ioctl_ret == -1)
    {
        _E("make none block error");
        return;
    }

    _handlers[acc_ret] = std::bind(&LiteHttpServer::read_handler,
                                   this,
                                   std::placeholders::_1
                                   );

    _I("accept " << acc_ret);

    pfd.fd = acc_ret;

    //TODO : consider the out event
    pfd.events = POLL_IN/* | POLL_OUT*/;

    _pollfds.push_back(pfd);
}

void LiteHttpServer::read_handler(int fd)
{
    if (_context.find(fd) == _context.end())
    {
        _context[fd] = std::make_shared<HttpRequestParser>();
    }
    HttpRequestParserPtr request_parser = std::static_pointer_cast<HttpRequestParser>(_context[fd]);
    uint8_t buffer[65536];
    ssize_t recv_ret = ::recv(fd,buffer, sizeof(buffer),0);
    _I("http parse " << recv_ret << " " << fd);
    if (recv_ret == 0)
    {
        //close
        _E("recv_ret zero " << fd);
        this->shutdown(fd);
    }
    else if (recv_ret > 0)
    {
        //parse
        HttpRequestParser::ParseResult parse_res = request_parser->parse((const char *)buffer,(const char *)buffer + recv_ret);
        if (parse_res == HttpRequestParser::ParsingIncompleted)
        {
            //TODO : callback progress
            return;
        } else if (parse_res == HttpRequestParser::ParsingCompleted) {
            _I("http parse complete " << fd << " " << request_parser->request.uri << " " << request_parser->request.content.size());
            if (request_parser->request.uri == "/up")
            {
                std::stringstream ss;

                ss << "<body>\n"
                   << "<form action=\"uploaddata\" method=\"post\" enctype=\"multipart/form-data\">\n"
                   << "<input type=\"file\" name=\"fileUpload\" />\n"
                   << "<input type=\"submit\" value=\"Upload File\" />\n"
                   << "</form>\n"
                   << "</body>";

                std::string content = ss.str();

                auto resp_content = make_http_resp(request_parser->request,content.data(),content.size());
                ::write(fd,resp_content.data(),resp_content.size());
            }
            else if (request_parser->request.uri == "/uploaddata")
            {
                std::stringstream ss;

                ss << "upload suc\n";

                std::string content = ss.str();

                auto resp_content = make_http_resp(request_parser->request,content.data(),content.size());
                ::write(fd,resp_content.data(),resp_content.size());
                _upCompCb(request_parser->request.content.data(),
                          request_parser->request.content.size(),
                          request_parser->filename.data(),
                          request_parser->filename.length());
            }
            this->shutdown(fd);
            return;
        } else {
            _E("http parse error " << fd);
            this->shutdown(fd);
            return;
        }
    }
    else
    {
        //error
        if (errno == EWOULDBLOCK
            || errno == EAGAIN
            || errno == EINTR)
        {
            return;
        }
        else
        {
            _E("recv ret neg " << fd);
            this->shutdown(fd);
        }

    }

}

void LiteHttpServer::shutdown(int fd) {
    dispatch([=](){
        for (int i = 0 ; i < _pollfds.size() ; ++ i) {
            if (_pollfds[i].fd == fd)
            {
                _pollfds[i] = _pollfds.back();
                _pollfds.pop_back();
                _I("shutdown " << fd);
                break;
            }
        }
        _handlers.erase(fd);
        _context.erase(fd);
        ::close(fd);
    });
}


std::string LiteHttpServer::make_http_resp(HttpRequest& req, const char *content, size_t size)
{
    std::stringstream ss;
    ss << "HTTP/" << req.versionMajor << "." << req.versionMinor << " " << 200 << " " <<
       "OK" << "\nContent-Length:" <<
       size << "\n\n";

    std::string res(ss.str().data(),ss.str().size());
    res.append(content, size);
    return res;
}

void LiteHttpServer::wakeup_handler(int fd)
{
    int8_t temp[4096];
    ssize_t wakeup_ret = ::read(fd,temp, sizeof(temp));
    _I("wakeup_handler " << fd << " " << wakeup_ret);
}
