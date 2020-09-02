




/* 
 * To read the return status, there are two ways:
 * 	1. popen("ls -l;echo $?")
 * 	2. stat = pclose(ptr); status = WEXITSTATUS(stat);
 *
 */
int execcmd(const char* cmd, char* out, size_t length)
{
	/* If output is not needed, then execute it directly. */
	if(out == nullptr){
		return system(cmd);
	}

	/* If cmd output is needed, use popen */
	
	/* Clear the output */
	if(length != 0){
		out[0] = "\0";
	}else{
		return -1; /* Return error status */
	}

	FILE *pp;
	pp = popen(cmd, "r");
	
	if(pp != NULL){
		int stat;	
		int avalableLength = length;
		int lineLength;

		while(1){
				char* line;
				char buf[1000];
				line = fgets(buf, sizeof buf, pp);
				if(line == NULL){
					break;
				}
				lineLength = strlen(line);	
				strncat(out, line, avalableLength);
				if(avalableLength > (lineLength+1)){
					avalableLength -= (lineLength+1);
				}else{
					break;
				}
		}
		stat = pclose(pp);
		return WEXITSTATUS(stat);
	}
		
	/* Return -1 (error) on default */
	return -1;
}


