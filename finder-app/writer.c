#include <stdio.h>
#include <syslog.h>
#include <stdbool.h>

/**
* @return bool when is possible to write the content to the file
* or not.
*/
bool write(char *path, char *text){
	syslog(LOG_DEBUG,"Writing %s to %s", text, path);

	FILE *fptr = fopen(path, "w");
	if (fptr == NULL){
        syslog(LOG_ERR,"Error opening file %s!", path);
        return false;
    }

	int result = fprintf(fptr, "%s", text);
	if (result < 0){
		syslog(LOG_ERR,"Error writing to file %s!", path);
		fclose(fptr);
        return false;
	}

	syslog(LOG_DEBUG,"Successfully wrote text %s to file %s.", text, path);
	fclose(fptr);
	return true;
}

int main(int argc, char**argv){
	openlog(NULL, 0, LOG_USER);
	if (argc < 3){
		syslog(LOG_ERR,"You need to pass the file and string. Example: ./writer \"/tmp/aesd/assignment2/sample.txt\" \"ios\"");
		closelog();
		return 1;
	}

	if(write(argv[1], argv[2])){
		closelog();
		return 0;
	}

	closelog();
	return 1;
}
