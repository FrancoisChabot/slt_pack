# slt_pack
A simple incremental C++ bin packer

# Documenation

## Simple usage

1. Create a packer by instantiating `slt::RectPacker2D` with a bin size as constructor argument.
2. Call pack() for every rect you want to pack. (If you are doing packing in batches, it's best to call pack with larger rects first)

## Memory usage

`slt::RectPacker2D` needs to do a bit of dymanic memory allocation in order to keep track of available space. If you want control over this, you can
provide it with a custom rebindable allocator. 

## Custom packing heuristics

You can change the packing heuristics used by the packer if you want by passing a functor to the `pack()` call. The `DefaultRectScorer` function at the 
top of `rect_pack.h` should provide a good template to follow. Just remember that lower scores are better.

## Increasing the bin size after creation.

You can add more storage to the packer by invoking `addFreeRect()`. This can be done in order to "dealocate" regions that have been previosuly returned by 
`pack()`, or increase the overall storage.

# Usage Example:

```cpp
#include "slt/rect_pack.h"

#include <algorithm>
#include <iostream>
#include <vector>

int main() {
  using packer_t = slt::RectPacker2D<int>;
  
  std::vector<packer_t::Vec> rects_to_pack;
  
  rects_to_pack.emplace_back({2,2});
  rects_to_pack.emplace_back({4,2});
  rects_to_pack.emplace_back({3,1});

  //Larger rects first
  std::sort(rects_to_pack.begin(), rects_to_pack.end(), std::greater<packer_t::Vec>);

  packer_t packer({ 12, 12 });
  for(auto const& r : rects_to_pack) {
    packer_t result;
    bool success = packer.pack(r, result);
    if(success) {
      std::cout << "packed at: " << result.pos[0] <<  '-' << result.pos[1];
      if(result.flipped) {
        std::cout << " (flipped)";
      }
      std::cout << std::endl;
    }
  }

  return 0;
}
```


