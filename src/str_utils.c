#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#ifdef _WIN32
	#include <windows.h>
	#include <process.h>
#else
	#include <unistd.h>
#endif
#include "constants.h"

int check_list_contains_item(const char* const* list, const char* item)
{
	int i = 0;
	while (list[i] != NULL) {
		if (strcmp(list[i], item) == 0) {
			return i;
		}
		i++;
	}
	return 0;
}

int check_gtk_list_contains_item(GtkStringList *list, const char* item)
{
	if (G_IS_LIST_MODEL(list)) {
		guint i = 0;
		guint n = g_list_model_get_n_items((GListModel *)list);
		while(i < n) {
			const char *s = gtk_string_list_get_string(list, i);
			if (strcmp(s, item) == 0) {
				return i;
			}
			i++;
		}
		return 0;
	}
	return 0;
}

int compare_strings(const void *a, const void *b)
{
	return strcasecmp(*(const char **)a, *(const char **)b);
}

int count_digits(double n)
{
	long long int i = (int)n;
	if (i < 0) {
		i = -i;
	}
	int l = 0;
	if (i == 0) {
		l = 1;
	} else {
		while (i > 0) {
			i /= 10;
			l++;
		}
	}
	return l;
}

char *convert_int_to_string(int x)
{
	char* str = malloc(12);

	if (str == NULL) {
	return NULL;
	}

	snprintf(str, 12, "%d", x);
	return str;
}

char *convert_double_to_string(double n, char *str_format)
{
	int str_size = 0;

	if (strcmp(str_format, "%.2f") == 0) {
		str_size = count_digits(n) + 4;
	}
	if (strcmp(str_format, "%.1f") == 0) {
		str_size = count_digits(n) + 3;
	}
	if (strcmp(str_format, "%.0f") == 0) {
		if (n < 0) {
			str_size = count_digits(n) + 2;
		} else {
			str_size = count_digits(n) + 1;
		}
	}

	char *str = malloc(str_size * sizeof(char));
	snprintf(str, str_size, str_format, n);
	return str;
}

char *convert_long_long_int_to_string(long long int x)
{
	printf("Long long int is: %lld\n", x);
	char *str = malloc(LONGLONG_STR_SIZE);
	if (!str) {
		return NULL;
	}

	int success = snprintf(str, LONGLONG_STR_SIZE, "%lld", x);
	if (success < 0 || success >= LONGLONG_STR_SIZE) {
		free(str);
		return NULL;
	}
	return str;
}

char *format_lora_embedding_string(const gchar* item, int tb_type)
{
	if (item == NULL) return NULL;

	char *n_item = strdup(item); 
	if (n_item == NULL) return NULL;

	char *dot = strrchr(n_item, '.'); 
	if (dot != NULL) {
		*dot = '\0'; 
	}

	int t_len[2];

	if (tb_type == 0) {
		t_len[0] = 3;
		t_len[1] = 4;
	} else {
		t_len[0] = 7;
		t_len[1] = 5;
	}

	char text_pf[t_len[0]];
	char text_sf[t_len[1]];

	if (tb_type == 0) {
		strcpy(text_pf, ", (embedding:");
		strcpy(text_sf, ")");
	} else {
		strcpy(text_pf, "<lora:");
		strcpy(text_sf, ":1> ");
	}


	int f_len = strlen(n_item) + strlen(text_pf) + strlen(text_sf) + 1;
	char *f_str = malloc(f_len);
	if (f_str == NULL) {
		free(n_item);
		return NULL;
	}
	f_str[0] = '\0';
	strcat(f_str, text_pf);
	strcat(f_str, n_item);
	strcat(f_str, text_sf);
	f_str[f_len -1] = '\0';
	free(n_item);

	return f_str;
}

char* generate_sd_seed()
{
	long long int seed;

	#ifdef _WIN32
		LARGE_INTEGER counter;
		if (QueryPerformanceCounter(&counter)) {
			seed = (long long int)counter.QuadPart;
		} else {
			seed = (long long int)time(NULL);
			seed ^= (long long int)GetCurrentThreadId() << 16;
		}
		seed ^= (long long int)GetCurrentProcessId() << 32;
	#else
		struct timespec ts;
		if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
			seed = (long long int)ts.tv_sec * 1000000000LL;
			seed += (long long int)ts.tv_nsec;
		} else {
			seed = (long long int)time(NULL);
		}
		seed ^= (long long int)getpid() << 32;
	#endif

	seed ^= seed >> 33;
	seed *= 0xFF51AFD7ED558CCD;
	seed ^= seed >> 33;
	seed *= 0xC4CEB9FE1A85EC53;
	seed ^= seed >> 33;

	seed &= 0x7FFFFFFFFFFFFFFF;

	char *seed_str = malloc(LONGLONG_STR_SIZE);
	if (!seed_str) {
		return NULL;
	}

	int chars_written = snprintf(seed_str, LONGLONG_STR_SIZE, "%lld", seed);

	if (chars_written < 0 || chars_written >= LONGLONG_STR_SIZE) {
		free(seed_str);
		return NULL;
	}

	return seed_str;
}

char *get_time_str()
{
	time_t rt;
	struct tm timeinfo;

	time(&rt);
	
	#ifdef _WIN32
		if (localtime_s(&timeinfo, &rt) != 0) {
			return NULL;
		}
	#else
		if (localtime_r(&rt, &timeinfo) == NULL) {
			return NULL;
		}
	#endif

	char *t = malloc(16);
	if (t == NULL) {
	return NULL;
	}

	if (strftime(t, 16, "%Y%m%d_%H%M%S", &timeinfo) == 0) {
	free(t);
	return NULL;
	}

	return t;
}
