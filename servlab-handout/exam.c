#include "csapp.h"
#include "dictionary.h"

static dictionary_t *db;
static sem_t db_lock;

static void sigchld_handler(int sig) {
	while (waiitpid(-1,NULL,WNOHANG) < 0) ;
}

static int make_socket(char *portno) {
	int s;
	struct addrinfo hints, *addrs;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	Getaddrinfo(NULL,portno,&hints,&addrs);
	
	s = Socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
	Bind(s, addrs->ai_addr, addrs->ai_addrlen);
	Freeaddrinfo(addrs);

	return s;	
}

static void write_entry(dictionary_t *db, int i, int s) {
	const char *key = dictionary_key(db,i);
	int len = strlen(key);
	int votes = *(int *)dictionary_value(db,i);
	Rio_writen(s, &len,sizeof(len));
	Rio_writen(s,(void *)key,len);
	Rio_writen(s,&votes,sizeof(votes));
}

static void receive_votes(char *portno) {
	int s = make_socket(portno);
	while(1) {
		int *p, *new_p;
		
		char buffer[MAXBUF];
		size_t amt;
		amt = Recv(s, buffer,MAXBUF -1, 0);
		buffer[amt] = 0;

		new_p = malloc(sizeof(int));
		*new_p = 1;
		P(&db_lock);
		p = dictionary_get(db,buffer);
		if (p) *new_p += *p;
		dictionary_set(db,buffer,new_p);
		V(&db_lock);
	}
}

static void *serve_counts(void *portno) {
	int ls, s;
	ls = Open_listenfd(portno);

	while(1) {
		struct sockaddr_storage addr;
		unsigned int len =  sizeof(addr);
		s = Accept(ls, (struct sockaddr *)&addr, &len);
		P(&db_lock);

		if (Fork() == 0) {
			int i, count = dictionary_count(db);
			Rio_writen(s, &count,sizeof(count));
			for (i = 0; i < count; i++) 
				write_entry(db,i,s);
			exit(0);
			Close(s);
		}

	}
}

int main(int argc, char **argv) {
	pthread_t th;
	if (argc != 2) app_error("need <port>");
	db = make_dictionary(COMPARE_CASE_SENS, free);
	Sem_init(&db_lock,0,1);
	Signal(SIGCHILD,sigchld_handler);
	Pthread_create(&th, NULL, serve_counts,argv[1]);
	Pthread_detach(th);
	receive_votes(argv[1]);
	return 0;
}