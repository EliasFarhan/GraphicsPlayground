#pragma once
#include <thread>
#include <condition_variable>
#include <future>
#include <shared_mutex>

#ifdef TRACY_ENABLE
#include "Tracy.hpp"
#endif
namespace core
{

class Task
{
public:
    Task(std::function<void()> task);
    Task(const Task&) = delete;
    Task& operator=(Task&) = delete;
    Task(Task&&) noexcept = delete;
    Task& operator=(Task&& job) noexcept = delete;
    void Join();
    void Execute();
    [[nodiscard]] bool CheckDependenciesStarted() const;
    [[nodiscard]] bool IsDone() const;
    [[nodiscard]] bool HasStarted() const;
    void AddDependency(const std::weak_ptr<Task>& newDependencyPtr);
    /**
     * \brief This function clears the dependencies, the promise/future and status, but keeps the task function
     */
    void Reset();
private:
    enum TaskStatus : std::uint8_t
    {
        NONE = 0u,
        STARTED = 1u << 0u,
        DONE = 1u << 1u,
    };
    std::function<void()> task_;
    std::atomic<std::uint8_t> status_;

    std::promise<void> promise_;
    std::shared_future<void> taskDoneFuture_;
    std::vector<std::weak_ptr<Task>> dependencies_;
};

class WorkerQueue
{
public:
    ~WorkerQueue();
    [[nodiscard]] bool IsEmpty() const;
    [[nodiscard]] std::shared_ptr<Task> PopNextTask();
    void AddTask(std::shared_ptr<Task> task);
    void WaitForTask();
    void Destroy();
private:
    std::vector<std::shared_ptr<Task>> tasks_;
#ifdef TRACY_ENABLE
    mutable TracySharedLockable ( std::shared_mutex , queueMutex_ );
#else
    mutable std::shared_mutex queueMutex_;
#endif
    std::condition_variable_any conditionVariable_;
};


class WorkerThread
{
public:
    WorkerThread(WorkerQueue& queue);
    ~WorkerThread();
    void Start();
    void Destroy();
private:
    void Loop();
    WorkerQueue& taskQueue_;
    std::thread thread_;
    bool isRunning_ = true;
};

class Jobsystem
{
public:
    enum class QueueType
    {
        RENDER_THREAD = 0,
        OTHER_THREAD
    };
    Jobsystem();
private:
    std::vector<WorkerQueue> queues_;
    std::vector<WorkerThread> threads_;
};


}