/* Copyright (c) 2013 Wildfire Games
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "dynamic_pool.h"


#if MSC_VERSION // VC++
	#define threadlocal __declspec(thread)
#else // all others (GCC, CLANG, etc..) use __thread
	#define threadlocal __thread
#endif
static threadlocal global_pool_t* _tls_pools = NULL;	// storage array for thread-local pools


/** 
 * Edit these values according to memory tuning data
 * Each value represents a COUNT HINT of nodes to initially allocate per pool
 * Buckets [0..24]		4 8 12 16 20 24 
 * Buckets [25..64]		32 40 48 56 64 
 * Buckets [65..128]	80 96 112 128 
 * Buckets [129..256]	160 192 224 256 
 * Buckets [257..1024]	384 512 640 768 896 1024
 */
enum E_TLS_POOLS_RESERVE_COUNT_HINTS
{
	POOL_4 = 4096,
	POOL_8 = 4096,
	POOL_12 = 4096,
	POOL_16 = 2048,
	POOL_20 = 1024,
	POOL_24 = 512,

	// strings that don't fit std::string static buffer fall here first
	// so give 32 a good reserve
	POOL_32 = 512,
	POOL_40 = 224,
	POOL_48 = 192,
	POOL_56 = 160,
	POOL_64 = 128,

	// this is a pretty allocator intensive range:
	POOL_80 = 128,
	POOL_96 = 96,
	POOL_112 = 64,
	POOL_128 = 64,

	POOL_160 = 48,
	POOL_192 = 48,
	POOL_224 = 48,
	POOL_256 = 48,

	// these esoteric ranges need a lot more tuning:
	POOL_384 = 48,
	POOL_512 = 32,
	POOL_640 = 24,
	POOL_768 = 24,
	POOL_896 = 16,
	POOL_1024 = 16,
};



// lookup table for _tls_pools [4..1024]
// element index: ((TSIZE+3)/4)-1
static unsigned char _pool_indices[256] = 
{
	// 0..24
	// 4 8 12 16 20 24 
	0,	// 4 (4)
	1,	// 8 (8)
	2,	// 12 (12)
	3,	// 16 (16)
	4,	// 20 (20)
	5,	// 24 (24)

	// 25..64
	// 32 40 48 56 64 
	6,		// 28 (32)
	6,		// 32 (32)
	7,		// 36 (40)
	7,		// 40 (40)
	8,		// 44 (48)
	8,		// 48 (48)
	9,		// 52 (56)
	9,		// 56 (56)
	10,		// 60 (64)
	10,		// 64 (64)

	// 65..128
	// 80 96 112 128 
	11,		// 68 (80)
	11,		// 72 (80)
	11,		// 76 (80)
	11,		// 80 (80)
		12,		// 84 (96)
		12,		// 88 (96)
		12,		// 92 (96)
		12,		// 96 (96)
	13,		// 100 (112)
	13,		// 104 (112)
	13,		// 108 (112)
	13,		// 112 (112)
		14,		// 116 (128)
		14,		// 120 (128)
		14,		// 124 (128)
		14,		// 128 (128)

	// 129..256
	// 160 192 224 256 
	15, 15,	// (160)
	15, 15,	// (160)
	15, 15,	// (160)
	15, 15,	// (160)
		16, 16,	// (192)
		16, 16,	// (192)
		16, 16,	// (192)
		16, 16,	// (192)
	17, 17,	// (224)
	17, 17,	// (224)
	17, 17,	// (224)
	17, 17,	// (224)
		18, 18,	// (256)
		18, 18,	// (256)
		18, 18,	// (256)
		18, 18,	// (256)

	// 257..1024
	// 384 512 640 768 896 1024
	19,	19, 19, 19, // 384
	19,	19, 19, 19, // 384
	19,	19, 19, 19, // 384
	19,	19, 19, 19, // 384
	19,	19, 19, 19, // 384
	19,	19, 19, 19, // 384
	19,	19, 19, 19, // 384
	19,	19, 19, 19, // 384

	20,	20, 20, 20, // 512
	20,	20, 20, 20, // 512
	20,	20, 20, 20, // 512
	20,	20, 20, 20, // 512
	20,	20, 20, 20, // 512
	20,	20, 20, 20, // 512
	20,	20, 20, 20, // 512
	20,	20, 20, 20, // 512

	21,	21, 21, 21, // 640
	21,	21, 21, 21, // 640
	21,	21, 21, 21, // 640
	21,	21, 21, 21, // 640
	21,	21, 21, 21, // 640
	21,	21, 21, 21, // 640
	21,	21, 21, 21, // 640
	21,	21, 21, 21, // 640
	
	22,	22, 22, 22, // 768
	22,	22, 22, 22, // 768
	22,	22, 22, 22, // 768
	22,	22, 22, 22, // 768
	22,	22, 22, 22, // 768
	22,	22, 22, 22, // 768
	22,	22, 22, 22, // 768
	22,	22, 22, 22, // 768
	
	23,	23, 23, 23, // 896
	23,	23, 23, 23, // 896
	23,	23, 23, 23, // 896
	23,	23, 23, 23, // 896
	23,	23, 23, 23, // 896
	23,	23, 23, 23, // 896
	23,	23, 23, 23, // 896
	23,	23, 23, 23, // 896

	24,	24, 24, 24, // 1024
	24,	24, 24, 24, // 1024
	24,	24, 24, 24, // 1024
	24,	24, 24, 24, // 1024
	24,	24, 24, 24, // 1024
	24,	24, 24, 24, // 1024
	24,	24, 24, 24, // 1024
	24,	24, 24, 24, // 1024
};




static void _tls_pools_init()
{
	#define DYNAMIC_POOL(SIZE) global_pool_t(SIZE, POOL_##SIZE)

	// allocate storage for all the pools ( we never free this )
	_tls_pools = (global_pool_t*)malloc(sizeof(global_pool_t) * 25);
	_tls_pools[0] = DYNAMIC_POOL(4);
	_tls_pools[1] = DYNAMIC_POOL(8);
	_tls_pools[2] = DYNAMIC_POOL(12);
	_tls_pools[3] = DYNAMIC_POOL(16);
	_tls_pools[4] = DYNAMIC_POOL(20);
	_tls_pools[5] = DYNAMIC_POOL(24);
	_tls_pools[6] = DYNAMIC_POOL(32);
	_tls_pools[7] = DYNAMIC_POOL(40);
	_tls_pools[8] = DYNAMIC_POOL(48);
	_tls_pools[9] = DYNAMIC_POOL(56);
	_tls_pools[10] = DYNAMIC_POOL(64);
	_tls_pools[11] = DYNAMIC_POOL(80);
	_tls_pools[12] = DYNAMIC_POOL(96);
	_tls_pools[13] = DYNAMIC_POOL(112);
	_tls_pools[14] = DYNAMIC_POOL(128);
	_tls_pools[15] = DYNAMIC_POOL(160);
	_tls_pools[16] = DYNAMIC_POOL(192);
	_tls_pools[17] = DYNAMIC_POOL(224);
	_tls_pools[18] = DYNAMIC_POOL(256);
	_tls_pools[19] = DYNAMIC_POOL(384);
	_tls_pools[20] = DYNAMIC_POOL(512);
	_tls_pools[21] = DYNAMIC_POOL(640);
	_tls_pools[22] = DYNAMIC_POOL(768);
	_tls_pools[23] = DYNAMIC_POOL(896);
	_tls_pools[24] = DYNAMIC_POOL(1024);
}



global_pool_t* _get_tls_pool(unsigned int requestSize)
{
	if (!_tls_pools) // this thread has not initialized the _tls_pools array?
	{
		_tls_pools_init(); // initialize the pools for this thread
	}

	// calculating the index: alignto4(requestSize) / 4  -  1
	const uint32_t index = (uint32_t)_pool_indices[ ((requestSize + 3) >> 2) - 1 ];
	return &_tls_pools[index];
}



int TLSALLOCS = 0;



void* tls_alloc(const uint32_t numBytes)
{
	//return malloc(numBytes);

	const uint32_t size = sizeof(tls_node) + numBytes;
	if (size <= POOL_MAX_BUCKET_SIZE) // default MAX: 1024 bytes
	{
		global_pool_t* pool = _get_tls_pool(size + 16);
		tls_node* node = (tls_node*)pool->alloc();
		node->gpool = pool;
		return node->data;
	}
	tls_node* node = (tls_node*)malloc(size + 16);
	node->gpool = NULL; // no gpool, so its malloc
	return node->data;
}



void tls_free(void* ptr)
{
	//return free(ptr);

	tls_node* node = &((tls_node*)ptr)[-1];
	if (node->gpool)
	{
		node->gpool->dealloc(node); // dealloc self
		return;
	}
	free(ptr); // its nothing special :(
}


