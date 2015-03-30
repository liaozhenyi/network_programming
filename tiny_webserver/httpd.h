#ifndef __HTTPD_H_
#define __HTTPD_H_

#define BUFFER_SIZE	4096
#define METHOD_SIZE	256
#define URI_SIZE	1024
#define FILENAME_SIZE	512
#define CGI_SIZE	512
#define BODY_SIZE	4096

#define CGI_REP		"cgi-bin"
#define DEFAULT_PAGE	"index.html"
#define CONTENT_LENGTH	"Content-Length:"

void *handle_request(void *param);
void sigint_handler(int sig);
int prase_uri(char *uri, char *filename, char *cgiargs);
int check_filepath(char *filename);
void clienterror(int fd, const char *cause, const char *errnum, \
			const char *shortmsg, const char *longmsg);
void serve_static(int fd, const char *filename, int fileSize);
void serve_dynamic(int fd, const char *filename, \
			const char *method, const char *cgiargs);
void read_requesthdrs(buf_fd_t *bp);


#endif	// __HTTPD_H_
