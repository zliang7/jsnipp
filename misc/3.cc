// https://stackoverflow.com/questions/6992550/using-rtti-to-determine-inheritance-graph-in-c
// http://www.drdobbs.com/cpp/twisting-the-rtti-system-for-safe-dynami/229401004

#if __cpp_exceptions || __EXCEPTIONS
#else
#error "exception disabled"
#endif

class any_ptr {
    void* ptr_;
    void (*thr_)(void*);

    template <typename T>
    static void thrower(void* ptr) { throw static_cast<T*>(ptr); }

public:
    template <typename T>
    any_ptr(T* ptr) : ptr_(ptr), thr_(&thrower<T>) {}

    template <typename U>
    U* cast() const {
        try { thr_(ptr_); }
        catch (U* ptr) { return static_cast<U*>(ptr); }
        catch (...) {}
        return 0;
    }
};

struct A {
};
struct B {
    B() = default;
    ~B() = default;
    void test() {};
    int a;
};
struct D: public B {
};
template <typename T>
static void thrower(void* ptr) { throw static_cast<T*>(ptr); }
#include <stdio.h>
int main() {
  void* p = new D;
  void (*t)(void*) = &thrower<D>;
/*auto d = any_ptr(p);
  auto b = d.cast<B>();*/
  B* b = nullptr;
  try { t(p); }
  //catch (D* p) { b = p; }
  catch (B* p) { b = p; }
  catch (...) {}
  printf("%p, %p\n", b, p);
  return 0;
}
