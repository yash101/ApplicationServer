#ifdef ENABLE_HTTP_SERVER_EXTRA_FUNCTIONS
#ifndef HTTPSERVEREXTRAFUNCTIONS_H
#define HTTPSERVEREXTRAFUNCTIONS_H
#include <string>
#include "httpserver.h"

void getDefaultHomePage(server::HttpServerSession* session);
#endif // HTTPSERVEREXTRAFUNCTIONS_H
#endif
