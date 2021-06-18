//
// Created by efarhan on 6/18/21.
//

#include "jobsystem.h"

#include <utility>

namespace core
{

Task::Task(std::function<void()> task) : task_(std::move(task)),
                                         taskDoneFuture_(promise_.get_future()),
                                         status_(NONE)
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

void Task::AddDependency(std::weak_ptr<Task> dependency)
{
    //Be sure to not create a cycle of dependencies which would deadlock the thread
    //Also check if the dependencies is not already in the dependency tree

}
}