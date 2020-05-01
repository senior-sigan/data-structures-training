#pragma once

template<typename T>
static T* AllocateMem(int n) {
  return static_cast<T*>(::operator new(sizeof(T) * n));
}

template<typename T>
class CircularQueue {
 public:
  typedef int size_type;
  typedef T value_type;
 private:
  size_type capacity_ = 2;
  size_type size_ = 0;
  size_type head_ = 0;
  T* buffer_;

  void ReallocateMemory() {
    float mul;
    if (size_ == capacity_) {
      mul = 2;
    } else if (size_ != 0 && size_ <= static_cast<float>(capacity_) / 4.0) {
      mul = 0.5;
    } else {
      return;
    }

    // Most efficient way to reallocate memory
    auto new_buf = AllocateMem<T>(capacity_ * mul);
    for (int i = 0; i < size_; i++) {
      auto idx = (i + head_) % (capacity_ + 1);
      ::new((void*)(new_buf + i)) T(std::move(buffer_[idx]));
    }
    delete buffer_;
    head_ = 0; // because we moved to the beginning of the ne buffer
    capacity_ *= mul;
    buffer_ = new_buf;
  }

 public:
  CircularQueue() : buffer_(AllocateMem<T>(capacity_)) {}
  ~CircularQueue() {
    // raw-delete operator was used because raw memory was allocated
    // and manual iteration over left object to call destructor
    for (int i = 0; i < size_; i++) {
      auto idx = (i + head_) % (capacity_ + 1);
      buffer_[idx].~T();
    }
    ::operator delete(buffer_);
  }

  /**
   * Enqueue put a copy of object into the queue.
   * @param object to store
   */
  void Enqueue(const T& value) {
    ReallocateMemory();
    auto idx = (head_ + size_) % capacity_;
    ::new((void*)(buffer_ + idx)) T(value);
    size_ = (size_ + 1) % (capacity_ + 1);
  }

  void Enqueue(T&& value) {
    ReallocateMemory();
    // MoveInsertable semantic. We allocate memory in our buffer and move object inside.
    // This is useful for std::unique_ptr.
    auto idx = (head_ + size_) % capacity_;
    ::new((void*)(buffer_ + idx)) T(std::move(value));
    size_ = (size_ + 1) % (capacity_ + 1);
  }

  /**
   * Dequeue removes an element from the tail of the queue.
   * It doesn't return a value to no-exceptions and copy-safe.
   * Imagine Dequeue from an empty queue.
   * It should either return a nullptr OR raise an exception.
   */
  void Dequeue() {
    // TODO: destroy object
    buffer_[head_].~T();
//    ::operator delete(buffer_, buffer_ + head_);
//    auto a = std::move(buffer_[head_]);
    size_ = (size_ - 1) % (capacity_ + 1);
    head_ = (head_ + 1) % capacity_;
    if (size_ < 0) size_ = 0;
    ReallocateMemory();
  }

  /**
   * @return the number of objects stored in the queue.
   */
  [[nodiscard]] size_type Size() const {
    return size_;
  }

  [[nodiscard]] size_type Capacity() const {
    return capacity_;
  }

  /**
   * @return true if the queue is empty otherwise false.
   */
  [[nodiscard]] bool IsEmpty() const {
    return Size() == 0;
  }

  /**
   * @return a reference of the queue head element.
   */
  const T& Head() const {
    return buffer_[head_];
  }

  T& Head() {
    return buffer_[head_];
  }

  /**
   * @return a reference of the queue tail element.
   */
  const T& Tail() const {
    auto idx = (head_ + size_ - 1) % capacity_;
    return buffer_[idx];
  }

  T& Tail() {
    auto idx = (head_ + size_ - 1) % capacity_;
    return buffer_[idx];
  }
};
