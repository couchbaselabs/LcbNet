// This is the main DLL file.

#include "stdafx.h"
#include "CallbackContext.h"

using namespace msclr::interop;
using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Text;

namespace LCouchbase {


class KVBytesContainer
{
public:
    KVBytesContainer(const KVCommandBase^ cmd)
    {
        hKey = GCHandle::Alloc(cmd->Key, GCHandleType::Pinned);
        hasValue = false;
    }

    KVBytesContainer(const StoreCommand^ cmd)
    {
        hKey = GCHandle::Alloc(cmd->Key, GCHandleType::Pinned);
        hValue = GCHandle::Alloc(cmd->Value, GCHandleType::Pinned);
        hasValue = true;
    }

    ~KVBytesContainer() {
        hKey.Free();
        if (hasValue) {
            hValue.Free();
        }
    }


    static void barrayToPtrLen(GCHandle^ src,
                               const void **target_ptr,
                               lcb_size_t *target_len)
    {
        LcbMBufferH bsrc = static_cast<LcbMBufferH>(src->Target);
        if (bsrc->Length == 0) {
            return;
        }

        pin_ptr<unsigned char> p_pin = &bsrc[0];
        *target_ptr = p_pin;
        *target_len = bsrc->LongLength;
    }

    template <typename T> void setKeyV0(T* lcbcmd)
    {
        barrayToPtrLen(hKey, &lcbcmd->v.v0.key, &lcbcmd->v.v0.nkey);
    }

    template <typename T> void setKeyValueV0(T *lcbcmd)
    {
        setKeyV0<T>(lcbcmd);
        barrayToPtrLen(hValue, &lcbcmd->v.v0.bytes, &lcbcmd->v.v0.nbytes);
    }

private:
    GCHandle hKey;
    GCHandle hValue;
    bool hasValue;
};



Couchbase::Couchbase(String^ hostname, String^ bucket)
{
    marshal_context ctx;
    lcb_error_t err;
    lcb_t instance_tmp;
    struct lcb_create_st crOptions;

    memset(&crOptions, 0, sizeof(crOptions));
    crOptions.v.v0.host = ctx.marshal_as<const char*>(hostname);
    crOptions.v.v0.bucket = ctx.marshal_as<const char*>(bucket);

    err = lcb_create(&instance_tmp, &crOptions);
    instance = instance_tmp;
    assert (err == LCB_SUCCESS);

    SetupCallbacks();
}

Couchbase::~Couchbase()
{
    if (instance) {
        lcb_destroy(instance);
        instance = NULL;
    }
}

void Couchbase::Connect()
{
    lcb_error_t err = lcb_connect(instance);
    if (err != LCB_SUCCESS) {
        throw gcnew Exception("Couldn't schedule connection");
    }
    err = lcb_wait(instance);
    assert (err == LCB_SUCCESS);
}

OperationResult^
Couchbase::Store(const StoreCommand^ command)
{
    lcb_error_t err;
    lcb_store_cmd_t cmd;
    const lcb_store_cmd_t *cmdp = &cmd;

    memset(&cmd, 0, sizeof(cmd));
    CallbackContext ctx(this);

    cmd.v.v0.cas = command->Cas;
    cmd.v.v0.exptime = command->Expiration;
    cmd.v.v0.flags = command->Flags;
    cmd.v.v0.operation = (lcb_storage_t)(int)command->Mode;

    {
        KVBytesContainer kvb(command);
        kvb.setKeyValueV0<lcb_store_cmd_t>(&cmd);
        err = lcb_store(instance, &ctx, 1, &cmdp);
    }

    return PerformSingleWaitSequence(err, ctx);
}

OperationResult^
Couchbase::Get(const GetCommand^ command)
{
    lcb_error_t err;
    lcb_get_cmd_t cmd;
    const lcb_get_cmd_t *cmdp = &cmd;
    memset(&cmd, 0, sizeof(cmd));

    CallbackContext ctx(this);

    cmd.v.v0.exptime = command->Expiration;
    cmd.v.v0.lock = command->Lock ? 1 : 0;
    {
        KVBytesContainer kvb(command);
        kvb.setKeyV0<lcb_get_cmd_t>(&cmd);
        err = lcb_get(instance, &ctx, 1, &cmdp);
    }

    return PerformSingleWaitSequence(err, ctx);
}

};