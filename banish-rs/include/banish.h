
#ifndef cheddar_generated_banish_h
#define cheddar_generated_banish_h


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>



typedef void BanishServer;

typedef void BanishRepository;

typedef struct BanishData BanishData;

typedef struct BanishDataArray {
	uintptr_t length;
	char const* data;
} BanishDataArray;

BanishRepository* banish_create_repo(void);

void banish_pull_from_url(BanishRepository* repo_ptr, char const* url, void (*callback)(int ), int callback_param);

void banish_get_head_data(BanishRepository* repo_ptr, void (*callback)(int , BanishDataArray const* ), int callback_param);

void banish_set_head_data(BanishRepository* repo_ptr, uint8_t const* data, uintptr_t data_length);

BanishServer* banish_create_server(BanishRepository* repo, uint16_t port);

void banish_destroy_server(BanishServer* server);



#ifdef __cplusplus
}
#endif


#endif
