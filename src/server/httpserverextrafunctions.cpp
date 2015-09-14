#define ENABLE_HTTP_SERVER_EXTRA_FUNCTIONS
#include "httpserverextrafunctions.h"
#include <iostream>
#include <sstream>
void getDefaultHomePage(server::HttpServerSession* session)
{
  std::stringstream s;
#define o s << std::endl <<

  o "<!DOCTYPE html>";
  o "<html>";
    o "<head>";
      o "<title>Web Server Test Page</title>";

      o "<style type=\"text/css\">";

      o "body > div {";
        o "width: 100%;";
        o "top: 0px; left: 0px;";
        o "background-color: #C6C6C6;";
      o "}";

      o "table {";
        o "width: 100%;";
        o "border: 2px solid black;";
      o "}";

      o "table th:nth-child(1), table td:nth-child(1) {";
        o "width: 20%;";
      o "}";

      o "table th {";
        o "border: 1px solid black;";
      o "}";

      o "table td {";
        o "border-bottom: 1px solid black;";
        o "border-right: 1px solid black;";
      o "}";

      o "input {";
        o "width: 100%;";
      o "}";
      o "</style>";

    o "</head>";

    o "<body>";
      o "<div id=\"content\">";

        o "<h1><center>Web Server Test Page</center></h1>";
        o "<table>";
          o "<tr>";
            o "<th>Name</th><th>Value</th>";
          o "</tr>";

          o "<tr>";
            o "<td>Protocol</td><td>" << (session->ProtocolVersion == server::HTTP_1_0 ? "HTTP/1.0 (HTTP 1.0)" : "HTTP/1.1 (HTTP 1.1)") << "</td>";
          o "</tr>";

          o "<tr>";
            o "<td>Unprocessed Path</td><td>[" << session->UnprocessedPath << "]</td>";
          o "</tr>";

          o "<tr>";
            o "<td>Processed Path</td><td>[" << session->Path << "]</td>";
          o "</tr>";

          o "<tr>";
            o "<td>Request Type/Method</td><td>" << (session->RequestType == server::GET ? "GET" : "POST") << "</td>";
          o "</tr>";

          o "<tr>";
            o "<td>Incoming Headers</td>";
            o "<td>";
              o "<table>";
                o "<thead>";
                  o "<th>Key</th>";
                  o "<th>Value</th>";
                o "</thead>";
                o "<tbody>";
                for(std::map<std::string,std::string>::const_iterator it = session->incoming_headers.begin(); it != session->incoming_headers.end(); ++it)
                {
                  o "<tr>";
                    o "<td>[" << it->first << "]</td>";
                    o "<td>[" << it->second << "]</td>";
                  o "</tr>";
                }
                o "</tbody>";
              o "</table>";
            o "</td>";
          o "</tr>";

           o "<tr>";
            o "<td>Incoming Cookies</td>";
            o "<td>";
              o "<table>";
                o "<thead>";
                  o "<th>Key</th>";
                  o "<th>Value</th>";
                o "</thead>";
                o "<tbody>";
                for(std::map<std::string, server::HttpCookie>::const_iterator it = session->incoming_cookies.begin(); it != session->incoming_cookies.end(); ++it)
                {
                  o "<tr>";
                    o "<td>[" << it->first << "]</td>";
                    o "<td>[" << it->second.value << "]</td>";
                  o "</tr>";
                }
                o "</tbody>";
              o "</table>";
            o "</td>";
          o "</tr>";

         o "<tr>";
            o "<td>Cookies</td>";
            o "<td>";
              o "<table>";
                o "<thead>";
                  o "<th>Key</th>";
                  o "<th>Value</th>";
                o "</thead>";
                o "<tbody>";
                for(std::map<std::string, std::string>::const_iterator it = session->cookies.begin(); it != session->cookies.end(); ++it)
                {
                  o "<tr>";
                    o "<td>[" << it->first << "]</td>";
                    o "<td>[" << it->second << "]</td>";
                  o "</tr>";
                }
                o "</tbody>";
              o "</table>";
            o "</td>";
          o "</tr>";

          o "<tr>";
            o "<td>GET Queries</td>";
            o "<td>";
              o "<table>";
                o "<thead>";
                  o "<th>Key</th>";
                  o "<th>Value</th>";
                o "</thead>";
                o "<tbody>";
                for(std::map<std::string, std::string>::const_iterator it = session->get.begin(); it != session->get.end(); ++it)
                {
                  o "<tr>";
                    o "<td>[" << it->first << "]</td>";
                    o "<td>[" << it->second << "]</td>";
                  o "</tr>";
                }
                o "</tbody>";
              o "</table>";
            o "</td>";
          o "</tr>";

          o "<tr>";
            o "<td>POST Queries</td>";
            o "<td>";
              o "<table>";
                o "<thead>";
                  o "<th>Key</th>";
                  o "<th>Value</th>";
                o "</thead>";
                o "<tbody>";
                for(std::map<std::string, server::HttpPost>::const_iterator it = session->post.begin(); it != session->post.end(); ++it)
                {
                  o "<tr>";
                    o "<td>[" << it->first << "]</td>";
                    o "<td>[" << it->second.value << "]</td>";
                  o "</tr>";
                }
                o "</tbody>";
              o "</table>";
            o "</td>";
          o "</tr>";

          o "<tr>";
            o "<td>Headers</td>";
            o "<td>";
              o "<table>";
                o "<thead>";
                  o "<th>Key</th>";
                  o "<th>Value</th>";
                o "</thead>";
                o "<tbody>";
                for(std::map<std::string, std::string>::const_iterator it = session->headers.begin(); it != session->headers.end(); ++it)
                {
                  o "<tr>";
                    o "<td>[" << it->first << "]</td>";
                    o "<td>[" << it->second << "]</td>";
                  o "</tr>";
                }
                o "</tbody>";
              o "</table>";
            o "</td>";
          o "</tr>";

          o "<tr>";
            o "<td>POST/Cookie test</td>";
            o "<td>";
              o "<form action=\"/cookie_add\" method=\"POST\">";
                o "<table>";
                  o "<thead>";

                  for(std::map<std::string, server::HttpPost>::const_iterator it = session->post.begin(); it != session->post.end(); ++it)
                  {
                    o "<tr style=\"display: none;\">";
                      o "<td>" << it->first << "</td>";
                      o "<input type=\"text\" name=\"" << it->first << "\" value=\"" << it->second.value << "\">";
                    o "</tr>";
                  }

                  o "<tr>";
                    o "<td colspan=\"2\"><input type=\"text\" name=\"TestText\" value=\"Send as 'TestText'\"></td>";
                  o "</tr>";

                  o "</thead>";
                o "</table>";
              o "</form>";
            o "</td>";
          o "</tr>";

        o "</table>";

      o "</div>";
    o "</body>";

  o "</html>";

#undef o
  if(session->Path == "/")
  {

    session->Response.type = server::STRING;
    session->Response.stype = s.str();
  }
  else if(session->Path == "/cookie_add")
  {
    session->ResponseStatusCode = 301;
    session->headers["location"] = "/";
    session->cookies["TestText"] = session->post["TestText"].value;
    session->Response.type = server::STRING;
    session->Response.stype = s.str();
  }
  else
  {
    session->ResponseStatusCode = 404;
    session->Response.type = server::STRING;
    session->Response.stype = s.str();
  }
}
