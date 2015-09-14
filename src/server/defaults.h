#ifndef DEFAULTS_H
#define DEFAULTS_H

//This is the default file where the log is stored
#ifndef LOG_FILE_DEFAULT_LOCATION
#define LOG_FILE_DEFAULT_LOCATION "app_log.log"
#endif

//This is the default file where the configuration is stored
#ifndef CONFIGURATION_FILE_LOCATION
#define CONFIGURATION_FILE_LOCATION "server_configuration.conf"
#endif

//This is the default port. Can be reconfigured in the
//	configuration file
#ifndef DEFAULT_PORT
#define DEFAULT_PORT 1234
#endif

//This is the default socket timeout. When this time limit
//	is exceeded, the connection is closed and an internal server
//	error is sent. This is to free resources from slow clients
#ifndef DEFAULT_HTTP_SOCKET_TIMEOUT
#define DEFAULT_HTTP_SOCKET_TIMEOUT 1
#endif

//This is the maximum number of connections allowed to be processed
//	at any given time. Any more connections will not even be accepted
#ifndef DEFAULT_HTTP_SERVER_MAX_CONNECTIONS
#define DEFAULT_HTTP_SERVER_MAX_CONNECTIONS 512
#endif

//These are the file extensions which are to be cached
#ifndef DEFAULT_HTTP_CACHE_MIMES
#define DEFAULT_HTTP_CACHE_MIMES "*"
#endif

//This is the file cache size (MB)
#ifndef DEFAULT_FILE_CACHE_SIZE
#define DEFAULT_FILE_CACHE_SIZE 768
#endif

#endif // DEFAULTS_H