#pragma once

namespace my {

template<typename T>
class Stack {
  T* buffer_{};
  int capacity_ = 0;
  int size_ = 0;

  void ReallocateAndMove(int capacity) {
    capacity_ = capacity;
    auto new_buf = static_cast<T*>(::operator new(sizeof(T) * capacity_));

    for (int i = 0; i < size_; i++) {
      ::new (new_buf + i) T(std::move(buffer_[i]));
      buffer_[i].~T();  // old object "wrapper" should be deleted
    }

    ::operator delete(buffer_);
    buffer_ = new_buf;
  }

  void AllocateMemory() {
    if (size_ < capacity_) return;
    if (capacity_ == 0) capacity_ = 1;  // edge case when empty buffer_

    // Mem growth: 0 2 4 8 16 ...
    // read more about how to allocate memory for vector or ArrayList
    ReallocateAndMove(capacity_ * 2);
  }

  void DeallocateMemory() {
    if (size_ > capacity_ / 4) return;

    if (size_ == 0) {
      clear();
      return;
    }

    // read more about how to allocate memory for vector or ArrayList
    ReallocateAndMove(capacity_ / 2);
  }

 public:
  // TODO(senior-sigan): move constructor
  // TODO(senior-sigan): constructor with preallocated memory

  [[nodiscard]] int size() const {
    return size_;
  }

  [[nodiscard]] int capacity() const {
    return capacity_;
  }

  void push(const T& value) {
    AllocateMemory();

    ::new (buffer_ + size_) T(value);

    size_++;
  }

  void push(T&& value) {
    AllocateMemory();

    ::new (buffer_ + size_) T(std::move(value));

    size_++;
  }

  void pop() {
    top().~T();
    size_--;
    DeallocateMemory();
  }

  T& top() {
    return buffer_[size_ - 1];
  }

  const T& top() const {
    return buffer_[size_ - 1];
  }

  void clear() {
    // TODO(senior-sigan): not exception safe
    for (int i = 0; i < size_; i++) {
      buffer_[i].~T();
    }

    ::operator delete(buffer_);
    buffer_ = nullptr;
    capacity_ = 0;
    size_ = 0;
  }

  void reserve(int n) {
    if (n <= capacity_) return;
    ReallocateAndMove(n);
  }

  Stack<T>& operator=(const Stack<T>& rhs) {
    if (this == &rhs) return *this;

    clear();
    reserve(rhs.capacity());
    size_ = rhs.size();
    for (int i = 0; i < rhs.size(); i++) {
      buffer_[i] = rhs.buffer_[i];
    }
    return *this;
  }

  ~Stack() {
    clear();
  }
};

}  // namespace my
