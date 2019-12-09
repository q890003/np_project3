#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
using namespace std;

using namespace boost::asio;
using namespace boost::asio::ip;

io_service service;
class console_format{
    public:
        class Npshell {
            public:
                std::string name;
                std::string port;
                std::string file;
            }npshells[5];
        string console_response(string querry_env){
            std::string console_msg ="\
            <html lang=\"en\">\
                <head>\
                <meta charset=\"UTF-8\" />\
                <title>NP Project 3 Console</title>\
                <link\
                    rel=\"stylesheet\"\
                    href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\"\
                    integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\"\
                    crossorigin=\"anonymous\"\
                />\
                <link\
                    href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\
                    rel=\"stylesheet\"\
                />\
                <link\
                    rel=\"icon\"\
                    type=\"image/png\"\
                    href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\
                />\
                <style>\
                    * {\
                        font-family: 'Source Code Pro', monospace;\
                        font-size: 1rem !important;\
                    }\
                    body {\
                        background-color: #212529;\
                    }\
                    pre {\
                        color: #cccccc;\
                    }\
                    b {\
                        color: #ffffff;\
                    }\
                </style>\
            </html>\
            ";
            boost::format fmt("  <body>\n"
                      "    <table class=\"table table-dark table-bordered\">\n"
                      "      <thead>\n"
                      "        <th>\n"
                      "%1%"
                      "        </th>\n"
                      "      </thead>\n"
                      "      <tbody>\n"
                      "        <th>\n"
                      "%2%"
                      "        </th>\n"
                      "      </tbody>\n"
                      "    </table>\n"
                      "  </body>\n");
            vector<string> test;
            boost::split(test, querry_env, boost::is_any_of("&"));
            std::string key;
            std::string value;
            int shell_num = 0;
            int tt = 0;
            for(auto &it : test){
                key =  it.substr(0, it.find('='));
                value = it.substr(key.size() + 1);

                if (!value.empty()) {
                    if(key.front() == 'h')
                        npshells[shell_num].name = value;
                    else if(key.front() == 'p')
                        npshells[shell_num].port = value;
                    else if(key.front() == 'f'){
                        npshells[shell_num].file = value;
                    }
                    tt++;
                    if(tt%3==0)
                        shell_num ++;
                }
            }
            /*
            for(int i = 0 ; i < 5 ; i++){
            cout << npshells[i].name << "," << npshells[i].file << endl;
            }
            */
            std::string table;
            std::string session;
            int count = 0;
            for(auto npshell : npshells){
                if(!npshell.name.empty()){
                    table +="            <th scope=\"col\">" + npshell.name + ":" + npshell.port + "</th>\n";
                    session +="          <td><pre id=\"s" + std::to_string(count) + "\" class=\"mb-0\"></pre></td>";
                    count++;
                }
            }
            return console_msg + (fmt%table%session).str() + "</html>\n";
        }


};

class Appender {
 public:
  static Appender &getInstance() {
    static Appender instance;
    return instance;
  }

  Appender(const Appender &) = delete;

  void operator=(const Appender &) = delete;

  void output_shell(std::string session, std::string content) {
    encode(content);
    boost::replace_all(content, "\n", "&#13;");
    boost::replace_all(content, "\r", "");
    boost::format fmt
        ("<script>document.getElementById('%1%').innerHTML += '%2%';</script>");
    std::cout << fmt%session%content;
    std::cout.flush();
  }

  void output_command(std::string session, std::string content) {
    encode(content);
    boost::replace_all(content, "\n", "&#13;");
    boost::replace_all(content, "\r", "");
    boost::format fmt
        ("<script>document.getElementById('%1%').innerHTML += '<b>%2%</b>';</script>");
    std::cout << fmt%session%content;
    std::cout.flush();
  }

 private:
  Appender() = default;

  void encode(std::string &data) {
    using boost::algorithm::replace_all;
    replace_all(data, "&", "&amp;");
    replace_all(data, "\"", "&quot;");
    replace_all(data, "\'", "&apos;");
    replace_all(data, "<", "&lt;");
    replace_all(data, ">", "&gt;");
  }
};

struct Client : public std::enable_shared_from_this<Client> {
  Client(std::string session, std::string file_name, tcp::resolver::query q)
      : session(std::move(session)), q(std::move(q)) {
    file.open("test_case/" + file_name, std::ios::in);
    if (!file.is_open())
      Appender::getInstance().output_command(session, "DAMN file not open!");
  }

  void read_handler() {
    auto self(shared_from_this());
    tcp_socket.async_receive(buffer(bytes),
                             [this, self](boost::system::error_code ec,
                                          std::size_t length) {
                               if (!ec) {
                                 std::string data(bytes.begin(),
                                                  bytes.begin()
                                                      + length); // this might not be a entire line!
                                 Appender::getInstance()
                                     .output_shell(session, data);

                                 if (data.find("% ")!=std::string::npos) {
                                   std::string command;
                                   getline(file, command);
                                   Appender::getInstance()
                                       .output_command(session, command + '\n');
                                   tcp_socket
                                       .write_some(buffer(command + '\n'));
                                   //boost::asio::write(tcp_socket, buffer(command + "\r\n", command.size()+2));
                                 }
                                 read_handler();
                               }
                             });
  }

  void connect_handler() {
    read_handler();
  }

  void resolve_handler(tcp::resolver::iterator it) {
    auto self(shared_from_this());
    tcp_socket.async_connect(*it, [this, self](boost::system::error_code ec) {
      if (!ec)
        connect_handler();
    });
  }

  void resolve() {
    auto self(shared_from_this());
    resolver.async_resolve(q,
                           [this, self](const boost::system::error_code &ec,
                                        tcp::resolver::iterator it) {
                             if (!ec)
                               resolve_handler(it);
                           });
  }

  std::string session; // e.g. "s0"
  tcp::resolver::query q;
  tcp::resolver resolver{service};
  tcp::socket tcp_socket{service};
  std::array<char, 4096> bytes;
  std::fstream file; // e.g. "t1.txt"
};
int main() {
  /* [Required] HTTP Header */
           
    cout << "Content-type: text/html\r\n\r\n";
  
    string a = getenv("QUERY_STRING");
    console_format console_me;
    cout  << console_me.console_response(getenv("QUERY_STRING"));
    int k = 0;
    for (auto npshell : console_me.npshells) {
    if (!npshell.port.empty()) {
      tcp::resolver::query q{npshell.name, npshell.port};
      std::make_shared<Client>("s" + std::to_string(k), npshell.file, std::move(q))
          ->resolve();
        k++;
    }
  }

  service.run();

  return 0;
}