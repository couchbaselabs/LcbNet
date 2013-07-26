#include "Stdafx.h"
#include "CallbackContext.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace msclr::interop;

namespace LCouchbase {

CallbackContext::CallbackContext(Couchbase^ handle, bool is_multi) :
    obj(handle), multi(is_multi)
{
    results = gcnew ResultDict();
}

OperationResult^
CallbackContext::makeResult(LcbMBufferH key)
{
    OperationResult^ res;
    bool hasValue = results->TryGetValue(key, res);
    if (hasValue && res != nullptr) {
        return res;
    }
    res = gcnew OperationResult();
    res->Key = key;
    results->Add(key, res);
    return res;
}

OperationResult^
CallbackContext::makeResult(const void *key, const lcb_size_t nkey)
{
    LcbMBufferH buf;
    CopyBuffer(buf, key, nkey);
    if (buf == nullptr) {
        abort();
    }
    return makeResult(buf);
}

void
CallbackContext::CopyBuffer([Out] LcbMBufferH% dst, const void *src, lcb_size_t n)
{
    if (!n) {
        return;
    }

    dst = gcnew cli::array<Byte>(n);
    IntPtr pSrc(const_cast<void*>(src));
    Marshal::Copy(pSrc, dst, 0, n);
}

static void copy_buffer([Out] array<Byte>^% target,
                        const void *src,
                        lcb_size_t nsrc)
{
}


void
CallbackContext::fail(lcb_error_t err)
{
    // Copy the keys
    List<LcbMBufferH>^ curKeys = gcnew List<LcbMBufferH>(results->Keys);
    for each (LcbMBufferH k in curKeys) {
        OperationResult^ res = nullptr;
        results->TryGetValue(k, res);
        if (res == nullptr) {
            res = gcnew OperationResult();
            res->Key = k;
            results->Add(k, res);
        }
        res->Status.SetCode(err);
    }
}

void
CallbackContext::dispatchWithValue(lcb_error_t err, const lcb_get_resp_t *resp)
{
    OperationResult^ res = makeResult(resp->v.v0.key, resp->v.v0.nkey);
    if (err != LCB_SUCCESS) {
        res->Status.SetCode(err);
        return;
    }
    CopyBuffer(res->Key, resp->v.v0.key, resp->v.v0.nkey);
    CopyBuffer(res->Value, resp->v.v0.bytes, resp->v.v0.nbytes);
    res->Cas = resp->v.v0.cas;
    res->Flags = resp->v.v0.flags;
}

#define GET_CTX(c) (reinterpret_cast<CallbackContext *>(const_cast<void*>(c)))

static void get_callback(lcb_t instance,
                         const void *cookie,
                         lcb_error_t err,
                         const lcb_get_resp_t *resp)
{
    GET_CTX(cookie)->dispatchWithValue(err, resp);
}

static void store_callback(lcb_t instance,
                           const void *cookie,
                           lcb_storage_t op,
                           lcb_error_t err,
                           const lcb_store_resp_t *resp)
{
    GET_CTX(cookie)->dispatchWithCas<lcb_store_resp_t>(err, resp);
}

static void delete_callback(lcb_t instance,
                            const void *cookie,
                            lcb_error_t err,
                            const lcb_remove_resp_t *resp)
{
    GET_CTX(cookie)->dispatchWithCas<lcb_remove_resp_t>(err, resp);
}

static void touch_callback(lcb_t instance,
                           const void *cookie,
                           lcb_error_t err,
                           const lcb_touch_resp_t *resp)
{
    GET_CTX(cookie)->dispatchWithCas<lcb_touch_resp_t>(err, resp);
}

static void error_callback(lcb_t instance,
                           lcb_error_t err,
                           const char *msg)
{
    printf("Couldn't connect.. %d (%s)\n", err, msg);
    abort();
}

// Wire the callbacks
void
Couchbase::SetupCallbacks()
{
    lcb_set_get_callback(instance, get_callback);
    lcb_set_store_callback(instance, store_callback);
    lcb_set_error_callback(instance, error_callback);
    lcb_set_remove_callback(instance, delete_callback);
    lcb_set_touch_callback(instance, touch_callback);

}

}; // namespace