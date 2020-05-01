#pragma once

namespace my {

template<typename T>
static inline T* AllocateMem(int n) {
  return static_cast<T*>(::operator new(sizeof(T) * n));
}

static inline void DeallocateMem(void* buf) {
  ::operator delete(buf);
}

template<typename T>
static inline T& At(void* buf, int idx) {
  return reinterpret_cast<T*>(buf)[idx];
}

template<typename T>
static inline void* Ptr(void* buf, int idx) {
  return static_cast<void*>(&reinterpret_cast<T*>(buf)[idx]);
}

template<typename T>
class CircularQueue {
 public:
  // just to be like stdlib collections
  typedef int size_type;
  typedef T value_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;

 private:
  size_type capacity_ = 2;
  size_type size_ = 0;
  size_type head_ = 0;
  void* buffer_;

  [[nodiscard]] size_type GetIdx(int i) const {
    return (i + head_) % capacity_;
  }

  void AllocateAndCopy(size_type n) {
    auto new_buf = AllocateMem<T>(n);
    for (int i = 0; i < size_; i++) {
      ::new (Ptr<T>(new_buf, i)) T(std::move(At<T>(buffer_, GetIdx(i))));
    }
    DeallocateMem(buffer_);
    head_ = 0;  // because we moved to the beginning of the ne buffer
    capacity_ = n;
    buffer_ = new_buf;
  }

  void ReallocateMemory() {
    if (size_ == capacity_) {
      AllocateAndCopy(capacity_ * 2);
    } else if (size_ != 0 && size_ <= static_cast<float>(capacity_) / 4.0) {
      AllocateAndCopy(capacity_ / 2);
    }
  }

 public:
  CircularQueue() : buffer_(AllocateMem<T>(capacity_)) {}
  ~CircularQueue() {
    // raw-delete operator was used because raw memory was allocated
    // and manual iteration over left object to call destructor
    for (int i = 0; i < size_; i++) {
      At<T>(buffer_, GetIdx(i)).~T();
    }
    DeallocateMem(buffer_);
  }

  /**
   * Enqueue put a copy of object into the queue.
   * @param object to store
   */
  void Enqueue(const T& value) {
    ReallocateMemory();
    ::new (Ptr<T>(buffer_ + GetIdx(size_))) T(value);
    size_ = (size_ + 1) % (capacity_ + 1);
  }

  void Enqueue(T&& value) {
    ReallocateMemory();
    // MoveInsertable semantic. We allocate memory in our buffer and move object inside.
    // This is useful for std::unique_ptr.
    ::new (Ptr<T>(buffer_, GetIdx(size_))) T(std::move(value));
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
    At<T>(buffer_, head_).~T();
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
    return At<T>(buffer_, head_);
  }

  T& Head() {
    return At<T>(buffer_, head_);
  }

  /**
   * @return a reference of the queue tail element.
   */
  const T& Tail() const {
    return At<T>(buffer_, GetIdx(size_ - 1));
  }

  T& Tail() {
    return At<T>(buffer_, GetIdx(size_ - 1));
  }
};
}
