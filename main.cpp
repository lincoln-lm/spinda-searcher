#include <iostream>
#include <chrono>
#include <thread>
#include <array>
#include <Core/RNG/MT.hpp>

class ExtendedMT : public MT
{
public:
    ExtendedMT(std::array<u32, 2> seeds) : MT(0x12BD6AA)
    {
        u32 *ptr = &state[0].uint32[0];
        int i = 1, j = 0;
        for (int mti = 624; mti > 0; mti--)
        {
            ptr[i] = ((ptr[i] ^ ((ptr[i - 1] ^ (ptr[i - 1] >> 30)) * 0x19660D)) + seeds[j] + j);
            if (++i >= 624)
            {
                i = 1;
                ptr[0] = ptr[623];
            }
            j ^= 1;
        }
        for (int mti = 623; mti > 0; mti--)
        {
            ptr[i] = ((ptr[i] ^ ((ptr[i - 1] ^ (ptr[i - 1] >> 30)) * 0x5D588B65)) - i);
            if (++i >= 624)
            {
                i = 1;
                ptr[0] = ptr[623];
            }
        }
        ptr[0] = 0x80000000;
        index = 624;
    }

    template <u32 maximum>
    u32 nextRand()
    {
        return (static_cast<u64>(next()) * static_cast<u64>(maximum)) >> 32;
    }
};

struct EggResult
{
    u32 ec;
    u8 ivs[6] = {255, 255, 255, 255, 255, 255};
    u8 nature;
};

inline EggResult generateSpinda(u32 seed_0, u32 seed_1)
{
    EggResult result;
    ExtendedMT m({seed_0, seed_1});
    m.next(); // gender m.nextRand<252>();
    result.nature = m.nextRand<25>();
    m.next(); // ability m.nextRand<100>();
    for (int i = 0; i < 5;)
    {
        u32 index = m.nextRand<6>();
        if (result.ivs[index] == 255)
        {
            result.ivs[index] = 32 + m.nextRand<2>();
            i++;
        }
    }
    for (int i = 0; i < 6; i++)
    {
        u32 iv = m.nextRand<32>();
        if (result.ivs[i] == 255)
        {
            result.ivs[i] = iv;
        }
    }
    result.ec = m.next();
    return result;
}

class EggSearcher
{
public:
    void startSearch(int threads)
    {
        searching = true;

        auto *threadContainer = new std::thread[threads + 1];

        u32 split = (endingSeed - startingSeed) / threads;
        u32 start = startingSeed;
        for (int i = 0; i < threads; i++, start += split)
        {
            if (i == threads - 1)
            {
                threadContainer[i] = std::thread([=, this]
                                                 { search(start, endingSeed); });
            }
            else
            {
                threadContainer[i] = std::thread([=, this]
                                                 { search(start, start + split); });
            }
        }

        threadContainer[threads] = std::thread([=, this]
                                               { logProgress(); });

        for (int i = 0; i < threads; i++)
        {
            threadContainer[i].join();
        }

        searching = false;

        threadContainer[threads].join();

        delete[] threadContainer;
    }

    void search(u32 start, u32 end)
    {
        for (u32 initial_seed = start; initial_seed <= end; initial_seed++, progress++)
        {
            MT rng(initial_seed, 2 + startingFrame);
            u32 seed_0 = rng.next();
            u32 seed_1 = rng.next();
            for (u32 frame = startingFrame; frame < endingFrame; frame++)
            {
                EggResult result = generateSpinda(seed_0, seed_1);
                if (result.ec == 0x88888888)
                {
                    printf("initial seed: %08X\n", initial_seed);
                    printf("seed 0: %08X\n", seed_0);
                    printf("seed 1: %08X\n", seed_1);
                    printf("frame: %u\n", frame);
                    printf("ec: %08X\n", result.ec);
                    printf("ivs: %02d/%02d/%02d/%02d/%02d/%02d\n", result.ivs[0], result.ivs[1], result.ivs[2], result.ivs[3], result.ivs[4], result.ivs[5]);
                    printf("nature: %u\n", result.nature);
                }
                seed_0 = seed_1;
                seed_1 = rng.next();
            }
        }
    }

    void logProgress()
    {
        u32 last_progress = 0;
        auto start_time = std::chrono::high_resolution_clock::now();
        while (searching)
        {
            if (progress - last_progress > 1000)
            {
                auto duration = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start_time);
                printf("seeds checked: %u\n", progress);
                printf("in: %f ms\n", duration.count());
                printf("%f seeds per second\n", progress / (duration.count() / 1000));
                printf("%f frames per second\n\n", progress / (duration.count() / 1000) * (endingFrame - startingFrame));
                last_progress = progress;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    u32 startingFrame = 5000;
    u32 endingFrame = 10000;

    u32 startingSeed = 0x0;
    u32 endingSeed = 0xFFFF;

protected:
    u32 progress = 0;
    bool searching = false;
};

int main()
{
    EggSearcher searcher;
    int threadCount;

    printf("thread concurrency hint: %d\n", std::thread::hardware_concurrency());
    printf("thread count: ");
    scanf("%d", &threadCount);
    printf("starting frame: ");
    scanf("%d", &searcher.startingFrame);
    printf("ending frame: ");
    scanf("%d", &searcher.endingFrame);
    printf("starting seed: 0x");
    scanf("%x", &searcher.startingSeed);
    printf("ending seed: 0x");
    scanf("%x", &searcher.endingSeed);

    printf("\n");
    searcher.startSearch(threadCount);
    return 0;
}