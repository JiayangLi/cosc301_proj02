bool valid_command(const char **curr){

	struct stat statresult;
	int rv = stat(curr[0], &statresult);
	if (rv < 0){

		return false;
	}

	return true;

}//end valid_command

void update_path(char **curr){
	
	FILE *fp = fopen("shell-config", "r");
	if(NULL == fp){
		fprintf(stderr, "Failed to open shell-config.\n");
		return;
	}//end if
	
	char buffer[1024];
	while(NULL != fgets(buffer, sizeof(buffer), fp)){

		for(int i = 0; i < strlen(buffer); i++){//eliminate the trailing new line character
			if(buffer[i] == '\n'){
				buffer[i] = '\0';
			}//end if
		}//end for

		int pos = strlen(buffer);//find the end of the string

		if(buffer[pos - 1] != '/'){
			buffer[pos++] = '/';
			buffer[pos] = '\0';
		}//end if
		
		for(int i = 0; i < strlen(curr[0]); i++){
			buffer[pos] = curr[0][i];
			pos++;
		}//end for

		buffer[pos] = '\0';

		struct stat statresult;
		int rv = stat(buffer, &statresult);
		if(rv >= 0){//if the path is valid
			free(curr[0]);
			curr[0] = strdup(buffer);
		}//end if

	}//end while

	fclose(fp);

}//end update_path

void add_path(char ***cmd){

	for(int i = 0; cmd[i] != NULL; i++){

		if(!valid_command(cmd[i])){
			update_path(cmd[i]);
		
		}//end if
	}//end for

}//end add_path