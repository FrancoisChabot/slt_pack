#include "slt/rect_pack.h"
#include "gtest/gtest.h"

#include <random>
#include <functional>

TEST(slt_rect_pack, basic_api) {
  slt::RectPacker2D<int> packer({ 12, 12 });
  slt::RectPacker2D<int>::Result result;
  bool success = packer.pack({ 2, 2 }, result);
  
  EXPECT_TRUE(success);
  EXPECT_EQ(0, result.pos[0]);
  EXPECT_EQ(0, result.pos[1]);
  EXPECT_FALSE(result.flipped);
}

TEST(slt_rect_pack, snug_fit) {
  slt::RectPacker2D<int> packer({ 12,12 });
  slt::RectPacker2D<int>::Result result;
  bool success= packer.pack({ 12, 12 }, result);

  EXPECT_TRUE(success);
  EXPECT_EQ(0, result.pos[0]);
  EXPECT_EQ(0, result.pos[1]);
  EXPECT_FALSE(result.flipped);
}

TEST(slt_rect_pack, trivial_failure) {
  slt::RectPacker2D<int> packer({ 12,12 });

  slt::RectPacker2D<int>::Result result;
  bool success = packer.pack({ 13, 13 }, result);
  EXPECT_FALSE(success);
}

TEST(slt_rect_pack, full_fill) {
  slt::RectPacker2D<int> packer({ 5, 5 });

  bool matrix[5][5] = { false };
  for (int i = 0; i < 5 * 5; ++i) {
    slt::RectPacker2D<int>::Result result;
    bool success = packer.pack({ 1, 1 }, result);
    EXPECT_TRUE(success);
    EXPECT_FALSE(matrix[result.pos[0]][result.pos[1]]);
    matrix[result.pos[0]][result.pos[1]] = true;
  }

  slt::RectPacker2D<int>::Result result;
  //we should be full now
  bool success = packer.pack({ 1, 1 }, result);
  EXPECT_FALSE(success);
}


int MyAllocCount = 0;
template<typename T>
struct MyAlloc {

  MyAlloc() {}

  template<typename U>
  MyAlloc(MyAlloc<U> const&) {}

  using value_type = T;

  void deallocate(T* p, std::size_t) {
    MyAllocCount--;
    std::free(p);
  }

  T* allocate(std::size_t n) {
    ++MyAllocCount;
    return static_cast<T*>(std::malloc(n * sizeof(T)));
  }

  template<typename U>
  struct rebind {
    using other = MyAlloc<U>;
  };

  
};


TEST(slt_rect_pack, custom_allocator) {
  MyAllocCount = 0;

  MyAlloc<char> alloc;
  {
    slt::RectPacker2D<int, MyAlloc<char>> packer({ 12,12 }, alloc);
    slt::RectPacker2D<int, MyAlloc<char>>::Result result;

    bool success = packer.pack({ 12, 12 }, result);
    EXPECT_EQ(2, MyAllocCount);
    EXPECT_TRUE(success);
  }

  EXPECT_EQ(0, MyAllocCount);
}

//Does not actually test anything, but prints out a pattern that should be visually reasonable
TEST(slt_rect_pack, pretty_print) {
  using packer_t = slt::RectPacker2D<int>;
  packer_t packer({ 80, 15 });

  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_int_distribution<int> uniform_dist(1, 10);

  std::vector<std::vector<char>> result;

  std::vector<packer_t::Vec > data;
  for (int i = 0; i < 26; ++i) {
    int w = uniform_dist(e1);
    int h = uniform_dist(e1);

    data.emplace_back(packer_t::Vec{ w, h });
  }

  std::sort(data.begin(), data.end(), std::greater<packer_t::Vec>());

  char c = 'A';
  for (auto const & d : data) {
    std::cout << "BEGIN INSERTION FOR: " << c << " " << d[0] <<'-'<<d[1]<<  "\n";
    packer_t::Result packed;
    packer.pack(d, packed);
    auto size = d;
    if (packed.flipped) {
      std::swap(size[0], size[1]);
    }
    for (int i = packed.pos[0]; i < packed.pos[0] + size[0]; ++i) {
      for (int j = packed.pos[1]; j < packed.pos[1] + size[1]; ++j) {
        if (result.size() < j + 1) {
          result.resize(j+1, std::vector<char>(80, ' '));
        }
        result[j][i] = c;
      }
    }
    ++c;
  }

  for(auto const & l : result) {
    std::cout << '|';
    for (auto const & ch : l) {
      std::cout << ch;
    }
    std::cout << "|\n";
  }
}
