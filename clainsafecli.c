#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

size_t static write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	memcpy(userp, buffer, nmemb*size);
	return 0;
}

void
print_usage()
{
	printf("USAGE: clainsafecli [--server] file\n");
	return;
}

void
print_help()
{
	printf("--server <server>: specifies the lainsafe server\n%s\n%s",
		"--tor: uses tor",
		"--help: print this message\n");
	return;
}

int
main(int argc, char **argv)
{
	struct curl_httppost *post = NULL;
	struct curl_httppost *last = NULL;

	int tor_flag = 0;
	
	char *buffer = (char *)calloc(1024,sizeof(char));
	char server[256] = "https://lainsafe.kalli.st";
	
	CURL *easy_handle = curl_easy_init();
	if(!easy_handle) {
		fprintf(stderr,"Error initializing libcurl\n");
		return -1;
	}
	if(argc == optind) {
	     print_usage();
		return -1;
	}
	int option_index = 0;

	static struct option long_options[] = {
		{"server",required_argument,0,'s'},
		{"help"  ,no_argument      ,0,'h'},
		{"tor"   ,no_argument      ,0,'t'},
		{0       ,0                ,0, 0 }
	};

	int c = 0;
	while((c = getopt_long(argc,argv, "hts:",long_options,&option_index)) != -1) {
		switch(c) {
		case 's':
			strncpy(server,optarg,256);
			break;
		case 'h':
			print_help();
			return 0;
			break;
		case 't': tor_flag = 1; break;
		case '?':
			print_usage();
			return 0;
			break;
		default:
			print_usage();
			return 0;
			break;
		}

	}

	/* curl options */ 
	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(easy_handle,CURLOPT_WRITEDATA,buffer);
	curl_easy_setopt(easy_handle,CURLOPT_URL,server);

	if(tor_flag) {
		curl_easy_setopt(easy_handle,CURLOPT_PROXY,"127.0.0.1:9050");
		curl_easy_setopt(easy_handle,CURLOPT_PROXYTYPE,
			CURLPROXY_SOCKS5_HOSTNAME);
	}
	
	/* Form parameters */

	/* File name */
	curl_formadd(&post,&last,
		CURLFORM_COPYNAME, "file",
		CURLFORM_FILE,argv[optind],
		CURLFORM_END);
	/* Actual file content */
	curl_formadd(&post,&last,
		CURLFORM_COPYNAME, "file",
		CURLFORM_COPYCONTENTS,argv[optind],
		CURLFORM_END);

	curl_easy_setopt(easy_handle,CURLOPT_HTTPPOST,post);
	
	curl_easy_perform(easy_handle);

	puts(buffer);
	free(buffer);
	curl_formfree(post);
	curl_easy_cleanup(easy_handle);
	
	return 0;
}
