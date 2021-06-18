#pragma once
#include <thread>
#include <condition_variable>
#include <future>

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
    void AddDependency(std::weak_ptr<Task> dep);
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
    [[nodiscard]] bool IsEmpty() const;
    [[nodiscard]] std::weak_ptr<Task> GetNextTask();
    void AddTask(std::shared_ptr<Task>&& task);
private:
    std::vector<std::shared_ptr<Task>> tasks_;
    mutable std::mutex queueMutex_;
};


class WorkerThread
{
public:
    WorkerThread(WorkerQueue& queue);
    void Start(std::function<void()> workerFunction);
private:
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