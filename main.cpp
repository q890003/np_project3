#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>   //for repace_all, split and so on
#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <unistd.h>

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

boost::asio::io_service io_service_test;



//////////////////////////////////////////////////////////////////////
/*                           panel                                  */
//////////////////////////////////////////////////////////////////////
class panel_CGI {
  public: 
    static std::string bg_form(){
      vector<std::string> hosts_list = {
                                "<option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option>",
                                "<option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option>",
                                "<option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option>",
                                "<option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option>",
                                "<option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option>",
                                "<option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option>",
                                "<option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option>",
                                "<option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option>",
                                "<option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option>",
                                "<option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option>"};
      vector<std::string> testcase_list = {
                                      "<option value=\"t1.txt\">t1.txt</option>",
                                      "<option value=\"t2.txt\">t2.txt</option>",
                                      "<option value=\"t3.txt\">t3.txt</option>",
                                      "<option value=\"t4.txt\">t4.txt</option>",
                                      "<option value=\"t5.txt\">t5.txt</option>",
                                      "<option value=\"t6.txt\">t6.txt</option>",
                                      "<option value=\"t7.txt\">t7.txt</option>",
                                      "<option value=\"t8.txt\">t8.txt</option>",
                                      "<option value=\"t9.txt\">t9.txt</option>",
                                      "<option value=\"t10.txt\">t10.txt</option>"};

      std::string hosts_menu(boost::algorithm::join(hosts_list, ""));
      std::string testcase_menu(boost::algorithm::join(testcase_list, ""));
      std::string bg =
              "<!DOCTYPE html>\n"
              "<html lang=\"en\">\n"
              "  <head>\n"
              "    <title>NP Project 3 Panel</title>\n"
              "    <link\n"
              "      rel=\"stylesheet\"\n"
              "      "
              "href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/"
              "bootstrap.min.css\"\n"
              "      "
              "integrity=\"sha384-MCw98/"
              "SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\"\n"
              "      crossorigin=\"anonymous\"\n"
              "    />\n"
              "    <link\n"
              "      "
              "href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n"
              "      rel=\"stylesheet\"\n"
              "    />\n"
              "    <link\n"
              "      rel=\"icon\"\n"
              "      type=\"image/png\"\n"
              "      "
              "href=\"https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/"
              "512/dashboard-512.png\"\n"
              "    />\n"
              "    <style>\n"
              "      * {\n"
              "        font-family: 'Source Code Pro', monospace;\n"
              "      }\n"
              "    </style>\n"
              "  </head>\n"
              "  <body class=\"bg-secondary pt-5\">"
              "<form action=\"console.cgi\" method=\"GET\">\n"
              "      <table class=\"table mx-auto bg-light\" style=\"width: "
              "inherit\">\n"
              "        <thead class=\"thead-dark\">\n"
              "          <tr>\n"
              "            <th scope=\"col\">#</th>\n"
              "            <th scope=\"col\">Host</th>\n"
              "            <th scope=\"col\">Port</th>\n"
              "            <th scope=\"col\">Input File</th>\n"
              "          </tr>\n"
              "        </thead>\n"
              "        <tbody>";

      boost::format fmt(
              "          <tr>\n"
              "            <th scope=\"row\" class=\"align-middle\">Session "
              "%1%</th>\n"
              "            <td>\n"
              "              <div class=\"input-group\">\n"
              "                <select name=\"h%3%\" class=\"custom-select\">\n"
              "                  <option></option>{%2%}\n"
              "                </select>\n"
              "                <div class=\"input-group-append\">\n"
              "                  <span "
              "class=\"input-group-text\">.cs.nctu.edu.tw</span>\n"
              "                </div>\n"
              "              </div>\n"
              "            </td>\n"
              "            <td>\n"
              "              <input name=\"p%3%\" type=\"text\" "
              "class=\"form-control\" size=\"5\" />\n"
              "            </td>\n"
              "            <td>\n"
              "              <select name=\"f%3%\" class=\"custom-select\">\n"
              "                <option></option>\n"
              "               %4%\n"
              "              </select>\n"
              "            </td>\n"
              "          </tr>");
              
      for (int i = 0; i < 5; ++i) {
        bg += (fmt %(i+1) %hosts_menu %i %testcase_menu).str();
      }
      bg +="          <tr>\n"
            "            <td colspan=\"3\"></td>\n"
            "            <td>\n"
            "              <button type=\"submit\" class=\"btn btn-info "
            "btn-block\">Run</button>\n"
            "            </td>\n"
            "          </tr>\n"
            "        </tbody>\n"
            "      </table>\n"
            "    </form>\n"
            "  </body>\n"
            "</html>";
        return bg;
    }
};

//////////////////////////////////////////////////////////////////////
/*                          console                                 */
//////////////////////////////////////////////////////////////////////
class Shell_form{
    public:
        class Npshell {
            public:
                std::string name;   //differenciate from boost::algorithm::string.hpp>
                std::string port;
                std::string file;
            }npshells[5];
        string consoles_init(string querry_env){   //string is an object.

            /* format for intial setting */
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
            </html>";

            /* format for intial interface */
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
                        shell_num ++;
                    }
                }
            }
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

//////////////////////////////////////////////////////////////////////
/*                        Session_handler                           */
//////////////////////////////////////////////////////////////////////
class Session_handler : public std::enable_shared_from_this<Session_handler>{
  public:
    class Client : public std::enable_shared_from_this<Client> {         //structure default mem type is public.
    public:
      Client(const shared_ptr<Session_handler> &ptr, std::string session, std::string file_name, tcp::resolver::query q)    
          : session_ptr(ptr), session(std::move(session)), to_npsehll_info(std::move(q)) {
        file.open("test_case/" + file_name, std::ios::in);
        if (!file.is_open()){
          boost::asio::write(session_ptr->socket_to_client, boost::asio::buffer(output_command("file open failed!") ) );
          output_command("file open failed!");
          cout << "file open failed!!!!!!!!!!!!" << endl;
        }
      }

      void resolve() {
        auto self(shared_from_this());
        resolver.async_resolve(to_npsehll_info,
                              [this, self](const boost::system::error_code &ec,
                                            tcp::resolver::iterator it) {
                                if (!ec)
                                  cout <<session <<  " async_resolve success" << endl;
                                  resolve_handler(it);
                              });
      }

      void resolve_handler(tcp::resolver::iterator it) {
        auto self(shared_from_this());
        tcp_socket.async_connect(*it, [this, self](boost::system::error_code ec) {    //the socket is not returned to the closed state.
          if (!ec)
            cout <<session <<  " async_connect success"<< endl;
            read_handler();
        });
      }
      void read_handler() {
        auto self(shared_from_this());
        /* format note:
          this ptr for calling receive_msg, shell_output, etc. they all non-static member only can be used after instanced.
        self is for prolonging life time of smart_ptr.
          function note: 
          async_receive, when receive msg, it's triggered. 
        */
        tcp_socket.async_receive(boost::asio::buffer(receive_msg),
                                [this, self](boost::system::error_code ec,   
                                              std::size_t bytes_transferred) {
                                  if (!ec) { //if no error code.
                                    cout << session << ",  async_receive\n"; cout.flush();
                                    std::string data(receive_msg.begin(),receive_msg.begin() + bytes_transferred);
                                    cout << session << ",  test1\n"; cout.flush(); 
                                    boost::asio::write(session_ptr->socket_to_client, boost::asio::buffer(shell_output(data) ));
                                    cout << session << ",  test2\n"; cout.flush();
                                    cout << "receive data is " << data << flush;
                                    if (data.find("% ")!=std::string::npos) {
                                      std::string command;
                                      cout << session << ",  test3\n"; cout.flush();
                                      getline(file, command);
                                      cout << session << ",  test4\n"; cout.flush();
                                      boost::asio::write(session_ptr->socket_to_client, boost::asio::buffer(output_command(command + '\n') ) );

                                      cout << session << " sending cmd:" <<command <<  endl;
                                      tcp_socket.write_some(buffer( command + '\n'));
                                      //boost::asio::write(tcp_socket, buffer(command + "\r\n", command.size()+2));
                                    }
                                    
                                    read_handler(); 
                                  }else{
                                    cout << session << "_err: " <<ec << endl;
                                  }
                                });
      }

      string shell_output(std::string content) {
        encode(content);
        boost::replace_all(content, "\n", "&#13;");
        boost::replace_all(content, "\r", "");
        boost::format fmt("<script>document.getElementById('%1%').innerHTML += '%2%';</script>");
        return (fmt%session%content).str();
      }

      string output_command(std::string content) {
        encode(content);
        boost::replace_all(content, "\n", "&#13;");
        boost::replace_all(content, "\r", "");
        boost::format fmt("<script>document.getElementById('%1%').innerHTML += '<b>%2%</b>';</script>");
        return (fmt%session%content).str();
      }

      void encode(std::string &data) {
        using boost::algorithm::replace_all;   
        /*
        1. using 名稱空間::成員
        2. using namespace 名稱空間
        two ways to indicate member's namespace. 
        */
        replace_all(data, "&", "&amp;");
        replace_all(data, "\"", "&quot;");
        replace_all(data, "\'", "&apos;");
        replace_all(data, "<", "&lt;");
        replace_all(data, ">", "&gt;");
      }
      shared_ptr<Session_handler> session_ptr;
      std::string session; // "s0~s4"
      tcp::resolver::query to_npsehll_info;
      tcp::resolver resolver{io_service_test};    //initial value, service which is global value.
      tcp::socket tcp_socket{io_service_test};    //initial value, service which is global value.
      std::array<char, 4096> receive_msg;
      std::fstream file; // "t1~t4.txt"
    };
    Session_handler(tcp::socket socket) : socket_to_client(std::move(socket)){}
    void cgi_handler(){
      auto self(shared_from_this());
      socket_to_client.async_read_some(boost::asio::buffer(receive_msg),
                                      [this,self](boost::system::error_code ec,
                                      std::size_t length) {
                                if (!ec) {
                                  vector<std::string> request_vec;
                                  vector<std::string> parse_request_line;
                                  vector<std::string> querry_line;

                                  map<std::string, std::string> client_env;
                                  std::string result(receive_msg.begin(),receive_msg.begin() + length);
                                  boost::split( request_vec, result, boost::is_any_of( "\n" ), boost::token_compress_on);
                                  for(auto &it : request_vec){
                                    if(it.back()== '\r')
                                      it.pop_back();
                                  }
                                  boost::split( parse_request_line, request_vec.at(0), boost::is_any_of( " " ), boost::token_compress_on );
                                  boost::split( querry_line, parse_request_line.at(1), boost::is_any_of( "?" ), boost::token_compress_on );
                                  if(querry_line.size() <2){ 
                                    client_env["QUERY_STRING"] = "";
                                  }else{
                                    client_env["QUERY_STRING"] = querry_line.at(1);
                                  }

                                  client_env["REQUEST_URI"] = querry_line.at(0);
                                  if(client_env["REQUEST_URI"] == "/panel.cgi"){
                                    std::string print_panel_cgi = "HTTP/1.1 200 OK\r\n"
                                                              "Content-type: text/html\r\n\r\n";
                                    print_panel_cgi += panel_CGI::bg_form();
                                    do_write(print_panel_cgi);
                                    cout << client_env["REQUEST_URI"] << endl;
                                  }else if(client_env["REQUEST_URI"] == "/console.cgi"){
                                    std::string print_console_cgi = "HTTP/1.1 200 OK\r\n"
                                                                "Content-type: text/html\r\n\r\n";
                                    Shell_form form1;
                                    print_console_cgi += form1.consoles_init(client_env["QUERY_STRING"]);   //set initial ip/file_name/port sesssion name and so on.
                                    do_write(print_console_cgi);
                                           cout << "do_write(print_console_cgi)\n"; cout.flush();
                                    int k = 0;
                                    for (auto npshell : form1.npshells) {
                                      if (!npshell.port.empty()) {
                                        tcp::resolver::query shell_connection_info{npshell.name, npshell.port};     //{ip, port}
                                        std::make_shared<Client>(self, "s" + std::to_string(k), npshell.file, shell_connection_info)   //socket{1~5}, file name, query(name,port)
                                            ->resolve();
                                        k++;
                                      }
                                    }
                                  }
                                }
                              });
    }
    void do_write(string s) {
      auto self(shared_from_this());
      boost::asio::async_write(
          socket_to_client, boost::asio::buffer(s.c_str(), s.length()),
          [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
              //no other operation.
            }
          });
    }
    tcp::socket socket_to_client;   // initial at constructor.
    std::array<char, 4096> receive_msg;
};
//////////////////////////////////////////////////////////////////////
/*                        http_server                               */
//////////////////////////////////////////////////////////////////////
class http_server{
 public:
  http_server(short port) : _acceptor(io_service_test,  tcp::endpoint(tcp::v4(), port)) {
    do_accept();

  }
 private:
  void do_accept() {
    _acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket_to_client) {
          if (!ec) {
            std::make_shared<Session_handler>(std::move(socket_to_client))->cgi_handler();
          }
            do_accept();
        });
  }
  tcp::acceptor _acceptor;
};
int main(int argc, char *argv[]) {
  try {
    if (argc!=2) {
      std::cerr << "Usage: process_per_connection <port>\n";
      return 1;
    }
    http_server session(std::atoi(argv[1]));
    io_service_test.run();
  }
  catch (exception &ecep) {
    cerr << "Exception: " << ecep.what() << endl;
  }

  
  return 0;
}