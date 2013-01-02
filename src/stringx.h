/*
	http-client-c 
	Copyright (C) 2012-2013  Swen Kooij
	
	This file is part of http-client-c.

    http-client-c is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    http-client-c is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with http-client-c. If not, see <http://www.gnu.org/licenses/>.

	Warning:
	This library does not tend to work that stable nor does it fully implent the
	standards described by IETF. For more information on the precise implentation of the
	Hyper Text Transfer Protocol:
	
	http://www.ietf.org/rfc/rfc2616.txt
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
	Gets the offset of one string in another string
*/
int str_index_of(const char *a, char *b)
{
	char *offset = strstr(a, b);
	return offset - a;
}

/*
	Checks if one string contains another string
*/
int str_contains(const char *haystack, const char *needle)
{
	char *pos = strstr(haystack, needle);
	if(pos)
		return 1;
	else
		return 0;
}

/*
	Removes last character from string
*/
char* trim_end(char *string, char to_trim)
{
	char last_char = string[strlen(string) -1];
	if(last_char == to_trim)
	{
		char *new_string = string;
		new_string[strlen(string) - 1] = 0;
		return new_string;
	}
	else
	{
		return string;
	}
}

/*
	Concecates two strings
*/
char* str_cat(char *a, char *b)
{
	char *target = malloc(strlen(a) + strlen(b) + 1);
	strcpy(target, a);
	strcat(target, b);
	return target;
}

/* 
	Converts an integer value to its hex character
*/
char to_hex(char code) 
{
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

/*
	URL encodes a string
*/
char *urlencode(char *str) 
{
	char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  	while (*pstr) 
	{
    	if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      		*pbuf++ = *pstr;
    	else if (*pstr == ' ') 
      		*pbuf++ = '+';
    	else 
      		*pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    	pstr++;
  	}
  	*pbuf = '\0';
  	return buf;
}

/*
	Replacement for the string.h strndup, fixes a bug
*/
char *str_ndup (const char *str, size_t max)
{
    size_t len = strnlen (str, max);
    char *res = malloc (len + 1);
    if (res)
    {
        memcpy (res, str, len);
        res[len] = '\0';
    }
    return res;
}

/*
	Replacement for the string.h strdup, fixes a bug
*/
char *str_dup(const char *src)
{
   char *tmp = malloc(strlen(src) + 1);
   if(tmp)
       strcpy(tmp, src);
   return tmp;
}

/*
 	Search and replace a string with another string , in a string
*/
char *str_replace(char *search , char *replace , char *subject)
{
	char  *p = NULL , *old = NULL , *new_subject = NULL ;
	int c = 0 , search_size;
	
	search_size = strlen(search);
	for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search))
	{
		c++;
	}
	
	c = ( strlen(replace) - search_size )*c + strlen(subject);
	new_subject = malloc( c );
	strcpy(new_subject , "");
	
	//The start position
	old = subject;
	
	for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search))
	{
		//move ahead and copy some text from original subject , from a certain position
		strncpy(new_subject + strlen(new_subject) , old , p - old);
		
		//move ahead and copy the replacement text
		strcpy(new_subject + strlen(new_subject) , replace);
		
		//The new start position after this search match
		old = p + search_size;
	}
	
	//Copy the part after the last search match
	strcpy(new_subject + strlen(new_subject) , old);
	
	return new_subject;
}

/*
	Does strstr() but reversed
*/
char* get_until(char *haystack, char *until)
{
	int offset = str_index_of(haystack, until);
	return str_ndup(haystack, offset);
}