#define MEM_DEFAULT_RESERVE_SIZE MB(64)

#include "Base.h"
#include "LongOS.h"
#include "Base.c"
#include "LongOS_Win32.c"
#include "LongGFX.h"
#include "LongGFX_Win32.c"
#include "LongGFX_OpenGL.h"
#include "Win32_OpenGL.c"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_STDIO
#define STBI_REALLOC(p, newsz) Stmnt(UNUSED(p); UNUSED(newsz);)
#define STBI_FREE(p) UNUSED(p)

Arena* stbArena;
#define STBI_MALLOC(sz) ArenaPushNZ(stbArena, sz)
#define STBI_REALLOC_SIZED(p, oldsz, newsz) CopyMem(ArenaPushNZ(stbArena, newsz), p, oldsz)

MSVC(WarnDisable(6262))
#include "stb_image.h"
MSVC(WarnEnable(6262))

#define CHANNEL_COUNT 4

typedef struct Image
{
    u32 width, height, blockSize;
    union
    {
        struct
        {
            u8* r;
            u8* g;
            u8* b;
            u8* a;
        };
        u8* channels[CHANNEL_COUNT];
    };
} Image;

typedef struct PatternTable
{
    u32 blockSize, maxBlockCount, usedBlocks;
    u64* hashes;
    u8** patterns;
} PatternTable;

function Image LoadImageFromFile(Arena* arena, String file, u32 blockSize)
{
    Image result = { .blockSize = blockSize };
    ScratchBlock(scratch, arena)
    {
        stbArena = scratch;
        
        i32 channelCount = 0;
        String data = OSReadFile(scratch, file, false);
        u8* pixels = stbi_load_from_memory(data.str, (i32)data.size, &result.width, &result.height, &channelCount, 4);
        //Assert(channelCount == CHANNEL_COUNT);
        
        u32 size = result.width * result.height;
        result.r = ArenaPush(arena, size * CHANNEL_COUNT);
        result.g = result.r + size;
        result.b = result.g + size;
        result.a = result.b + size;
        
#if 0
        for (u32 i = 0; i < size; ++i)
        {
            // TODO: add padding when size isn't a multiple of blockSize
            u32 index = i * CHANNEL_COUNT;
            result.r[i] = pixels[index + 0];
            result.g[i] = pixels[index + 1];
            result.b[i] = pixels[index + 2];
            result.a[i] = pixels[index + 3];
        }
#else
        result.r = CopyMem(result.r, pixels, result.width * result.height);
#endif
    }
    
    return result;
}

function PatternTable CreateTable(Arena* arena, u32 maxBlocks, u32 blockSize)
{
    PatternTable result = { .blockSize = blockSize, .maxBlockCount = maxBlocks };
    result.hashes = ArenaPush(arena, maxBlocks * (sizeof(*result.hashes) + sizeof(*result.patterns)));
    result.patterns = (u8**)(result.hashes + maxBlocks);
    return result;
}

function u8* GetBlockPattern(Arena* arena, Image image, u32 index, u32 channel)
{
    u8* result = 0;
    if (channel < CHANNEL_COUNT && index < image.width * image.height)
    {
        result = ArenaPush(arena, image.blockSize);
        if (result)
        {
            u8* block = image.channels[channel] + index;
            for (u32 i = 1; i < image.blockSize; ++i)
                result[i] = block[i] - block[i - 1];
#if 0
            block = result;
            result = ArenaPush(arena, image.blockSize);
            if (result)
            {
                for (u32 i = 2; i < image.blockSize; ++i)
                    result[i] = block[i] - block[i - 1];
            }
#endif
        }
    }
    
    return result;
}

function u8* HashBlockPattern(PatternTable* table, u8* pattern)
{
    u32 size = table->blockSize;
    u64 hash = Hash64(pattern, size);
    u32 i = hash % table->maxBlockCount;
    u32 iteration = 0;
    while (table->patterns[i] != 0 && !StrCompare(Str(pattern, size), Str(table->patterns[i], size), 0))
    {
        if (iteration == table->maxBlockCount)
            return 0;
        i = (i + 1) % table->maxBlockCount;
        iteration++;
    }
    if (table->patterns[i] == 0)
    {
        table->usedBlocks++;
        table->hashes[i] = hash;
        table->patterns[i] = pattern;
    }
    return table->patterns[i];
}

#define BLOCK_SIZE 16

int main(int argc, char** argv)
{
    OSInit(argc, argv);
    Arena* arena = ArenaReserve(GB(1), MEM_DEFAULT_ALIGNMENT, 0);
    
    StringList args = OSCmdArgs();;
    if (args.nodeCount != 2)
        return 0;
    
    u32 minBlock = 10000000;
    u32 minBlockSize = 0;
    String minName = {0};
    
    u32 totalBlocks = 0;
    u32 totalCount = 0;
    u32 total = 0;
    
    String path = args.first->next->string;
    PatternTable totalTable = CreateTable(arena, MB(1), BLOCK_SIZE);
    FileIterBlock(arena, iter, path)
    {
        ScratchBlock(scratch)
        {
            String finalPath = StrJoin3(scratch, path, StrLit("\\"));
            Image image = LoadImageFromFile(scratch, finalPath, BLOCK_SIZE);
            if (!image.width || !image.height)
                goto END;
            u32 size = image.width * image.height;
            u32 blockCount = size / BLOCK_SIZE;
            PatternTable table = CreateTable(scratch, blockCount, BLOCK_SIZE);
            u8* writter = ArenaPushNZ(scratch, image.width * image.height);;
            
            for (u32 i = 0; i < size; i += BLOCK_SIZE)
            {
                u8* pattern = GetBlockPattern(scratch, image, i, 0);
                Assert(pattern);
                u8* hash = HashBlockPattern(&table, pattern);
                HashBlockPattern(&totalTable, CopyMem(ArenaPushNZ(arena, BLOCK_SIZE), pattern, BLOCK_SIZE));
                //writter += WriteBlockPattern(hash, pattern, writter);
            }
            
            totalCount++;
            totalBlocks += table.usedBlocks;
            total += blockCount;
            f32 rate = table.usedBlocks/(f32)blockCount;
            if (table.usedBlocks < minBlock)
            {
                minBlock = table.usedBlocks;
                minName = iter.name;
                minBlockSize = blockCount;
            }
            Outf("%-50.*s: %3u/%u blocks, %f%%\n", StrExpand(iter.name), table.usedBlocks, blockCount, rate);
            END:;
        }
    }
    
    Outf("\n");
    Outf("MIN: %.*s: %u, %f\n", StrExpand(minName), minBlock, minBlock / (f32)minBlockSize);
    Outf("AVG: %f, %f\n", totalBlocks / (f32)totalCount, totalBlocks / (f32)total);
    Outf("Total pattern used: %u/%u\n", totalTable.usedBlocks, totalTable.maxBlockCount);
    return 0;
}