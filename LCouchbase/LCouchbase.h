// LCouchbase.h

#pragma once
#include "stdafx.h"
#include "CallbackContext.h"
#include "OperationResult.h"

using namespace System;
using namespace System::Net;
using namespace System::Text;

namespace LCouchbase {


public ref class KVCommandBase {
public:
    KVCommandBase(LcbMBufferH key) {
        Key = key;
    }

    LcbMBufferH Key;
};

public ref class GetCommand : KVCommandBase {
public:
    GetCommand(LcbMBufferH key) : KVCommandBase(key) { }
    UInt32 Expiration;
    bool Lock;
};

public ref class CasCommand : KVCommandBase {
public:
    CasCommand(LcbMBufferH key) : KVCommandBase(key) { }
    UInt64 Cas;
};

public enum class StoreMode : int {
	Add = LCB_ADD,
	Set = LCB_SET,
	Append = LCB_APPEND,
	Prepend = LCB_PREPEND,
	Replace = LCB_REPLACE
};

public ref class StoreCommand : CasCommand {
public:
    StoreCommand(LcbMBufferH key) : CasCommand(key) { }
    UInt32 Expiration;
    UInt32 Flags;
    StoreMode Mode;
    LcbMBufferH Value;
};

public ref class UnlockCommand : CasCommand {
public:
    UnlockCommand(LcbMBufferH key) : CasCommand(key) { }
};

public ref class DeleteCommand : CasCommand {
public:
    DeleteCommand(LcbMBufferH key) : CasCommand(key) { }
};

public ref class TouchCommand : CasCommand {
public:
    TouchCommand(LcbMBufferH key) : CasCommand(key) { }
};

public ref class HttpCommand {
public:
    HttpWebRequest^ Request;
    bool IsChunked;
    enum class ApiType : int {
        View = LCB_HTTP_TYPE_VIEW,
        Raw = LCB_HTTP_TYPE_RAW,
        Management = LCB_HTTP_TYPE_MANAGEMENT
    };
    ApiType Type;
};

public ref struct HttpResult {
public:
    HttpWebResponse^ Response;
    ErrorCode LcbError;
};

public ref class Couchbase
{
public:
    Couchbase(String^ hostname, String^ bucket);
    ~Couchbase();
    void Connect();

    OperationResult^ Store(const StoreCommand^ cmd);
    OperationResult^ Get(const GetCommand^ cmd);
    //OperationResult^ Delete(const CasCommand^ cmd);
    //OperationResult^ Touch(const CasCommand^ cmd);
    //OperationResult^ Unlock(const CasCommand^ cmd);
    //HttpResult^ MakeHttpRequest(const HttpCommand^ cmd);

protected:
    lcb_t instance;

private:
    void SetupCallbacks();
    OperationResult^ PerformSingleWaitSequence(lcb_error_t lasterr,
                                               CallbackContext& ctx)
    {
        if (lasterr != LCB_SUCCESS) {
            ctx.fail(lasterr);
        }
        lasterr = lcb_wait(instance);
        if (lasterr != LCB_SUCCESS) {
            ctx.fail(lasterr);
        }
        return ctx.returnSingle();
    }

};
}
