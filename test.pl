if(write(fd,msg,strlen(msg))<0) {
	perror("write");
}

void print_help() {
	print_msg(1,"\x1b[35;5m-Builtinco
