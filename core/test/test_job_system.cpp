
#include <gtest/gtest.h>
#include <jobsystem.h>

TEST(JobSystem, SimpleJob)
{
    bool value = false;
    core::Task changeValueTask([&value]() { value = true; });
    changeValueTask.Execute();
    EXPECT_EQ(value, true);
}

TEST(JobSystem, SimpleThreadedJob)
{
    bool value = false;
    auto changeValueTask = std::make_shared<core::Task>(
            [&value]() { value = true; });
    core::WorkerQueue queue;
    core::WorkerThread thread(queue);
    thread.Start();
    queue.AddTask(changeValueTask);
    changeValueTask->Join();
    EXPECT_EQ(value, true);

    queue.Destroy();
    thread.Destroy();
}

TEST(JobSystem, SimpleThreadedJobWithDependencies)
{
    bool value1 = false;
    bool value2 = false;
    auto changeValueTask1 = std::make_shared<core::Task>([&value1, &value2]() {
        EXPECT_EQ(value2, false);
        value1 = true;
    });
    auto changeValueTask2 = std::make_shared<core::Task>(
            [&value1, &value2]() {
                EXPECT_EQ(value1, true);
                value2 = true;
            });
    changeValueTask2->AddDependency(changeValueTask1);
    core::WorkerQueue queue;
    core::WorkerThread thread(queue);
    thread.Start();
    queue.AddTask(changeValueTask1);
    queue.AddTask(changeValueTask2);
    changeValueTask1->Join();
    EXPECT_EQ(value1, true);
    changeValueTask2->Join();
    EXPECT_EQ(value2, true);

    queue.Destroy();
    thread.Destroy();
}

TEST(JobSystem, SimpleThreadedJobWithDependenciesInverted)
{
    bool value1 = false;
    bool value2 = false;
    auto changeValueTask1 = std::make_shared<core::Task>([&value1, &value2]() {
        EXPECT_EQ(value2, false);
        value1 = true;
    });
    auto changeValueTask2 = std::make_shared<core::Task>(
            [&value1, &value2]() {
                EXPECT_EQ(value1, true);
                value2 = true;
            });
    changeValueTask2->AddDependency(changeValueTask1);
    core::WorkerQueue queue;
    core::WorkerThread thread(queue);
    thread.Start();
    queue.AddTask(changeValueTask2);
    queue.AddTask(changeValueTask1);
    changeValueTask1->Join();
    EXPECT_EQ(value1, true);
    changeValueTask2->Join();
    EXPECT_EQ(value2, true);

    queue.Destroy();
    thread.Destroy();
}