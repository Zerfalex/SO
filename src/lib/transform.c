#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dataStructs.h>
#include <transform.h>

/**
 * Replaces the chosen substring with another in the main string, if the substring doesn't exist the main string doesn't change
 * @param  target 			a pointer to the main string that will get something replaced
 * @param  needle 			a pointer to the substring that will get replaced
 * @param  replacement 		a pointer to the string that will replace the substring
 */
void str_replace(char *target, const char *needle, const char *replacement)
{
    char buffer[1024] = { 0 };
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);
    while (1)
    {
        const char *p = strstr(tmp, needle);
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;
        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;
        tmp = p + needle_len;
    }
    strcpy(target, buffer);
}

/**
 * Returns the number of times the char appears in the string
 * @param  string 			a pointer to the string that will get counted
 * @param  ch 				a char to be counted in the string
 * @return   				an int
 */
int count_chars(const char* string, char ch)
{
    int count = 0;
    int i;
    int length = strlen(string);
    for (i = 0; i < length; i++)
    {
        if (string[i] == ch)
        {
            count++;
        }
    }
    return count;
}

/**
 * Turns the chosen notebook file into the structure LIST
 * @param  argc				an int
 * @param  argv 			a pointer to the array of strings
 * @param  list 			a LIST
 */
void noteToList(int argc, char *argv[], LIST list)
{
    if(argc<2) {printf("Arguments missing! Use ./program <filepath>\n"); exit(-1);}
    FILE* fil = fopen(argv[1], "r");
    if(fil==NULL) {perror("Error opening file!\n"); exit(-1);}
    char line[1024];
    char command[1024];
    int quantity=0;
    char* comment=malloc(1024);
    int block=0;
    while(fgets(line, sizeof(line), fil) != NULL)
    {
        int ref=-1;
        if(sscanf(line, "$%d| %[^\t\n]", &quantity, command)==2)
        {
            ref = quantity;//referencia fica "$ " - 0, "$| " - 1, "$n| " - n
        }
        else if(sscanf(line, "$| %[^\t\n]", command)==1)
        {
            ref = 1;//referencia fica "$ " - 0, "$| " - 1, "$n| " - n
        }
        else if(sscanf(line, "$ %[^\t\n]", command)==1)
    	{
            ref = 0;//referencia fica "$ " - 0, "$| " - 1, "$n| " - n
    	}
        if(ref!=-1)
        {
            char* remove = " | ";
            char* replace = "|";
            str_replace(command, remove, replace);
            int length = strlen(command);
            int left = count_chars(command, '|');//quantos characteres '|' existem, usado como sline em thing se for 0 é o ultimo argumento que falta porque é sempre decrementado ao adicionar um
            char word[1024];//argumentos diferentes
            int wordI=0;
            for(int i=0;i<length;i++)
            {
                if(command[i]=='|')
                {
                    word[wordI]=0;
                    list_add(list, ref, word, NULL, left, comment);
                    strcpy(comment,"");
                    strcpy(word,"");
                    left--;
                    ref=1;
                    wordI=0;
                }
                else
                {
                    word[wordI]=command[i];
                    wordI++;
                }
            }
            word[wordI]=0;
            list_add(list, ref, word, NULL, left, comment);
            strcpy(comment,"");
            strcpy(word,"");
        }
        else
        {
            if(sscanf(line, ">>%[^\t\n]", command)==1 && strcmp(command,">")==0)
            {
                block=1;
            }
            if(block==0)
            {
                strcat(comment,line);
            }
            if(sscanf(line, "<<%[^\t\n]", command)==1 && strcmp(command,"<")==0)
            {
                block=0;
            }
        }
    }
    list_set_pre(list, comment);
    fclose(fil);
}

/**
 * Turns the chosen structure LIST into the notebook file
 * @param  argc				an int
 * @param  argv 			a pointer to the array of strings
 * @param  list 			a LIST
 */
void listToNote(int argc, char *argv[], LIST list)
{
	THING thing;
	int sline;
	int ref;
	char* output;
	char* params;
	int size = list_size(list);
	if(argc<2) {printf("Arguments missing! Use ./program <filepath>\n"); exit(-1);}
	FILE* fil = fopen(argv[1], "w");
	if(fil==NULL) {perror("Error opening file!\n"); exit(-1);}
	for(int i=0;i<size;i++)
	{
		thing = list_get_thing(list, i);
		sline = thing_get_sline(thing);
		ref = thing_get_ref(thing);
		params = thing_get_params(thing);
		output = thing_get_output(thing);
        fprintf(fil, "%s", thing_get_comment(thing));
		if(sline==0)
		{
			if(ref==0)
			{
				fprintf(fil, "$ %s\n", params);
			}
			else if(ref==1)
			{
				fprintf(fil, "$| %s\n", params);
			}
			else
			{
				fprintf(fil, "$%d| %s\n", ref, params);
			}
			fprintf(fil, ">>>\n");
			fprintf(fil, "%s\n", output);
			fprintf(fil, "<<<\n");
		}
		else
		{
            if(ref==0)
            {
                fprintf(fil, "$ %s", params);
            }
            else if(ref==1)
            {
                fprintf(fil, "$| %s", params);
            }
            else
            {
                fprintf(fil, "$%d| %s", ref, params);
            }
			while(sline>0)
			{
				i++;
				thing = list_get_thing(list, i);
				sline = thing_get_sline(thing);
				params = thing_get_params(thing);
				fprintf(fil, " | %s", params);
			}
			output = thing_get_output(thing);
			fprintf(fil, "\n>>>\n");
			fprintf(fil, "%s\n", output);
			fprintf(fil, "<<<\n");
		}
	}
    fprintf(fil, "%s", list_get_pre(list));
    fclose(fil);
}