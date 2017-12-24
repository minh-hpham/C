/*
 * friendlist.c - [Starting code for] a web-based friend-graph manager.
 *
 * Based on:
 *  tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *      GET method to serve static and dynamic content.
 *   Tiny Web server
 *   Dave O'Hallaron
 *   Carnegie Mellon University
 */
#include "csapp.h"
#include "dictionary.h"
#include "more_string.h"

static void doit(int fd);
void *go_doit(void *connfdp);
static dictionary_t *read_requesthdrs(rio_t *rp);
static void read_postquery(rio_t *rp, dictionary_t *headers, dictionary_t *d);
static void clienterror(int fd, char *cause, char *errnum, 
                        char *shortmsg, char *longmsg);
static void print_stringdictionary(dictionary_t *d);
static void introduce_request(int fd, dictionary_t *query);
static void friend_request(int fd, dictionary_t *query);
static void befriend_request(int fd, dictionary_t *query);
static void unfriend_request(int fd, dictionary_t *query);

static dictionary_t *db;
static sem_t db_lock;

int main(int argc, char **argv) 
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);

  db = make_dictionary(COMPARE_CASE_SENS, (free_proc_t) free_dictionary);
  Sem_init(&db_lock,0,1);

  /* Don't kill the server if there's an error, because
     we want to survive errors due to a client. But we
     do want to report errors. */
  exit_on_error(0);

  /* Also, don't stop on broken connections: */
  Signal(SIGPIPE, SIG_IGN);

  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    if (connfd >= 0) {
      Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                  port, MAXLINE, 0);
      printf("Accepted connection from (%s, %s)\n", hostname, port);
      int *connfdp;
      pthread_t th;
      connfdp = malloc(sizeof(int));
      *connfdp = connfd;
      Pthread_create(&th,NULL, go_doit, connfdp);
      Pthread_detach(th);
    }
  }
}

/*
 * doit - handle one HTTP request/response transaction
 */
void doit(int fd) 
{
  char buf[MAXLINE], *method, *uri, *version;
  rio_t rio;
  dictionary_t *headers, *query;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;
  printf("%s", buf);
  
  if (!parse_request_line(buf, &method, &uri, &version)) {
    clienterror(fd, method, "400", "Bad Request",
                "Friendlist did not recognize the request");
  } else {
    if (strcasecmp(version, "HTTP/1.0")
        && strcasecmp(version, "HTTP/1.1")) {
      clienterror(fd, version, "501", "Not Implemented",
                  "Friendlist does not implement that version");
    } else if (strcasecmp(method, "GET")
               && strcasecmp(method, "POST")) {
      clienterror(fd, method, "501", "Not Implemented",
                  "Friendlist does not implement that method");
    } else {
      if (strcmp(&uri[strlen(uri)-1],"/") == 0) {
        uri[strlen(uri)-1]=0;
      }
      headers = read_requesthdrs(&rio);

      /* Parse all query arguments into a dictionary */
      query = make_dictionary(COMPARE_CASE_SENS, free);
      parse_uriquery(uri, query);
      if (!strcasecmp(method, "POST"))
        read_postquery(&rio, headers, query);

      /* For debugging, print the dictionary */
      print_stringdictionary(query);

      /* You'll want to handle different queries here,
         but the intial implementation always returns
         nothing: */
      if (starts_with("/friends?",uri)) {
        friend_request(fd, query);
      } else if (starts_with("/befriend?",uri)) {
        befriend_request(fd, query);
      } else if (starts_with("/unfriend?",uri)) {
        unfriend_request(fd,query);
      } else if (starts_with("/introduce?",uri)) {
        introduce_request(fd,query);
      } else {
        clienterror(fd, "", "501", "Not Implemented",
                    "Friendlist does not implement that request");
      }

      /* Clean up */
      free_dictionary(query);
      free_dictionary(headers);
    }

    /* Clean up status line */
    free(method);
    free(uri);
    free(version);
  }
}

/*
 *  go_doit : handle concurrency
 */
void  *go_doit(void *connfdp) {
  int connfd = *(int *)connfdp;
  free(connfdp);
  doit(connfd);
  Close(connfd);
  return NULL;
}
/*
 * read_requesthdrs - read HTTP request headers
 */
dictionary_t *read_requesthdrs(rio_t *rp) 
{
  char buf[MAXLINE];
  dictionary_t *d = make_dictionary(COMPARE_CASE_INSENS, free);

  Rio_readlineb(rp, buf, MAXLINE);
  printf("%s", buf);
  while(strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    parse_header_line(buf, d);
  }
  
  return d;
}

void read_postquery(rio_t *rp, dictionary_t *headers, dictionary_t *dest)
{
  char *len_str, *type, *buffer;
  int len;
  
  len_str = dictionary_get(headers, "Content-Length");
  len = (len_str ? atoi(len_str) : 0);

  type = dictionary_get(headers, "Content-Type");
  
  buffer = malloc(len+1);
  Rio_readnb(rp, buffer, len);
  buffer[len] = 0;

  if (!strcasecmp(type, "application/x-www-form-urlencoded")) {
    parse_query(buffer, dest);
  }

  free(buffer);
}

static char *ok_header(size_t len, const char *content_type) {
  char *len_str, *header;
  
  header = append_strings("HTTP/1.0 200 OK\r\n",
                          "Server: Friendlist Web Server\r\n",
                          "Connection: close\r\n",
                          "Content-length: ", len_str = to_string(len), "\r\n",
                          "Content-type: ", content_type, "\r\n\r\n",
                          NULL);
  free(len_str);

  return header;
}

/*
 * friend_request requests
 * Returns the friends of ‹user›, each on a separate 
 * newline-terminated line as plain text (i.e., text/plain; charset=utf-8). 
 * The result is empty if no friends have been registered for the user.
 */
static void friend_request(int fd, dictionary_t *query)
{
  size_t len;
  char *body, *header, *user;

  // find user's name
  user = (char *)dictionary_get(query, "user");
  if (!user) {
    clienterror(fd, "GET", "400", "Bad Request",
                "You did not provide user");
    return;
  }

  P(&db_lock);
  // find friendlist of this user
  dictionary_t* friendlist; 
  friendlist = (dictionary_t *) dictionary_get(db,user);

  if (!friendlist) {
    body = strdup("");
  } else {
    body = join_strings((const char * const *)dictionary_keys(friendlist),'\n');
  }

  len = strlen(body);

  /* Send response headers to client */
  header = ok_header(len, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  printf("Response headers:\n");
  printf("%s", header);

  free(header);

  /* Send response body to client */
  Rio_writen(fd, body, len);

  free(body);
  V(&db_lock);
}
/*
 * /befriend?user=‹user›&friends=‹friends›
 */
static void befriend_request(int fd, dictionary_t *query)
{
  char  *user, *friends;
  int i;

  user = (char *)dictionary_get(query, "user");
  if (!user) {
    clienterror(fd, "GET", "400", "Bad Request",
                "You did not provide user");
    return;
  }


  P(&db_lock);
  // add friends to this user
  dictionary_t* user_list; 
  user_list = (dictionary_t *) dictionary_get(db,user);
  if (!user_list) {
    user_list = make_dictionary(COMPARE_CASE_INSENS,free);
    dictionary_set(db,user,user_list);
  }

  friends = (char *)dictionary_get(query, "friends");
  char** str = split_string(friends, '\n');
  
  for (i = 0; str[i] != NULL; i++) {
    dictionary_set(user_list,str[i],NULL);
    // add user to friend's list
    dictionary_t* friend_list = (dictionary_t *) dictionary_get(db,str[i]);
    if (!friend_list) {
      friend_list = make_dictionary(COMPARE_CASE_INSENS,free);      
      dictionary_set(db,str[i],friend_list);
    }
    dictionary_set(friend_list,user,NULL);
  }
  dictionary_remove(user_list,user);
  V(&db_lock);
  friend_request(fd,query);
}
/*
 * /unfriend?user=‹user›&friends=‹friends›
 */
static void unfriend_request(int fd, dictionary_t *query)
{
  char  *user, *friends;
  int i;

  user = (char *)dictionary_get(query, "user");
  if (!user) {
    clienterror(fd, "GET", "400", "Bad Request",
                "You did not provide user");
    return;
  }

  P(&db_lock);
  // find friendlist of this user
  dictionary_t* user_list; 
  user_list = (dictionary_t *) dictionary_get(db,user);
  if (!user_list) {
    friend_request(fd,query);
    return;
  }

  friends = (char *)dictionary_get(query, "friends");
  char** str = split_string(friends, '\n');
  
  for (i = 0; str[i] != NULL; i++) {
    // remove this friend from the list
    dictionary_remove(user_list,str[i]);
    // rmove user from friend's list
    dictionary_t* friend_list = (dictionary_t *) dictionary_get(db,str[i]);
    if (friend_list) {
      dictionary_remove(friend_list,user);
    }
  }

  V(&db_lock);
  friend_request(fd,query);
}
/*
 * /introduce?user=‹user›&friend=‹friend›&host=‹host›&port=‹port›
 * Contacts a friend-list server running on ‹host› at ‹port› 
 * to get all of the friends of ‹friend›, and adds ‹friend› 
 * plus all of ‹friend›’s friends as friends of ‹user› 
 * and vice versa.
 */
static void introduce_request(int fd, dictionary_t *query) {
  char *hostname = (char *)dictionary_get(query, "host");
  char *port = (char *)dictionary_get(query, "port");
  int clientfd;
  clientfd = Open_clientfd(hostname, port);
  exit_on_error(0);
  Signal(SIGPIPE, SIG_IGN);

  char *user = (char *)dictionary_get(query, "user");
  if (!user) {
    clienterror(fd, "GET", "400", "Bad Request",
                "You did not provide user");
    return;
  }
  char* friend = (char *)dictionary_get(query, "friend");
  if (!friend) {
    clienterror(fd, "GET", "400", "Bad Request",
                "You did not provide user");
    return;
  }

  // send GET request to server at hostname and port
  char buf[MAXLINE];
  rio_t rio;

  char *request = append_strings("GET /friends?user=", friend, 
                                " HTTP/1.1","\r\n\r\n",NULL);
  Rio_writen(clientfd, request, strlen(request));

  Rio_readinitb(&rio, clientfd);
  char * newFriend = "";
  size_t amt;
  int flag = 0;
  while((amt = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
    if (starts_with("\r\n", buf)) {
      flag = 1;
      continue;
    }

    if (flag) {
      newFriend = append_strings(newFriend, buf, NULL);
    } 
  }
  newFriend = append_strings(newFriend,friend, NULL);

  dictionary_set(query,"friends",newFriend);
  befriend_request(fd,query);

}
/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, 
     char *shortmsg, char *longmsg) 
{
  size_t len;
  char *header, *body, *len_str;

  body = append_strings("<html><title>Friendlist Error</title>",
                        "<body bgcolor=""ffffff"">\r\n",
                        errnum, " ", shortmsg,
                        "<p>", longmsg, ": ", cause,
                        "<hr><em>Friendlist Server</em>\r\n",
                        NULL);
  len = strlen(body);

  /* Print the HTTP response */
  header = append_strings("HTTP/1.0 ", errnum, " ", shortmsg, "\r\n",
                          "Content-type: text/html; charset=utf-8\r\n",
                          "Content-length: ", len_str = to_string(len), "\r\n\r\n",
                          NULL);
  free(len_str);
  
  Rio_writen(fd, header, strlen(header));
  Rio_writen(fd, body, len);

  free(header);
  free(body);
}

static void print_stringdictionary(dictionary_t *d)
{
  int i, count;

  count = dictionary_count(d);
  for (i = 0; i < count; i++) {
    printf("%s=%s\n",
           dictionary_key(d, i),
           (const char *)dictionary_value(d, i));
  }
  printf("\n");
}
