//
// Created by efarhan on 6/18/21.
//

#include "jobsystem.h"

#include <utility>


namespace core
{

Task::Task(std::function<void()> task) : task_(std::move(task)),
                                         status_(NONE),
                                         taskDoneFuture_(promise_.get_future())
{

}

void Task::Join()
{
    if (!IsDone())
    {
        taskDoneFuture_.get();
    }
}

void Task::Execute()
{
    for (auto& depPtr : dependencies_)
    {
        std::shared_ptr<Task> dependency = depPtr.lock();
        if (dependency == nullptr)
            continue;

        if (!dependency->IsDone())
        {
            dependency->Join();
        }
    }
    status_ |= STARTED;
    task_();
    status_ |= DONE;
    promise_.set_value();
}

bool Task::CheckDependenciesStarted() const
{
    if (dependencies_.empty())
        return true;
    for (const auto& depPtr : dependencies_)
    {
        std::shared_ptr<Task> dependency = depPtr.lock();
        if (dependency == nullptr)
            continue;
        if (!dependency->HasStarted())
            return false;
    }
    return true;
}

bool Task::IsDone() const
{
    return status_ & DONE;
}

bool Task::HasStarted() const
{
    return status_ & STARTED;
}

void Task::Reset()
{
    promise_ = std::promise<void>();
    taskDoneFuture_ = promise_.get_future();
    status_ = NONE;
    dependencies_.clear();
}

void Task::AddDependency(const std::weak_ptr<Task>& newDependencyPtr)
{
    auto dependency = newDependencyPtr.lock();
    //Be sure to not create a cycle of dependencies which would deadlock the thread
    //Also check if the dependencies is not already in the dependency tree
    std::function<bool(const std::vector<std::weak_ptr<Task>>&,
                       const Task*)> checkDependencies =
            [&checkDependencies, dependency](
                    const std::vector<std::weak_ptr<Task>>& dependencies,
                    const Task* job) {
                for (const auto& depPtr : dependencies)
                {
                    auto dep = depPtr.lock();
                    if (dep.get() == job || dep == dependency)
                        return false;
                    const bool recursiveDep = checkDependencies(
                            dep->dependencies_, job);
                    if (!recursiveDep)
                        return false;
                }
                return true;
            };
    if (checkDependencies(dependencies_, this))
    {
        dependencies_.push_back(newDependencyPtr);
    }
}


WorkerThread::WorkerThread(WorkerQueue& queue) : taskQueue_(queue)
{

}

void WorkerThread::Start()
{
    thread_ = std::thread(&WorkerThread::Loop, this);
}

void WorkerThread::Loop()
{
    while (isRunning_)
    {
        if (taskQueue_.IsEmpty())
        {
            taskQueue_.WaitForTask();
        }
        else
        {
            while (!taskQueue_.IsEmpty())
            {
                auto newTask = taskQueue_.PopNextTask();
                if (!newTask->CheckDependenciesStarted())
                {
                    taskQueue_.AddTask(std::move(newTask));
                }
                else
                {
                    newTask->Execute();
                }
            }
        }
    }
}

WorkerThread::~WorkerThread()
{
    Destroy();
}

void WorkerThread::Destroy()
{
    isRunning_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
}

bool WorkerQueue::IsEmpty() const
{
#ifdef TRACY_ENABLE
    std::shared_lock<SharedLockableBase(std::shared_mutex) > lock(queueMutex_);
#else
    std::shared_lock<std::shared_mutex> lock(queueMutex_);
#endif
    return tasks_.empty();
}

std::shared_ptr<Task> WorkerQueue::PopNextTask()
{
#ifdef TRACY_ENABLE
    std::unique_lock<SharedLockableBase (std::shared_mutex) > lock(queueMutex_);
#else
    std::unique_lock<std::shared_mutex> lock(queueMutex_);
#endif
    auto task = tasks_.front();
    tasks_.erase(tasks_.cbegin());
    return task;
}

void WorkerQueue::AddTask(std::shared_ptr<Task> task)
{
#ifdef TRACY_ENABLE
    std::unique_lock<SharedLockableBase (std::shared_mutex) > lock(queueMutex_);
#else
    std::unique_lock<std::shared_mutex> lock(queueMutex_);
#endif
    tasks_.push_back(std::move(task));
    conditionVariable_.notify_one();
}

void WorkerQueue::WaitForTask()
{
#ifdef TRACY_ENABLE
    std::unique_lock<SharedLockableBase (std::shared_mutex) > lock(queueMutex_);
#else
    std::unique_lock<std::shared_mutex> lock(queueMutex_);
#endif
    conditionVariable_.wait(lock);
}

void WorkerQueue::Destroy()
{
    conditionVariable_.notify_all();
}

WorkerQueue::~WorkerQueue()
{
    Destroy();
}
}