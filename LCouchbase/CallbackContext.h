#pragma once
#include "Stdafx.h"
#include "OperationResult.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace msclr::interop;
using namespace System::Collections::Generic;
using namespace cli;

namespace LCouchbase {
ref class Couchbase;

class CallbackContext {
public:
    typedef Dictionary<LcbMBufferH, OperationResult^> ResultDict;
    CallbackContext(Couchbase^ handle, bool multi=false);
    void fail(lcb_error_t err);

    void registerKeys(IEnumerable<LcbMBufferH>^ keys)
    {
        for each (LcbMBufferH k in keys) {
            registerKey(k);
        }
    }
    
    bool isMulti() {
        return multi;
    }

    void registerKey(LcbMBufferH key)
    {
        results->Add(key, nullptr);
    }

    // Convenience function to return the result on error.
    OperationResult^ returnSingle()
    {
        for each (OperationResult^ res in results->Values) {
            return res;
        }
        return nullptr;
    }

    void handleKCas(const void *key, lcb_size_t nkey, lcb_cas_t cas);

    template <typename T>
    void dispatchWithCas(lcb_error_t err, const T* resp)
    {
        OperationResult^ res = makeResult(resp->v.v0.key,resp->v.v0.nkey);

        if (err != LCB_SUCCESS) {
            res->Status.SetCode(err);
            return;
        }

        res->Cas = resp->v.v0.cas;
    }

    void dispatchWithValue(lcb_error_t err, const lcb_get_resp_t *resp);

    static void CopyBuffer([Out] LcbMBufferH% target, const void *src, lcb_size_t n);

private:
    gcroot<Couchbase^> obj;
    gcroot<ResultDict^> results;
    bool multi;
    OperationResult^ makeResult(const void *key, lcb_size_t nkey);
    OperationResult^ makeResult(LcbMBufferH key);
};

};