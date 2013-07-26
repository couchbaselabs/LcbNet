#pragma once

#include "stdafx.h"

using namespace System;
namespace LCouchbase {

typedef cli::array<Byte> LcbMBuffer;
typedef cli::array<Byte>^ LcbMBufferH;


public ref class ErrorCode {
public:
	virtual String^ ToString() override
	{
		const char *err = lcb_strerror(NULL, (lcb_error_t)Code);
		return gcnew String(err);
	}

	virtual bool Equals(Object^ other) override {
		if (other == nullptr) {
			return false;
		}

		if (other->GetType() != this->GetType()) {
			return false;
		}

		return ((ErrorCode^)other)->Code == Code;
	}

	ErrorCode(lcb_error_t c) : Code(c) { }
	ErrorCode(int c) : Code(c) {}
	ErrorCode() : Code(0) { }

	property bool Success {
		bool get() {
			return Code == 0;
		}
	};

internal:
	int Code;
    bool IsSet;

    void SetCode(lcb_error_t err) {
        if (IsSet) {
            return;
        }
        IsSet = true;
        Code = (int)err;
    }
};

typedef cli::array<Byte>^ Foo;

public ref struct OperationResult {
    UInt64 Cas;
    UInt32 Flags;
    LcbMBufferH Key;
    LcbMBufferH Value;
    ErrorCode Status;
};

};