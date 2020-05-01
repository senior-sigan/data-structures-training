#include <gtest/gtest.h>

#include <circular-queue/circular_queue.hpp>
#include <memory>

TEST(circular_queue, enqueue_and_peek_head) {
  CircularQueue<int> queue{};
  ASSERT_EQ(queue.IsEmpty(), true);
  queue.Enqueue(42);
  ASSERT_EQ(queue.Tail(), 42);
  queue.Enqueue(13);
  ASSERT_EQ(queue.Tail(), 13);
  ASSERT_EQ(queue.IsEmpty(), false);
}

TEST(circular_queue, const_head) {
  struct A {
    CircularQueue<int> queue_;

    A() {
      queue_.Enqueue(42);
    }

    [[nodiscard]] int NonConst() {
      return queue_.Head() + queue_.Tail();
    }

    [[nodiscard]] int Const() const {
      return queue_.Head() + queue_.Tail();
    }
  };

  A a;
  ASSERT_EQ(a.NonConst(), 42+42);
  ASSERT_EQ(a.Const(), 42+42);
}

TEST(circular_queue, check_size) {
  CircularQueue<int> queue{};
  ASSERT_EQ(queue.Size(), 0);
  queue.Enqueue(42);
  ASSERT_EQ(queue.Size(), 1);
  queue.Enqueue(13);
  ASSERT_EQ(queue.Size(), 2);
}

TEST(circular_queue, peek_head_and_tail) {
  CircularQueue<int> queue{};
  queue.Enqueue(42);
  ASSERT_EQ(queue.Head(), 42);
  ASSERT_EQ(queue.Tail(), 42);

  queue.Enqueue(13);
  ASSERT_EQ(queue.Head(), 42);
  ASSERT_EQ(queue.Tail(), 13);
}

TEST(circular_queue, enq_and_deq) {
  CircularQueue<int> queue{};
  ASSERT_EQ(queue.IsEmpty(), true);
  queue.Enqueue(42);
  ASSERT_EQ(queue.Size(), 1);
  ASSERT_EQ(queue.Head(), 42);
  ASSERT_EQ(queue.Tail(), 42);
  queue.Enqueue(13);
  ASSERT_EQ(queue.Size(), 2);
  ASSERT_EQ(queue.Head(), 42);
  ASSERT_EQ(queue.Tail(), 13);
  queue.Dequeue();
  ASSERT_EQ(queue.Size(), 1);
  ASSERT_EQ(queue.Head(), 13);
  ASSERT_EQ(queue.Tail(), 13);
  queue.Dequeue();
  ASSERT_EQ(queue.IsEmpty(), true);
}

class ElementClonable {
  int* d_;

 public:
  explicit ElementClonable(int* d) : d_(d) {}

  ~ElementClonable() {
    (*d_)++;
  }
};

class ElementMovable {
  int* d_;

 public:
  explicit ElementMovable(int* d) : d_(d) {}
  ElementMovable(const ElementMovable&) = delete;
  ElementMovable(ElementMovable&& that) noexcept : d_(that.d_) {
    that.d_ = nullptr;
  }

  ~ElementMovable() {
    if (d_) (*d_)++;
  }
};

TEST(circular_queue, should_destroy_objects) {
  int destructions = 0;
  {
    CircularQueue<ElementClonable> queue;
    queue.Enqueue(ElementClonable(&destructions));
    ASSERT_EQ(destructions, 1); // one destruction for copy
  }
  ASSERT_EQ(destructions, 2);
}

TEST(circular_queue, deq_should_delete_object) {
  int destructions = 0;
  {
    CircularQueue<ElementClonable> queue;
    queue.Enqueue(ElementClonable(&destructions));
    ASSERT_EQ(destructions, 1);  // one destruction for the copy

    queue.Dequeue();
    ASSERT_EQ(destructions, 2);
  }
  ASSERT_EQ(destructions, 2);
}

TEST(circular_queue, deq_should_delete_movable_object) {
  int destructions = 0;
  {
    CircularQueue<ElementMovable> queue;
    queue.Enqueue(ElementMovable(&destructions));
    queue.Enqueue(ElementMovable(&destructions));
    ASSERT_EQ(destructions, 0);  // no destructions because object was moved

    queue.Dequeue();
    ASSERT_EQ(destructions, 1);
  }
  ASSERT_EQ(destructions, 2);
}

TEST(circular_queue, store_unique_ptr) {
  CircularQueue<std::unique_ptr<int>> queue;
  queue.Enqueue(std::make_unique<int>(42));
  ASSERT_EQ(*queue.Head().get(), 42);
  queue.Dequeue();
  ASSERT_EQ(queue.Size(), 0);
}

TEST(circular_queue, cause_reallocation) {
  CircularQueue<std::unique_ptr<int>> queue;
  ASSERT_EQ(queue.Size(), 0);
  ASSERT_EQ(queue.Capacity(), 2);
  queue.Enqueue(std::make_unique<int>(1));
  queue.Enqueue(std::make_unique<int>(2));
  queue.Enqueue(std::make_unique<int>(3));
  ASSERT_EQ(queue.Size(), 3);
  ASSERT_EQ(queue.Capacity(), 4);
  queue.Enqueue(std::make_unique<int>(4));
  queue.Enqueue(std::make_unique<int>(5));
  ASSERT_EQ(queue.Size(), 5);
  ASSERT_EQ(queue.Capacity(), 8);
  ASSERT_EQ(*queue.Head().get(), 1);
  ASSERT_EQ(*queue.Tail().get(), 5);
  queue.Dequeue();
  ASSERT_EQ(*queue.Head().get(), 2);
  ASSERT_EQ(queue.Size(), 4);
}

TEST(circular_queue, enqueue_dequeu_unique) {
  CircularQueue<std::unique_ptr<int>> queue;
  queue.Enqueue(std::make_unique<int>(42));
  queue.Enqueue(std::make_unique<int>(13));
  ASSERT_EQ(*queue.Head().get(), 42);
  ASSERT_EQ(*queue.Tail().get(), 13);
  queue.Dequeue();
  ASSERT_EQ(*queue.Head().get(), 13);
  ASSERT_EQ(*queue.Tail().get(), 13);
  queue.Enqueue(std::make_unique<int>(7));
  ASSERT_EQ(*queue.Head().get(), 13);
  ASSERT_EQ(*queue.Tail().get(), 7);
  queue.Dequeue();
  queue.Enqueue(std::make_unique<int>(1));
  ASSERT_EQ(*queue.Head().get(), 7);
  ASSERT_EQ(*queue.Tail().get(), 1);
}

TEST(circular_queue, shrink_memory) {
  CircularQueue<std::unique_ptr<int>> queue;
  queue.Enqueue(std::make_unique<int>(42));
  queue.Enqueue(std::make_unique<int>(13));
  queue.Enqueue(std::make_unique<int>(13));
  queue.Enqueue(std::make_unique<int>(13));
  queue.Enqueue(std::make_unique<int>(13));
  ASSERT_EQ(queue.Capacity(), 8);
  ASSERT_EQ(queue.Size(), 5);
  queue.Dequeue();
  queue.Dequeue();
  queue.Dequeue();
  queue.Dequeue();
  ASSERT_EQ(queue.Capacity(), 2);
  ASSERT_EQ(queue.Size(), 1);
}