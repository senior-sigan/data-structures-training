#pragma once

namespace my {

template<typename T>
static inline T* AllocateMem(int n) {
  return static_cast<T*>(::operator new(sizeof(T) * n));
}

static inline void DeallocateMem(void* buf) {
  ::operator delete(static_cast<char*>(buf));
}

template<typename T>
static inline T& At(void* buf, int idx) {
  return static_cast<T*>(buf)[idx];
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
  size_type capacity_ = 0;  // actual size of the buffer_
  size_type size_ = 0;      // amount of real elements stored in the queue
  size_type head_ = 0;      // index (pointer) to the head of the Circular buffer_
  void* buffer_{};          // store data in void* to show we have manual memory management

  /**
   * GetIdx wrap Logical indexes into Real indexes.
   * Literally walks in the cycled Z_capacity_ space.
   * @param i Logical Index of the element in the circular buffer
   * @return Real Index of the element in the buffer
   */
  [[nodiscard]] size_type GetIdx(int i) const {
    return (i + head_) % capacity_;
  }

  /**
   * Helper function to Allocate new chunk of memory and move old values to the new location.
   * Function modify buffer_ and return nothing
   * @param n Size of new memory in T objects
   */
  void AllocateAndCopy(size_type n) {
    auto new_buf = AllocateMem<T>(n);
    for (int i = 0; i < size_; i++) {
      // it's a hack with inplacement new and moving objects from the old memory to the new one.
      ::new (Ptr<T>(new_buf, i)) T(std::move(At<T>(buffer_, GetIdx(i))));
    }
    DeallocateMem(buffer_);
    head_ = 0;  // because we moved to the beginning of the ne buffer
    capacity_ = n;
    buffer_ = new_buf;
  }

  /**
   * Check some edge cases of the size and capacity ratio.
   * Allocates x2 memory if run out of memory.
   * Deallocate x2 memory if reach 0.25 ration of size.
   */
  void ReallocateMemory() {
    if (buffer_ == nullptr || capacity_ == 0) {
      capacity_ = 2;
      buffer_ = AllocateMem<T>(capacity_);
    }

    if (size_ == capacity_) {
      AllocateAndCopy(capacity_ * 2);
    } else if (size_ != 0 && size_ <= static_cast<float>(capacity_) / 4.0) {
      AllocateAndCopy(capacity_ / 2);
    }
  }

 public:
  CircularQueue() = default;
  explicit CircularQueue(size_type n) : capacity_(n), buffer_(AllocateMem<T>(n)) {}
  ~CircularQueue() {
    Clear();
  }

  void Clear() {
    if (capacity_ == 0 || buffer_ == nullptr) {
      return;
    }

    // raw-delete operator was used because raw memory was allocated
    // and manual iteration over left object to call destructor
    for (int i = 0; i < size_; i++) {
      At<T>(buffer_, GetIdx(i)).~T();
    }
    DeallocateMem(buffer_);
    head_ = 0;
    size_ = 0;
    capacity_ = 0;
    buffer_ = nullptr;
  }

  /**
   * Reserves memory for future use.
   * Can be useful to avoid reallocations during adding new elements.
   * @param n number of elements to reserve memory for
   */
  void Reserve(size_type n) {
    if (n <= capacity_) return;

    AllocateAndCopy(n);
  }

  /**
   * Puts the value copy to the queue's tail.
   * Allocates memory if needed.
   * @param value to store
   */
  void Enqueue(const T& value) {
    ReallocateMemory();
    ::new (Ptr<T>(buffer_, GetIdx(size_))) T(value);
    size_ = (size_ + 1) % (capacity_ + 1);
  }

  /**
   * Puts the moved value to the queue's tail.
   * Allocates memory if needed.
   * @param value to store
   */
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
   * We choose neither of them.
   */
  void Dequeue() {
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

  /**
   * @return the actual memory reserved by the collection
   */
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
   * @return a const reference of the queue head element.
   */
  const T& Head() const {
    return At<T>(buffer_, head_);
  }

  /**
   * @return a reference of the queue head element.
   */
  T& Head() {
    return At<T>(buffer_, head_);
  }

  /**
   * @return a const reference of the queue tail element.
   */
  const T& Tail() const {
    return At<T>(buffer_, GetIdx(size_ - 1));
  }

  /**
   * @return a reference of the queue tail element.
   */
  T& Tail() {
    return At<T>(buffer_, GetIdx(size_ - 1));
  }

  /**
   * Deep collection copy.
   * @param rhs collection from which copy elements
   * @return
   */
  CircularQueue<T>& operator=(const CircularQueue<T>& rhs) {
    if (this == &rhs) return *this;

    Clear();
    Reserve(rhs.capacity_);
    for (int i = 0; i < rhs.Size(); i++) {
      Enqueue(At<T>(rhs.buffer_, rhs.GetIdx(i)));
    }
    return *this;
  }
};
}
