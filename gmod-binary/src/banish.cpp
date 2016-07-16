#include "GarrysMod/Lua/Interface.h"
#include "banish.h"
#include <stdio.h>
#include "concurrentqueue.h"

using namespace GarrysMod::Lua;

class CallbackDataParam {
public:
	virtual int push(lua_State* state) {
		return 0;
	}
};
class CDPString: public CallbackDataParam {
public:
	int length;
	char* data;
	CDPString(int len, char* dat) : length(len), data(dat) {}
	virtual int push(lua_State* state) {
		LUA->PushString(data, length);
		return 1;
	}
};
class CDPNil : CallbackDataParam {
	virtual int push(lua_State* state) {
		LUA->PushNil();
		return 1;
	}
};

struct CallbackData {
	int callbackReference;
	CallbackDataParam* param;
};
moodycamel::ConcurrentQueue<CallbackData> callbackQueue;

int InvokeCallbacks(lua_State* state)
{
	CallbackData data;
	while (callbackQueue.try_dequeue(data)) {
		LUA->ReferencePush(data.callbackReference);

		auto param = data.param;
		if (param != NULL) {
			int pushed = param->push(state);
			LUA->Call(pushed, 0);

			delete param;
		}
		else {
			LUA->Call(0, 0);
		}

		LUA->ReferenceFree(data.callbackReference);
	}

	return 0;
}

const char* SERVER_META_NAME = "BanishServer";
const int SERVER_META_ID = 142;
const char* REPO_META_NAME = "BanishRepository";
const int REPO_META_ID = 143;

int CreateServer(lua_State* state)
{
	void* repo_ptr = ((GarrysMod::Lua::UserData *) LUA->GetUserdata(1))->data;
	int port = 8091;
	if ( LUA->IsType(2, Type::NUMBER ))
	{
		// TODO validity checks
		port = (int) LUA->GetNumber(2);
	}

	void* server = banish_create_server(repo_ptr, port);
	
	GarrysMod::Lua::UserData *userdata = (GarrysMod::Lua::UserData *)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
	userdata->data = server;
	userdata->type = SERVER_META_ID;

	LUA->CreateMetaTableType(SERVER_META_NAME, SERVER_META_ID);
	LUA->SetMetaTable(-2);

	return 1;
}

int Banish_CloseServer(lua_State* state)
{
	GarrysMod::Lua::UserData *userdata = (GarrysMod::Lua::UserData *) LUA->GetUserdata(1);
	
	if (userdata->data != NULL) {
		banish_destroy_server(userdata->data);
		userdata->data = NULL;
	}
	
	return 0;
}

int CreateRepository(lua_State* state)
{
	void* repo = banish_create_repo();

	GarrysMod::Lua::UserData *userdata = (GarrysMod::Lua::UserData *)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
	userdata->data = repo;
	userdata->type = REPO_META_ID;

	LUA->CreateMetaTableType(REPO_META_NAME, REPO_META_ID);
	LUA->SetMetaTable(-2);

	return 1;
}

int Banish_PullRemoteURL(lua_State* state)
{
	void* ptr = ((GarrysMod::Lua::UserData *) LUA->GetUserdata(1))->data;
	const char* url = LUA->CheckString(2);

	int callbackReference = -1;

	if (LUA->IsType(3, GarrysMod::Lua::Type::FUNCTION))
	{
		LUA->Push(3); // presumably this pushes the function to top of stack?? WTB proper docs
		callbackReference = LUA->ReferenceCreate();
	}

	banish_pull_from_url(ptr, url, [](int callbackReference) {
		if (callbackReference != -1) {
			CallbackData data = { callbackReference, NULL };
			callbackQueue.enqueue(data);
		}
	}, callbackReference);
	
	return 0;
}

int Banish_GetHeadData(lua_State* state)
{
	void* ptr = ((GarrysMod::Lua::UserData *) LUA->GetUserdata(1))->data;

	int callbackReference = -1;

	if (LUA->IsType(2, GarrysMod::Lua::Type::FUNCTION))
	{
		LUA->Push(2);
		callbackReference = LUA->ReferenceCreate();
	}

	banish_get_head_data(ptr, [](int callbackReference, const BanishDataArray* arr) {
		if (callbackReference != -1) {

			CallbackDataParam* param = NULL;
			if (arr != NULL) {
				// Create a copy of data
				char* data = new char[arr->length];
				memcpy(data, arr->data, arr->length);

				param = new CDPString(arr->length, data);
			}

			CallbackData data = { callbackReference, param };
			callbackQueue.enqueue(data);
		}
	}, callbackReference);

	return 0;
}

int Banish_SetHeadData(lua_State* state)
{
	void* ptr = ((GarrysMod::Lua::UserData *) LUA->GetUserdata(1))->data;

	unsigned int dataLength = 0;
	const char* data = LUA->GetString(2, &dataLength);

	const uint8_t* bytedata = reinterpret_cast<const uint8_t*>(data);

	banish_set_head_data(ptr, bytedata, dataLength);

	return 0;
}

//
// Called when you module is opened
//
GMOD_MODULE_OPEN()
{
	LUA->CreateMetaTableType(SERVER_META_NAME, SERVER_META_ID);
		LUA->CreateTable();
			LUA->PushCFunction(Banish_CloseServer);
			LUA->SetField(-2, "Close");
		LUA->SetField(-2, "__index");

		LUA->PushString("BanishServer");
		LUA->SetField(-2, "__type");

		LUA->PushCFunction(Banish_CloseServer);
		LUA->SetField(-2, "__gc");
		LUA->Pop();
	LUA->CreateMetaTableType(REPO_META_NAME, REPO_META_ID);
		LUA->CreateTable();
			LUA->PushCFunction(Banish_PullRemoteURL);
			LUA->SetField(-2, "PullFromRemoteURL");
			LUA->PushCFunction(Banish_GetHeadData);
			LUA->SetField(-2, "GetHeadData");
			LUA->PushCFunction(Banish_SetHeadData);
			LUA->SetField(-2, "SetHeadData");
		LUA->SetField(-2, "__index");

		LUA->PushString("BanishRepository");
		LUA->SetField(-2, "__type");
	LUA->Pop();

	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->PushString("banish");
	LUA->CreateTable();

		LUA->PushString("CreateServer");
		LUA->PushCFunction(CreateServer);
		LUA->SetTable(-3);

		LUA->PushString("CreateRepository");
		LUA->PushCFunction(CreateRepository);
		LUA->SetTable(-3);

	LUA->SetTable(-3);

	// Add the Think hook to call callbacks
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->GetField(-1, "hook");
	LUA->GetField(-1, "Add");
	LUA->PushString("Think");
	LUA->PushString("BanishCallbackInvoker");
	LUA->PushCFunction(InvokeCallbacks);
	LUA->Call(3, 0);

	return 0;
}

//
// Called when your module is closed
//
GMOD_MODULE_CLOSE()
{
	return 0;
}