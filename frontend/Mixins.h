#ifndef MIXINS_H
#define MIXINS_H

template <class T>
class NonCopyable {
public:
  NonCopyable(const NonCopyable &) = delete;
  T &operator=(const T &) = delete;

protected:
  NonCopyable() = default;
  ~NonCopyable() = default;
};

template <class T>
class NonCopyableAndMovable : private NonCopyable<T> {
public:
  NonCopyableAndMovable(const NonCopyableAndMovable &&) = delete;
  T &operator=(T &&) = delete;

protected:
  NonCopyableAndMovable() = default;
  ~NonCopyableAndMovable() = default;
};

#endif
