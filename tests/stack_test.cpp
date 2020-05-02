#include <gtest/gtest.h>
#include <stack/stack.h>

#include <memory>

TEST(stack, just_create_stack) {
  my::Stack<int> stack;
}

TEST(stack, push_object_to_stack_changes_size) {
  my::Stack<int> stack;
  ASSERT_EQ(stack.size(), 0);
  stack.push(42);
  ASSERT_EQ(stack.size(), 1);
  stack.push(7);
  ASSERT_EQ(stack.size(), 2);
}

TEST(stack, pushed_object_is_on_top) {
  my::Stack<int> stack;
  stack.push(42);
  ASSERT_EQ(stack.top(), 42);
  stack.push(7);
  ASSERT_EQ(stack.top(), 7);
}

TEST(stack, pop_objects_freed_space) {
  my::Stack<int> stack;
  stack.push(42);
  stack.push(7);
  ASSERT_EQ(stack.size(), 2);
  ASSERT_EQ(stack.top(), 7);
  stack.pop();
  ASSERT_EQ(stack.size(), 1);
  ASSERT_EQ(stack.top(), 42);
  stack.pop();
  ASSERT_EQ(stack.size(), 0);
}

TEST(stack, push_object_without_default_constructor) {
  class Box {
   public:
    int a_;
    explicit Box(int a) : a_(a) {}
  };

  my::Stack<Box> stack;
  stack.push(Box(42));
  ASSERT_EQ(stack.top().a_, 42);
}

class Box {
 public:
  int a_;
  int* counter_;
  explicit Box(int* counter, int a) : a_(a), counter_(counter) {}
  ~Box() {
    (*counter_)++;
  }
};

TEST(stack, push_const_ref) {
  int counter = 0;
  {
    my::Stack<Box> stack;
    Box box(&counter, 42);
    stack.push(box);
    ASSERT_EQ(counter, 0);
    ASSERT_EQ(stack.top().a_, 42);
    ASSERT_EQ(counter, 0);
  }

  ASSERT_EQ(counter, 2);
}

TEST(stack, push_and_move) {
  my::Stack<std::unique_ptr<int>> stack;
  stack.push(std::make_unique<int>(42));
  ASSERT_EQ(*stack.top().get(), 42);
}

TEST(stack, pop_should_delete_object) {
  int counter = 0;
  {
    my::Stack<Box> stack;
    stack.push(Box(&counter, 13));
    stack.push(Box(&counter, 42));
    ASSERT_EQ(counter, 2); // one deletion for temporal object
    stack.pop();
    ASSERT_EQ(counter, 3);
  }
  ASSERT_EQ(counter, 4);
}

TEST(stack, top_const_check) {
  struct A {
    my::Stack<int> stack;

    A() {
      stack.push(42);
    }

    int non_const_top() {
      stack.top()++;
      return stack.top();
    }

    int const_top() const {
      return stack.top();
    }
  };

  A a;
  ASSERT_EQ(a.const_top(), 42);
  ASSERT_EQ(a.non_const_top(), 43);
}

TEST(stack, reallocate_memory_for_copyable_objects) {
  my::Stack<int> stack;

  stack.push(1);
  stack.push(2);
  ASSERT_EQ(stack.size(), 2);
  ASSERT_EQ(stack.capacity(), 2);

  stack.push(3);
  ASSERT_EQ(stack.size(), 3);
  ASSERT_EQ(stack.capacity(), 4);

  stack.push(4);
  ASSERT_EQ(stack.size(), 4);
  ASSERT_EQ(stack.capacity(), 4);

  stack.push(4);
  ASSERT_EQ(stack.size(), 5);
  ASSERT_EQ(stack.capacity(), 8);
}

TEST(stack, reallocate_memory_for_movable_objects) {
  my::Stack<std::unique_ptr<int>> stack;

  stack.push(std::make_unique<int>(1));
  stack.push(std::make_unique<int>(2));
  ASSERT_EQ(stack.size(), 2);
  ASSERT_EQ(stack.capacity(), 2);
  ASSERT_EQ(*stack.top().get(), 2);

  stack.push(std::make_unique<int>(3));
  ASSERT_EQ(stack.size(), 3);
  ASSERT_EQ(stack.capacity(), 4);
  ASSERT_EQ(*stack.top().get(), 3);

  stack.push(std::make_unique<int>(4));
  ASSERT_EQ(stack.size(), 4);
  ASSERT_EQ(stack.capacity(), 4);
  ASSERT_EQ(*stack.top().get(), 4);

  stack.push(std::make_unique<int>(5));
  ASSERT_EQ(stack.size(), 5);
  ASSERT_EQ(stack.capacity(), 8);
  ASSERT_EQ(*stack.top().get(), 5);
}

TEST(stack, deallocate_momoey_for_movable_objects) {
  my::Stack<std::unique_ptr<int>> stack;

  stack.push(std::make_unique<int>(1));
  stack.push(std::make_unique<int>(2));
  stack.push(std::make_unique<int>(3));
  stack.push(std::make_unique<int>(4));
  stack.push(std::make_unique<int>(5));
  ASSERT_EQ(stack.size(), 5);
  ASSERT_EQ(stack.capacity(), 8);

  stack.pop(); // 4
  stack.pop(); // 3
  stack.pop(); // 2
  ASSERT_EQ(stack.size(), 2);
  ASSERT_EQ(stack.capacity(), 4);

  stack.pop(); // 1
  stack.pop(); // 0
  ASSERT_EQ(stack.size(), 0);
  ASSERT_EQ(stack.capacity(), 0);
}

TEST(stack, deep_copy) {
  my::Stack<int> stack1;
  stack1.push(1);
  stack1.push(2);
  stack1.push(3);

  my::Stack<int> stack2;
  ASSERT_EQ(stack2.size(), 0);
  ASSERT_EQ(stack2.capacity(), 0);

  stack2 = stack1;
  ASSERT_EQ(stack2.size(), 3);
  ASSERT_EQ(stack2.capacity(), 4);
  ASSERT_EQ(stack1.size(), 3);
  ASSERT_EQ(stack1.capacity(), 4);

  ASSERT_EQ(stack1.top(), 3);
  ASSERT_EQ(stack2.top(), 3);
}