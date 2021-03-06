#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>		// struct sockaddr_in
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "util.h"
#include "socketio.h"
#include "error.h"
#include "httpd.h"


int main(int argc, char *argv[]) {
	int sockfd, port, clientfd;
	socklen_t addrlen;
	struct sockaddr_in clientaddr;

	if (argc != 2)
		err_msg("Usage: %s port", argv[1]);

	port = atoi(argv[1]);
	addrlen = sizeof(clientaddr);
	sockfd = open_listenfd(port);

	Signal(SIGINT, sigint_handler);

	while (1) {
		pthread_t thread;

		clientfd = Accept(sockfd, (SA *)&clientaddr, &addrlen);
		pthread_create(&thread, NULL, handle_request, &clientfd);
		pthread_join(thread, NULL);
	}
	close(sockfd);

	return 0;
}

void sigint_handler(int sig) {
	printf("Caught SIGINT\n");
	exit(0);
}

void *handle_request(void *param) {
	int clientfd = *((int *)param);
	int is_static;
	char buf[BUFFER_SIZE], method[METHOD_SIZE];
	char uri[URI_SIZE], filename[FILENAME_SIZE];
	char cgiargs[CGI_SIZE];
	buf_fd_t bf;
	ssize_t nread;
	
	memset(buf, 0, sizeof(buf));
	buf_init(clientfd, &bf);
	nread = buf_readline(&bf, buf, sizeof(buf));
	if (nread < 1) {
		clienterror(clientfd, buf, "400", "Bad request", \
				"Tiny couldn't parse the request");
		return NULL;
	}
	
	sscanf(buf, "%s %s", method, uri);
	if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
		clienterror(clientfd, method, "501", "Not implemented", \
				"Tiny doesn't implement this method");
		return NULL;
	}

	is_static = prase_uri(uri, filename, cgiargs);
	struct stat fileStat;
	if (!check_filepath(filename) || stat(filename, &fileStat) < 0) {
		clienterror(clientfd, filename, "404", "Not found", \
				"Tiny couldn't find this file");
		return NULL;
	}

	if (is_static) {
		if (!S_ISREG(fileStat.st_mode) || !(S_IRUSR & fileStat.st_mode)) {
			clienterror(clientfd, filename, "403", "Forbidden", \
					"Tiny couldn't read this file");
			return NULL;
		}
		serve_static(clientfd, filename, fileStat.st_size);
	} else {
		if (!S_ISREG(fileStat.st_mode) || !(S_IXUSR & fileStat.st_mode)) {
			clienterror(clientfd, filename, "403", "Forbidden", \
					"Tiny couldn't run the CGI program");
			return NULL;
		}
		serve_dynamic(clientfd, filename, method, cgiargs);
	}
		
	close(clientfd);
	return NULL;
}

/*
 *  If there exists "cgi_bin" substr in uri, then the request is dynamic,
 * the filepath and the cgiargs is seperate by the '?'; otherwise the 
 * request is static.
 *  Also notice that the root directory of tinyhttpd is not the same as
 * the root of your OS root filesystem, change it to a relative path
 * by adding '.' before the filepath; if the last character of filepath
 * is '/', add "index.html" after the filename.
 *  Return value, 1: static request; 0: dynamic request
 */
int prase_uri(char *uri, char *filename, char *cgiargs) {
	char *pos;
	int ret;

	if (strstr(uri, CGI_REP)) {
		ret = 0;
		if((pos = strchr(uri, '?'))) {
			*pos = '\0';
			strncpy(cgiargs, pos+1, CGI_SIZE-1);
		} else {
			*cgiargs = '\0';
		}
	} else {
		ret = 1;
		*cgiargs = '\0';
	}

	strcpy(filename, ".");
	strncat(filename, uri, FILENAME_SIZE-1-strlen(filename));
	if (filename[strlen(filename)-1] == '/')
		strncat(filename, DEFAULT_PAGE, FILENAME_SIZE-1-strlen(filename));

	cgiargs[CGI_SIZE-1] = '\0';
	filename[FILENAME_SIZE-1] = '\0';

	return ret;
}

/*
 * If client request files which locate out of the web root directory,
 * block them.
 * Return 1: filename is safe    0: unsafe
 */
int check_filepath(char *filename) {
	int safe = 0;

	if (*filename != '/')
		return 0;

	char filepath[FILENAME_SIZE];
	char *start = filepath, *end = NULL;
	strncpy(filepath, filename, FILENAME_SIZE-1);

	while ((end = strchr(start, '/'))) {
		if (start != end) {
			*end = '\0';
			if (0 == strcmp(start, "..")) {
				if (--safe < 0)
					return 0;
			} else {
				++safe;
			}
		}
		start = end+1;
	}
	++safe;
	
	return safe > 0;
}

void clienterror(int fd, const char *cause, const char *errnum, \
			const char *shortmsg, const char *longmsg) {
	char buf[BUFFER_SIZE], body[BODY_SIZE];

	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body>\r\n%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr>The Tiny Web server\r\n", body);

	sprintf(buf, "HTTP/1.0 %s %s \r\n", errnum, shortmsg);
	writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
	writen(fd, buf, strlen(buf));
	writen(fd, body, strlen(body));
}

void serve_static(int fd, const char *filename, int filesize) {
	int filefd;
	ssize_t nread;
	char buf[BUFFER_SIZE], body[BODY_SIZE];
	buf_fd_t bf;
	
	buf_init(fd, &bf);
	read_requesthdrs(&bf);
	sprintf(body, "HTTP/1.0 200 OK\r\n");
	sprintf(body, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(body, "%sContent-length: %d\r\n", buf, filesize);
	sprintf(body, "%sContent-type: %s\r\n\r\n", buf, "static file");
	writen(fd, body, strlen(body));

	filefd = open(filename, O_RDONLY, 0);
	while ((nread = readn(filefd, buf, sizeof(buf)))) {
		writen(fd, buf, nread);
	}
}

/*
 * Method == GET.
 * Discard the request head and the request content.
 */
void read_requesthdrs(buf_fd_t *bp) {
	char buf[BUFFER_SIZE];
	ssize_t nread;

	nread = buf_readline(bp, buf, BUFFER_SIZE-1);
	while (nread > 0 && strcmp(buf, "\r\n")) {
		nread = buf_readline(bp, buf, BUFFER_SIZE);
	}
}

/*
 * Method == POST 
 * We don't care too much about the header except for the content-length.
 *
 * Discard requests head until the line contains "content-length",
 * Return the content-length if it was found, otherwise return -1 
 * which indicates an error.
 */
int read_request_content_length(buf_fd+t *bp) {
	int nread;
	char buf[BUFFER_SIZE];

	nread = buf_readline(bp, buf, BUFFER_SIZE-1);
	while (nread > 0 && 0 != strcmp(buf, "\r\n")) {
		if (strcasecmp(buf, CONTENT_LENGTH))
			return atoi(&buf[sizeof(CONTENT_LENGTH)]);
		nread = buf_readline(bp, buf, BUFFER_SIZE-1);
	}
	return -1;
}

void serve_dynamic(int fd, const char *filename, \
			const char *method, const char *cgiargs) {
	char buf[BUFFER_SIZE], body[BODY_SIZE];
	int pipe_in[2], pipe_out[2];
	buf_fd_t bf;
	ssize_t nread;
	int content_length = 0;
	int pid, status;

	buf_init(fd, &bf);
	if (!strcasecmp(method, "GET")) {
		read_requesthdrs(&bf);
	} else {
		content_length = read_request_content_length(&bf);
	}

	sprintf(body, "HTTP/1.0 200 OK\r\n");
	sprintf(body, "%sServer: Tiny Web Server\r\n", body);
	writen(fd, body, strlen(body));

	// pipe_in: child->parent; pipe_out: parent->child
	Pipe(pipe_in);
	Pipe(pipe_out);

	if ((pid = Fork())) {
		close(pipe_in[1]);
		close(pipe_out[0]);
		if (0 == strcasecmp("POST", method)) {
			nread = buf_readline(&bf, buf, BUFFER_SIZE-1);
			while (nread > 0 && strcmp("\r\n", buf)) {
				writen(pipe_out[1], buf, nread);
				nread = buf_readline(&bf, buf, BUFFER_SIZE-1);
			}
		}

		buf_fd_t in_bf;
		buf_init(pipe_in[0], &in_bf);
		nread = buf_readline(&in_bf, body, BODY_SIZE-1);
		while (nread > 0 && strcmp("\n", body)) {
			writen(fd, body, strlen(body));
			nread = buf_readline(&in_bf, body, BODY_SIZE-1);
		}
			
		close(pipe_in[0]);
		close(pipe_out[1]);
		close(fd);
		waitpid(pid, &status, 0);
		
	} else {
		char meth_env[METHOD_SIZE], cgi_env[CGI_SIZE], length_env[BUFFER_SIZE];

		close(fd);
		close(pipe_in[0]);
		close(pipe_out[1]);
		dup2(pipe_in[1], STDOUT_FILENO);
		dup2(pipe_out[0], STDIN_FILENO);

		sprintf(meth_env, "REQUEST_METHOD=%s", method);
		putenv(meth_env);
		if (0 == strcasecmp(method, "GET")) {
			sprintf(cgi_env, "QUERY_METHOD=%s", cgiargs);
			putenv(cgi_env);
		} else {
			sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
			putenv(length_env);
		}
		execl(filename, NULL, NULL);

		close(pipe_out[0]);
		close(pipe_in[1]);
		exit(0);
	}
	
	exit(0);
}

