#pragma once

#include "resource_manager.h"

#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

namespace ssme {

class AsyncResourceManager {
private:
  ResourceManager resourceManager;
  std::thread workerThread;
  std::queue<std::function<void()>> taskQueue;
  std::mutex queueMutex;
  std::condition_variable condition;
  bool running = false;

public:
  AsyncResourceManager() { Start(); }

  ~AsyncResourceManager() { Stop(); }

  void Start() {
    running = true;
    workerThread = std::thread([this]() { WorkerThread(); });
  }

  void Stop() {
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      running = false;
    }
    condition.notify_one();
    if (workerThread.joinable()) {
      workerThread.join();
    }
  }

  template <typename T>
  void LoadAsync(const std::string &resourceId,
                 std::function<void(ResourceHandle<T>)> callback) {
    std::lock_guard<std::mutex> lock(queueMutex);
    taskQueue.push([this, resourceId, callback]() {
      auto handle = resourceManager.load<T>(resourceId);
      callback(handle);
    });
    condition.notify_one();
  }

private:
  void WorkerThread() {
    while (running) {
      std::function<void()> task;
      {
        std::unique_lock<std::mutex> lock(queueMutex);
        condition.wait(lock,
                       [this]() { return !taskQueue.empty() || !running; });

        if (!running && taskQueue.empty()) {
          return;
        }

        task = std::move(taskQueue.front());
        taskQueue.pop();
      }

      task();
    }
  }
};

// Usage example
// AsyncResourceManager asyncResourceManager;

// asyncResourceManager.LoadAsync<Texture>(
//     "large_texture", [](ResourceHandle<Texture> texture) {
//       // This callback will be called when the texture is loaded
//       if (texture) {
//         std::cout << "Texture loaded successfully!" << std::endl;
//       } else {
//         std::cout << "Failed to load texture." << std::endl;
//       }
//     });

} // namespace ssme
