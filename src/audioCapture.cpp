// Copyright (c) 2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#include "audioCapture.h"

#include "logging.h"

audioCapture::audioCapture(uint32_t capacity, uint32_t rate, uint32_t channels) :
pHandle(NULL),
pBuff(NULL),
mCapacity(capacity),
mBitrate(rate),
mChannels(channels) {
    initialize();
}

audioCapture::~audioCapture() {
    finalize();
}

void audioCapture::initialize() {
    pBuff = new uint8_t [mCapacity];

    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = (uint32_t)mBitrate,
        .channels= (uint8_t)mChannels
    };

    // for low latency. (response time)
    pa_buffer_attr attr;
    memset(&attr, 0x00, sizeof(pa_buffer_attr));
    attr.maxlength = attr.prebuf  = attr.minreq = (uint32_t) -1; // default value
    attr.fragsize  = attr.tlength = mCapacity;

    int error;

    if (!(pHandle = pa_simple_new(NULL, "PulseCapture", PA_STREAM_RECORD, NULL, "record", &ss, NULL, &attr, &error))) {
        GOOGLEAI_LOG_ERROR("%s failed. %s", __FUNCTION__, pa_strerror(error));
    }
}

void audioCapture::finalize() {
    if (pHandle) {
        pa_simple_free(pHandle);
        pHandle = NULL;
    }

    if (pBuff) {
        delete pBuff;
        pBuff = NULL;
    }
}

void audioCapture::flush() {
    if (pHandle) {
        pa_simple_flush(pHandle, NULL);
    }
}

uint8_t* audioCapture::get(size_t size) {
    if (!size) size = mCapacity;

    if (pHandle && size <= mCapacity) {
        int error;

        if (pa_simple_read(pHandle, pBuff, size, &error) < 0) {
            GOOGLEAI_LOG_ERROR("%s failed. %s", __FUNCTION__, pa_strerror(error));
            return NULL;
        }

        return pBuff;
    }

    return NULL;
}
