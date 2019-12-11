#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <map>
#include <sys/wait.h>
#include <unistd.h>
#include "string_manipulator.h"

using boost::asio::ip::tcp;
using namespace std;

class http_server {
 public:
  explicit http_server(boost::asio::io_service &io_service, uint16_t port)
      : _io_service(io_service), _signal(io_service, SIGCHLD), _acceptor(io_service, {tcp::v4(), port}),
        _socket(io_service) {
    wait_Newsignal();
    do_accept();
  }

 private:
  void wait_Newsignal() {
    _signal.async_wait(
        [this](boost::system::error_code /*ec*/, int /*signo*/) {
          // Only the parent process should check for this signal. We can
          // determine whether we are in the parent by checking if the acceptor
          // is still open.
          if (_acceptor.is_open()) {
            // Reap completed child processes so that we don't end up with
            // zombies.
            int status = 0;
            while (waitpid(-1, &status, WNOHANG) > 0) {}
            wait_Newsignal();
          }
        });
  }

  void do_accept() {
    _acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket new_socket) {
          if (!ec) {
            // Take ownership of the newly accepted socket.
            _socket = std::move(new_socket);

            // Inform the io_service that we are about to fork. The io_service
            // cleans up any internal resources, such as threads, that may
            // interfere with forking.
            _io_service.notify_fork(boost::asio::io_service::fork_prepare);

            if (fork()==0) {
              // Inform the io_service that the fork is finished and that this
              // is the child process. The io_service uses this opportunity to
              // create any internal file descriptors that must be private to
              // the new process.
              _io_service.notify_fork(boost::asio::io_service::fork_child);

              // The child won't be accepting new connections, so we can close
              // the acceptor. It remains open in the parent.
              _acceptor.close();

              // The child process is not interested in processing the SIGCHLD
              // signal.
              _signal.cancel();

              Parse_http_request();

            } else {
              // Inform the io_service that the fork is finished (or failed)
              // and that this is the parent process. The io_service uses this
              // opportunity to recreate any internal resources that were
              // cleaned up during preparation for the fork.
              _io_service.notify_fork(boost::asio::io_service::fork_parent);

              // The parent process can now close the newly accepted socket. It
              // remains open in the child.
              _socket.close();
              do_accept();
            }
          } else {
            std::cerr << "Accept error: " << ec.message() << std::endl;
            do_accept();

          }
        });
  }

  void Parse_http_request(){
     _socket.async_read_some(boost::asio::buffer(_data),
                            [this](boost::system::error_code ec,
                                   std::size_t length) {
                              if (!ec) {
                                vector<std::string> request_vec;                       
                                auto result = string(_data.begin(), _data.begin() + length);
                                boost::split( request_vec, result, boost::is_any_of( "\n" ), boost::token_compress_on );
                                //it'll delete "?" which is key to tell if there is query string.   gboost::split( msg1, request_vec.at(0), boost::is_any_of( ":? " ), boost::token_compress_on );
                                map<string, string> client_env;
                                string msg1 = request_vec.at(0);
                                string function;
                                string query;
                                auto method = msg1.substr(0, msg1.find(" "));
                                std::size_t found = msg1.find("?");

                                if (found!=std::string::npos){
                                  function = msg1.substr(msg1.find(" ")+1, msg1.find("?")-1 - msg1.find(" "));
                                  query =  msg1.substr(msg1.find("?")+1, msg1.find_last_of(" ")-1 - msg1.find("?"));
                                }else{
                                  function = msg1.substr(msg1.find(" ")+1, msg1.find_last_of(" ")-1 - msg1.find(" "));
                                  query = "";
                                }

                                auto protocol = msg1.substr(msg1.find_last_of(" ")+1);
                                auto host = request_vec.at(1).substr(request_vec.at(1).find(" ")+1);

                                client_env["REQUEST_METHOD"] = method;
                                client_env["REQUEST_URI"] = function;
                                client_env["QUERY_STRING"] = query;
                                client_env["SERVER_PROTOCOL"] = protocol;
                                client_env["HTTP_HOST"] = host;
                                
                                setenv("REQUEST_METHOD", client_env["REQUEST_METHOD"].c_str(), 1);
                                setenv("REQUEST_URI", client_env["REQUEST_URI"].c_str(), 1);
                                setenv("QUERY_STRING", client_env["QUERY_STRING"].c_str(), 1);
                                setenv("SERVER_PROTOCOL", client_env["SERVER_PROTOCOL"].c_str(), 1);
                                setenv("HTTP_HOST", client_env["HTTP_HOST"].c_str(), 1);
                                setenv("SERVER_ADDR", _socket.local_endpoint().address().to_string().c_str(), 1);
                                setenv("SERVER_PORT", to_string(_socket.local_endpoint().port()).c_str(), 1);
                                setenv("REMOTE_ADDR", _socket.remote_endpoint().address().to_string().c_str(), 1);
                                setenv("REMOTE_PORT", to_string(_socket.remote_endpoint().port()).c_str(), 1);
                                
                                // ------------------------------------- dup -------------------------------------
                                
                                dup2(_socket.native_handle(), 0);
                                dup2(_socket.native_handle(), 1);
                                dup2(_socket.native_handle(), 2);
 
                                // ------------------------------------- exec -------------------------------------
                                
                                std::cout << "HTTP/1.1" << " 200 OK\r\n" << flush;
                                auto program_path = client_env["REQUEST_URI"];
                                program_path = program_path.substr(1);
                                cout << program_path << endl;
                                char *argv[] = {nullptr};
                                if (execv(program_path.c_str(), argv)==-1) {
                                  perror("execv error");
                                  exit(-1);
                                }
                                
                                
                              }
                            });
  }
  boost::asio::io_service &_io_service;
  boost::asio::signal_set _signal;
  tcp::acceptor _acceptor;
  tcp::socket _socket;
  std::array<char, 1024> _data;
};

int main(int argc, char *argv[]) {
  signal(SIGCHLD, SIG_IGN);
  using namespace std;
  try {
    if (argc!=2) {
      std::cerr << "Usage: process_per_connection <port>\n";
      return 1;
    }
    boost::asio::io_service io_service;
    http_server s(io_service, stoi(string(argv[1])));
    io_service.run();
  }
  catch (exception &e) {
    cerr << "Exception: " << e.what() << endl;
  }
}