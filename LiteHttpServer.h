//
// Created by 袁健 on 2017/12/11.
//

#ifndef LITEHTTPSERVER_LITEHTTPSERVER_H
#define LITEHTTPSERVER_LITEHTTPSERVER_H

#include <string>
#include <netdb.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
#include "Log.h"
#include <map>
#include <functional>
#include "HttpRequest.h"
#include <atomic>
#include <sys/poll.h>


class LiteHttpServer
{
public:
    typedef std::function<void (int)> Handler;
    typedef std::function<void ()> Functor;
public:
    LiteHttpServer(const std::string & address,uint32_t port):_address(address),_port(port),_stop(false)
    {
        int pipe_ret = ::pipe(_pipe_fd);
        if (pipe_ret)
        {
            _E("pipe error " << pipe_ret);
        }
    }
    ~LiteHttpServer()
    {
    }
    void start(bool is_ipv4 = true);
    void stop();
    void dispatch(Functor func);

private:
    std::string _address;
    uint32_t _port;
    int _listen_fd;

    int _pipe_fd[2];

    std::vector<struct pollfd> _pollfds;

    std::map<int,Handler> _handlers;
    std::map<int,std::shared_ptr<void>> _context;

    std::atomic_bool _stop;
    std::atomic_bool _polling;

    std::vector<Functor> _functors;

    std::mutex _mutex;


private:
    // event loop
    void loop();

    void accept_handler(int fd);

    void read_handler(int fd);

    void wakeup_handler(int fd);

    void wake_up();

    void shutdown(int fd);
    std::string make_http_resp(HttpRequest& req, const char *content, size_t size);

};


#endif //LITEHTTPSERVER_LITEHTTPSERVER_H
