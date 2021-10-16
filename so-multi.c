#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include "map.h"

#define LEN_LINE 256

typedef struct directoare {
	char **dirs;
	int len;
	int pos;
} Tdir;

int resolve(HashMap *map, Tdir *d, FILE *in, FILE *out, char *w_dir, int flag);

int addDir(Tdir *dir, char *name)
{ /* adauga un director la lista de directoare */
	if (dir->dirs == NULL) {
		dir->dirs = (char **)malloc(2 * sizeof(char *));
		if (!dir->dirs)
			return 0;
		dir->len = 2;
	}
	if (dir->pos + 1 == dir->len) {
		dir->len *= 2;
		dir->dirs = realloc(dir->dirs, dir->len * sizeof(char *));
		if (!dir->dirs)
			return 0;
	}
	dir->dirs[dir->pos] = name;
	dir->pos++;
	return 1;
}

void removeDir(Tdir *dir)
{ /* elimereaza memoria directoarelor */
	if (dir->dirs)
		free(dir->dirs);
}

char *replaced_line(HashMap *map, char *line, int len)
{ /* inlocuieste variabilele din hashmap pentru un string */
	char *value = NULL, c, *res, word[LEN_LINE];
	int k, i, j = 0, pos = 0;

	res = (char *)calloc(LEN_LINE, sizeof(char));
	if (!res)
		return NULL;
	memset(word, 0, LEN_LINE);
	if (!res)
		return NULL;

	for (i = 0; i < len; i++) {
		c = line[i];
		if (c == '\t' || c == ' ' || c == '[' || c == ']' || c == '{'
		|| c == '}' || c == '<' || c == '>' || c == '=' || c == '+' ||
		c == '-' || c == '*' || c == '/' || c == '%' || c == '!' ||
		c == '&' || c == '|' || c == '^' || c == '.' || c == ',' ||
		c == ':' || c == ';' || c == '(' || c == ')' || c == '\\') {
			if (pos > 0)
				value = find(map, word);
			else
				value = NULL;

			if (value) {
				for (k = 0; k < (int)strlen(value); k++) {
					res[j] = value[k];
					j++;
				}
			} else {
				for (k = 0; k < pos; k++) {
					res[j] = word[k];
					j++;
				}
			}
			pos = 0;
			res[j] = c;
			j++;
			memset(word, 0, LEN_LINE);
		} else {
			word[pos] = c;
			pos++;
		}
	}
	if (pos > 0) {
		value = find(map, word);
		if (value) {
			for (k = 0; k < (int)strlen(value); k++) {
				res[j] = value[k];
				j++;
			}	
		} else {
			for (k = 0; k < pos; k++) {
				res[j] = word[k];
				j++;
			}
		}
	}
	return res;
}

int f_include(HashMap *map, char *line, Tdir *dir, char *work_dir,
				FILE *out, int flag)
{ /* funtia de #include */
	char *file = NULL, tmp[LEN_LINE];
	FILE *f = NULL;
	int i;

	if (line[9] == '"') {
		file = line;
		while (file[strlen(file) - 1] != '"')
			file[strlen(file) - 1] = '\0';
		file[strlen(file) - 1] = '\0';
		file += 10;
		if (work_dir) {
			sprintf(tmp, "%s%s", work_dir, file);
			file = tmp;
		}
		f = fopen(file, "r");
		if (!f) {
			i = 0;
			while (dir->dirs && i < dir->pos) {
				memset(tmp, 0, 255);
				file = line;
				file += 10;
				sprintf(tmp, "%s/%s", dir->dirs[i], file);
				file = tmp;
				f = fopen(file, "r");
				if (f)
					break;
				i++;
			}
			if (!f)
				exit(-1);
		}
		if (!resolve(map, dir, f, out, work_dir, flag))
			return 0;
		fclose(f);
	}
	return 1;
}

int f_define(HashMap *map, char *line, FILE *in)
{ /* funtia de #define */
	char *key, *value, *file = NULL, tmp[LEN_LINE];
	int more_line = 0, len = LEN_LINE;
	char *def = (char *)malloc(LEN_LINE * sizeof(char));

	if (!def)
		return 0;
	memset(def, 0, LEN_LINE);
	file = line + 8;
	key = strtok(file, " ");
	strcpy(tmp, key);
	value = strtok(NULL, "\n");
	while (value && strlen(value) && value[strlen(value) - 1] == '\\') {
		if (strlen(def))
			def[strlen(def) - 1] = '\0';
		def = strcat(def, value);
		fgets(line, len, in);
		value = strtok(line, "\n");
		more_line = 1;
	}
	if (more_line) {
		if (strlen(def))
			def[strlen(def) - 1] = '\0';
		def = strcat(def, value);
		value = def;
		key = tmp;
	}
	if (value) {
		value = replaced_line(map, value, strlen(value));
		if (!value)
			return 0;
	}
	if (!value) {
		if (!add(map, key, ""))
			return 0;
	} else {
		if (!add(map, key, value))
			return 0;
	}

	free(value);
	free(def);
	return 1;
}

int resolve(HashMap *map, Tdir *dir, FILE *in, FILE *out,
			char *work_dir, int flag)
{ /* prelucrarea fisierului linie cu linie */
	char *key, *value, *line, *file = NULL;
	int f_tr = 0, f_if = 0, d_tr = 0, d_if = 0, len = LEN_LINE;

	line = (char *)calloc(LEN_LINE, sizeof(char));
	if (!line)
		return 0;
	while (fgets(line, len, in)) {
		if (strncmp(line, "#include", 8) == 0) {
			if ((f_tr && !f_if) || (d_tr && !d_if))
				continue;
			if (!f_include(map, line, dir, work_dir, out, flag))
				return 0;
		} else if (strncmp(line, "#define", 7) == 0) {
			if ((f_tr && !f_if) || (d_tr && !d_if))
				continue;
			if (!f_define(map, line, in))
				return 0;
		} else if (strncmp(line, "#undef", 6) == 0) {
			if ((f_tr && !f_if) || (d_tr && !d_if))
				continue;
			file = line + 7;
			key = strtok(file, "\n");
			removeKey(map, key);
		} else if (strncmp(line, "#elif", 5) == 0) {
			if (f_if) {
				while (fgets(line, len, in) &&
					strncmp(line, "#endif", 6) != 0)
					;
				f_if = f_tr = 0;
			} else {
				key = strtok(line + 6, "\n");
				value = find(map, key);
				if (!value)
					value = key;
				if (atoi(value) != 0)
					f_if = 1;
			}
		} else if (strncmp(line, "#else", 5) == 0) {
			if (f_if) {
				while (fgets(line, len, in)
					&& strncmp(line, "#endif", 6) != 0)
					;
				f_if = f_tr = 0;
			} else {
				f_if = f_tr = 1;
			}
			if (d_if) {
				while (fgets(line, len, in)
					&& strncmp(line, "#endif", 6) != 0)
					;
				d_if = d_tr = 0;
			} else {
				d_if = d_tr = 1;
			}
		} else if (strncmp(line, "#endif", 6) == 0) {
			f_tr = f_if = d_if = d_tr = 0;
			if (flag)
				return 1;
		} else if (strncmp(line, "#ifdef", 6) == 0) {
			d_tr = 1;
			key = strtok(line + 7, "\n");
			if (find(map, key))
				d_if = 1;
		} else if (strncmp(line, "#ifndef", 7) == 0) {
			d_tr = 1;
			key = strtok(line + 8, "\n");
			if (!find(map, key))
				d_if = 1;
			else
				d_if = 0;
		} else if (strncmp(line, "#if", 3) == 0) {
			f_tr = 1;
			key = strtok(line + 4, "\n");
			value = find(map, key);
			if (!value)
				value = key;
			if (atoi(value) != 0)
				f_if = 1;
		} else {
			if ((f_tr && !f_if) || (d_tr && !d_if))
				continue;
			value = replaced_line(map, line, strlen(line));
			if (!value)
				return 0;
			fprintf(out, "%s", value);
			free(value);
		}
	}
	free(line);
	return 1;
}


void resolve_argc(char **args, int argc, HashMap *map, Tdir *dir,
				char **work_dir, FILE **in, FILE **out)
{ /* prelucrarea argumentelor din linia de comanda */
	int i, j;
	char *key, *value;

	for (i = 1 ; i < argc; i++) {
		if (strncmp(args[i], "-D", 2) == 0) {
			if (strlen(args[i]) <= 2)
				i++;
			key = strtok(args[i], "=");
			if (key[0] == '-')
				key += 2;
			value = strtok(NULL, "");
			if (value == NULL)
				value = "";
			if (!add(map, key, value))
				exit(12);
		} else if (strncmp(args[i], "-I", 2) == 0) {
			if (strlen(args[i]) < 3)
				i++;
			else
				args[i] += 2;

			if (!addDir(dir, args[i]))
				exit(12);
		} else if (strncmp(args[i], "-o", 2) == 0) {
			if (strlen(args[i]) <= 2)
				i++;
			else
				args[i] += 2;
			*out = fopen(args[i], "w");
			if (!(*out))
				exit(-1);
		} else {
			if (*in != stdin && *out != stdout)
				exit(-1);
			if (*in == stdin) {
				*in = fopen(args[i], "r");
				if (!(*in))
					exit(-1);
				*work_dir = args[i];
				j = strlen(*work_dir) - 1;
				while (j >= 0 && (*work_dir)[j] != '/')
					j--;
				if (j != 0)
					(*work_dir)[j + 1] = '\0';
			} else {
				*out = fopen(args[i], "w");
				if (!(*out))
					exit(-1);
			}
		}
	}
}

int main(int argc, char **args)
{ /* functia principala a programului */
	HashMap *map;
	char *work_dir = NULL;
	FILE *in = stdin, *out = stdout;
	Tdir dir;

	map = init(strcmp, hashFunction);
	if (!map)
		exit(12);
	dir.dirs = NULL;
	dir.len = 0;
	dir.pos = 0;

	resolve_argc(args, argc, map, &dir, &work_dir, &in, &out);
	if (!resolve(map, &dir, in, out, work_dir, 0))
		exit(12);

	if (in != stdin)
		fclose(in);
	if (out != stdout)
		fclose(out);

	removeDir(&dir);
	removeH(map);
	return 0;
}
