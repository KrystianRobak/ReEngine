#include "ThreadPool.h"

ThreadPool::~ThreadPool() {
	shutdown();
}

void ThreadPool::Init(size_t threads)
{
    unsigned int cores = std::thread::hardware_concurrency();
    // Reserve 1 core for Main Thread, 1-2 for IO, rest for Compute
    unsigned int ioCount = 4;
    unsigned int computeCount = (cores > 5) ? cores - 5 : 1;

    // --- Create Compute Workers ---
    for (unsigned int i = 0; i < computeCount; ++i) {
        computeThreads.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(computeMutex);
                    computeCv.wait(lock, [this] { return stop || !computeQueue.empty(); });
                    if (stop && computeQueue.empty()) return;
                    task = std::move(computeQueue.front());
                    computeQueue.pop();
                }
                task();
            }
            });
    }

    // --- Create IO Workers ---
    for (unsigned int i = 0; i < ioCount; ++i) {
        ioThreads.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(ioMutex);
                    ioCv.wait(lock, [this] { return stop || !ioQueue.empty(); });
                    if (stop && ioQueue.empty()) return;
                    task = std::move(ioQueue.front());
                    ioQueue.pop();
                }
                task();
            }
            });
    }
}


void ThreadPool::shutdown() {
    stop = true;
    computeCv.notify_all();
    ioCv.notify_all();

    for (auto& t : computeThreads) if (t.joinable()) t.join();
    for (auto& t : ioThreads) if (t.joinable()) t.join();

    computeThreads.clear();
    ioThreads.clear();
}

void ThreadPool::Dispatch(uint32_t elementCount, uint32_t batchSize, std::function<void(uint32_t, uint32_t)> job)
{
    if (elementCount == 0 || batchSize == 0) return;

    // Calculate how many chunks/jobs we need
    uint32_t jobCount = (elementCount + batchSize - 1) / batchSize;
    std::atomic<uint32_t> jobsRemaining = jobCount;
    std::promise<void> donePromise;
    auto doneFuture = donePromise.get_future();

    for (uint32_t i = 0; i < jobCount; ++i) {
        uint32_t start = i * batchSize;
        uint32_t end = std::min(start + batchSize, elementCount);

        // Submit to COMPUTE queue
        submit(JobType::General, [start, end, &job, &jobsRemaining, &donePromise]() {
            job(start, end); // Execute the actual ECS logic

            // Decrement atomic counter
            if (jobsRemaining.fetch_sub(1) == 1) {
                donePromise.set_value(); // Unblock the main thread
            }
            });
    }

    // Wait for all chunks to finish
    // NOTE: In a real engine, the main thread should help execute jobs here instead of sleeping!
    doneFuture.wait();
}

void ThreadPool::pushToCompute(std::function<void()>& task)
{
    {
        std::lock_guard<std::mutex> lock(computeMutex);
        computeQueue.push(task);
    }
    computeCv.notify_one();
}

void ThreadPool::pushToIO(std::function<void()>& task)
{
    {
        std::lock_guard<std::mutex> lock(ioMutex);
        ioQueue.push(task);
    }
    ioCv.notify_one();
}


