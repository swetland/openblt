// RPC Interface Compiler (ic)
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>

typedef struct ic_type ic_type;
typedef struct ic_param ic_param;
typedef struct ic_func ic_func;
typedef struct ic_ifc ic_ifc;

struct ic_type
{
	ic_type *next;
	
	char *name;
	enum {
		BASIC,
		ARRAY,
		STRING,
		STRUCT
	} type;
	int size;
	ic_type *parts;
};

struct ic_param
{
	ic_param *next;
	
	char *name;
	ic_type *type;
	enum {
		IN = 1,
		OUT = 2
	} flags;
};

struct ic_func
{
	ic_func *next;
	
	char *name;
	ic_param *params;	
};

struct ic_ifc
{
	char *name;
	char *key;
	ic_func *funcs;
};

ic_type *types = NULL;

ic_type builtins[] = {
	{ NULL, "int32",   BASIC, 4, NULL },
	{ NULL, "uint32",  BASIC, 4, NULL },
	{ NULL, "int16",   BASIC, 2, NULL },
	{ NULL, "uint16",  BASIC, 2, NULL },
	{ NULL, NULL,      BASIC, 0, NULL }
};

int line = 1;
char token[256];
char *data;
char *file;

void die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	fprintf(stderr,"%s:%d - ",file,line);
	vfprintf(stderr,fmt,ap);
	fprintf(stderr,"\n");
	va_end(ap);
	exit(1);
}


void next(void)
{
	char *x = token;
	
	if(*data == 0) {
		token[0] = 0;
		return;
	}
	
eat_space:
	while(*data <= ' ') {
		if(*data == '\n') line++;
		data++;
	}
	
	if(*data == '#') {
		while(*data != '\n') data++;
		goto eat_space;
	}
	
	if(*data == '"') {
		data++;
		while(*data != '"') {
			*x++ = *data++;
		}
		*x = 0;
		data++;
		return;
	}
	
	for(;;){
		switch(*data){
		case '{':
		case '}':
		case '(':
		case ')':
		case ',':
		case '#':
		case '"':
		case ':':
			if(x == token){
				*x++ = *data++;
			}
			*x = 0;
			return;
		default:
			if(*data <= ' '){
				*x = 0;
				return;
			} else {
				*x++ = *data++;
				if((x - token) > 255) die("token too long");
			}
		}
	}	
}

int match(const char *tok)
{
	if(!strcmp(token,tok)){
		next();
		return 1;
	} else {
		return 0;
	}
}

void force(const char *tok)
{
	if(strcmp(token,tok)){
		die("expecting '%s', found '%s'",tok,token);
	}
	next();
}

ic_type *type(void)
{
	ic_type *t;
	
	if(match("string")){
		t = (ic_type *) malloc(sizeof(ic_type));
		if(!t) die("out of memory");
		
		force(":");
		t->size = atoi(token); next();
		t->name = "string";
		t->type = STRING;
		t->next = NULL;
		t->parts = NULL;
		
		return t;
	}
		
	for(t = types; t; t = t->next){
		if(match(t->name)) return t;
	}
	
	die("unknown type '%s'",token);
	return NULL;
}

ic_param *param(void)
{
	ic_param *p = (ic_param*) malloc(sizeof(ic_param));
	if(!p) die("out of memory");
	
	if(match("in")){
		p->flags = IN;
	} else if(match("out")) {
		p->flags = OUT;
	} else {
		die("expecting 'in' or 'out', found '%s'",token);
	}
	
	p->type = type();
	p->name = strdup(token); next();
	p->next = NULL;
	
	return p;
}

ic_func *function(void)
{
	ic_param *p,*last = NULL;
	ic_func *func = (ic_func*) malloc(sizeof(ic_func));
	if(!func) die("out of memory");

	func->name = strdup(token); next();
	func->next = NULL;
	func->params = NULL;
	
	force("(");
	
	for(;;){
		if(match(")")) {	
			match(";"); /* optional semi */
			return func;
		}
		p = param();
		if(last) {
			last->next = p;
		} else {
			func->params = p;
		}
		last = p;
		match(",");
	}
}



ic_ifc *interface(void)
{
	ic_func *func,*last = NULL;
	ic_ifc *ifc = (ic_ifc*) malloc(sizeof(ic_ifc));
	
	if(!ifc) die("out of memory");

	ifc->name = strdup(token); next();
	ifc->key = strdup(token); next();
	ifc->funcs = NULL;
	
	force("{");
	
	for(;;){
		if(match("}")) {
			match(";"); /* optional semi */
			return ifc;
		}
		func = function();
		if(last){
			last->next = func;
		} else {
			ifc->funcs = func;
		}
		last = func;
	}
}

void mkheader(FILE *f, ic_ifc *ifc)
{
	ic_func *func;
	ic_param *p;
	char *upname,*x;
	
	x = upname = strdup(ifc->name);
	while(*x) {
		*x = toupper(*x);
		x++;
	}
	fprintf(f,"/* Generated File - DO NOT EDIT */\n\n");
	fprintf(f,"#define IFC_%s \"%s\"\n\n",upname,ifc->key);
	fprintf(f,"typedef struct {\n");
	for(func = ifc->funcs; func; func = func->next){
		fprintf(f,"\tstatus_t (*%s)(\n",func->name);
		fprintf(f,"\t\tconst ifc_handle *handle%s\n",func->params?",":"");
		
		for(p = func->params; p; p = p->next){
			fprintf(f,"\t\t%s %s%s\n",p->type->name,p->name,
					p->next ? "," : "");
		}
		fprintf(f,"\t\t);\n");
	}
	fprintf(f,"} ifc_%s;\n",ifc->name);
}

void mkstub(FILE *f, ic_ifc *ifc)
{
	ic_func *func;
	ic_param *p;
	
	fprintf(f,"/* Generated File - DO NOT EDIT */\n\n");
	
	/* eject the stubs */
	for(func = ifc->funcs; func; func = func->next){
		fprintf(f,"static status_t\n");
		fprintf(f,"stub_%s(\n",func->name);
		fprintf(f,"\t)\n");
		fprintf(f,"{\n");
		fprintf(f,"\treturn -1;\n"); 
		fprintf(f,"}\n\n");
	}
		
	/* eject the decl */
	fprintf(f,"ifc_%s ifc_%s_impl = {\n",ifc->name,ifc->name);	
	for(func = ifc->funcs; func; func = func->next){
		fprintf(f,"\t&stub_%s%s\n",func->name,func->next?",":"");
	}
	fprintf(f,"};\n");	
}

void parse(void)
{
	ic_ifc *ifc;
	next();
	
	for(;;){
		if(match("interface")) {
			ifc = interface();
			mkheader(stdout,ifc);
			mkstub(stdout,ifc);
		}
		if(match("")) return;
		die("unexpected token '%s'",token);
	}
}

int main(int argc, char *argv[])
{
	int i,fd;
	struct stat s;
	
	/* install built-in types */
	for(i=0;builtins[i].name;i++){
		builtins[i].next = types;
		types = builtins + i;
	}
	
	if(argc != 2) {
		fprintf(stderr,"error: no input file\n");
		return 1;
	}
	
	file = argv[1];
	
	if(((fd = open(file,O_RDONLY)) < 0) || fstat(fd,&s)){
		fprintf(stderr,"error: cannot open \"%s\"\n",file);
		return 1;
	}
	
	data = (char*) malloc(s.st_size + 1);
	read(fd, data, s.st_size);
	data[s.st_size] = 0;
	close(fd);
	
	parse();
}





		